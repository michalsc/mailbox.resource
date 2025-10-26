#include <exec/types.h>
#include <proto/exec.h>
#include <common/compiler.h>

#include "mailbox.h"

ULONG L_StringCommand(REGARG(STRPTR command, "a0"), REGARG(STRPTR reply, "a1"), REGARG(ULONG reply_capacity, "d0"), REGARG(struct MailboxBase *MBBase, "a6"))
{
    struct ExecBase *SysBase = MBBase->mb_ExecBase;
    ULONG *FBReq = MBBase->mb_Request;
    ULONG len = 0;
    ULONG i;
    ULONG retval;

    /* Command must be a valid pointer */
    if (command == NULL) return -1;

    /* Get command length */
    for (i = 0; i < 1024; i++) {
        if (command[i] == 0) {
            len = i - 1;
            break;
        }
    }

    /* Command length including \0 must not exceed 1024 bytes */
    if (len > 1023) return -1;

    ObtainSemaphore(&MBBase->mb_Lock);

    i = 0;
    FBReq[i++] = 0;
    FBReq[i++] = 0x00000000;
    FBReq[i++] = LE32(0x00030080);
    FBReq[i++] = LE32(1024);
    FBReq[i++] = 0; // request_len (set to response length)
    FBReq[i++] = 0;

    /* Copy string */
    CopyMem(command, &FBReq[i], len + 1);

    i += 1024 >> 2;

    FBReq[i++] = 0x00000000; // end tag

    /* Update length */
    FBReq[0] = LE32(4 * i);

    CachePreDMA(FBReq, &len, 0);
    mbox_send(8, (ULONG)FBReq, MBBase);
    ULONG resp = mbox_recv(8, MBBase);
    CachePostDMA(FBReq, &len, 0);

    if (reply && reply_capacity > 0) {
        CopyMem(&FBReq[6], reply, reply_capacity > 1024 ? 1024 : reply_capacity);
    }

    retval = LE32(FBReq[5]);

    ReleaseSemaphore(&MBBase->mb_Lock);

    return retval;
}
