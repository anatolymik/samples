#include "../basic.h"

/***************************************************************************************\
|	Declarations																		|
\***************************************************************************************/

#define DRIVER_PAGED_POOL		'dvpg'

/***************************************************************************************\
|	Variables																			|
\***************************************************************************************/

PDRIVER_OBJECT		g_DriverObject = nullptr;
UNICODE_STRING		g_RegistryPath = { 0, 0, nullptr };

/***************************************************************************************\
|	Functions																			|
\***************************************************************************************/

static NTSTATUS DispatchCommon( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	// direct IRP to corresponding device
	return ((SDevice*)DeviceObject->DeviceExtension)->MajorFunction[IoGetCurrentIrpStackLocation( Irp )->MajorFunction]( DeviceObject, Irp );

}

static NTSTATUS DispatchCreate( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	// direct IRP to corresponding device
	return ((SDevice*)DeviceObject->DeviceExtension)->MajorFunction[IRP_MJ_CREATE]( DeviceObject, Irp );

}

static NTSTATUS DispatchCreateNamedPipe( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	// direct IRP to corresponding device
	return ((SDevice*)DeviceObject->DeviceExtension)->MajorFunction[IRP_MJ_CREATE_NAMED_PIPE]( DeviceObject, Irp );

}

static NTSTATUS DispatchClose( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	// direct IRP to corresponding device
	return ((SDevice*)DeviceObject->DeviceExtension)->MajorFunction[IRP_MJ_CLOSE]( DeviceObject, Irp );

}

static NTSTATUS DispatchRead( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	// direct IRP to corresponding device
	return ((SDevice*)DeviceObject->DeviceExtension)->MajorFunction[IRP_MJ_READ]( DeviceObject, Irp );

}

static NTSTATUS DispatchWrite( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	// direct IRP to corresponding device
	return ((SDevice*)DeviceObject->DeviceExtension)->MajorFunction[IRP_MJ_WRITE]( DeviceObject, Irp );

}

static NTSTATUS DispatchQueryInformation( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	// direct IRP to corresponding device
	return ((SDevice*)DeviceObject->DeviceExtension)->MajorFunction[IRP_MJ_QUERY_INFORMATION]( DeviceObject, Irp );

}

static NTSTATUS DispatchSetInformation( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	// direct IRP to corresponding device
	return ((SDevice*)DeviceObject->DeviceExtension)->MajorFunction[IRP_MJ_SET_INFORMATION]( DeviceObject, Irp );

}

static NTSTATUS DispatchQueryEa( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	// direct IRP to corresponding device
	return ((SDevice*)DeviceObject->DeviceExtension)->MajorFunction[IRP_MJ_QUERY_EA]( DeviceObject, Irp );

}

static NTSTATUS DispatchSetEa( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	// direct IRP to corresponding device
	return ((SDevice*)DeviceObject->DeviceExtension)->MajorFunction[IRP_MJ_SET_EA]( DeviceObject, Irp );

}

static NTSTATUS DispatchFlushBuffers( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	// direct IRP to corresponding device
	return ((SDevice*)DeviceObject->DeviceExtension)->MajorFunction[IRP_MJ_FLUSH_BUFFERS]( DeviceObject, Irp );

}

static NTSTATUS DispatchQueryVolumeInformation( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	// direct IRP to corresponding device
	return ((SDevice*)DeviceObject->DeviceExtension)->MajorFunction[IRP_MJ_QUERY_VOLUME_INFORMATION]( DeviceObject, Irp );

}

static NTSTATUS DispatchSetVolumeInformation( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	// direct IRP to corresponding device
	return ((SDevice*)DeviceObject->DeviceExtension)->MajorFunction[IRP_MJ_SET_VOLUME_INFORMATION]( DeviceObject, Irp );

}

static NTSTATUS DispatchDirectoryControl( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	// direct IRP to corresponding device
	return ((SDevice*)DeviceObject->DeviceExtension)->MajorFunction[IRP_MJ_DIRECTORY_CONTROL]( DeviceObject, Irp );

}

static NTSTATUS DispatchFileSystemControl( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	// direct IRP to corresponding device
	return ((SDevice*)DeviceObject->DeviceExtension)->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL]( DeviceObject, Irp );

}

static NTSTATUS DispatchDeviceControl( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	// direct IRP to corresponding device
	return ((SDevice*)DeviceObject->DeviceExtension)->MajorFunction[IRP_MJ_DEVICE_CONTROL]( DeviceObject, Irp );

}

static NTSTATUS DispatchInternalDeviceControl( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	// direct IRP to corresponding device
	return ((SDevice*)DeviceObject->DeviceExtension)->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL]( DeviceObject, Irp );

}

static NTSTATUS DispatchShutdown( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	// direct IRP to corresponding device
	return ((SDevice*)DeviceObject->DeviceExtension)->MajorFunction[IRP_MJ_SHUTDOWN]( DeviceObject, Irp );

}

static NTSTATUS DispatchLockControl( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	// direct IRP to corresponding device
	return ((SDevice*)DeviceObject->DeviceExtension)->MajorFunction[IRP_MJ_LOCK_CONTROL]( DeviceObject, Irp );

}

static NTSTATUS DispatchCleanup( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	// direct IRP to corresponding device
	return ((SDevice*)DeviceObject->DeviceExtension)->MajorFunction[IRP_MJ_CLEANUP]( DeviceObject, Irp );

}

static NTSTATUS DispatchCreateMailslot( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	// direct IRP to corresponding device
	return ((SDevice*)DeviceObject->DeviceExtension)->MajorFunction[IRP_MJ_CREATE_MAILSLOT]( DeviceObject, Irp );

}

static NTSTATUS DispatchQuerySecurity( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	// direct IRP to corresponding device
	return ((SDevice*)DeviceObject->DeviceExtension)->MajorFunction[IRP_MJ_QUERY_SECURITY]( DeviceObject, Irp );

}

static NTSTATUS DispatchSetSecurity( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	// direct IRP to corresponding device
	return ((SDevice*)DeviceObject->DeviceExtension)->MajorFunction[IRP_MJ_SET_SECURITY]( DeviceObject, Irp );

}

static NTSTATUS DispatchPower( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	// direct IRP to corresponding device
	return ((SDevice*)DeviceObject->DeviceExtension)->MajorFunction[IRP_MJ_POWER]( DeviceObject, Irp );

}

static NTSTATUS DispatchSystemControl( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	// direct IRP to corresponding device
	return ((SDevice*)DeviceObject->DeviceExtension)->MajorFunction[IRP_MJ_SYSTEM_CONTROL]( DeviceObject, Irp );

}

static NTSTATUS DispatchDeviceChange( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	// direct IRP to corresponding device
	return ((SDevice*)DeviceObject->DeviceExtension)->MajorFunction[IRP_MJ_DEVICE_CHANGE]( DeviceObject, Irp );

}

static NTSTATUS DispatchQueryQuota( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	// direct IRP to corresponding device
	return ((SDevice*)DeviceObject->DeviceExtension)->MajorFunction[IRP_MJ_QUERY_QUOTA]( DeviceObject, Irp );

}

static NTSTATUS DispatchSetQuota( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	// direct IRP to corresponding device
	return ((SDevice*)DeviceObject->DeviceExtension)->MajorFunction[IRP_MJ_SET_QUOTA]( DeviceObject, Irp );

}

static NTSTATUS DispatchPnp( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	// direct IRP to corresponding device
	return ((SDevice*)DeviceObject->DeviceExtension)->MajorFunction[IRP_MJ_PNP]( DeviceObject, Irp );

}

NTSTATUS InitializeDriverObject( PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath ) {

	// initialize globals
	g_DriverObject = DriverObject;

	// copy registry path
	g_RegistryPath.Buffer = (PWSTR)ExAllocatePoolWithTag( PagedPool, RegistryPath->Length, DRIVER_PAGED_POOL );
	if ( !g_RegistryPath.Buffer ) {
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	RtlCopyMemory( g_RegistryPath.Buffer, RegistryPath->Buffer, RegistryPath->Length );
	g_RegistryPath.MaximumLength = g_RegistryPath.Length = RegistryPath->Length;


	// initialize common dispatch routine
	for ( ULONG i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++ ) {
		DriverObject->MajorFunction[i] = DispatchCommon;
	}

	// initialize separate dispatch routine for every known major function
	DriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchCreate;
	DriverObject->MajorFunction[IRP_MJ_CREATE_NAMED_PIPE] = DispatchCreateNamedPipe;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchClose;
	DriverObject->MajorFunction[IRP_MJ_READ] = DispatchRead;
	DriverObject->MajorFunction[IRP_MJ_WRITE] = DispatchWrite;
	DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION] = DispatchQueryInformation;
	DriverObject->MajorFunction[IRP_MJ_SET_INFORMATION] = DispatchSetInformation;
	DriverObject->MajorFunction[IRP_MJ_QUERY_EA] = DispatchQueryEa;
	DriverObject->MajorFunction[IRP_MJ_SET_EA] = DispatchSetEa;
	DriverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS] = DispatchFlushBuffers;
	DriverObject->MajorFunction[IRP_MJ_QUERY_VOLUME_INFORMATION] = DispatchQueryVolumeInformation;
	DriverObject->MajorFunction[IRP_MJ_SET_VOLUME_INFORMATION] = DispatchSetVolumeInformation;
	DriverObject->MajorFunction[IRP_MJ_DIRECTORY_CONTROL] = DispatchDirectoryControl;
	DriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL] = DispatchFileSystemControl;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchDeviceControl;
	DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = DispatchInternalDeviceControl;
	DriverObject->MajorFunction[IRP_MJ_SHUTDOWN] = DispatchShutdown;
	DriverObject->MajorFunction[IRP_MJ_LOCK_CONTROL] = DispatchLockControl;
	DriverObject->MajorFunction[IRP_MJ_CLEANUP] = DispatchCleanup;
	DriverObject->MajorFunction[IRP_MJ_CREATE_MAILSLOT] = DispatchCreateMailslot;
	DriverObject->MajorFunction[IRP_MJ_QUERY_SECURITY] = DispatchQuerySecurity;
	DriverObject->MajorFunction[IRP_MJ_SET_SECURITY] = DispatchSetSecurity;
	DriverObject->MajorFunction[IRP_MJ_POWER] = DispatchPower;
	DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = DispatchSystemControl;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CHANGE] = DispatchDeviceChange;
	DriverObject->MajorFunction[IRP_MJ_QUERY_QUOTA] = DispatchQueryQuota;
	DriverObject->MajorFunction[IRP_MJ_SET_QUOTA] = DispatchSetQuota;
	DriverObject->MajorFunction[IRP_MJ_PNP] = DispatchPnp;

	return STATUS_SUCCESS;

}

void UninitializeDriverObject() {

	// cleanup
	RtlFreeUnicodeString( &g_RegistryPath );

}

NTSTATUS ForwardIrpSynchronously( PDEVICE_OBJECT LowerDeviceObject, PIRP Irp ) {

	auto Callback = []( PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Signal ) -> NTSTATUS {

		UNREFERENCED_PARAMETER( DeviceObject );

		// signal event if request is pending
		if ( Irp->PendingReturned ) {
			KeSetEvent( (PKEVENT)Signal, IO_NO_INCREMENT, false );
		}

		return STATUS_MORE_PROCESSING_REQUIRED;

	};

	KEVENT		Signal;
	NTSTATUS	Status;

	// prepare event for synchonizing
	KeInitializeEvent( &Signal, NotificationEvent, false );

	// set completion routine
	IoCopyCurrentIrpStackLocationToNext( Irp );
	IoSetCompletionRoutine( Irp, Callback, &Signal, true, true, true );

	// and forward request and wait for it's completed
	Status = IoCallDriver( LowerDeviceObject, Irp );
	if ( Status == STATUS_PENDING ) {
		KeWaitForSingleObject( &Signal, Executive, KernelMode, false, nullptr );
		Status = Irp->IoStatus.Status;
	}

	return Status;

}
