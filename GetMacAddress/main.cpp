#include <stdio.h>
#include "macaddr.h"
#include "GetMacByCmd.h"

void GetMacAddressTest1();
int GetMacAddressTest2();

int main()
{
	GetMacAddressTest1();
	GetMacAddressTest2();
	system("pause");
	return 0;
}

void GetMacAddressTest1()
{
	std::string strMac;
	GetMacByCmd(strMac);
	printf("%s",strMac.c_str());
}

int GetMacAddressTest2()
{
	MACADDRESS macaddr = { 0 };;
	char mac[128] = { 0 }; 

	if (WDK_MacAddress(1, &macaddr, 1) < 1)
		return -1;

	char szMac[20] = {0};
	int n = 0;
	for (; n < 5; n++)
		sprintf(szMac + n * 3, "%02X-", macaddr.SrcMacAddr[n]);
	sprintf(szMac + 5 * 3, "%02X", macaddr.SrcMacAddr[n]);
	printf(szMac);
	printf("\n");
}
