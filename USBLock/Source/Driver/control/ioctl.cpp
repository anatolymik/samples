#include "incs.h"

/***************************************************************************************\
|	Functions section associations														|
\***************************************************************************************/

extern "C" {

	NTSTATUS			CtrlDispatchDeviceIoControl( PDEVICE_OBJECT DeviceObject, PIRP Irp );

}

#ifdef ALLOC_PRAGMA

	#pragma alloc_text(PAGE, CtrlDispatchDeviceIoControl)

#endif

/***************************************************************************************\
|	Functions																			|
\***************************************************************************************/

NTSTATUS CtrlDispatchDeviceIoControl( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	UNREFERENCED_PARAMETER( DeviceObject );

	PIO_STACK_LOCATION	StackLocation;

	USHORT				Vid;
	USHORT				Pid;

	NTSTATUS			Status = STATUS_NOT_SUPPORTED;

	PAGED_CODE();

	StackLocation = IoGetCurrentIrpStackLocation( Irp );

	for ( ;; ) {

		// check user supplied buffer
		if ( StackLocation->Parameters.DeviceIoControl.InputBufferLength < sizeof( SDeviceId ) ) {
			Status = STATUS_BUFFER_TOO_SMALL;
			break;
		}

		// read parameters from user buffer
		__try {
			Vid = ((SDeviceId*)StackLocation->Parameters.DeviceIoControl.Type3InputBuffer)->Vid;
			Pid = ((SDeviceId*)StackLocation->Parameters.DeviceIoControl.Type3InputBuffer)->Pid;
		} __except ( EXCEPTION_EXECUTE_HANDLER ) {
			Status = STATUS_INVALID_USER_BUFFER;
			break;
		}

		// process command
		switch ( StackLocation->Parameters.DeviceIoControl.IoControlCode ) {

			case USB_CONTROL_IOCTL_ADD_DEVICE_TO_ALLOWED_LIST:
				Status = UsbAddDeviceToAllowedList( Vid, Pid );
				break;

			case USB_CONTROL_IOCTL_REMOVE_DEVICE_FROM_ALLOWED_LIST:
				Status = UsbRemoveDeviceFromAllowedList( Vid, Pid );
				break;

			default:
				break;

		}

		break;

	}


	// compete request
	Irp->IoStatus.Status = Status;
	IoCompleteRequest( Irp, IO_NO_INCREMENT );

	return Status;

}
