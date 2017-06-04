#include <windows.h>

#include <stdio.h>

#include "../Driver/exp.h"

/***************************************************************************************\
|	Functions																			|
\***************************************************************************************/

bool IsStringHexNumber( LPWSTR Str ) {

	SIZE_T		Length = wcslen( Str );

	for ( SIZE_T i = 0; i < Length; i++ ) {
		if ( !iswxdigit( Str[i] ) ) {
			return false;
		}
	}

	return true;

}

int main() {

	HANDLE		DeviceHandle;

	LPWSTR*		CommandLine;
	int			CommandLineArgs;

	SDeviceId	DeviceId;

	long		Vid;
	long		Pid;

	bool		AddCommand = false;

	DWORD		BytesReturned;

	bool		Result;

	DWORD		Status = ERROR_SUCCESS;

	for ( ;; ) {

		// open interface
		DeviceHandle = CreateFileW(
			USB_CONTROL_INTERFACE_WIN32_W,
			FILE_READ_ATTRIBUTES,
			0,
			nullptr,
			OPEN_EXISTING,
			0,
			nullptr
		);
		if ( DeviceHandle == INVALID_HANDLE_VALUE ) {
			printf( "Driver is not working or you don't have access rights\n\r" );
			return GetLastError();
		}

		// get arguments
		CommandLine = CommandLineToArgvW( GetCommandLineW(), &CommandLineArgs );
		if ( !CommandLine || CommandLineArgs < 4 ) {
			printf( "Wrong arguments number\n\r" );
			Status = ERROR_GEN_FAILURE;
			break;
		}

		// check first argument
		if ( wcscmp( CommandLine[1], L"add" ) != 0 && wcscmp( CommandLine[1], L"remove" ) != 0 ) {
			printf( "Wrong command argument\n\r" );
			Status = ERROR_GEN_FAILURE;
			break;
		}
		if ( wcscmp( CommandLine[1], L"add" ) == 0 ) {
			AddCommand = true;
		}

		// get vid
		Vid = wcstol( CommandLine[2], nullptr, 16 );
		if ( !IsStringHexNumber( CommandLine[2] ) || Vid < 0 || Vid > MAXWORD ) {
			printf( "Wrong VID argument\n\r" );
			Status = ERROR_GEN_FAILURE;
			break;
		}

		// get pid
		Pid = wcstol( CommandLine[3], nullptr, 16 );
		if ( !IsStringHexNumber( CommandLine[3] ) || Vid < 0 || Vid > MAXWORD ) {
			printf( "Wrong VID argument\n\r" );
			Status = ERROR_GEN_FAILURE;
			break;
		}

		// initialize id
		DeviceId.Vid = (USHORT)Vid;
		DeviceId.Pid = (USHORT)Pid;

		// process command
		Result = DeviceIoControl(
			DeviceHandle,
			AddCommand ? USB_CONTROL_IOCTL_ADD_DEVICE_TO_ALLOWED_LIST : USB_CONTROL_IOCTL_REMOVE_DEVICE_FROM_ALLOWED_LIST,
			&DeviceId,
			sizeof( DeviceId ),
			nullptr,
			0,
			&BytesReturned,
			nullptr
		) ? true : false;
		if ( !Result ) {
			printf( "Command failed\n\r" );
			Status = GetLastError();
			break;
		}

		printf( "Success, changes would be applied after PC reboot\n\r" );
		break;

	}

	// close interface
	CloseHandle( DeviceHandle );

	return Status;

}
