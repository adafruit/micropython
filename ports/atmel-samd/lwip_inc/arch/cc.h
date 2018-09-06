#ifndef MICROPY_INCLUDED_ATMEL_SAMD_LWIP_ARCH_CC_H
#define MICROPY_INCLUDED_ATMEL_SAMD_LWIP_ARCH_CC_H

#include <assert.h>

#ifdef LWIP_DEBUG
#include <stdio.h>
#define LWIP_PLATFORM_DIAG(x)    do { printf x; } while(0)
#define LWIP_PLATFORM_ASSERT(x)  { assert(x); }
#else
#define LWIP_PLATFORM_DIAG(x)
#define LWIP_PLATFORM_ASSERT(x)  { assert(1); }
#endif

#define U8_F "u"
#define S8_F "d"
#define X8_F "x"
#define U16_F "u"
#define S16_F "d"
#define X16_F "x"
#define U32_F "lu"
#define S32_F "ld"
#define X32_F "lx"

#endif // MICROPY_INCLUDED_ATMEL_SAMD_LWIP_ARCH_CC_H
