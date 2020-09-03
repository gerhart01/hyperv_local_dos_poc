#include "hypervbsod.h"

VOID     UnloadRoutine(IN PDRIVER_OBJECT DriverObject);
NTSTATUS Create_File_IRPprocessing(IN PDEVICE_OBJECT fdo, IN PIRP Irp);
NTSTATUS Close_HandleIRPprocessing(IN PDEVICE_OBJECT fdo, IN PIRP Irp);
NTSTATUS ReadWrite_IRPhandler(IN PDEVICE_OBJECT fdo, IN PIRP Irp);

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject,
	IN PUNICODE_STRING RegistryPath)
{
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_OBJECT  fdo;
	UNICODE_STRING  devName;
	PHVDETECT_DEVICE_EXTENSION dx;
	UNICODE_STRING symLinkName;
	UNICODE_STRING DeviceSDDLString;
	UNREFERENCED_PARAMETER(RegistryPath);

	DriverObject->DriverUnload = UnloadRoutine;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = Create_File_IRPprocessing;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = Close_HandleIRPprocessing;
	DriverObject->MajorFunction[IRP_MJ_READ] = ReadWrite_IRPhandler;
	DriverObject->MajorFunction[IRP_MJ_WRITE] = ReadWrite_IRPhandler;

	RtlInitUnicodeString(&devName, DEVICE_NAME);
	RtlInitUnicodeString(&DeviceSDDLString, DEVICE_SDDL);

	status = IoCreateDeviceSecure(DriverObject,
		sizeof(HVDETECT_DEVICE_EXTENSION),
		&devName,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		FALSE,
		&DeviceSDDLString,
		NULL,
		&fdo);

	if (!NT_SUCCESS(status)) return status;

	dx = (PHVDETECT_DEVICE_EXTENSION)fdo->DeviceExtension;
	dx->fdo = fdo;

	RtlInitUnicodeString(&symLinkName, SYM_LINK_NAME);
	dx->ustrSymLinkName = symLinkName;

	status = IoCreateSymbolicLink(&symLinkName, &devName);
	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(fdo);
		return status;
	}

	HvActivateVpPages();

	return status;
}

NTSTATUS CompleteIrp(PIRP Irp, NTSTATUS status, ULONG info)
{
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = info;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS ReadWrite_IRPhandler(IN PDEVICE_OBJECT fdo, IN PIRP Irp)
{
	ULONG BytesTxd = 0;
	NTSTATUS status = STATUS_SUCCESS;
	UNREFERENCED_PARAMETER(fdo);
	return CompleteIrp(Irp, status, BytesTxd);
}

NTSTATUS Create_File_IRPprocessing(IN PDEVICE_OBJECT fdo, IN PIRP Irp)
{
	UNREFERENCED_PARAMETER(fdo);
	return CompleteIrp(Irp, STATUS_SUCCESS, 0);
}

NTSTATUS Close_HandleIRPprocessing(IN PDEVICE_OBJECT fdo, IN PIRP Irp)
{
	UNREFERENCED_PARAMETER(fdo);
	return CompleteIrp(Irp, STATUS_SUCCESS, 0);
}


VOID UnloadRoutine(IN PDRIVER_OBJECT pDriverObject)
{
	PDEVICE_OBJECT	pNextDevObj;
	int i;

	pNextDevObj = pDriverObject->DeviceObject;

	for (i = 0; pNextDevObj != NULL; i++)
	{
		PHVDETECT_DEVICE_EXTENSION dx =
			(PHVDETECT_DEVICE_EXTENSION)pNextDevObj->DeviceExtension;
		UNICODE_STRING* pLinkName = &(dx->ustrSymLinkName);
		pNextDevObj = pNextDevObj->NextDevice;
		IoDeleteSymbolicLink(pLinkName);
		IoDeleteDevice(dx->fdo);
	}
}