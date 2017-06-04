#include "incs.h"

/***************************************************************************************\
|	Functions section associations														|
\***************************************************************************************/

extern "C" {

	NTSTATUS	CtrlDispatchCommon( PDEVICE_OBJECT DeviceObject, PIRP Irp );

}

#ifdef ALLOC_PRAGMA

	#pragma alloc_text(PAGE, CtrlDispatchCommon)

#endif

/***************************************************************************************\
|	Functions																			|
\***************************************************************************************/

NTSTATUS CtrlDispatchCommon( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	UNREFERENCED_PARAMETER( DeviceObject );

	NTSTATUS	Status;

	PAGED_CODE();

	// unsupported request, compete it
	Status = Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
	IoCompleteRequest( Irp, IO_NO_INCREMENT );

	return Status;

}
