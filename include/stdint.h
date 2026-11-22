#ifndef STDINT_H
#define STDINT_H

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

typedef signed   char      int8_t;
typedef signed   short     int16_t;
typedef signed   int       int32_t;
typedef signed   long long int64_t;

#define INT8_MIN  (-128)
#define INT16_MIN (-32768)
#define INT32_MIN (-2147483647 - 1)
#define INT64_MIN (-9223372036854775807LL - 1)

#define INT8_MAX  (127)
#define INT16_MAX (32767)
#define INT32_MAX (2147483647)
#define INT64_MAX (9223372036854775807LL)

#define UINT8_MAX  (0xff)
#define UINT16_MAX (0xffff)
#define UINT32_MAX (0xffffffffU)
#define UINT64_MAX (0xffffffffffffffffULL)

#define INT8_C(c)  (c)
#define INT16_C(c) (c)
#define INT32_C(c) (c)
#define INT64_C(c) (c ## LL)

#define UINT8_C(c)  (c)
#define UINT16_C(c) (c)
#define UINT32_C(c) (c ## U)
#define UINT64_C(c) (c ## ULL)

#define INTMAX_MAX   INT64_MAX
#define INTMAX_MIN   INT64_MIN
#define UINTMAX_MAX  UINT64_MAX

#define INTPTR_MAX   INT32_MAX
#define INTPTR_MIN   INT32_MIN
#define UINTPTR_MAX  UINT32_MAX
#define PTRDIFF_MAX  INT32_MAX
#define PTRDIFF_MIN  INT32_MIN
#define SIZE_MAX     UINT32_MAX

#endif
