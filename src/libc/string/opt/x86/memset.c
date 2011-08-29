/*!The Tiny Box Library
 * 
 * TBox is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 * 
 * TBox is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with TBox; 
 * If not, see <a href="http://www.gnu.org/licenses/"> http://www.gnu.org/licenses/</a>
 * 
 * Copyright (C) 2009 - 2011, ruki All rights reserved.
 *
 * \author		ruki
 * \file		memset.c
 *
 */

/* /////////////////////////////////////////////////////////
 * includes
 */
#include "prefix.h"
#ifdef TB_CONFIG_OPTI_SSE2_ENABLE
# 	include <emmintrin.h>
#endif

/* /////////////////////////////////////////////////////////
 * macros
 */
#define TB_LIBC_STRING_OPT_MEMSET_U8

#if defined(TB_CONFIG_ASSEMBLER_GAS) || \
		defined(TB_CONFIG_OPTI_SSE2_ENABLE)
# 	define TB_LIBC_STRING_OPT_MEMSET_U16
# 	define TB_LIBC_STRING_OPT_MEMSET_U32
#endif
/* /////////////////////////////////////////////////////////
 * implemention
 */

#ifdef TB_CONFIG_ASSEMBLER_GAS
static __tb_inline__ tb_void_t tb_memset_u8_opt_v1(tb_byte_t* s, tb_byte_t c, tb_size_t n)
{
	tb_int_t reg, edi;
	__tb_asm__ __tb_volatile__
	(
		/* most of the time, count is divisible by 4 and nonzero */
		/* it's better to make this case faster */
		/*	"	jecxz	9f\n" - (optional) count == 0: goto ret */
		" 	mov	%%ecx, %1\n"
		" 	shr	$2, %%ecx\n"
		" 	jz	1f\n" /* zero words: goto fill_bytes */
		/* extend 8-bit fill to 32 bits */
		" 	movzx	%%al, %%eax\n" /* 3 bytes */
		/* or:	"	and	$0xff, %%eax\n" - 5 bytes */
		" 	imul	$0x01010101, %%eax\n" /* 6 bytes */
		/* fill full words */
		" 	rep; stosl\n"
		/* fill 0-3 bytes */
		"1:	and	$3, %1\n"
		"	jz	9f\n" /* (count & 3) == 0: goto end */
		"2:	stosb\n"
		"	dec	%1\n"
		"	jnz	2b\n"
		/* end */
		"9:\n"

		: "=&D" (edi), "=&r" (reg)
		: "0" (s), "a" (c), "c" (n)
 		: "memory"
	);
	return s;
}
#endif

#ifdef TB_CONFIG_OPTI_SSE2_ENABLE
static __tb_inline__ tb_void_t tb_memset_u8_opt_v2(tb_byte_t* s, tb_byte_t c, tb_size_t n)
{
    if (n >= 64) 
	{
		// aligned by 16-bytes
        for (; ((tb_size_t)s) & 0x0f; --n) *s++ = c;

		// l = n % 64
		tb_size_t l = n & 0x3f; n = (n - l) >> 6;

		// fill 4 x 16 bytes
        __m128i* 	d = (__m128i*)(s);
        __m128i 	v = _mm_set1_epi8(c);
        while (n) 
		{
            _mm_store_si128(d++, v);
            _mm_store_si128(d++, v);
            _mm_store_si128(d++, v);
            _mm_store_si128(d++, v);
            --n;
        }
        s = (tb_byte_t*)(d);
		n = l;
    }
	while (n--) *s++ = c;
}
#endif

#ifdef TB_LIBC_STRING_OPT_MEMSET_U8
tb_void_t* tb_memset(tb_void_t* s, tb_size_t c, tb_size_t n)
{
	TB_ASSERT_RETURN_VAL(s, TB_NULL);
	if (!n) return s;

# 	if 1
	memset(s, c, n);
# 	elif defined(TB_CONFIG_ASSEMBLER_GAS)
	tb_memset_u8_opt_v1(s, (tb_byte_t)c, n);
# 	elif defined(TB_CONFIG_OPTI_SSE2_ENABLE)
	tb_memset_u8_opt_v2(s, (tb_byte_t)c, n);
# 	else
# 		error
# 	endif

	return s;
}
#endif


#ifdef TB_CONFIG_ASSEMBLER_GAS
static __tb_inline__ tb_void_t tb_memset_u16_opt_v1(tb_uint16_t* s, tb_uint16_t c, tb_size_t n)
{
	// align by 4-bytes 
	if (((tb_size_t)s) & 0x3)
	{
		*s++ = c;
		--n;
	}

	__tb_asm__ __tb_volatile__
	(
		"cld\n\t" 							// clear the direction bit, s++, not s--
		"rep stosw" 						// *s++ = ax
		: 									// no output registers
		: "c" (n), "a" (c), "D" (s) 	// ecx = n, eax = c, edi = s
	);
}
#endif

#ifdef TB_CONFIG_OPTI_SSE2_ENABLE
static __tb_inline__ tb_void_t tb_memset_u16_opt_v2(tb_uint16_t* s, tb_uint16_t c, tb_size_t n)
{
    if (n >= 32) 
	{
		// aligned by 16-bytes
        for (; ((tb_size_t)s) & 0x0f; --n) *s++ = c;

		// l = n % 32
		tb_size_t l = n & 0x1f; n = (n - l) >> 5;

		// fill 4 x 16 bytes
        __m128i* 	d = (__m128i*)(s);
        __m128i 	v = _mm_set1_epi16(c);
        while (n) 
		{
            _mm_store_si128(d++, v);
            _mm_store_si128(d++, v);
            _mm_store_si128(d++, v);
            _mm_store_si128(d++, v);
            --n;
        }
        s = (tb_uint16_t*)(d);
		n = l;
    }
	while (n--) *s++ = c;
}
#endif

#ifdef TB_LIBC_STRING_OPT_MEMSET_U16
tb_void_t* tb_memset_u16(tb_void_t* s, tb_size_t c, tb_size_t n)
{
	TB_ASSERT_RETURN_VAL(s, TB_NULL);

	// align by 2-bytes 
	TB_ASSERT(!(((tb_size_t)s) & 0x1));
	if (!n) return s;

# 	if defined(TB_CONFIG_ASSEMBLER_GAS) && \
		defined(TB_CONFIG_OPTI_SSE2_ENABLE)
	if (n < 2049) tb_memset_u16_opt_v2(s, (tb_uint16_t)c, n);
	else tb_memset_u16_opt_v1(s, (tb_uint16_t)c, n);
# 	elif defined(TB_CONFIG_ASSEMBLER_GAS)
	tb_memset_u16_opt_v1(s, (tb_uint16_t)c, n);
# 	elif defined(TB_CONFIG_OPTI_SSE2_ENABLE)
	tb_memset_u16_opt_v2(s, (tb_uint16_t)c, n);
# 	else
# 		error
# 	endif

	return s;
}
#endif

#ifdef TB_CONFIG_ASSEMBLER_GAS
static __tb_inline__ tb_void_t tb_memset_u32_opt_v1(tb_uint32_t* s, tb_uint32_t c, tb_size_t n)
{
	__tb_asm__ __tb_volatile__
	(
		"cld\n\t" 							// clear the direction bit, s++, not s--
		"rep stosl" 						// *s++ = eax
		: 									// no output registers
		: "c" (n), "a" (c), "D" (s) 	// ecx = n, eax = c, edi = s
	);
}
#endif

#ifdef TB_CONFIG_OPTI_SSE2_ENABLE
static __tb_inline__ tb_void_t tb_memset_u32_opt_v2(tb_uint32_t* s, tb_uint32_t c, tb_size_t n)
{
    if (n >= 16) 
	{
		// aligned by 16-bytes
        for (; ((tb_size_t)s) & 0x0f; --n) *s++ = c;

		// l = n % 16
		tb_size_t l = n & 0x0f; n = (n - l) >> 4;

		// fill 4 x 16 bytes
        __m128i* 	d = (__m128i*)(s);
        __m128i 	v = _mm_set1_epi32(c);
        while (n) 
		{
            _mm_store_si128(d++, v);
            _mm_store_si128(d++, v);
            _mm_store_si128(d++, v);
            _mm_store_si128(d++, v);
            --n;
        }
        s = (tb_uint32_t*)(d);
		n = l;
    }
    while (n--) *s++ = c;
}
#endif

#ifdef TB_LIBC_STRING_OPT_MEMSET_U32
tb_void_t* tb_memset_u32(tb_void_t* s, tb_size_t c, tb_size_t n)
{
	TB_ASSERT_RETURN_VAL(s, TB_NULL);

	// align by 4-bytes 
	TB_ASSERT(!(((tb_size_t)s) & 0x3));
	if (!n) return s;

# 	if defined(TB_CONFIG_ASSEMBLER_GAS) && \
	defined(TB_CONFIG_OPTI_SSE2_ENABLE)
	if (n < 2049) tb_memset_u32_opt_v2(s, (tb_uint32_t)c, n);
	else tb_memset_u32_opt_v1(s, (tb_uint32_t)c, n);
# 	elif defined(TB_CONFIG_ASSEMBLER_GAS)
	tb_memset_u32_opt_v1(s, (tb_uint32_t)c, n);
# 	elif defined(TB_CONFIG_OPTI_SSE2_ENABLE)
	tb_memset_u32_opt_v2(s, (tb_uint32_t)c, n);
# 	else
# 		error
# 	endif

	return s;
}
#endif


