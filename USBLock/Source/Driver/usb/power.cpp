#include "incs.h"

/***************************************************************************************\
|	Functions																			|
\***************************************************************************************/

NTSTATUS UsbDispatchPower( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	SUSBDevice*	USBDevice = (SUSBDevice*)DeviceObject->DeviceExtension;
	NTSTATUS	Status;

	// acquire device
	Status = IoAcquireRemoveLock( &USBDevice->Lock, Irp );
	if ( !NT_SUCCESS( Status ) ) {
		PoStartNextPowerIrp( Irp );

		Irp->IoStatus.Status = Status;
		IoCompleteRequest( Irp, IO_NO_INCREMENT );

		return Status;
	}

	// skip request
	PoStartNextPowerIrp( Irp );
	IoSkipCurrentIrpStackLocation( Irp );
	Status = PoCallDriver( USBDevice->LowerDevice, Irp );

	// release device
	IoReleaseRemoveLock( &USBDevice->Lock, Irp );

	return Status;

}
