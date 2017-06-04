#include "incs.h"

/***************************************************************************************\
|	Variables																			|
\***************************************************************************************/

#pragma data_seg("PAGEDATA")

	static PDEVICE_OBJECT	g_ControlDevice = nullptr;

#pragma data_seg()

/***************************************************************************************\
|	Functions section associations														|
\***************************************************************************************/

extern "C" {

	NTSTATUS		CtrlInitialize();
	void			CtrlUninitialize();

}

#ifdef ALLOC_PRAGMA

	#pragma alloc_text(PAGE, CtrlInitialize)
	#pragma alloc_text(PAGE, CtrlUninitialize)

#endif

/***************************************************************************************\
|	Functions																			|
\***************************************************************************************/

NTSTATUS CtrlInitialize() {

	SDevice*		ControlInstance;

	PDEVICE_OBJECT	ControlDevice;
	UNICODE_STRING	NtName;
	UNICODE_STRING	DosName;

	NTSTATUS		Status;

	PAGED_CODE();

	// create device interface
	RtlInitUnicodeString( &NtName, USB_CONTROL_INTERFACE_NT_NAME_W );
	Status = IoCreateDeviceSecure(
		g_DriverObject,
		sizeof( SDevice ),
		&NtName,
		USB_CONTROL_DEVICE_TYPE,
		FILE_DEVICE_SECURE_OPEN,
		true,
		&SDDL_DEVOBJ_SYS_ALL_ADM_ALL,
		nullptr,
		&ControlDevice
	);
	if ( !NT_SUCCESS( Status ) ) {
		return Status;
	}

	// get instance pointer
	ControlInstance = (SDevice*)ControlDevice->DeviceExtension;

	// initialize its dispatch routines
	for ( ULONG i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++ ) {
		ControlInstance->MajorFunction[i] = CtrlDispatchCommon;
	}
	ControlInstance->MajorFunction[IRP_MJ_CREATE] = CtrlDispatchCreate;
	ControlInstance->MajorFunction[IRP_MJ_CLEANUP] = CtrlDispatchCleanup;
	ControlInstance->MajorFunction[IRP_MJ_CLOSE] = CtrlDispatchClose;
	ControlInstance->MajorFunction[IRP_MJ_DEVICE_CONTROL] = CtrlDispatchDeviceIoControl;

	// create symbolic link for user mode
	RtlInitUnicodeString( &DosName, USB_CONTROL_INTERFACE_DOS_NAME_W );
	Status = IoCreateSymbolicLink( &DosName, &NtName );
	if ( !NT_SUCCESS( Status ) ) {
		IoDeleteDevice( ControlDevice );
		return Status;
	}

	// remember our control device
	g_ControlDevice = ControlDevice;

	// we don't reset DO_DEVICE_INITIALIZING flag here because it'll be done by the system
	// after returning from DriverEntry routine

	return Status;

}

void CtrlUninitialize() {

	UNICODE_STRING	DosName;

	PAGED_CODE();

	// delete control device
	if ( g_ControlDevice ) {
		IoDeleteSymbolicLink( &DosName );
		IoDeleteDevice( g_ControlDevice );
	}

}
