
#include <exec/types.h>
#include <exec/execbase.h>
#include <proto/exec.h>
#include <proto/expansion.h>
#include <proto/devicetree.h>

#include <libraries/configregs.h>
#include <libraries/configvars.h>

#include <common/compiler.h>

#include "mailbox.h"

extern UBYTE rom_end;
extern const char deviceName[];
extern const char deviceIdString[];

/*
    Some properties, like e.g. #size-cells, are not always available in a key, but in that case the properties
    should be searched for in the parent. The process repeats recursively until either root key is found
    or the property is found, whichever occurs first
*/
CONST_APTR GetPropValueRecursive(APTR key, CONST_STRPTR property, APTR DeviceTreeBase)
{
    do {
        /* Find the property first */
        APTR prop = DT_FindProperty(key, property);

        if (prop)
        {
            /* If property is found, get its value and exit */
            return DT_GetPropValue(prop);
        }
        
        /* Property was not found, go to the parent and repeat */
        key = DT_GetParent(key);
    } while (key);

    return NULL;
}

APTR Init(REGARG(struct ExecBase *SysBase, "a6"))
{
    struct MailboxBase *MailboxBase = NULL;
    struct ExpansionBase *ExpansionBase = NULL;
    struct CurrentBinding binding;

    APTR base_pointer = NULL;
    
    ExpansionBase = (struct ExpansionBase *)OpenLibrary("expansion.library", 0);
    GetCurrentBinding(&binding, sizeof(binding));

    base_pointer = AllocMem(BASE_NEG_SIZE + BASE_POS_SIZE, MEMF_PUBLIC | MEMF_CLEAR);

    if (base_pointer)
    {
        ULONG relFuncTable[NUMBER_OF_FUNCTIONS + 1];

        relFuncTable[0] = (ULONG)&L_RawCommand;
        relFuncTable[1] = (ULONG)&L_StringCommand;
        relFuncTable[2] = (ULONG)&L_GetClockRate;
        relFuncTable[3] = (ULONG)&L_SetClockRate;
        relFuncTable[4] = (ULONG)&L_GetClockState;
        relFuncTable[5] = (ULONG)&L_SetClockState;
        relFuncTable[6] = (ULONG)&L_GetPowerState;
        relFuncTable[7] = (ULONG)&L_SetPowerState;
        relFuncTable[8] = (ULONG)&L_GetGPIOState;
        relFuncTable[9] = (ULONG)&L_SetGPIOState;
        relFuncTable[NUMBER_OF_FUNCTIONS] = (ULONG)-1;

        MailboxBase = (struct MailboxBase *)((UBYTE *)base_pointer + BASE_NEG_SIZE);
        MakeFunctions(MailboxBase, relFuncTable, 0);
        
        MailboxBase->mb_Node.lib_Node.ln_Type = NT_RESOURCE;
        MailboxBase->mb_Node.lib_Node.ln_Pri = 120;
        MailboxBase->mb_Node.lib_Node.ln_Name = (STRPTR)deviceName;

        MailboxBase->mb_Node.lib_NegSize = BASE_NEG_SIZE;
        MailboxBase->mb_Node.lib_PosSize = BASE_POS_SIZE;
        MailboxBase->mb_Node.lib_Version = MB_VERSION;
        MailboxBase->mb_Node.lib_Revision = MB_REVISION;
        MailboxBase->mb_Node.lib_IdString = (STRPTR)deviceIdString;

        MailboxBase->mb_RequestBase = AllocMem(512*4, MEMF_FAST);
        MailboxBase->mb_Request = (ULONG *)(((ULONG)MailboxBase->mb_RequestBase + 127) & ~127);

        MailboxBase->mb_ExecBase = SysBase;

        InitSemaphore(&MailboxBase->mb_Lock);

        APTR DeviceTreeBase = OpenResource("devicetree.resource");

        if (DeviceTreeBase != NULL)
        {
            /* Get VC4 physical address of mailbox interface. Subsequently it will be translated to m68k physical address */
            APTR key = DT_OpenKey("/aliases");

            if (key)
            {
                CONST_STRPTR mbox_alias = DT_GetPropValue(DT_FindProperty(key, "mailbox"));

                DT_CloseKey(key);
               
                if (mbox_alias != NULL)
                {
                    key = DT_OpenKey(mbox_alias);

                    if (key)
                    {
                        int size_cells = 1;
                        int address_cells = 1;

                        const ULONG * siz = GetPropValueRecursive(key, "#size_cells", DeviceTreeBase);
                        const ULONG * addr = GetPropValueRecursive(key, "#address-cells", DeviceTreeBase);

                        if (siz != NULL)
                            size_cells = *siz;
                        
                        if (addr != NULL)
                            address_cells = *addr;

                        const ULONG *reg = DT_GetPropValue(DT_FindProperty(key, "reg"));

                        MailboxBase->mb_MailBox = (APTR)reg[address_cells - 1];

                        DT_CloseKey(key);
                    }
                }
                DT_CloseKey(key);
            }

            /* Open /soc key and learn about VC4 to CPU mapping. Use it to adjust the addresses obtained above */
            key = DT_OpenKey("/soc");
            if (key)
            {
                int size_cells = 1;
                int address_cells = 1;
                int cpu_address_cells = 1;

                const ULONG * siz = GetPropValueRecursive(key, "#size_cells", DeviceTreeBase);
                const ULONG * addr = GetPropValueRecursive(key, "#address-cells", DeviceTreeBase);
                const ULONG * cpu_addr = DT_GetPropValue(DT_FindProperty(DT_OpenKey("/"), "#address-cells"));
            
                if (siz != NULL)
                    size_cells = *siz;
                
                if (addr != NULL)
                    address_cells = *addr;

                if (cpu_addr != NULL)
                    cpu_address_cells = *cpu_addr;

                const ULONG *reg = DT_GetPropValue(DT_FindProperty(key, "ranges"));

                ULONG phys_vc4 = reg[address_cells - 1];
                ULONG phys_cpu = reg[address_cells + cpu_address_cells - 1];

                MailboxBase->mb_MailBox = (APTR)((ULONG)MailboxBase->mb_MailBox - phys_vc4 + phys_cpu);

                DT_CloseKey(key);
            }

            SumLibrary((struct Library*)MailboxBase);
            AddResource(MailboxBase);

            binding.cb_ConfigDev->cd_Flags &= ~CDF_CONFIGME;
            binding.cb_ConfigDev->cd_Driver = MailboxBase;
        }
        else
        {
            FreeMem(base_pointer, BASE_NEG_SIZE + BASE_POS_SIZE);
            MailboxBase = NULL;
        }
    }

    CloseLibrary((struct Library*)ExpansionBase);

    return MailboxBase;
}
