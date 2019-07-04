#ifndef __OS_DBG_H__
#define __OS_DBG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#define ioprintf printf

#ifdef DEBUG
#define DBG ioprintf
#else
#define DBG(...) do{}while(0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __OS_DBG_H__ */
