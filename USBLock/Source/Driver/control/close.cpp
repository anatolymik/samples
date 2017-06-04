#include "incs.h"

/***************************************************************************************\
|	Functions section associations														|
\***************************************************************************************/

extern "C" {

	NTSTATUS	CtrlDispatchClose( PDEVICE_OBJECT DeviceObject, PIRP Irp );

}

#ifdef ALLOC_PRAGMA

	#pragma alloc_text(PAGE, CtrlDispatchClose)

#endif

/***************************************************************************************\
|	Functions																			|
\***************************************************************************************/

NTSTATUS CtrlDispatchClose( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	UNREFERENCED_PARAMETER( DeviceObject );

	NTSTATUS	Status;

	PAGED_CODE();

	// compete close
	Status = Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest( Irp, IO_NO_INCREMENT );

	return Status;

}
