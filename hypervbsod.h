#pragma once
#include "Ntifs.h"
#include "wdmsec.h"
#include "hvgdk.h"
#include "wdf.h"
// #include "hypervenlightments.h"

//
// SDDL string used when creating the device. This string
// limits access to this driver to system and admins only.
//

#define DEVICE_SDDL             L"D:P(A;;GA;;;SY)(A;;GA;;;BA)"

//
// driver object vars
//

#define   SYM_LINK_NAME   L"\\DosDevices\\hypervbsod"
#define   DEVICE_NAME	  L"\\Device\\hypervbsod"

//
// device extension
//

typedef struct _HVDETECT_DEVICE_EXTENSION
{
	PDEVICE_OBJECT	fdo;
	UNICODE_STRING	ustrSymLinkName;
} HVDETECT_DEVICE_EXTENSION, * PHVDETECT_DEVICE_EXTENSION;

#define MAX_CPU_COUNT 0x100
#define POOLTAG 'Hvhv'
#define MSR_IA32_VMX_BASIC                  0x480

typedef union _IA32_VMX_BASIC_MSR
{
	ULONG64 All;
	struct
	{
		ULONG32 RevisionIdentifier : 31;   // [0-30]
		ULONG32 Reserved1 : 1;             // [31]
		ULONG32 RegionSize : 12;           // [32-43]
		ULONG32 RegionClear : 1;           // [44]
		ULONG32 Reserved2 : 3;             // [45-47]
		ULONG32 SupportedIA64 : 1;         // [48]
		ULONG32 SupportedDualMoniter : 1;  // [49]
		ULONG32 MemoryType : 4;            // [50-53]
		ULONG32 VmExitReport : 1;          // [54]
		ULONG32 VmxCapabilityHint : 1;     // [55]
		ULONG32 Reserved3 : 8;             // [56-63]
	} Fields;
} IA32_VMX_BASIC_MSR, * PIA32_VMX_BASIC_MSR;

BOOLEAN HvActivateVpPages();
VOID ArchEnableVmx(VOID);
VOID ArchChangeCr3(VOID);
