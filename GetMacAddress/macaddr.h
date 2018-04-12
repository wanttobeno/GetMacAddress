#ifndef __MACADDR_H__
#define __MACADDR_H__

#include <windows.h>

#define MACADDRESS_BYTELEN  6                       // MAC地址字节长度

typedef struct _MACADDRESS
{
    BYTE    SrcMacAddr[MACADDRESS_BYTELEN];         // 原生MAC地址
    BYTE    CurMacAddr[MACADDRESS_BYTELEN];         // 当前MAC地址
} MACADDRESS;

#ifdef __cplusplus
extern "C"{
#endif

    /*

    */
    /*
        功能     ： 结合WMI和DeviceIoControl获取网卡原生MAC地址和当前MAC地址
        入口参数  ：
                   iQueryType  需要获取的网卡类型
                               0 ： 包括USB网卡
                               1 ： 不包括USB网卡
                   pMacAddress 存储网卡MAC地址
                   uSize       可存储的最大网卡数目
        返回值：
                   -1  不支持的设备属性值
                   -2  WMI连接失败
                   -3  不正确的WQL查询语句
                   >=0 获取的网卡数目
    */
    INT WDK_MacAddress(INT iQueryType, MACADDRESS * pMacAddress, INT iSize);

#ifdef __cplusplus
}
#endif

#endif