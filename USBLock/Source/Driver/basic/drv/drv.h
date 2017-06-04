#pragma once

/***************************************************************************************\
|	Prototypes																			|
\***************************************************************************************/

typedef void						(*PDELETE_DEVICE)( PDEVICE_OBJECT );

/***************************************************************************************\
|	Structures																			|
\***************************************************************************************/

struct SDevice {

	PDELETE_DEVICE		DeleteDevice;

	PDRIVER_DISPATCH	MajorFunction[IRP_MJ_MAXIMUM_FUNCTION + 1];

};

/***************************************************************************************\
|	Variables																			|
\***************************************************************************************/

extern PDRIVER_OBJECT	g_DriverObject;
extern UNICODE_STRING	g_RegistryPath;

/***************************************************************************************\
|	Functions																			|
\***************************************************************************************/

NTSTATUS	InitializeDriverObject( PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath );
void		UninitializeDriverObject();

NTSTATUS	ForwardIrpSynchronously( PDEVICE_OBJECT LowerDeviceObject, PIRP Irp );
