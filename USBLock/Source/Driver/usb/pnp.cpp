#include "incs.h"

/***************************************************************************************\
|	Functions section associations														|
\***************************************************************************************/

extern "C" {

	static NTSTATUS		UsbDispatchPnpQueryIds( PWSTR* HardwareIds, SUSBDevice* USBDevice, bool Compatible );

	static bool			UsbCompareIds( PUNICODE_STRING Subname, PWSTR Ids );
	static bool			UsbIsUsbEnumerator( PWSTR Ids );
	static bool			UsbIsRootPort( PWSTR Ids );
	static bool			UsbIsHub( PWSTR Ids );
	static bool			UsbIsComposite( PWSTR Ids );
	static bool			UsbIsKeyboard( PWSTR Ids );
	static bool			UsbIsMouse( PWSTR Ids );
	static bool			UsbIsDeviceInAllowedList( PWSTR Ids );
	static NTSTATUS		UsbIsDeviceAllowedToWork( bool* HubOrComposite, SUSBDevice* USBDevice );

	static NTSTATUS		UsbDispatchPnpStartDevice( SUSBDevice* USBDevice, PIRP Irp );
	static NTSTATUS		UsbDispatchPnpRemoveDevice( SUSBDevice* USBDevice, PIRP Irp );
	static NTSTATUS		UsbDispatchPnpDeviceUsageNotification( SUSBDevice* USBDevice, PIRP Irp );
	static NTSTATUS		UsbDispatchPnpQueryDeviceRelations( SUSBDevice* USBDevice, PIRP Irp );

	NTSTATUS			UsbDispatchPnp( PDEVICE_OBJECT DeviceObject, PIRP Irp );

}

#ifdef ALLOC_PRAGMA

	#pragma alloc_text(PAGE, UsbDispatchPnpQueryIds)

	#pragma alloc_text(PAGE, UsbCompareIds)
	#pragma alloc_text(PAGE, UsbIsUsbEnumerator)
	#pragma alloc_text(PAGE, UsbIsRootPort)
	#pragma alloc_text(PAGE, UsbIsHub)
	#pragma alloc_text(PAGE, UsbIsComposite)
	#pragma alloc_text(PAGE, UsbIsKeyboard)
	#pragma alloc_text(PAGE, UsbIsMouse)
	#pragma alloc_text(PAGE, UsbIsDeviceInAllowedList)
	#pragma alloc_text(PAGE, UsbIsDeviceAllowedToWork)

	#pragma alloc_text(PAGE, UsbDispatchPnpStartDevice)
	#pragma alloc_text(PAGE, UsbDispatchPnpRemoveDevice)
	#pragma alloc_text(PAGE, UsbDispatchPnpDeviceUsageNotification)
	#pragma alloc_text(PAGE, UsbDispatchPnpQueryDeviceRelations)

	#pragma alloc_text(PAGE, UsbDispatchPnp)

#endif

/***************************************************************************************\
|	Functions																			|
\***************************************************************************************/

extern "C" static NTSTATUS UsbDispatchPnpQueryIds( PWSTR* Ids, SUSBDevice* USBDevice, bool Compatible ) {

	ULONG		Length = 0;
	NTSTATUS	Status;

	PAGED_CODE();

	*Ids = nullptr;

	for ( ;; ) {

		// get hardware id length
		Status = IoGetDeviceProperty(
			USBDevice->BaseDevice,
			!Compatible ? DevicePropertyHardwareID : DevicePropertyCompatibleIDs,
			Length,
			*Ids,
			&Length
		);
		if ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {

			// device doesn't have ids
			if ( *Ids ) {
				ExFreePoolWithTag( *Ids, USBDEVICE_PAGED_POOL );
				*Ids = nullptr;
			}
			Status = STATUS_SUCCESS;

		} else if ( Status == STATUS_BUFFER_TOO_SMALL ) {

			// free previous buffer if it is
			if ( *Ids ) {
				ExFreePoolWithTag( *Ids, USBDEVICE_PAGED_POOL );
			}

			// allocate buffer for it
			*Ids = (PWSTR)ExAllocatePoolWithTag( PagedPool, Length, USBDEVICE_PAGED_POOL );
			if ( !*Ids ) {
				Status = STATUS_INSUFFICIENT_RESOURCES;
				break;
			}

			continue;

		}

		break;

	}

	// cleanup
	if ( !NT_SUCCESS( Status ) ) {

		if ( *Ids ) {
			ExFreePoolWithTag( *Ids, USBDEVICE_PAGED_POOL );
			*Ids = nullptr;
		}

	}

	return Status;

}

static bool UsbCompareIds( PUNICODE_STRING Subname, PWSTR Ids ) {

	UNICODE_STRING	DeviceSubName;
	SIZE_T			IdLength;

	PAGED_CODE();

	// scan all ids
	for ( ; Ids[0]; Ids += IdLength + 1 ) {
		IdLength = wcslen( Ids );
		if ( IdLength < Subname->Length / sizeof( wchar_t ) ) {
			continue;
		}
		DeviceSubName.Buffer = Ids;
		DeviceSubName.MaximumLength = DeviceSubName.Length = Subname->Length;
		if ( RtlCompareUnicodeString( &DeviceSubName, Subname, true ) == 0 ) {
			return true;
		}
	}

	return false;

}

static bool UsbIsUsbEnumerator( PWSTR Ids ) {

	UNICODE_STRING	SubName;

	PAGED_CODE();

	// make sure enumerator is USB
	RtlInitUnicodeString( &SubName, L"USB\\" );
	return UsbCompareIds( &SubName, Ids );

}

static bool UsbIsRootPort( PWSTR Ids ) {

	UNICODE_STRING	SubName;

	PAGED_CODE();

	// initialize root hub begin, we allow them to work
	RtlInitUnicodeString( &SubName, L"USB\\ROOT_HUB" );
	return UsbCompareIds( &SubName, Ids );

}

static bool UsbIsHub( PWSTR Ids ) {

	UNICODE_STRING	SubName;

	PAGED_CODE();

	// we allow hubs to work
	RtlInitUnicodeString( &SubName, L"USB\\Class_09" );
	if ( UsbCompareIds( &SubName, Ids ) ) {
		return true;
	}

	RtlInitUnicodeString( &SubName, L"USB\\USB20_HUB" );
	if ( UsbCompareIds( &SubName, Ids ) ) {
		return true;
	}

	RtlInitUnicodeString( &SubName, L"USB\\USB30_HUB" );
	if ( UsbCompareIds( &SubName, Ids ) ) {
		return true;
	}

	return false;

}

static bool UsbIsComposite( PWSTR Ids ) {

	UNICODE_STRING	SubName;

	PAGED_CODE();

	// initialize composite device begin, we allow them to work
	RtlInitUnicodeString( &SubName, L"USB\\COMPOSITE" );
	return UsbCompareIds( &SubName, Ids );

}

static bool UsbIsKeyboard( PWSTR Ids ) {

	UNICODE_STRING	SubName;

	PAGED_CODE();

	// initialize keyboard compatible id, we allow them to work
	RtlInitUnicodeString( &SubName, L"USB\\Class_03&SubClass_01&Prot_01" );
	return UsbCompareIds( &SubName, Ids );

}

static bool UsbIsMouse( PWSTR Ids ) {

	UNICODE_STRING	SubName;

	PAGED_CODE();

	// initialize keyboard compatible id, we allow them to work
	RtlInitUnicodeString( &SubName, L"USB\\Class_03&SubClass_01&Prot_02" );
	return UsbCompareIds( &SubName, Ids );

}

static bool UsbIsDeviceInAllowedList( PWSTR Ids ) {

	PWSTR			AllowedDevices = g_UsbAllowedDevices;
	UNICODE_STRING	SubName;
	SIZE_T			IdLength;

	PAGED_CODE();

	// no devices at all
	if ( !AllowedDevices ) {
		return false;
	}

	// scan all ids
	for ( ; AllowedDevices[0]; AllowedDevices += IdLength + 1 ) {

		IdLength = wcslen( AllowedDevices );

		// initialize custom vid and pid
		RtlInitUnicodeString( &SubName, AllowedDevices );
		if ( UsbCompareIds( &SubName, Ids ) ) {
			return true;
		}

	}

	return false;

}

static NTSTATUS UsbIsDeviceAllowedToWork( bool* HubOrComposite, SUSBDevice* USBDevice ) {

	PWSTR		HardwareIds = nullptr;
	PWSTR		CompatibleIds = nullptr;

	NTSTATUS	Status;

	PAGED_CODE();

	*HubOrComposite = false;

	for ( ;; ) {

		// get hardware ids
		Status = UsbDispatchPnpQueryIds( &HardwareIds, USBDevice, false );
		if ( !NT_SUCCESS( Status ) ) {
			break;
		}

		// is device USB? we must do this check, if device wasn't
		// enumerated by USB bus we must allow it to work
		if ( !UsbIsUsbEnumerator( HardwareIds ) ) {
			break;
		}

		// is device root hub?
		if ( UsbIsRootPort( HardwareIds ) ) {
			*HubOrComposite = true;
			break;
		}

		// get compatible ids
		Status = UsbDispatchPnpQueryIds( &CompatibleIds, USBDevice, true );
		if ( !NT_SUCCESS( Status ) ) {
			break;
		}

		// device may not have compatible ids
		if ( CompatibleIds ) {

			// is device hub?
			if ( UsbIsHub( CompatibleIds ) ) {
				*HubOrComposite = true;
				break;
			}

			// is composite device
			if ( UsbIsComposite( CompatibleIds ) ) {
				*HubOrComposite = true;
				break;
			}

			// we allow keyboards and mouses
			if ( UsbIsKeyboard( CompatibleIds ) || UsbIsMouse( CompatibleIds ) ) {
				break;
			}

		}

		// check whether device in allowed list
		if ( UsbIsDeviceInAllowedList( HardwareIds ) ||
			 (CompatibleIds && UsbIsDeviceInAllowedList( CompatibleIds )) ) {

			break;

		}

		// we didn't recognize device, lock it
		Status = STATUS_ACCESS_DENIED;
		break;

	}

	// cleanup
	if ( CompatibleIds ) {
		ExFreePoolWithTag( CompatibleIds, USBDEVICE_PAGED_POOL );
	}

	if ( HardwareIds ) {
		ExFreePoolWithTag( HardwareIds, USBDEVICE_PAGED_POOL );
	}

	return Status;

}

static NTSTATUS UsbDispatchPnpStartDevice( SUSBDevice* USBDevice, PIRP Irp ) {

	bool		HubOrComposite;
	NTSTATUS	Status;

	PAGED_CODE();

	for ( ;; ) {

		// check if we allow device to work and update hub or composite flag
		Status = UsbIsDeviceAllowedToWork( &HubOrComposite, USBDevice );
		if ( !NT_SUCCESS( Status ) ) {
			break;
		}
		USBDevice->HubOrComposite = HubOrComposite;

		// forward request
		Status = ForwardIrpSynchronously( USBDevice->LowerDevice, Irp );
		if ( !NT_SUCCESS( Status ) ) {
			break;
		}

		break;

	}

	// complete request
	Irp->IoStatus.Status = Status;
	IoCompleteRequest( Irp, IO_NO_INCREMENT );

	// and release device
	IoReleaseRemoveLock( &USBDevice->Lock, Irp );

	return Status;

}

static NTSTATUS UsbDispatchPnpRemoveDevice( SUSBDevice* USBDevice, PIRP Irp ) {

	NTSTATUS	Status;

	PAGED_CODE();

	// wait for all requests will is completed
	IoReleaseRemoveLockAndWait( &USBDevice->Lock, Irp );

	// forward request
	IoSkipCurrentIrpStackLocation( Irp );
	Status = IoCallDriver( USBDevice->LowerDevice, Irp );

	// delete device
	USBDevice->DeleteDevice( USBDevice->SelfDevice );

	return Status;

}

static NTSTATUS UsbDispatchPnpDeviceUsageNotification( SUSBDevice* USBDevice, PIRP Irp ) {

	PIO_STACK_LOCATION	StackLocation;
	NTSTATUS			Status;

	PAGED_CODE();

	// get stack location
	StackLocation = IoGetCurrentIrpStackLocation( Irp );

	// lock paging state
	KeEnterCriticalRegion();
	KeWaitForSingleObject( &USBDevice->PagingLock, Executive, KernelMode, false, nullptr );

	// depending on are we top filter or not, do following
	if ( USBDevice->SelfDevice->AttachedDevice ) {

		// we are not first filter, so we have to propagate DO_POWER_PAGABLE flag from upper filter
		if ( USBDevice->SelfDevice->AttachedDevice->Flags & DO_POWER_PAGABLE ) {
			USBDevice->SelfDevice->Flags |= DO_POWER_PAGABLE;
		} else {
			USBDevice->SelfDevice->Flags &= ~DO_POWER_PAGABLE;
		}

	} else {

		// we are top filter, determine whether we should set DO_POWER_PAGABLE flag or not

		// for last page file we must set DO_POWER_PAGABLE before forwarding the request
		if ( !StackLocation->Parameters.UsageNotification.InPath && USBDevice->PagingCount == 1 ) {
			USBDevice->SelfDevice->Flags |= DO_POWER_PAGABLE;
		}

	}

	// forward request
	Status = ForwardIrpSynchronously( USBDevice->LowerDevice, Irp );
	if ( NT_SUCCESS( Status ) ) {

		// depending on lower filter success update page file counter

		if ( !StackLocation->Parameters.UsageNotification.InPath ) {
			USBDevice->PagingCount--;
		} else {
			USBDevice->PagingCount++;
		}

	}

	// propagate DO_POWER_PAGABLE from lower filter
	if ( USBDevice->LowerDevice->Flags & DO_POWER_PAGABLE ) {
		USBDevice->SelfDevice->Flags |= DO_POWER_PAGABLE;
	} else {
		USBDevice->SelfDevice->Flags &= ~DO_POWER_PAGABLE;
	}

	// release lock
	KeSetEvent( &USBDevice->PagingLock, IO_NO_INCREMENT, false );
	KeLeaveCriticalRegion();

	// complete request
	IoCompleteRequest( Irp, IO_NO_INCREMENT );

	// and release device
	IoReleaseRemoveLock( &USBDevice->Lock, Irp );

	return Status;

}

static NTSTATUS UsbDispatchPnpQueryDeviceRelations( SUSBDevice* USBDevice, PIRP Irp ) {

	PDEVICE_RELATIONS	Devices;
	PIO_STACK_LOCATION	StackLocation;
	NTSTATUS			Status;

	PAGED_CODE();

	for ( ;; ) {

		// forward request
		Status = ForwardIrpSynchronously( USBDevice->LowerDevice, Irp );
		if ( !NT_SUCCESS( Status ) ) {
			break;
		}

		// we don't process this request in lower filters
		if ( !USBDevice->UpperFilter ) {
			break;
		}

		// we don't process this request for non-bus devices
		if ( !USBDevice->HubOrComposite ) {
			break;
		}

		// attach our filter to child devices
		StackLocation = IoGetCurrentIrpStackLocation( Irp );
		if ( StackLocation->MinorFunction == IRP_MN_QUERY_DEVICE_RELATIONS &&
			StackLocation->Parameters.QueryDeviceRelations.Type == BusRelations &&
			Irp->IoStatus.Information ) {

			Devices = (PDEVICE_RELATIONS)Irp->IoStatus.Information;
			for ( ULONG i = 0; i < Devices->Count; i++ ) {
				UsbCreateAndAttachFilter( Devices->Objects[i], false );
			}

		}

		break;

	}

	// complete request
	IoCompleteRequest( Irp, IO_NO_INCREMENT );

	// and release device
	IoReleaseRemoveLock( &USBDevice->Lock, Irp );

	return Status;

}

NTSTATUS UsbDispatchPnp( PDEVICE_OBJECT DeviceObject, PIRP Irp ) {

	SUSBDevice*			USBDevice = (SUSBDevice*)DeviceObject->DeviceExtension;

	PIO_STACK_LOCATION	StackLocation;

	NTSTATUS			Status;

	PAGED_CODE();

	// acquire device
	Status = IoAcquireRemoveLock( &USBDevice->Lock, Irp );
	if ( !NT_SUCCESS( Status ) ) {
		Irp->IoStatus.Status = Status;
		IoCompleteRequest( Irp, IO_NO_INCREMENT );

		return Status;
	}

	StackLocation = IoGetCurrentIrpStackLocation( Irp );

	switch ( StackLocation->MinorFunction ) {

		case IRP_MN_START_DEVICE:

			// start device
			Status = UsbDispatchPnpStartDevice( USBDevice, Irp );
			break;

		case IRP_MN_REMOVE_DEVICE:

			// remove device
			Status = UsbDispatchPnpRemoveDevice( USBDevice, Irp );
			break;

		case IRP_MN_DEVICE_USAGE_NOTIFICATION:

			// handle usage notification
			Status = UsbDispatchPnpDeviceUsageNotification( USBDevice, Irp );
			break;

		case IRP_MN_QUERY_DEVICE_RELATIONS:

			// handle device relations
			Status = UsbDispatchPnpQueryDeviceRelations( USBDevice, Irp );
			break;

		default:

			// skip request
			IoSkipCurrentIrpStackLocation( Irp );
			Status = IoCallDriver( USBDevice->LowerDevice, Irp );

			// and release device
			IoReleaseRemoveLock( &USBDevice->Lock, Irp );

			break;

	}

	return Status;

}
