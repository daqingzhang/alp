#ifndef __OS_DBG_H__
#define __OS_DBG_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>

typedef unsigned int uint32_t;

#define USB_TR_LEVEL	0x00000005
#define COM_TR_LEVEL	0x00000005
#define SOCK_TR_LEVEL	0x00000005

extern uint32_t sock_trmask;
extern uint32_t comm_trmask;
extern uint32_t usb_trmask;

#define COMM_TRACE(mask, str, ...) {if (comm_trmask & (1<<mask)) \
	{fprintf(stdout, str, ##__VA_ARGS__);}}

#define USB_TRACE(mask, str, ...) {if (usb_trmask & (1<<mask)) \
	{fprintf(stdout, str, ##__VA_ARGS__);}}

#define SOCK_TRACE(mask, str, ...) {if (sock_trmask & (1<<mask)) \
	{fprintf(stdout, str, ##__VA_ARGS__);}}

#define TRACE(str, ...) fprintf(stdout, str, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* __OS_DBG_H__ */
