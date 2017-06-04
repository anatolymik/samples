#pragma once

/***************************************************************************************\
|	Functions																			|
\***************************************************************************************/

extern "C" NTSTATUS		CtrlDispatchDeviceIoControl( PDEVICE_OBJECT DeviceObject, PIRP Irp );
