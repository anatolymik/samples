#pragma once

/***************************************************************************************\
|	Declarations																		|
\***************************************************************************************/

#define USBDEVICE_PAGED_POOL				'ubpg'
#define USBDEVICE_NONPAGED_POOL				'ubnp'

#define USBDEVICE_REMOVE_LOCK_TAG			'ublt'

/***************************************************************************************\
|	Structures																			|
\***************************************************************************************/

struct SUSBDevice : SDevice {

	IO_REMOVE_LOCK			Lock;

	PDEVICE_OBJECT			SelfDevice;
	PDEVICE_OBJECT			BaseDevice;
	PDEVICE_OBJECT			LowerDevice;

	ULONG					PagingCount;
	KEVENT					PagingLock;

	bool					UpperFilter;
	bool					HubOrComposite;

};

/***************************************************************************************\
|	Variables																			|
\***************************************************************************************/

extern PWSTR	g_UsbAllowedDevices;
extern ULONG	g_UsbAllowedDevicesLength;
