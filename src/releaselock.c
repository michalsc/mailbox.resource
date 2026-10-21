#include <exec/types.h>
#include <proto/exec.h>
#include <common/compiler.h>

#include "mailbox.h"

/* Releases the lock taken by MB_ObtainLock(). See there. */
VOID L_ReleaseLock(REGARG(struct MailboxBase *MBBase, "a6"))
{
    struct ExecBase *SysBase = MBBase->mb_ExecBase;

    ReleaseSemaphore(&MBBase->mb_Lock);
}
