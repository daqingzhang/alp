#ifndef __OS_DBG_H__
#define __OS_DBG_H__
#include <stdio.h>

#define ioprintf printf

#ifdef DEBUG
#define DBG ioprintf
#else
#define DBG(...) do{}while(0)
#endif

#endif /* __OS_DBG_H__ */
