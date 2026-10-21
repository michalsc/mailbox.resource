#ifndef RESOURCES_MAILBOX_H
#define RESOURCES_MAILBOX_H
/*
**	$VER: mailbox.h 1.2
**
**	Public definitions of "mailbox.resource", the VideoCore mailbox of the Raspberry Pi
**	(PiStorm / Emu68): the ids that its calls take, and the property tags of MB_RawCommand().
**	The vector offsets are in the FD and the sfd. See Autodocs/mailbox.doc.
**
*/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#define MAILBOXNAME "mailbox.resource"

/* What the calls give back when the firmware does not answer within three seconds (V1.1) */
#define MB_FAILED               ((ULONG)0xffffffff)

/* MB_RawCommand(): the buffer holds at most this many longwords, command[0] being its size in bytes */
#define MB_RAW_MAXLONGS         480

/* command[1] of a buffer: the code of the request, then of the answer of the firmware */
#define MB_REQUEST              (0x00000000)
#define MB_SUCCESS              (0x80000000)
#define MB_ERROR_PARSING        (0x80000001)

/* Clocks, for MB_GetClockRate(), MB_SetClockRate(), MB_GetClockState(), MB_SetClockState() */
#define MB_CLOCK_EMMC           (0x01)
#define MB_CLOCK_UART           (0x02)
#define MB_CLOCK_ARM            (0x03)
#define MB_CLOCK_CORE           (0x04)
#define MB_CLOCK_V3D            (0x05)
#define MB_CLOCK_H264           (0x06)
#define MB_CLOCK_ISP            (0x07)
#define MB_CLOCK_SDRAM          (0x08)
#define MB_CLOCK_PIXEL          (0x09)
#define MB_CLOCK_PWM            (0x0a)
#define MB_CLOCK_HEVC           (0x0b)
#define MB_CLOCK_EMMC2          (0x0c)
#define MB_CLOCK_M2MC           (0x0d)
#define MB_CLOCK_PIXEL_BVB      (0x0e)

/* Devices, for MB_GetPowerState() and MB_SetPowerState() */
#define MB_POWER_SDCARD         (0x00)
#define MB_POWER_UART0          (0x01)
#define MB_POWER_UART1          (0x02)
#define MB_POWER_USBHCD         (0x03)
#define MB_POWER_I2C0           (0x04)
#define MB_POWER_I2C1           (0x05)
#define MB_POWER_I2C2           (0x06)
#define MB_POWER_SPI            (0x07)
#define MB_POWER_CCP2TX         (0x08)
#define MB_POWER_UNKNOWN9       (0x09)   /* Raspberry Pi 4, not documented by the firmware */
#define MB_POWER_UNKNOWN10      (0x0a)   /* idem */

/* Pins, for MB_GetGPIOState() and MB_SetGPIOState() */
#define MB_GPIO_STATUS_LED      (42)
#define MB_GPIO_POWER_LED       (130)

/* Property tags that the calls above send, for those who build their own buffer for MB_RawCommand() */
#define MB_TAG_GET_FIRMWARE_REV (0x00000001)
#define MB_TAG_GET_POWER_STATE  (0x00020001)
#define MB_TAG_SET_POWER_STATE  (0x00028001)
#define MB_TAG_GET_CLOCK_STATE  (0x00030001)
#define MB_TAG_GET_CLOCK_RATE   (0x00030002)
#define MB_TAG_GET_LED_STATUS   (0x00030041)
#define MB_TAG_SET_CLOCK_STATE  (0x00038001)
#define MB_TAG_SET_CLOCK_RATE   (0x00038002)
#define MB_TAG_SET_LED_STATUS   (0x00038041)

#endif /* RESOURCES_MAILBOX_H */
