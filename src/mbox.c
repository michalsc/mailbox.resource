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
    ULONG timer = LE32(*(volatile ULONG*)0xf2003004);

    do
    {
        do
        {
            status = LE32(*mbox_status);
            asm volatile("nop");

            /* If waiting more than three seconds, return with error */
            if (LE32(*(volatile ULONG*)0xf2003004) - timer > 3000000)
                return -1;
        }
        while (status & MBOX_RX_EMPTY);

        asm volatile("nop");
        response = LE32(*mbox_read);
        asm volatile("nop");

        /* If waiting more than three seconds, return with error */
        if (LE32(*(volatile ULONG*)0xf2003004) - timer > 3000000)
            return -1;
    }
    while ((response & MBOX_CHANMASK) != channel);

    return (response & ~MBOX_CHANMASK);
}

void mbox_send(ULONG channel, ULONG data, struct MailboxBase * Base)
{
    volatile ULONG *mbox_write = (ULONG*)((ULONG)Base->mb_MailBox + 0x20);
    volatile ULONG *mbox_status = (ULONG*)((ULONG)Base->mb_MailBox + 0x18);
    ULONG status;
    ULONG timer = LE32(*(volatile ULONG*)0xf2003004);

    data &= ~MBOX_CHANMASK;
    data |= channel & MBOX_CHANMASK;

    do
    {
        status = LE32(*mbox_status);
        asm volatile("nop");

        /* If waiting more than three seconds, return with error */
        if (LE32(*(volatile ULONG*)0xf2003004) - timer > 3000000)
            return;
    }
    while (status & MBOX_TX_FULL);

    asm volatile("nop");
    *mbox_write = LE32(data);
}
