/*
 *----------------------------------------------------------------------
 *
 * tclTomMathDecls.h --
 *
 *	This file contains the declarations for the 'libtommath'
 *	functions that are exported by the Tcl library.
 *
 * Copyright (c) 2005 by Kevin B. Kenny.  All rights reserved.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * RCS: @(#) $Id: tclTomMathDecls.h,v 1.1.2.9 2010/02/09 17:53:09 dgp Exp $
 */

#ifndef _TCLTOMMATHDECLS
#define _TCLTOMMATHDECLS

#include "tcl.h"

/*
 * Define the version of the Stubs table that's exported for tommath
 */

#define TCLTOMMATH_EPOCH 0
#define TCLTOMMATH_REVISION 0

#define Tcl_TomMath_InitStubs(interp,version) \
    (TclTomMathInitializeStubs((interp),(version),\
                               TCLTOMMATH_EPOCH,TCLTOMMATH_REVISION))

/* Define custom memory allocation for libtommath */

/* MODULE_SCOPE void* TclBNAlloc( size_t ); */
#define TclBNAlloc(s) ((void*)ckalloc((size_t)(s)))
/* MODULE_SCOPE void* TclBNRealloc( void*, size_t ); */
#define TclBNRealloc(x,s) ((void*)ckrealloc((char*)(x),(size_t)(s)))
/* MODULE_SCOPE void  TclBNFree( void* ); */
#define TclBNFree(x) (ckfree((char*)(x)))
/* MODULE_SCOPE void* TclBNCalloc( size_t, size_t ); */
/* unused - no macro */

#define XMALLOC(x) TclBNAlloc(x)
#define XFREE(x) TclBNFree(x)
#define XREALLOC(x,n) TclBNRealloc(x,n)
#define XCALLOC(n,x) TclBNCalloc(n,x)

/* Rename the global symbols in libtommath to avoid linkage conflicts */

#define KARATSUBA_MUL_CUTOFF TclBNKaratsubaMulCutoff
#define KARATSUBA_SQR_CUTOFF TclBNKaratsubaSqrCutoff
#define TOOM_MUL_CUTOFF TclBNToomMulCutoff
#define TOOM_SQR_CUTOFF TclBNToomSqrCutoff

#define bn_reverse TclBN_reverse
#define fast_s_mp_mul_digs TclBN_fast_s_mp_mul_digs
#define fast_s_mp_sqr TclBN_fast_s_mp_sqr
#define mp_add TclBN_mp_add
#define mp_add_d TclBN_mp_add_d
#define mp_and TclBN_mp_and
#define mp_clamp TclBN_mp_clamp
#define mp_clear TclBN_mp_clear
#define mp_clear_multi TclBN_mp_clear_multi
#define mp_cmp TclBN_mp_cmp
#define mp_cmp_d TclBN_mp_cmp_d
#define mp_cmp_mag TclBN_mp_cmp_mag
#define mp_copy TclBN_mp_copy
#define mp_count_bits TclBN_mp_count_bits
#define mp_div TclBN_mp_div
#define mp_div_2 TclBN_mp_div_2
#define mp_div_2d TclBN_mp_div_2d
#define mp_div_3 TclBN_mp_div_3
#define mp_div_d TclBN_mp_div_d
#define mp_exch TclBN_mp_exch
#define mp_expt_d TclBN_mp_expt_d
#define mp_grow TclBN_mp_grow
#define mp_init TclBN_mp_init
#define mp_init_copy TclBN_mp_init_copy
#define mp_init_multi TclBN_mp_init_multi
#define mp_init_set TclBN_mp_init_set
#define mp_init_size TclBN_mp_init_size
#define mp_karatsuba_mul TclBN_mp_karatsuba_mul
#define mp_karatsuba_sqr TclBN_mp_karatsuba_sqr
#define mp_lshd TclBN_mp_lshd
#define mp_mod TclBN_mp_mod
#define mp_mod_2d TclBN_mp_mod_2d
#define mp_mul TclBN_mp_mul
#define mp_mul_2 TclBN_mp_mul_2
#define mp_mul_2d TclBN_mp_mul_2d
#define mp_mul_d TclBN_mp_mul_d
#define mp_neg TclBN_mp_neg
#define mp_or TclBN_mp_or
#define mp_radix_size TclBN_mp_radix_size
#define mp_read_radix TclBN_mp_read_radix
#define mp_rshd TclBN_mp_rshd
#define mp_s_rmap TclBNMpSRmap
#define mp_set TclBN_mp_set
#define mp_shrink TclBN_mp_shrink
#define mp_sqr TclBN_mp_sqr
#define mp_sqrt TclBN_mp_sqrt
#define mp_sub TclBN_mp_sub
#define mp_sub_d TclBN_mp_sub_d
#define mp_to_unsigned_bin TclBN_mp_to_unsigned_bin
#define mp_to_unsigned_bin_n TclBN_mp_to_unsigned_bin_n
#define mp_toom_mul TclBN_mp_toom_mul
#define mp_toom_sqr TclBN_mp_toom_sqr
#define mp_toradix_n TclBN_mp_toradix_n
#define mp_unsigned_bin_size TclBN_mp_unsigned_bin_size
#define mp_xor TclBN_mp_xor
#define mp_zero TclBN_mp_zero
#define s_mp_add TclBN_s_mp_add
#define s_mp_mul_digs TclBN_s_mp_mul_digs
#define s_mp_sqr TclBN_s_mp_sqr
#define s_mp_sub TclBN_s_mp_sub

#undef TCL_STORAGE_CLASS
#ifdef BUILD_tcl
#   define TCL_STORAGE_CLASS DLLEXPORT
#else
#   ifdef USE_TCL_STUBS
#      define TCL_STORAGE_CLASS
#   else
#      define TCL_STORAGE_CLASS DLLIMPORT
#   endif
#endif

/*
 * WARNING: This file is automatically generated by the tools/genStubs.tcl
 * script.  Any modifications to the function declarations below should be made
 * in the generic/tclInt.decls script.
 */

/* !BEGIN!: Do not edit below this line. */

/*
 * Exported function declarations:
 */

#ifndef TclBN_epoch_TCL_DECLARED
#define TclBN_epoch_TCL_DECLARED
/* 0 */
EXTERN int		TclBN_epoch(void);
#endif
#ifndef TclBN_revision_TCL_DECLARED
#define TclBN_revision_TCL_DECLARED
/* 1 */
EXTERN int		TclBN_revision(void);
#endif
#ifndef TclBN_mp_add_TCL_DECLARED
#define TclBN_mp_add_TCL_DECLARED
/* 2 */
EXTERN int		TclBN_mp_add(mp_int *a, mp_int *b, mp_int *c);
#endif
#ifndef TclBN_mp_add_d_TCL_DECLARED
#define TclBN_mp_add_d_TCL_DECLARED
/* 3 */
EXTERN int		TclBN_mp_add_d(mp_int *a, mp_digit b, mp_int *c);
#endif
#ifndef TclBN_mp_and_TCL_DECLARED
#define TclBN_mp_and_TCL_DECLARED
/* 4 */
EXTERN int		TclBN_mp_and(mp_int *a, mp_int *b, mp_int *c);
#endif
#ifndef TclBN_mp_clamp_TCL_DECLARED
#define TclBN_mp_clamp_TCL_DECLARED
/* 5 */
EXTERN void		TclBN_mp_clamp(mp_int *a);
#endif
#ifndef TclBN_mp_clear_TCL_DECLARED
#define TclBN_mp_clear_TCL_DECLARED
/* 6 */
EXTERN void		TclBN_mp_clear(mp_int *a);
#endif
#ifndef TclBN_mp_clear_multi_TCL_DECLARED
#define TclBN_mp_clear_multi_TCL_DECLARED
/* 7 */
EXTERN void		TclBN_mp_clear_multi(mp_int *a, ...);
#endif
#ifndef TclBN_mp_cmp_TCL_DECLARED
#define TclBN_mp_cmp_TCL_DECLARED
/* 8 */
EXTERN int		TclBN_mp_cmp(mp_int *a, mp_int *b);
#endif
#ifndef TclBN_mp_cmp_d_TCL_DECLARED
#define TclBN_mp_cmp_d_TCL_DECLARED
/* 9 */
EXTERN int		TclBN_mp_cmp_d(mp_int *a, mp_digit b);
#endif
#ifndef TclBN_mp_cmp_mag_TCL_DECLARED
#define TclBN_mp_cmp_mag_TCL_DECLARED
/* 10 */
EXTERN int		TclBN_mp_cmp_mag(mp_int *a, mp_int *b);
#endif
#ifndef TclBN_mp_copy_TCL_DECLARED
#define TclBN_mp_copy_TCL_DECLARED
/* 11 */
EXTERN int		TclBN_mp_copy(mp_int *a, mp_int *b);
#endif
#ifndef TclBN_mp_count_bits_TCL_DECLARED
#define TclBN_mp_count_bits_TCL_DECLARED
/* 12 */
EXTERN int		TclBN_mp_count_bits(mp_int *a);
#endif
#ifndef TclBN_mp_div_TCL_DECLARED
#define TclBN_mp_div_TCL_DECLARED
/* 13 */
EXTERN int		TclBN_mp_div(mp_int *a, mp_int *b, mp_int *q,
				mp_int *r);
#endif
#ifndef TclBN_mp_div_d_TCL_DECLARED
#define TclBN_mp_div_d_TCL_DECLARED
/* 14 */
EXTERN int		TclBN_mp_div_d(mp_int *a, mp_digit b, mp_int *q,
				mp_digit *r);
#endif
#ifndef TclBN_mp_div_2_TCL_DECLARED
#define TclBN_mp_div_2_TCL_DECLARED
/* 15 */
EXTERN int		TclBN_mp_div_2(mp_int *a, mp_int *q);
#endif
#ifndef TclBN_mp_div_2d_TCL_DECLARED
#define TclBN_mp_div_2d_TCL_DECLARED
/* 16 */
EXTERN int		TclBN_mp_div_2d(mp_int *a, int b, mp_int *q,
				mp_int *r);
#endif
#ifndef TclBN_mp_div_3_TCL_DECLARED
#define TclBN_mp_div_3_TCL_DECLARED
/* 17 */
EXTERN int		TclBN_mp_div_3(mp_int *a, mp_int *q, mp_digit *r);
#endif
#ifndef TclBN_mp_exch_TCL_DECLARED
#define TclBN_mp_exch_TCL_DECLARED
/* 18 */
EXTERN void		TclBN_mp_exch(mp_int *a, mp_int *b);
#endif
#ifndef TclBN_mp_expt_d_TCL_DECLARED
#define TclBN_mp_expt_d_TCL_DECLARED
/* 19 */
EXTERN int		TclBN_mp_expt_d(mp_int *a, mp_digit b, mp_int *c);
#endif
#ifndef TclBN_mp_grow_TCL_DECLARED
#define TclBN_mp_grow_TCL_DECLARED
/* 20 */
EXTERN int		TclBN_mp_grow(mp_int *a, int size);
#endif
#ifndef TclBN_mp_init_TCL_DECLARED
#define TclBN_mp_init_TCL_DECLARED
/* 21 */
EXTERN int		TclBN_mp_init(mp_int *a);
#endif
#ifndef TclBN_mp_init_copy_TCL_DECLARED
#define TclBN_mp_init_copy_TCL_DECLARED
/* 22 */
EXTERN int		TclBN_mp_init_copy(mp_int *a, mp_int *b);
#endif
#ifndef TclBN_mp_init_multi_TCL_DECLARED
#define TclBN_mp_init_multi_TCL_DECLARED
/* 23 */
EXTERN int		TclBN_mp_init_multi(mp_int *a, ...);
#endif
#ifndef TclBN_mp_init_set_TCL_DECLARED
#define TclBN_mp_init_set_TCL_DECLARED
/* 24 */
EXTERN int		TclBN_mp_init_set(mp_int *a, mp_digit b);
#endif
#ifndef TclBN_mp_init_size_TCL_DECLARED
#define TclBN_mp_init_size_TCL_DECLARED
/* 25 */
EXTERN int		TclBN_mp_init_size(mp_int *a, int size);
#endif
#ifndef TclBN_mp_lshd_TCL_DECLARED
#define TclBN_mp_lshd_TCL_DECLARED
/* 26 */
EXTERN int		TclBN_mp_lshd(mp_int *a, int shift);
#endif
#ifndef TclBN_mp_mod_TCL_DECLARED
#define TclBN_mp_mod_TCL_DECLARED
/* 27 */
EXTERN int		TclBN_mp_mod(mp_int *a, mp_int *b, mp_int *r);
#endif
#ifndef TclBN_mp_mod_2d_TCL_DECLARED
#define TclBN_mp_mod_2d_TCL_DECLARED
/* 28 */
EXTERN int		TclBN_mp_mod_2d(mp_int *a, int b, mp_int *r);
#endif
#ifndef TclBN_mp_mul_TCL_DECLARED
#define TclBN_mp_mul_TCL_DECLARED
/* 29 */
EXTERN int		TclBN_mp_mul(mp_int *a, mp_int *b, mp_int *p);
#endif
#ifndef TclBN_mp_mul_d_TCL_DECLARED
#define TclBN_mp_mul_d_TCL_DECLARED
/* 30 */
EXTERN int		TclBN_mp_mul_d(mp_int *a, mp_digit b, mp_int *p);
#endif
#ifndef TclBN_mp_mul_2_TCL_DECLARED
#define TclBN_mp_mul_2_TCL_DECLARED
/* 31 */
EXTERN int		TclBN_mp_mul_2(mp_int *a, mp_int *p);
#endif
#ifndef TclBN_mp_mul_2d_TCL_DECLARED
#define TclBN_mp_mul_2d_TCL_DECLARED
/* 32 */
EXTERN int		TclBN_mp_mul_2d(mp_int *a, int d, mp_int *p);
#endif
#ifndef TclBN_mp_neg_TCL_DECLARED
#define TclBN_mp_neg_TCL_DECLARED
/* 33 */
EXTERN int		TclBN_mp_neg(mp_int *a, mp_int *b);
#endif
#ifndef TclBN_mp_or_TCL_DECLARED
#define TclBN_mp_or_TCL_DECLARED
/* 34 */
EXTERN int		TclBN_mp_or(mp_int *a, mp_int *b, mp_int *c);
#endif
#ifndef TclBN_mp_radix_size_TCL_DECLARED
#define TclBN_mp_radix_size_TCL_DECLARED
/* 35 */
EXTERN int		TclBN_mp_radix_size(mp_int *a, int radix, int *size);
#endif
#ifndef TclBN_mp_read_radix_TCL_DECLARED
#define TclBN_mp_read_radix_TCL_DECLARED
/* 36 */
EXTERN int		TclBN_mp_read_radix(mp_int *a, const char *str,
				int radix);
#endif
#ifndef TclBN_mp_rshd_TCL_DECLARED
#define TclBN_mp_rshd_TCL_DECLARED
/* 37 */
EXTERN void		TclBN_mp_rshd(mp_int *a, int shift);
#endif
#ifndef TclBN_mp_shrink_TCL_DECLARED
#define TclBN_mp_shrink_TCL_DECLARED
/* 38 */
EXTERN int		TclBN_mp_shrink(mp_int *a);
#endif
#ifndef TclBN_mp_set_TCL_DECLARED
#define TclBN_mp_set_TCL_DECLARED
/* 39 */
EXTERN void		TclBN_mp_set(mp_int *a, mp_digit b);
#endif
#ifndef TclBN_mp_sqr_TCL_DECLARED
#define TclBN_mp_sqr_TCL_DECLARED
/* 40 */
EXTERN int		TclBN_mp_sqr(mp_int *a, mp_int *b);
#endif
#ifndef TclBN_mp_sqrt_TCL_DECLARED
#define TclBN_mp_sqrt_TCL_DECLARED
/* 41 */
EXTERN int		TclBN_mp_sqrt(mp_int *a, mp_int *b);
#endif
#ifndef TclBN_mp_sub_TCL_DECLARED
#define TclBN_mp_sub_TCL_DECLARED
/* 42 */
EXTERN int		TclBN_mp_sub(mp_int *a, mp_int *b, mp_int *c);
#endif
#ifndef TclBN_mp_sub_d_TCL_DECLARED
#define TclBN_mp_sub_d_TCL_DECLARED
/* 43 */
EXTERN int		TclBN_mp_sub_d(mp_int *a, mp_digit b, mp_int *c);
#endif
#ifndef TclBN_mp_to_unsigned_bin_TCL_DECLARED
#define TclBN_mp_to_unsigned_bin_TCL_DECLARED
/* 44 */
EXTERN int		TclBN_mp_to_unsigned_bin(mp_int *a, unsigned char *b);
#endif
#ifndef TclBN_mp_to_unsigned_bin_n_TCL_DECLARED
#define TclBN_mp_to_unsigned_bin_n_TCL_DECLARED
/* 45 */
EXTERN int		TclBN_mp_to_unsigned_bin_n(mp_int *a,
				unsigned char *b, unsigned long *outlen);
#endif
#ifndef TclBN_mp_toradix_n_TCL_DECLARED
#define TclBN_mp_toradix_n_TCL_DECLARED
/* 46 */
EXTERN int		TclBN_mp_toradix_n(mp_int *a, char *str, int radix,
				int maxlen);
#endif
#ifndef TclBN_mp_unsigned_bin_size_TCL_DECLARED
#define TclBN_mp_unsigned_bin_size_TCL_DECLARED
/* 47 */
EXTERN int		TclBN_mp_unsigned_bin_size(mp_int *a);
#endif
#ifndef TclBN_mp_xor_TCL_DECLARED
#define TclBN_mp_xor_TCL_DECLARED
/* 48 */
EXTERN int		TclBN_mp_xor(mp_int *a, mp_int *b, mp_int *c);
#endif
#ifndef TclBN_mp_zero_TCL_DECLARED
#define TclBN_mp_zero_TCL_DECLARED
/* 49 */
EXTERN void		TclBN_mp_zero(mp_int *a);
#endif
#ifndef TclBN_reverse_TCL_DECLARED
#define TclBN_reverse_TCL_DECLARED
/* 50 */
EXTERN void		TclBN_reverse(unsigned char *s, int len);
#endif
#ifndef TclBN_fast_s_mp_mul_digs_TCL_DECLARED
#define TclBN_fast_s_mp_mul_digs_TCL_DECLARED
/* 51 */
EXTERN int		TclBN_fast_s_mp_mul_digs(mp_int *a, mp_int *b,
				mp_int *c, int digs);
#endif
#ifndef TclBN_fast_s_mp_sqr_TCL_DECLARED
#define TclBN_fast_s_mp_sqr_TCL_DECLARED
/* 52 */
EXTERN int		TclBN_fast_s_mp_sqr(mp_int *a, mp_int *b);
#endif
#ifndef TclBN_mp_karatsuba_mul_TCL_DECLARED
#define TclBN_mp_karatsuba_mul_TCL_DECLARED
/* 53 */
EXTERN int		TclBN_mp_karatsuba_mul(mp_int *a, mp_int *b,
				mp_int *c);
#endif
#ifndef TclBN_mp_karatsuba_sqr_TCL_DECLARED
#define TclBN_mp_karatsuba_sqr_TCL_DECLARED
/* 54 */
EXTERN int		TclBN_mp_karatsuba_sqr(mp_int *a, mp_int *b);
#endif
#ifndef TclBN_mp_toom_mul_TCL_DECLARED
#define TclBN_mp_toom_mul_TCL_DECLARED
/* 55 */
EXTERN int		TclBN_mp_toom_mul(mp_int *a, mp_int *b, mp_int *c);
#endif
#ifndef TclBN_mp_toom_sqr_TCL_DECLARED
#define TclBN_mp_toom_sqr_TCL_DECLARED
/* 56 */
EXTERN int		TclBN_mp_toom_sqr(mp_int *a, mp_int *b);
#endif
#ifndef TclBN_s_mp_add_TCL_DECLARED
#define TclBN_s_mp_add_TCL_DECLARED
/* 57 */
EXTERN int		TclBN_s_mp_add(mp_int *a, mp_int *b, mp_int *c);
#endif
#ifndef TclBN_s_mp_mul_digs_TCL_DECLARED
#define TclBN_s_mp_mul_digs_TCL_DECLARED
/* 58 */
EXTERN int		TclBN_s_mp_mul_digs(mp_int *a, mp_int *b, mp_int *c,
				int digs);
#endif
#ifndef TclBN_s_mp_sqr_TCL_DECLARED
#define TclBN_s_mp_sqr_TCL_DECLARED
/* 59 */
EXTERN int		TclBN_s_mp_sqr(mp_int *a, mp_int *b);
#endif
#ifndef TclBN_s_mp_sub_TCL_DECLARED
#define TclBN_s_mp_sub_TCL_DECLARED
/* 60 */
EXTERN int		TclBN_s_mp_sub(mp_int *a, mp_int *b, mp_int *c);
#endif

typedef struct TclTomMathStubs {
    int magic;
    const struct TclTomMathStubHooks *hooks;

    int (*tclBN_epoch) (void); /* 0 */
    int (*tclBN_revision) (void); /* 1 */
    int (*tclBN_mp_add) (mp_int *a, mp_int *b, mp_int *c); /* 2 */
    int (*tclBN_mp_add_d) (mp_int *a, mp_digit b, mp_int *c); /* 3 */
    int (*tclBN_mp_and) (mp_int *a, mp_int *b, mp_int *c); /* 4 */
    void (*tclBN_mp_clamp) (mp_int *a); /* 5 */
    void (*tclBN_mp_clear) (mp_int *a); /* 6 */
    void (*tclBN_mp_clear_multi) (mp_int *a, ...); /* 7 */
    int (*tclBN_mp_cmp) (mp_int *a, mp_int *b); /* 8 */
    int (*tclBN_mp_cmp_d) (mp_int *a, mp_digit b); /* 9 */
    int (*tclBN_mp_cmp_mag) (mp_int *a, mp_int *b); /* 10 */
    int (*tclBN_mp_copy) (mp_int *a, mp_int *b); /* 11 */
    int (*tclBN_mp_count_bits) (mp_int *a); /* 12 */
    int (*tclBN_mp_div) (mp_int *a, mp_int *b, mp_int *q, mp_int *r); /* 13 */
    int (*tclBN_mp_div_d) (mp_int *a, mp_digit b, mp_int *q, mp_digit *r); /* 14 */
    int (*tclBN_mp_div_2) (mp_int *a, mp_int *q); /* 15 */
    int (*tclBN_mp_div_2d) (mp_int *a, int b, mp_int *q, mp_int *r); /* 16 */
    int (*tclBN_mp_div_3) (mp_int *a, mp_int *q, mp_digit *r); /* 17 */
    void (*tclBN_mp_exch) (mp_int *a, mp_int *b); /* 18 */
    int (*tclBN_mp_expt_d) (mp_int *a, mp_digit b, mp_int *c); /* 19 */
    int (*tclBN_mp_grow) (mp_int *a, int size); /* 20 */
    int (*tclBN_mp_init) (mp_int *a); /* 21 */
    int (*tclBN_mp_init_copy) (mp_int *a, mp_int *b); /* 22 */
    int (*tclBN_mp_init_multi) (mp_int *a, ...); /* 23 */
    int (*tclBN_mp_init_set) (mp_int *a, mp_digit b); /* 24 */
    int (*tclBN_mp_init_size) (mp_int *a, int size); /* 25 */
    int (*tclBN_mp_lshd) (mp_int *a, int shift); /* 26 */
    int (*tclBN_mp_mod) (mp_int *a, mp_int *b, mp_int *r); /* 27 */
    int (*tclBN_mp_mod_2d) (mp_int *a, int b, mp_int *r); /* 28 */
    int (*tclBN_mp_mul) (mp_int *a, mp_int *b, mp_int *p); /* 29 */
    int (*tclBN_mp_mul_d) (mp_int *a, mp_digit b, mp_int *p); /* 30 */
    int (*tclBN_mp_mul_2) (mp_int *a, mp_int *p); /* 31 */
    int (*tclBN_mp_mul_2d) (mp_int *a, int d, mp_int *p); /* 32 */
    int (*tclBN_mp_neg) (mp_int *a, mp_int *b); /* 33 */
    int (*tclBN_mp_or) (mp_int *a, mp_int *b, mp_int *c); /* 34 */
    int (*tclBN_mp_radix_size) (mp_int *a, int radix, int *size); /* 35 */
    int (*tclBN_mp_read_radix) (mp_int *a, const char *str, int radix); /* 36 */
    void (*tclBN_mp_rshd) (mp_int *a, int shift); /* 37 */
    int (*tclBN_mp_shrink) (mp_int *a); /* 38 */
    void (*tclBN_mp_set) (mp_int *a, mp_digit b); /* 39 */
    int (*tclBN_mp_sqr) (mp_int *a, mp_int *b); /* 40 */
    int (*tclBN_mp_sqrt) (mp_int *a, mp_int *b); /* 41 */
    int (*tclBN_mp_sub) (mp_int *a, mp_int *b, mp_int *c); /* 42 */
    int (*tclBN_mp_sub_d) (mp_int *a, mp_digit b, mp_int *c); /* 43 */
    int (*tclBN_mp_to_unsigned_bin) (mp_int *a, unsigned char *b); /* 44 */
    int (*tclBN_mp_to_unsigned_bin_n) (mp_int *a, unsigned char *b, unsigned long *outlen); /* 45 */
    int (*tclBN_mp_toradix_n) (mp_int *a, char *str, int radix, int maxlen); /* 46 */
    int (*tclBN_mp_unsigned_bin_size) (mp_int *a); /* 47 */
    int (*tclBN_mp_xor) (mp_int *a, mp_int *b, mp_int *c); /* 48 */
    void (*tclBN_mp_zero) (mp_int *a); /* 49 */
    void (*tclBN_reverse) (unsigned char *s, int len); /* 50 */
    int (*tclBN_fast_s_mp_mul_digs) (mp_int *a, mp_int *b, mp_int *c, int digs); /* 51 */
    int (*tclBN_fast_s_mp_sqr) (mp_int *a, mp_int *b); /* 52 */
    int (*tclBN_mp_karatsuba_mul) (mp_int *a, mp_int *b, mp_int *c); /* 53 */
    int (*tclBN_mp_karatsuba_sqr) (mp_int *a, mp_int *b); /* 54 */
    int (*tclBN_mp_toom_mul) (mp_int *a, mp_int *b, mp_int *c); /* 55 */
    int (*tclBN_mp_toom_sqr) (mp_int *a, mp_int *b); /* 56 */
    int (*tclBN_s_mp_add) (mp_int *a, mp_int *b, mp_int *c); /* 57 */
    int (*tclBN_s_mp_mul_digs) (mp_int *a, mp_int *b, mp_int *c, int digs); /* 58 */
    int (*tclBN_s_mp_sqr) (mp_int *a, mp_int *b); /* 59 */
    int (*tclBN_s_mp_sub) (mp_int *a, mp_int *b, mp_int *c); /* 60 */
} TclTomMathStubs;

#if defined(USE_TCL_STUBS) && !defined(USE_TCL_STUB_PROCS)
extern const TclTomMathStubs *tclTomMathStubsPtr;
#endif /* defined(USE_TCL_STUBS) && !defined(USE_TCL_STUB_PROCS) */

#if defined(USE_TCL_STUBS) && !defined(USE_TCL_STUB_PROCS)

/*
 * Inline function declarations:
 */

#ifndef TclBN_epoch
#define TclBN_epoch \
	(tclTomMathStubsPtr->tclBN_epoch) /* 0 */
#endif
#ifndef TclBN_revision
#define TclBN_revision \
	(tclTomMathStubsPtr->tclBN_revision) /* 1 */
#endif
#ifndef TclBN_mp_add
#define TclBN_mp_add \
	(tclTomMathStubsPtr->tclBN_mp_add) /* 2 */
#endif
#ifndef TclBN_mp_add_d
#define TclBN_mp_add_d \
	(tclTomMathStubsPtr->tclBN_mp_add_d) /* 3 */
#endif
#ifndef TclBN_mp_and
#define TclBN_mp_and \
	(tclTomMathStubsPtr->tclBN_mp_and) /* 4 */
#endif
#ifndef TclBN_mp_clamp
#define TclBN_mp_clamp \
	(tclTomMathStubsPtr->tclBN_mp_clamp) /* 5 */
#endif
#ifndef TclBN_mp_clear
#define TclBN_mp_clear \
	(tclTomMathStubsPtr->tclBN_mp_clear) /* 6 */
#endif
#ifndef TclBN_mp_clear_multi
#define TclBN_mp_clear_multi \
	(tclTomMathStubsPtr->tclBN_mp_clear_multi) /* 7 */
#endif
#ifndef TclBN_mp_cmp
#define TclBN_mp_cmp \
	(tclTomMathStubsPtr->tclBN_mp_cmp) /* 8 */
#endif
#ifndef TclBN_mp_cmp_d
#define TclBN_mp_cmp_d \
	(tclTomMathStubsPtr->tclBN_mp_cmp_d) /* 9 */
#endif
#ifndef TclBN_mp_cmp_mag
#define TclBN_mp_cmp_mag \
	(tclTomMathStubsPtr->tclBN_mp_cmp_mag) /* 10 */
#endif
#ifndef TclBN_mp_copy
#define TclBN_mp_copy \
	(tclTomMathStubsPtr->tclBN_mp_copy) /* 11 */
#endif
#ifndef TclBN_mp_count_bits
#define TclBN_mp_count_bits \
	(tclTomMathStubsPtr->tclBN_mp_count_bits) /* 12 */
#endif
#ifndef TclBN_mp_div
#define TclBN_mp_div \
	(tclTomMathStubsPtr->tclBN_mp_div) /* 13 */
#endif
#ifndef TclBN_mp_div_d
#define TclBN_mp_div_d \
	(tclTomMathStubsPtr->tclBN_mp_div_d) /* 14 */
#endif
#ifndef TclBN_mp_div_2
#define TclBN_mp_div_2 \
	(tclTomMathStubsPtr->tclBN_mp_div_2) /* 15 */
#endif
#ifndef TclBN_mp_div_2d
#define TclBN_mp_div_2d \
	(tclTomMathStubsPtr->tclBN_mp_div_2d) /* 16 */
#endif
#ifndef TclBN_mp_div_3
#define TclBN_mp_div_3 \
	(tclTomMathStubsPtr->tclBN_mp_div_3) /* 17 */
#endif
#ifndef TclBN_mp_exch
#define TclBN_mp_exch \
	(tclTomMathStubsPtr->tclBN_mp_exch) /* 18 */
#endif
#ifndef TclBN_mp_expt_d
#define TclBN_mp_expt_d \
	(tclTomMathStubsPtr->tclBN_mp_expt_d) /* 19 */
#endif
#ifndef TclBN_mp_grow
#define TclBN_mp_grow \
	(tclTomMathStubsPtr->tclBN_mp_grow) /* 20 */
#endif
#ifndef TclBN_mp_init
#define TclBN_mp_init \
	(tclTomMathStubsPtr->tclBN_mp_init) /* 21 */
#endif
#ifndef TclBN_mp_init_copy
#define TclBN_mp_init_copy \
	(tclTomMathStubsPtr->tclBN_mp_init_copy) /* 22 */
#endif
#ifndef TclBN_mp_init_multi
#define TclBN_mp_init_multi \
	(tclTomMathStubsPtr->tclBN_mp_init_multi) /* 23 */
#endif
#ifndef TclBN_mp_init_set
#define TclBN_mp_init_set \
	(tclTomMathStubsPtr->tclBN_mp_init_set) /* 24 */
#endif
#ifndef TclBN_mp_init_size
#define TclBN_mp_init_size \
	(tclTomMathStubsPtr->tclBN_mp_init_size) /* 25 */
#endif
#ifndef TclBN_mp_lshd
#define TclBN_mp_lshd \
	(tclTomMathStubsPtr->tclBN_mp_lshd) /* 26 */
#endif
#ifndef TclBN_mp_mod
#define TclBN_mp_mod \
	(tclTomMathStubsPtr->tclBN_mp_mod) /* 27 */
#endif
#ifndef TclBN_mp_mod_2d
#define TclBN_mp_mod_2d \
	(tclTomMathStubsPtr->tclBN_mp_mod_2d) /* 28 */
#endif
#ifndef TclBN_mp_mul
#define TclBN_mp_mul \
	(tclTomMathStubsPtr->tclBN_mp_mul) /* 29 */
#endif
#ifndef TclBN_mp_mul_d
#define TclBN_mp_mul_d \
	(tclTomMathStubsPtr->tclBN_mp_mul_d) /* 30 */
#endif
#ifndef TclBN_mp_mul_2
#define TclBN_mp_mul_2 \
	(tclTomMathStubsPtr->tclBN_mp_mul_2) /* 31 */
#endif
#ifndef TclBN_mp_mul_2d
#define TclBN_mp_mul_2d \
	(tclTomMathStubsPtr->tclBN_mp_mul_2d) /* 32 */
#endif
#ifndef TclBN_mp_neg
#define TclBN_mp_neg \
	(tclTomMathStubsPtr->tclBN_mp_neg) /* 33 */
#endif
#ifndef TclBN_mp_or
#define TclBN_mp_or \
	(tclTomMathStubsPtr->tclBN_mp_or) /* 34 */
#endif
#ifndef TclBN_mp_radix_size
#define TclBN_mp_radix_size \
	(tclTomMathStubsPtr->tclBN_mp_radix_size) /* 35 */
#endif
#ifndef TclBN_mp_read_radix
#define TclBN_mp_read_radix \
	(tclTomMathStubsPtr->tclBN_mp_read_radix) /* 36 */
#endif
#ifndef TclBN_mp_rshd
#define TclBN_mp_rshd \
	(tclTomMathStubsPtr->tclBN_mp_rshd) /* 37 */
#endif
#ifndef TclBN_mp_shrink
#define TclBN_mp_shrink \
	(tclTomMathStubsPtr->tclBN_mp_shrink) /* 38 */
#endif
#ifndef TclBN_mp_set
#define TclBN_mp_set \
	(tclTomMathStubsPtr->tclBN_mp_set) /* 39 */
#endif
#ifndef TclBN_mp_sqr
#define TclBN_mp_sqr \
	(tclTomMathStubsPtr->tclBN_mp_sqr) /* 40 */
#endif
#ifndef TclBN_mp_sqrt
#define TclBN_mp_sqrt \
	(tclTomMathStubsPtr->tclBN_mp_sqrt) /* 41 */
#endif
#ifndef TclBN_mp_sub
#define TclBN_mp_sub \
	(tclTomMathStubsPtr->tclBN_mp_sub) /* 42 */
#endif
#ifndef TclBN_mp_sub_d
#define TclBN_mp_sub_d \
	(tclTomMathStubsPtr->tclBN_mp_sub_d) /* 43 */
#endif
#ifndef TclBN_mp_to_unsigned_bin
#define TclBN_mp_to_unsigned_bin \
	(tclTomMathStubsPtr->tclBN_mp_to_unsigned_bin) /* 44 */
#endif
#ifndef TclBN_mp_to_unsigned_bin_n
#define TclBN_mp_to_unsigned_bin_n \
	(tclTomMathStubsPtr->tclBN_mp_to_unsigned_bin_n) /* 45 */
#endif
#ifndef TclBN_mp_toradix_n
#define TclBN_mp_toradix_n \
	(tclTomMathStubsPtr->tclBN_mp_toradix_n) /* 46 */
#endif
#ifndef TclBN_mp_unsigned_bin_size
#define TclBN_mp_unsigned_bin_size \
	(tclTomMathStubsPtr->tclBN_mp_unsigned_bin_size) /* 47 */
#endif
#ifndef TclBN_mp_xor
#define TclBN_mp_xor \
	(tclTomMathStubsPtr->tclBN_mp_xor) /* 48 */
#endif
#ifndef TclBN_mp_zero
#define TclBN_mp_zero \
	(tclTomMathStubsPtr->tclBN_mp_zero) /* 49 */
#endif
#ifndef TclBN_reverse
#define TclBN_reverse \
	(tclTomMathStubsPtr->tclBN_reverse) /* 50 */
#endif
#ifndef TclBN_fast_s_mp_mul_digs
#define TclBN_fast_s_mp_mul_digs \
	(tclTomMathStubsPtr->tclBN_fast_s_mp_mul_digs) /* 51 */
#endif
#ifndef TclBN_fast_s_mp_sqr
#define TclBN_fast_s_mp_sqr \
	(tclTomMathStubsPtr->tclBN_fast_s_mp_sqr) /* 52 */
#endif
#ifndef TclBN_mp_karatsuba_mul
#define TclBN_mp_karatsuba_mul \
	(tclTomMathStubsPtr->tclBN_mp_karatsuba_mul) /* 53 */
#endif
#ifndef TclBN_mp_karatsuba_sqr
#define TclBN_mp_karatsuba_sqr \
	(tclTomMathStubsPtr->tclBN_mp_karatsuba_sqr) /* 54 */
#endif
#ifndef TclBN_mp_toom_mul
#define TclBN_mp_toom_mul \
	(tclTomMathStubsPtr->tclBN_mp_toom_mul) /* 55 */
#endif
#ifndef TclBN_mp_toom_sqr
#define TclBN_mp_toom_sqr \
	(tclTomMathStubsPtr->tclBN_mp_toom_sqr) /* 56 */
#endif
#ifndef TclBN_s_mp_add
#define TclBN_s_mp_add \
	(tclTomMathStubsPtr->tclBN_s_mp_add) /* 57 */
#endif
#ifndef TclBN_s_mp_mul_digs
#define TclBN_s_mp_mul_digs \
	(tclTomMathStubsPtr->tclBN_s_mp_mul_digs) /* 58 */
#endif
#ifndef TclBN_s_mp_sqr
#define TclBN_s_mp_sqr \
	(tclTomMathStubsPtr->tclBN_s_mp_sqr) /* 59 */
#endif
#ifndef TclBN_s_mp_sub
#define TclBN_s_mp_sub \
	(tclTomMathStubsPtr->tclBN_s_mp_sub) /* 60 */
#endif

#endif /* defined(USE_TCL_STUBS) && !defined(USE_TCL_STUB_PROCS) */

/* !END!: Do not edit above this line. */

#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLIMPORT

#endif /* _TCLINTDECLS */
