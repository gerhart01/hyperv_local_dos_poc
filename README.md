Hyper-V DoS PoC
This is code files of Visual Studio 2019 driver project. 
1. Compile driver.
2. Enable nested virtualization for guest OS using command:

```
Set-VMProcessor -VMName <VMName> -ExposeVirtualizationExtensions $true
```
3. Run driver inside guest OS. Host OS will be rebooted or generate BSOD with HYPERVISOR_ERROR code.

Mitigations:
Simple disable nested virtualization on untrusted VM. Execute command on host server:

```
Set-VMProcessor -VMName <VMName> -ExposeVirtualizationExtensions $false
```

