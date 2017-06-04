#pragma once

/***************************************************************************************\
|	Functions																			|
\***************************************************************************************/

extern "C" NTSTATUS		UsbInitialize();
extern "C" void			UsbUninitialize();

extern "C" NTSTATUS		UsbAddDeviceToAllowedList( USHORT Vid, USHORT Pid );
extern "C" NTSTATUS		UsbRemoveDeviceFromAllowedList( USHORT Vid, USHORT Pid );

extern "C" NTSTATUS		UsbCreateAndAttachFilter( PDEVICE_OBJECT PhysicalDeviceObject, bool UpperFilter );
