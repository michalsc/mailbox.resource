# mailbox.resource

Access to the Raspberry Pi VideoCore mailbox (property channel) from AmigaOS under Emu68. The resource is embedded in the
Emu68 ROM: nothing to install, only the headers to copy to your SDK.

## Why use it

Several programs talk to the mailbox at the same time (drivers, network, monitoring tools). The resource gives each of them:

* __semaphoring__: calls queue up instead of overwriting each other;
* __big-endian__ conversion: you write the message as the CPU sees it;
* a __Pi-aligned__ buffer, and a timeout (3 s) instead of a hang.

## Usage

```c
#include <proto/exec.h>
#include <proto/mailbox.h>

APTR MailboxBase;

if (MailboxBase = OpenResource("mailbox.resource"))
{
    /* code starts here */
}
```

A resource is never closed: it is always there, in the ROM.

### MB_RawCommand()

`VOID MB_RawCommand(ULONG *command)`

For a single command: one message of your own, for any tag which has no shortcut. The message is a plain array of longwords,
written as the CPU sees it: the resource swaps it for the mailbox and back, and gives it to you with the answer, in place.

```c
ULONG cmd[7];

cmd[0] = 7 * 4;         /* request size in bytes */
cmd[1] = 0;             /* request code */
cmd[2] = 0x00010002;    /* tag: GET_BOARD_REVISION */
cmd[3] = 4;             /* value size in bytes */
cmd[4] = 0;             /* tag request code */
cmd[5] = 0;             /* the answer comes here */
cmd[6] = 0;             /* end tag */

MB_RawCommand(cmd);     /* cmd[0] is 0xffffffff if it failed or timed out */
```

`cmd[0]` is `0xffffffff` when the call failed: the mailbox did not answer within 3 s, or the message is longer than 480 longwords.

### MB_RawCommand() with nested commands

A message can carry several tags one after the other. They are sent together, in one round trip to the mailbox, and no other
task can come in between them: the answers are consistent with each other. Each tag has its own header and its own values,
and the message ends with a single end tag.

Here the temperature and its maximum are asked in one message:

```c
ULONG cmd[13];

cmd[0]  = 13 * 4;       /* request size in bytes */
cmd[1]  = 0;            /* request code */

cmd[2]  = 0x00030006;   /* first tag: GET_TEMPERATURE */
cmd[3]  = 2 * 4;        /* value size in bytes */
cmd[4]  = 0;            /* tag request code */
cmd[5]  = 0;            /* value[0]: temperature id, 0 */
cmd[6]  = 0;            /* value[1]: the temperature comes here, in thousandths of a degree */

cmd[7]  = 0x0003000a;   /* second tag: GET_MAX_TEMPERATURE */
cmd[8]  = 2 * 4;        /* value size in bytes */
cmd[9]  = 0;            /* tag request code */
cmd[10] = 0;            /* value[0]: temperature id, 0 */
cmd[11] = 0;            /* value[1]: the maximum comes here */

cmd[12] = 0;            /* end tag */

MB_RawCommand(cmd);

if (cmd[0] != 0xffffffff)
    Printf("Temperature: %ld / %ld (thousandths of a degree)\n", (LONG)cmd[6], (LONG)cmd[11]);
```

### MB_RawCommand() with non-nested commands

`VOID MB_ObtainLock(VOID)` and `VOID MB_ReleaseLock(VOID)`

For several separate commands which must not be separated: nested commands (above) are one message, but some work needs several messages in a row. Where the firmware keeps a state from one message to the next, a first message selects something and the next one acts on it. These two calls make the whole sequence one block for the other tasks.

Each call is safe on its own, two calls in a row are not: another task can send its message in between. It matters when the firmware keeps a state from one message to the next. For example `SET_DISPLAY_NUM` selects the display that the following framebuffer tags act on, and it stays selected after the message.

`MB_ObtainLock()` keeps the lock until `MB_ReleaseLock()`. The calls made in between take it again, it nests for its owner.

A complete program:
- open the resource
- check the version
- lock / execute / unlock

```c
#include <exec/types.h>
#include <exec/libraries.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/mailbox.h>

APTR MailboxBase;

LONG MB_GetPitch(ULONG displayNum)
{
    LONG result = 0;
    
    struct Library * lib = (struct Library *)MailboxBase;
    
    /* Check version */
    if (lib->lib_Version >= 1 && lib->lib_Revision >= 2)
    {
        /* SET_DISPLAY_NUM */
        ULONG cmd1[7] = { 7 * 4, 0, 0x00048013, 4, 0, displayNum, 0 };
        
        /* GET_PITCH */
        ULONG cmd2[7] = { 7 * 4, 0, 0x00040008, 4, 0, 0, 0 };
        
        /* Mailbox transaction */
        MB_ObtainLock();
        MB_RawCommand(cmd1);
        MB_RawCommand(cmd2);
        MB_ReleaseLock();

        /* Mailbox result */
        if (cmd2[0] != 0xffffffff)
        {
            result = (LONG)cmd2[5];
        }
    }
    
    return result;
}

int main(void)
{
    MailboxBase = OpenResource("mailbox.resource");
    
    if (MailboxBase != NULL)
    {
        LONG pitch;
        
        pitch = MB_GetPitch(0);
        Printf("Pitch of display #0: %ld bytes", pitch);
        
        pitch = MB_GetPitch(1);
        Printf("Pitch of display #1: %ld bytes", pitch);
    }
    
    return (0);
}
```

* Take it, send, release: the other tasks wait while you hold it.
* Not from an interrupt, and wait for nothing else meanwhile.
* Every `MB_ObtainLock()` needs its `MB_ReleaseLock()`.
* On a ROM older than 1.2 the two calls do not exist and would jump into nothing: always check the version first, as above.

### MB_StringCommand()

The function `MB_StringCommand()` sends a `vcgencmd`-style string:

```c
UBYTE result[1024];

MB_StringCommand("measure_temp", result, sizeof(result));

PutStr(result);
```

## Build

`mkdir build && cd build && cmake .. && make`, with the m68k-amigaos gcc and `sfdc` in the path. `build/include/` gets the
prototypes, pragmas and inlines made from `sfd/mailbox_lib.sfd`.
