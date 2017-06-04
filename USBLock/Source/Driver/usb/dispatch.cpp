#include "incs.h"

/***************************************************************************************\
|	Functions																			|
\***************************************************************************************/

NTSTATUS UsbDispatchCommon( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	SUSBDevice*	USBDevice = (SUSBDevice*)DeviceObject->DeviceExtension;
	NTSTATUS	Status;

	// acquire device
	Status = IoAcquireRemoveLock( &USBDevice->Lock, Irp );
	if ( !NT_SUCCESS( Status ) ) {
		Irp->IoStatus.Status = Status;
		IoCompleteRequest( Irp, IO_NO_INCREMENT );

		return Status;
	}

	// skip request
	IoSkipCurrentIrpStackLocation( Irp );
	Status = IoCallDriver( USBDevice->LowerDevice, Irp );

	// release device
	IoReleaseRemoveLock( &USBDevice->Lock, Irp );

	return Status;

}
