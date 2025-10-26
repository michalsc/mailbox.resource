#ifndef _MAILBOX_H
#define _MAILBOX_H

#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/semaphores.h>
#include <common/compiler.h>

#define MB_PRIORITY     119
#define MB_VERSION      1
#define MB_REVISION     0

struct MailboxBase {
    struct Library          mb_Node;
    struct ExecBase *       mb_ExecBase;
    struct SignalSemaphore  mb_Lock;
    APTR                    mb_MailBox;
    APTR                    mb_RequestBase;
    ULONG *                 mb_Request;
};

#define NUMBER_OF_FUNCTIONS 10
#define BASE_NEG_SIZE (NUMBER_OF_FUNCTIONS * 6)
#define BASE_POS_SIZE ((sizeof(struct MailboxBase)))

void mbox_send(ULONG channel, ULONG data, struct MailboxBase * Base);
ULONG mbox_recv(ULONG channel, struct MailboxBase * Base);

static inline ULONG LE32(ULONG x) { return __builtin_bswap32(x); }

VOID L_RawCommand(REGARG(ULONG * command, "a0"), REGARG(struct MailboxBase *MBBase, "a6"));
ULONG L_StringCommand(REGARG(STRPTR command, "a0"), REGARG(STRPTR reply, "a1"), REGARG(ULONG reply_capacity, "d0"), REGARG(struct MailboxBase *MBBase, "a6"));
ULONG L_GetClockRate(REGARG(ULONG clock_id, "d0"), REGARG(struct MailboxBase *MBBase, "a6"));
ULONG L_SetClockRate(REGARG(ULONG clock_id, "d0"), REGARG(ULONG speed, "d1"), REGARG(struct MailboxBase *MBBase, "a6"));
ULONG L_GetClockState(REGARG(ULONG clock_id, "d0"), REGARG(struct MailboxBase *MBBase, "a6"));
ULONG L_SetClockState(REGARG(ULONG clock_id, "d0"), REGARG(ULONG state, "d1"), REGARG(struct MailboxBase *MBBase, "a6"));
ULONG L_GetPowerState(REGARG(ULONG id, "d0"), REGARG(struct MailboxBase *MBBase, "a6"));
ULONG L_SetPowerState(REGARG(ULONG id, "d0"), REGARG(ULONG state, "d1"), REGARG(struct MailboxBase *MBBase, "a6"));
ULONG L_GetGPIOState(REGARG(ULONG gpio, "d0"), REGARG(struct MailboxBase *MBBase, "a6"));
ULONG L_SetGPIOState(REGARG(ULONG gpio, "d0"), REGARG(ULONG state, "d1"), REGARG(struct MailboxBase *MBBase, "a6"));



#endif /* _MAILBOX_H */
