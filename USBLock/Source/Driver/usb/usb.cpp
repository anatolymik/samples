#include "incs.h"

/***************************************************************************************\
|	Declarations																		|
\***************************************************************************************/

#define USB_ALLOWED_DEVICE_SUBKEY					L"AllowedDevices"

/***************************************************************************************\
|	Functions section associations														|
\***************************************************************************************/

extern "C" {

	NTSTATUS			UsbInitialize();
	void				UsbUninitialize();

	NTSTATUS			UsbAddDeviceToAllowedList( USHORT Vid, USHORT Pid );
	NTSTATUS			UsbRemoveDeviceFromAllowedList( USHORT Vid, USHORT Pid );

	static void			DetachAndDeleteDevice( PDEVICE_OBJECT USBDeviceObject );
	NTSTATUS			UsbCreateAndAttachFilter( PDEVICE_OBJECT PhysicalDeviceObject, bool UpperFilter );

}

#ifdef ALLOC_PRAGMA

	#pragma alloc_text(PAGE, UsbInitialize)
	#pragma alloc_text(PAGE, UsbUninitialize)

	#pragma alloc_text(PAGE, UsbAddDeviceToAllowedList)
	#pragma alloc_text(PAGE, UsbRemoveDeviceFromAllowedList)

	#pragma alloc_text(PAGE, DetachAndDeleteDevice)
	#pragma alloc_text(PAGE, UsbCreateAndAttachFilter)

#endif

/***************************************************************************************\
|	Functions																			|
\***************************************************************************************/

NTSTATUS UsbInitialize() {

	PKEY_VALUE_BASIC_INFORMATION	BasicInformation = nullptr;
	ULONG							BasicInformationLength = sizeof( KEY_VALUE_BASIC_INFORMATION );

	ULONG							ReturnedLength;

	OBJECT_ATTRIBUTES				Attributies;

	UNICODE_STRING					Name = { 0, 0, nullptr };

	HANDLE							Root = nullptr;
	HANDLE							Key = nullptr;

	PWSTR							Buffer = 0;
	SIZE_T							BufferLength = 0;

	NTSTATUS						Status;

	PAGED_CODE();

	for ( ;; ) {

		// allocate memory for information
		BasicInformation = (PKEY_VALUE_BASIC_INFORMATION)ExAllocatePoolWithTag( PagedPool, BasicInformationLength, USBDEVICE_PAGED_POOL );
		if ( !BasicInformation ) {
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		// open key
		InitializeObjectAttributes( &Attributies, &g_RegistryPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, nullptr, nullptr );
		Status = ZwOpenKey( &Root, KEY_CREATE_SUB_KEY, &Attributies );
		if ( !NT_SUCCESS( Status ) ) {
			break;
		}

		// open or create subkey
		RtlInitUnicodeString( &Name, USB_ALLOWED_DEVICE_SUBKEY );
		InitializeObjectAttributes( &Attributies, &Name, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, Root, nullptr );
		Status = ZwCreateKey( &Key, KEY_QUERY_VALUE | KEY_SET_VALUE | KEY_ENUMERATE_SUB_KEYS, &Attributies, 0, nullptr, REG_OPTION_NON_VOLATILE, nullptr );
		if ( !NT_SUCCESS( Status ) ) {
			break;
		}

		// enumerate all values
		for ( ULONG i = 0; ; i++ ) {

			// get value name
			do {

				Status = ZwEnumerateValueKey( Key, i, KeyValueBasicInformation, BasicInformation, BasicInformationLength, &ReturnedLength );
				if ( Status == STATUS_BUFFER_OVERFLOW ) {

					// not enough memory, add it
					if ( BasicInformation ) {
						ExFreePoolWithTag( BasicInformation, USBDEVICE_PAGED_POOL );
					}

					BasicInformationLength = ReturnedLength;
					BasicInformation = (PKEY_VALUE_BASIC_INFORMATION)ExAllocatePoolWithTag( PagedPool, BasicInformationLength, USBDEVICE_PAGED_POOL );
					if ( !BasicInformation ) {
						Status = STATUS_INSUFFICIENT_RESOURCES;
					}

				}

			} while ( Status == STATUS_BUFFER_OVERFLOW );

			if ( !NT_SUCCESS( Status ) ) {
				break;
			}

			// we don't process not binary values
			if ( BasicInformation->Type != REG_BINARY ) {
				continue;
			}

			// append allowed device id to list
			BufferLength = BasicInformation->NameLength + sizeof( wchar_t );
			if ( g_UsbAllowedDevicesLength > 0 ) {
				BufferLength += g_UsbAllowedDevicesLength;
			} else {
				BufferLength += sizeof( wchar_t );
			}
			Buffer = (PWSTR)ExAllocatePoolWithTag( PagedPool, BufferLength, USBDEVICE_PAGED_POOL );
			if ( !Buffer ) {
				Status = STATUS_INSUFFICIENT_RESOURCES;
				break;
			}
			if ( g_UsbAllowedDevicesLength > 0 ) {
				RtlCopyMemory( Buffer, g_UsbAllowedDevices, g_UsbAllowedDevicesLength - sizeof( wchar_t ) );
				RtlCopyMemory( &Buffer[g_UsbAllowedDevicesLength / sizeof( wchar_t ) - 1], BasicInformation->Name, BasicInformation->NameLength );
			} else {
				RtlCopyMemory( Buffer, BasicInformation->Name, BasicInformation->NameLength );
			}
			Buffer[BufferLength / sizeof( wchar_t ) - 2] = L'\0';
			Buffer[BufferLength / sizeof( wchar_t ) - 1] = L'\0';

			// update global list
			if ( g_UsbAllowedDevices ) {
				ExFreePoolWithTag( g_UsbAllowedDevices, USBDEVICE_PAGED_POOL );
			}
			g_UsbAllowedDevices = Buffer;
			g_UsbAllowedDevicesLength = (ULONG)BufferLength;

		}

		if ( Status == STATUS_NO_MORE_ENTRIES ) {
			Status = STATUS_SUCCESS;
		}

		break;

	}

	// cleanup

	if ( !NT_SUCCESS( Status ) ) {

		if ( g_UsbAllowedDevices ) {
			ExFreePoolWithTag( g_UsbAllowedDevices, USBDEVICE_PAGED_POOL );
			g_UsbAllowedDevices = nullptr;
		}

	}

	if ( Key ) {
		ZwClose( Key );
	}

	if ( Root ) {
		ZwClose( Root );
	}

	if ( BasicInformation ) {
		ExFreePoolWithTag( BasicInformation, USBDEVICE_PAGED_POOL );
	}

	return Status;

}

void UsbUninitialize()  {

	// cleanup
	if ( g_UsbAllowedDevices ) {
		ExFreePoolWithTag( g_UsbAllowedDevices, USBDEVICE_PAGED_POOL );
	}

}

NTSTATUS UsbAddDeviceToAllowedList( USHORT Vid, USHORT Pid ) {

	OBJECT_ATTRIBUTES				Attributies;

	wchar_t							DeviceId[128];

	UNICODE_STRING					Name = { 0, 0, nullptr };

	HANDLE							Root = nullptr;
	HANDLE							Key = nullptr;

	UCHAR							Data = 0;

	NTSTATUS						Status;

	PAGED_CODE();

	for ( ;; ) {

		// initialize name
		Status = RtlStringCchPrintfW( DeviceId, sizeof( DeviceId ) / sizeof( wchar_t ), L"USB\\VID_%04X&PID_%04X", Vid, Pid );
		if ( !NT_SUCCESS( Status ) ) {
			break;
		}

		// open key
		InitializeObjectAttributes( &Attributies, &g_RegistryPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, nullptr, nullptr );
		Status = ZwOpenKey( &Root, KEY_CREATE_SUB_KEY, &Attributies );
		if ( !NT_SUCCESS( Status ) ) {
			break;
		}

		// open or create subkey
		RtlInitUnicodeString( &Name, USB_ALLOWED_DEVICE_SUBKEY );
		InitializeObjectAttributes( &Attributies, &Name, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, Root, nullptr );
		Status = ZwCreateKey( &Key, KEY_QUERY_VALUE | KEY_SET_VALUE | KEY_ENUMERATE_SUB_KEYS, &Attributies, 0, nullptr, REG_OPTION_NON_VOLATILE, nullptr );
		if ( !NT_SUCCESS( Status ) ) {
			break;
		}

		// write value
		RtlInitUnicodeString( &Name, DeviceId );
		Status = ZwSetValueKey( Key, &Name, 0, REG_BINARY, &Data, sizeof( Data ) );
		if ( !NT_SUCCESS( Status ) ) {
			break;
		}

		break;

	}

	// cleanup

	if ( Key ) {
		ZwClose( Key );
	}

	if ( Root ) {
		ZwClose( Root );
	}

	return Status;

}

NTSTATUS UsbRemoveDeviceFromAllowedList( USHORT Vid, USHORT Pid ) {

	OBJECT_ATTRIBUTES				Attributies;

	wchar_t							DeviceId[128];

	UNICODE_STRING					Name = { 0, 0, nullptr };

	HANDLE							Root = nullptr;
	HANDLE							Key = nullptr;

	NTSTATUS						Status;

	PAGED_CODE();

	for ( ;; ) {

		// initialize name
		Status = RtlStringCchPrintfW( DeviceId, sizeof( DeviceId ) / sizeof( wchar_t ), L"USB\\VID_%04X&PID_%04X", Vid, Pid );
		if ( !NT_SUCCESS( Status ) ) {
			break;
		}

		// open key
		InitializeObjectAttributes( &Attributies, &g_RegistryPath, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, nullptr, nullptr );
		Status = ZwOpenKey( &Root, KEY_CREATE_SUB_KEY, &Attributies );
		if ( !NT_SUCCESS( Status ) ) {
			break;
		}

		// open or create subkey
		RtlInitUnicodeString( &Name, USB_ALLOWED_DEVICE_SUBKEY );
		InitializeObjectAttributes( &Attributies, &Name, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, Root, nullptr );
		Status = ZwCreateKey( &Key, KEY_QUERY_VALUE | KEY_SET_VALUE | KEY_ENUMERATE_SUB_KEYS, &Attributies, 0, nullptr, REG_OPTION_NON_VOLATILE, nullptr );
		if ( !NT_SUCCESS( Status ) ) {
			break;
		}

		// delete value
		RtlInitUnicodeString( &Name, DeviceId );
		Status = ZwDeleteValueKey( Key, &Name );
		if ( !NT_SUCCESS( Status ) ) {
			break;
		}

		break;

	}

	// cleanup

	if ( Key ) {
		ZwClose( Key );
	}

	if ( Root ) {
		ZwClose( Root );
	}

	return Status;

}

static void DetachAndDeleteDevice( PDEVICE_OBJECT USBDeviceObject ) {

	SUSBDevice*			USBDevice = (SUSBDevice*)USBDeviceObject->DeviceExtension;

	PAGED_CODE();

	// detach device
	IoDetachDevice( USBDevice->LowerDevice );

	// and delete device
	IoDeleteDevice( USBDeviceObject );

}

NTSTATUS UsbCreateAndAttachFilter( PDEVICE_OBJECT PhysicalDeviceObject, bool UpperFilter ) {

	SUSBDevice*			USBDevice;
	PDEVICE_OBJECT		USBDeviceObject = nullptr;

	ULONG				Flags;

	NTSTATUS			Status = STATUS_SUCCESS;

	PAGED_CODE();

	for ( ;; ) {

		// if it's lower filter attaching, then we should be sure
		// there will be only one lower filter
		if ( !UpperFilter ) {
			USBDeviceObject = PhysicalDeviceObject;
			while ( USBDeviceObject->AttachedDevice ) {
				if ( USBDeviceObject->DriverObject == g_DriverObject ) {
					return STATUS_SUCCESS;
				}
				USBDeviceObject = USBDeviceObject->AttachedDevice;
			}
		}

		// create filter device
		Status = IoCreateDevice(
			g_DriverObject,
			sizeof( SUSBDevice ),
			nullptr,
			PhysicalDeviceObject->DeviceType,
			PhysicalDeviceObject->Characteristics,
			false,
			&USBDeviceObject
		);
		if ( !NT_SUCCESS( Status ) ) {
			break;
		}

		// reflect lower device flags in our created one
		Flags = PhysicalDeviceObject->Flags & (DO_BUFFERED_IO | DO_DIRECT_IO | DO_POWER_PAGABLE);
		USBDeviceObject->Flags |= Flags;

		// get instance pointer
		USBDevice = (SUSBDevice*)USBDeviceObject->DeviceExtension;

		// initialize device destructor routine
		USBDevice->DeleteDevice = DetachAndDeleteDevice;

		// initialize its dispatch routines
		for ( ULONG i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++ ) {
			USBDevice->MajorFunction[i] = UsbDispatchCommon;
		}
		USBDevice->MajorFunction[IRP_MJ_PNP] = UsbDispatchPnp;
		USBDevice->MajorFunction[IRP_MJ_POWER] = UsbDispatchPower;

		// initialize remove lock
		IoInitializeRemoveLock( &USBDevice->Lock, USBDEVICE_REMOVE_LOCK_TAG, 0, 0 );

		// initialize its device objects and attach it
		USBDevice->SelfDevice = USBDeviceObject;
		USBDevice->BaseDevice = PhysicalDeviceObject;
		USBDevice->UpperFilter = UpperFilter;

		// initialize paging usage lock
		USBDevice->PagingCount = 0;
		KeInitializeEvent( &USBDevice->PagingLock, SynchronizationEvent, true );

		// attach our device
		USBDevice->LowerDevice = IoAttachDeviceToDeviceStack( USBDeviceObject, PhysicalDeviceObject );
		if ( !USBDevice->LowerDevice ) {
			Status = STATUS_NO_SUCH_DEVICE;
			break;
		}

		break;

	}

	// depending on result status do following

	if ( !NT_SUCCESS( Status ) ) {

		// do cleanup

		if ( USBDeviceObject ) {
			IoDeleteDevice( USBDeviceObject );
		}

	} else {

		// reset initializing mark
		USBDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

	}

	return Status;

}
