#include <basic/basic.h>

#include <control/control.h>
#include <usb/usb.h>

/***************************************************************************************\
|	Functions section associations														|
\***************************************************************************************/

extern "C" {

	static NTSTATUS	DispatchAddDevice( PDRIVER_OBJECT DriverObject, PDEVICE_OBJECT PhysicalDeviceObject );
	static void		DriverUnload( PDRIVER_OBJECT DriverObject );
	NTSTATUS		DriverEntry( PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath );

}

#ifdef ALLOC_PRAGMA

	#pragma alloc_text(PAGE, DispatchAddDevice)
	#pragma alloc_text(PAGE, DriverUnload)

	#pragma alloc_text(INIT, DriverEntry)

#endif

/***************************************************************************************\
|	Functions																			|
\***************************************************************************************/

static NTSTATUS DispatchAddDevice( PDRIVER_OBJECT DriverObject, PDEVICE_OBJECT PhysicalDeviceObject ) {

	UNREFERENCED_PARAMETER( DriverObject );

	return UsbCreateAndAttachFilter( PhysicalDeviceObject, true );

}

static void DriverUnload( PDRIVER_OBJECT DriverObject ) {

	UNREFERENCED_PARAMETER( DriverObject );

	// usb initialize
	UsbUninitialize();

	// uninitialize interface
	CtrlUninitialize();

	// uninitialize driver
	UninitializeDriverObject();

}

extern "C" NTSTATUS DriverEntry( PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath ) {

	UNREFERENCED_PARAMETER( RegistryPath );

	NTSTATUS	Status;

	// initialize driver object
	Status = InitializeDriverObject( DriverObject, RegistryPath );
	if ( !NT_SUCCESS( Status ) ) {
		return Status;
	}

	// initialize unload and add device routines
	DriverObject->DriverUnload = DriverUnload;
	DriverObject->DriverExtension->AddDevice = DispatchAddDevice;

	// initialize user mode interface
	Status = CtrlInitialize();
	if ( !NT_SUCCESS( Status ) ) {
		UninitializeDriverObject();
		return Status;
	}

	// initialize usb
	Status = UsbInitialize();
	if ( !NT_SUCCESS( Status ) ) {
		CtrlUninitialize();
		UninitializeDriverObject();
		return Status;
	}

	return Status;

}
