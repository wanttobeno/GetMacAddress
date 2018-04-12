#include <tchar.h>
#include <algorithm>
#include "macaddr.h"

#ifdef __cplusplus
extern "C"
{
#endif
#include <ntddndis.h>
#include <setupapi.h>
//#include <hidsdi.h>
#ifdef __cplusplus
}
#endif

#pragma comment (lib, "setupapi.lib")
//#pragma comment (lib, "hid.lib")

const GUID GUID_QUERYSET[] = {
	// 网卡原生MAC地址（包含USB网卡）
	{0xAD498944, 0x762F, 0x11D0, 0x8D, 0xCB, 0x00, 0xC0, 0x4F, 0xC3, 0x35, 0x8C},

	// 网卡原生MAC地址（剔除USB网卡）-- 未完成
	{0xAD498944, 0x762F, 0x11D0, 0x8D, 0xCB, 0x00, 0xC0, 0x4F, 0xC3, 0x35, 0x8C},
};

// 获取网卡原生MAC地址
static BOOL WDK_GetMacAddress(TCHAR * DevicePath, MACADDRESS * pMacAddress, INT iIndex, BOOL isIncludeUSB)
{
	HANDLE  hDeviceFile;
	BOOL    isOK = FALSE;

	// 剔除虚拟网卡
	if (_tcsnicmp(DevicePath + 4, TEXT("root"), 4) == 0)
	{
		return FALSE;
	}

	if (!isIncludeUSB)
	{   // 剔除USB网卡
		if (_tcsnicmp(DevicePath + 4, TEXT("usb"), 4) == 0)
		{
			return FALSE;
		}
	}

	// 获取设备句柄
	hDeviceFile = CreateFile(DevicePath,
		0,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (hDeviceFile != INVALID_HANDLE_VALUE)
	{
		ULONG   dwID;
		BYTE    ucData[8];
		DWORD   dwByteRet;

		// 获取当前MAC地址
		dwID = OID_802_3_CURRENT_ADDRESS;
		isOK = DeviceIoControl(hDeviceFile, IOCTL_NDIS_QUERY_GLOBAL_STATS, &dwID, sizeof(dwID), ucData, sizeof(ucData), &dwByteRet, NULL);
		if (isOK)
		{
			memcpy(pMacAddress[iIndex].CurMacAddr, ucData, dwByteRet);

			// 获取原生MAC地址
			dwID = OID_802_3_PERMANENT_ADDRESS;
			isOK = DeviceIoControl(hDeviceFile, IOCTL_NDIS_QUERY_GLOBAL_STATS, &dwID, sizeof(dwID), ucData, sizeof(ucData), &dwByteRet, NULL);
			if (isOK)
			{
				memcpy(pMacAddress[iIndex].SrcMacAddr, ucData, dwByteRet);
			}
		}
		CloseHandle(hDeviceFile);
	}
	return isOK;
}

static BOOL WDK_GetProperty(TCHAR* DevicePath, INT iQueryType, MACADDRESS *pMacAddress, INT iIndex)
{
	BOOL isOK = FALSE;

	switch (iQueryType)
	{
	case 0:     // 网卡原生MAC地址（包含USB网卡）       
		isOK = WDK_GetMacAddress(DevicePath, pMacAddress, iIndex, TRUE);
		break;

	case 1:     // 网卡原生MAC地址（剔除USB网卡）       
		isOK = WDK_GetMacAddress(DevicePath, pMacAddress, iIndex, FALSE);
		break;

	default:
		break;
	}
	return isOK;
}

INT WDK_MacAddress(INT iQueryType, MACADDRESS * pMacAddress, INT iSize)
{
	HDEVINFO hDevInfo;
	DWORD MemberIndex, RequiredSize;
	SP_DEVICE_INTERFACE_DATA            DeviceInterfaceData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA    DeviceInterfaceDetailData;
	INT iTotal = 0;

	// 判断查询类型是否支持
	if ((iQueryType < 0) || (iQueryType >= sizeof(GUID_QUERYSET) / sizeof(GUID)))
	{
		return -2;  // 查询类型不支持
	}

	// 获取设备信息集
	hDevInfo = SetupDiGetClassDevs(GUID_QUERYSET + iQueryType, NULL, NULL, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
	if (hDevInfo == INVALID_HANDLE_VALUE)
	{
		return -1;
	}

	// 枚举设备信息集中所有设备
	DeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	for (MemberIndex = 0; ((pMacAddress == NULL) || (iTotal < iSize)); MemberIndex++)
	{   // 获取设备接口
		if (!SetupDiEnumDeviceInterfaces(hDevInfo, NULL, GUID_QUERYSET + iQueryType, MemberIndex, &DeviceInterfaceData))
		{   // 设备枚举完毕
			break;
		}

		// 获取接收缓冲区大小，函数返回值为FALSE，GetLastError()=ERROR_INSUFFICIENT_BUFFER
		SetupDiGetDeviceInterfaceDetail(hDevInfo, &DeviceInterfaceData, NULL, 0, &RequiredSize, NULL);

		// 申请接收缓冲区
		DeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(RequiredSize);
		DeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		// 获取设备细节信息
		if (SetupDiGetDeviceInterfaceDetail(hDevInfo, &DeviceInterfaceData, DeviceInterfaceDetailData, RequiredSize, NULL, NULL))
		{
			if (pMacAddress != NULL)
			{
				if (WDK_GetProperty(DeviceInterfaceDetailData->DevicePath, iQueryType, pMacAddress, iTotal))
				{
					iTotal++;
				}
			}
			else
			{
				iTotal++;
			}
		}

		free(DeviceInterfaceDetailData);
	}

	SetupDiDestroyDeviceInfoList(hDevInfo);

	return iTotal;
}
