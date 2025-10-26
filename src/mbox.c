#include <exec/types.h>

#include "mailbox.h"


/* status register flags */

#define MBOX_TX_FULL (1UL << 31)
#define MBOX_RX_EMPTY (1UL << 30)
#define MBOX_CHANMASK 0xF

/* VideoCore tags used. */

#define VCTAG_GET_ARM_MEMORY     0x00010005
#define VCTAG_GET_CLOCK_RATE     0x00030002

ULONG mbox_recv(ULONG channel, struct MailboxBase * Base)
{
    volatile ULONG *mbox_read = (ULONG*)(Base->mb_MailBox);
    volatile ULONG *mbox_status = (ULONG*)((ULONG)Base->mb_MailBox + 0x18);
    ULONG response, status;

    do
    {
        do
        {
            status = LE32(*mbox_status);
            asm volatile("nop");
        }
        while (status & MBOX_RX_EMPTY);

        asm volatile("nop");
        response = LE32(*mbox_read);
        asm volatile("nop");
    }
    while ((response & MBOX_CHANMASK) != channel);

    return (response & ~MBOX_CHANMASK);
}

void mbox_send(ULONG channel, ULONG data, struct MailboxBase * Base)
{
    volatile ULONG *mbox_write = (ULONG*)((ULONG)Base->mb_MailBox + 0x20);
    volatile ULONG *mbox_status = (ULONG*)((ULONG)Base->mb_MailBox + 0x18);
    ULONG status;

    data &= ~MBOX_CHANMASK;
    data |= channel & MBOX_CHANMASK;

    do
    {
        status = LE32(*mbox_status);
        asm volatile("nop");
    }
    while (status & MBOX_TX_FULL);

    asm volatile("nop");
    *mbox_write = LE32(data);
}
