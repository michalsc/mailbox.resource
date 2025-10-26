#include <exec/types.h>
#include <proto/exec.h>
#include <common/compiler.h>

#include "mailbox.h"

VOID L_RawCommand(REGARG(ULONG * command, "a0"), REGARG(struct MailboxBase *MBBase, "a6"))
{
    struct ExecBase *SysBase = MBBase->mb_ExecBase;
    ULONG *FBReq = MBBase->mb_Request;

    if (command == NULL) return;
    
    ULONG count = command[0] / 4;
    ULONG len = command[0];

    /* Command cannot be longer than 480 words, or it would overwrite the buffer */
    if (count > 480) {
        command[0] = 0xffffffff;
        return;
    }

    ObtainSemaphore(&MBBase->mb_Lock);

    /* Copy user's buffer converting it to little endian */
    for (ULONG i = 0; i < count; i++)
    {
        FBReq[i] = LE32(command[i]);
    }

    CachePreDMA(FBReq, &len, 0);
    mbox_send(8, (ULONG)FBReq, MBBase);
    ULONG resp = mbox_recv(8, MBBase);
    CachePostDMA(FBReq, &len, 0);

    /* Convert the buffer back to big endian for the caller */
    for (ULONG i = 0; i < count; i++)
    {
        command[i] = LE32(FBReq[i]);
    }

    ReleaseSemaphore(&MBBase->mb_Lock);
}
