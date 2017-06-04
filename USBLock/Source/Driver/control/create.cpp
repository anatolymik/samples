#include "incs.h"

/***************************************************************************************\
|	Functions section associations														|
\***************************************************************************************/

extern "C" {

	NTSTATUS	CtrlDispatchCreate( PDEVICE_OBJECT DeviceObject, PIRP Irp );

}

#ifdef ALLOC_PRAGMA

	#pragma alloc_text(PAGE, CtrlDispatchCreate)

#endif

/***************************************************************************************\
|	Functions																			|
\***************************************************************************************/

NTSTATUS CtrlDispatchCreate( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	UNREFERENCED_PARAMETER( DeviceObject );

	PIO_STACK_LOCATION	StackLocation;
	NTSTATUS			Status = STATUS_SUCCESS;

	PAGED_CODE();

	StackLocation = IoGetCurrentIrpStackLocation( Irp );

	for ( ;; ) {

		// we don't have namespace
		if ( StackLocation->FileObject->FileName.Length > 0 ) {
			Status = STATUS_UNSUCCESSFUL;
			break;
		}

		break;

	}

	// compete request
	Irp->IoStatus.Status = Status;
	IoCompleteRequest( Irp, IO_NO_INCREMENT );

	return Status;

}
