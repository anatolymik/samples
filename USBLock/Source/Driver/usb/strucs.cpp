#include "incs.h"

/***************************************************************************************\
|	Variables																			|
\***************************************************************************************/

#pragma data_seg("PAGEDATA")

	PWSTR	g_UsbAllowedDevices = nullptr;
	ULONG	g_UsbAllowedDevicesLength = 0;

#pragma data_seg()
