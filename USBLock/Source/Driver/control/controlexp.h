#pragma once

/***************************************************************************************\
|	Declaration																			|
\***************************************************************************************/

#define USB_CONTROL_INTERFACE_NT_NAME_W			L"\\Device\\USBLockControl"
#define USB_CONTROL_INTERFACE_DOS_NAME_W		L"\\GLOBAL??\\USBLockControl"
#define USB_CONTROL_INTERFACE_WIN32_W			L"\\\\.\\USBLockControl"

/***************************************************************************************\
|	I/O controls																		|
\***************************************************************************************/

#define USB_CONTROL_DEVICE_TYPE										FILE_DEVICE_UNKNOWN

#define USB_CONTROL_IOCTL_ADD_DEVICE_TO_ALLOWED_LIST				CTL_CODE( USB_CONTROL_DEVICE_TYPE, 0x800, METHOD_NEITHER, FILE_ANY_ACCESS )
#define USB_CONTROL_IOCTL_REMOVE_DEVICE_FROM_ALLOWED_LIST			CTL_CODE( USB_CONTROL_DEVICE_TYPE, 0x801, METHOD_NEITHER, FILE_ANY_ACCESS )

/***************************************************************************************\
|	Structures																			|
\***************************************************************************************/

#pragma pack( push, 1 )

struct SDeviceId {

	USHORT		Vid;
	USHORT		Pid;

};

#pragma pack( pop )
