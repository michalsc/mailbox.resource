#include <exec/types.h>
#include <proto/exec.h>
#include <common/compiler.h>

#include "mailbox.h"

/*
    Takes the lock which serialises the commands, and keeps it until MB_ReleaseLock().

    Every command is serialised on its own, but a sequence of commands is not: when the firmware keeps a state between two
    messages (a selection made by the first one and used by the next), another task can send its own message in between.
    A task which holds the lock sees no other command until it releases it, so the sequence cannot be cut.

    The lock is a SignalSemaphore. It nests for the task which owns it, so the commands sent between MB_ObtainLock() and
    MB_ReleaseLock() simply take it again. Each MB_ObtainLock() needs its MB_ReleaseLock().

    The other tasks wait while the lock is held: take it, send the commands, release it. Do not call it from an interrupt,
    and do not wait for anything else while holding it.
*/
VOID L_ObtainLock(REGARG(struct MailboxBase *MBBase, "a6"))
{
    struct ExecBase *SysBase = MBBase->mb_ExecBase;

    ObtainSemaphore(&MBBase->mb_Lock);
}
