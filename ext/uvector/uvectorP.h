/*
 * uvectorP.h - internal macros for the uniform vector module
 *
 *  Copyright(C) 2001-2002 by Shiro Kawai (shiro@acm.org)
 *
 *  Permission to use, copy, modify, distribute this software and
 *  accompanying documentation for any purpose is hereby granted,
 *  provided that existing copyright notices are retained in all
 *  copies and that this notice is included verbatim in all
 *  distributions.
 *  This software is provided as is, without express or implied
 *  warranty.  In no circumstances the author(s) shall be liable
 *  for any damages arising out of the use of this software.
 *
 *  $Id: uvectorP.h,v 1.8 2002-06-20 02:57:06 shirok Exp $
 */

#ifndef GAUCHE_UVECTOR_P_H
#define GAUCHE_UVECTOR_P_H

#include <float.h>

/* For each vector type VV, the following macro should be defined.
 * VVELTTYPE   - C type for elements of the vector
 * VVMAX       - Maximum value of the element, in VVELTTYPE
 * VVMIN       - Minimum value of the element, in VVELTTYPE
 * VVBOX(ScmObj obj, VVELTTYPE elt) - box given element and set it
 *               to obj.
 * VVUNBOX(VVELTTYPE elt, ScmObj obj, int clamp) - unbox the given
 *               ScmObj and set it to elt.  Check the range of obj.
 *               If it is out of range, deal with it according to clamp.
 * VVELTPRINT(ScmPort *out, VVELTTYPE elt) - print the element.
 * VVELTEQ(VVELTTYPE x, VVELTTYPE y) - compare two elements.
 *
 * VVADD(VVELTTYPE dst, VVELTTYPE val0, VVELTTYPE val1, int clamp)
 * VVADDOBJ(VVELTTYPE dst, VVELTTYPE val0, ScmObj val1, int clamp)
 *             - dst = val0 + val1, with clamping.
 * VVSUB(VVELTTYPE dst, VVELTTYPE val0, VVELTTYPE val1, int clamp)
 * VVSUBOBJ(VVELTTYPE dst, VVELTTYPE val0, ScmObj val1, int clamp)
 *             - dst = val0 - val1, with clamping.
 * VVMUL(VVELTTYPE dst, VVELTTYPE val0, VVELTTYPE val1, int clamp)
 * VVMULOBJ(VVELTTYPE dst, VVELTTYPE val0, ScmObj val1, int clamp)
 *             - dst = val0 * val1, with clamping.
 * VVDIV(VVELTTYPE dst, VVELTTYPE val0, VVELTTYPE val1)
 * VVDIVOBJ(VVELTTYPE dst, VVELTTYPE val0, ScmObj val1)
 *             - dst = val0 / val1, with clamping.
 *               This is only used in f32 and f64 vector.
 * VVAND(VVELTTYPE dst, VVELTTYPE val0, VVELTTYPE val1)
 * VVIOR(VVELTTYPE dst, VVELTTYPE val0, VVELTTYPE val1)
 * VVXOR(VVELTTYPE dst, VVELTTYPE val0, VVELTTYPE val1)
 *             - This is only used in integer vectors.
 */

/*
 * Some common macros
 */

#define BADOBJ(obj)      Scm_Error("bad type of object: %S", obj)
#define TOOSMALLOBJ(obj) Scm_Error("value too small: %S", obj)
#define TOOLARGEOBJ(obj) Scm_Error("value too large: %S", obj)
#define OVERFLOW         Scm_Error("vector arithmetic overflow")

#ifdef HAVE_ISINF
#define ISINF(x)  isinf(x)
#else
#define ISINF(x)  ((x) != 0 && (x) == (x)/2.0)
#endif

#define CLAMP_HI_P(clamp)   ((clamp)&SCM_UVECTOR_CLAMP_HI)
#define CLAMP_LO_P(clamp)   ((clamp)&SCM_UVECTOR_CLAMP_LO)

/* clamp within integer range.  should be used for small numbers. */
#define CLAMP_INT(val, min, max, clamp)                 \
  do {                                                  \
    if (val < min) {                                    \
      if (CLAMP_LO_P(clamp)) val = min;                 \
      else Scm_Error("value too small: %d", val);       \
    } else if (val > max) {                             \
      if (CLAMP_HI_P(clamp)) val = max;                 \
      else Scm_Error("value too large: %d", val);       \
    }                                                   \
  } while (0)

/* val must be a bignum.  exclude it unconditionally.  should be used for
   small numbers. */
#define CLAMP_BIG(elt, val, min, max, clamp)    \
  do {                                          \
    if (SCM_BIGNUM_SIGN(val) < 0) {             \
      if (CLAMP_LO_P(clamp)) elt = min;         \
      else TOOSMALLOBJ(val);                    \
    } else if (SCM_BIGNUM_SIGN(val) > 0) {      \
      if (CLAMP_HI_P(clamp)) elt = max;         \
      else TOOLARGEOBJ(val);                    \
    }                                           \
  } while (0)

/* val must be float or double. */
#define CLAMP_FLT(val, min, max, clamp)                 \
  do {                                                  \
    if (ISINF(val)) {                                   \
      if (val < 0) {                                    \
        if (CLAMP_LO_P(clamp)) val = min;               \
        else Scm_Error("value too small: -infinity");   \
      } else {                                          \
        if (CLAMP_HI_P(clamp)) val = max;               \
        else Scm_Error("value too small: +infinity");   \
      }                                                 \
    }                                                   \
  } while (0)

/* small integer binop with clamping */
#define SMALL_BINOP_CLAMP(dst, v0, v1, op, min, max, clamp)     \
  do {                                                          \
    long V__ = v0 op v1;                                        \
    CLAMP_INT(V__, min, max, clamp);                            \
    dst = V__;                                                  \
  } while (0)

/* small integer binop without clamping */
#define SMALL_BINOP(dst, v0, v1, op)            \
  do { dst = v0 op v1; } while (0)

/* bignum binop with clamping */
#define BIG_BINOP_CLAMP(dst, v0, v1, op, min, max, clamp)       \
  do {                                                          \
    dst = op(v0, v1);                                           \
    if (Scm_NumCmp(dst, min) < 0) {                             \
        if (CLAMP_LO_P(clamp)) dst = min; else OVERFLOW;        \
    } else if (Scm_NumCmp(dst, max) > 0) {                      \
        if (CLAMP_HI_P(clamp)) dst = max; else OVERFLOW;        \
    }                                                           \
  } while (0)

#define BIG_BINOP(dst, v0, v1, op)  dst = op(v0, v1)

/*
 * S8Vector
 */
#define S8ELTTYPE  signed char
#define S8MIN      -128
#define S8MAX       127
#define S8BOX(obj, elt)    (obj) = SCM_MAKE_INT(elt)
#define S8UNBOX(elt, obj, clamp)                                        \
  do {                                                                  \
    if (SCM_BIGNUMP(obj)) CLAMP_BIG(elt, obj, S8MIN, S8MAX, clamp);     \
    else if (!SCM_INTP(obj)) BADOBJ(obj);                               \
    else {                                                              \
      int V__ = SCM_INT_VALUE(obj);                                     \
      CLAMP_INT(V__, S8MIN, S8MAX, clamp);                              \
      elt = (S8ELTTYPE)V__;                                             \
    }                                                                   \
  } while (0)
#define S8ELTPRINT(out, elt)    Scm_Printf(out, "%d", elt)
#define S8ELTEQ(x, y)           ((x)==(y))

#define S8ADD(dst, x, y, clamp) \
  SMALL_BINOP_CLAMP(dst, x, y, +, S8MIN, S8MAX, clamp)
#define S8SUB(dst, x, y, clamp) \
  SMALL_BINOP_CLAMP(dst, x, y, -, S8MIN, S8MAX, clamp)
#define S8MUL(dst, x, y, clamp) \
  SMALL_BINOP_CLAMP(dst, x, y, *, S8MIN, S8MAX, clamp)
#define S8AND(dst, x, y) SMALL_BINOP(dst, x, y, &)
#define S8IOR(dst, x, y) SMALL_BINOP(dst, x, y, |)
#define S8XOR(dst, x, y) SMALL_BINOP(dst, x, y, ^)

#define S8ADDOBJ(dst, x, y, clamp) \
  dst = saddobj_small(x, y, S8MIN, S8MAX, clamp)
#define S8SUBOBJ(dst, x, y, clamp) \
  dst = ssubobj_small(x, y, S8MIN, S8MAX, clamp)
#define S8MULOBJ(dst, x, y, clamp) \
  dst = smulobj_small(x, y, S8MIN, S8MAX, clamp)

/*
 * U8Vector
 */
#define U8ELTTYPE unsigned char
#define U8MIN     0
#define U8MAX     255
#define U8BOX(obj, elt)    (obj) = SCM_MAKE_INT(elt)
#define U8UNBOX(elt, obj, clamp)                                        \
  do {                                                                  \
    if (SCM_BIGNUMP(obj)) CLAMP_BIG(elt, obj, U8MIN, U8MAX, clamp);     \
    else if (!SCM_INTP(obj)) BADOBJ(obj);                               \
    else {                                                              \
      int V__ = SCM_INT_VALUE(obj);                                     \
      CLAMP_INT(V__, U8MIN, U8MAX, clamp);                              \
      elt = (U8ELTTYPE)V__;                                             \
    }                                                                   \
  } while (0)
#define U8ELTPRINT(out, elt)    Scm_Printf(out, "%u", elt)
#define U8ELTEQ(x, y)           ((x)==(y))

#define U8ADD(dst, x, y, clamp) \
  SMALL_BINOP_CLAMP(dst, x, y, +, U8MIN, U8MAX, clamp)
#define U8SUB(dst, x, y, clamp) \
  SMALL_BINOP_CLAMP(dst, x, y, -, U8MIN, U8MAX, clamp)
#define U8MUL(dst, x, y, clamp) \
  SMALL_BINOP_CLAMP(dst, x, y, *, U8MIN, U8MAX, clamp)
#define U8AND(dst, x, y) SMALL_BINOP(dst, x, y, &)
#define U8IOR(dst, x, y) SMALL_BINOP(dst, x, y, |)
#define U8XOR(dst, x, y) SMALL_BINOP(dst, x, y, ^)

#define U8ADDOBJ(dst, x, y, clamp) \
  dst = uaddobj_small(x, y, U8MIN, U8MAX, clamp)
#define U8SUBOBJ(dst, x, y, clamp) \
  dst = usubobj_small(x, y, U8MIN, U8MAX, clamp)
#define U8MULOBJ(dst, x, y, clamp) \
  dst = umulobj_small(x, y, U8MIN, U8MAX, clamp)

/*
 * S16Vector
 */
#define S16ELTTYPE  signed short
#define S16MIN      -32768
#define S16MAX       32767
#define S16BOX(obj, elt)    (obj) = SCM_MAKE_INT(elt)
#define S16UNBOX(elt, obj, clamp)                                       \
  do {                                                                  \
    if (SCM_BIGNUMP(obj)) CLAMP_BIG(elt, obj, S16MIN, S16MAX, clamp);   \
    else if (!SCM_INTP(obj)) BADOBJ(obj);                               \
    else {                                                              \
      int V__ = SCM_INT_VALUE(obj);                                     \
      CLAMP_INT(V__, S16MIN, S16MAX, clamp);                            \
      elt = (S16ELTTYPE)V__;                                            \
    }                                                                   \
  } while (0)
#define S16ELTPRINT(out, elt)    Scm_Printf(out, "%d", elt)
#define S16ELTEQ(x, y)           ((x)==(y))

#define S16ADD(dst, x, y, clamp) \
  SMALL_BINOP_CLAMP(dst, x, y, +, S16MIN, S16MAX, clamp)
#define S16SUB(dst, x, y, clamp) \
  SMALL_BINOP_CLAMP(dst, x, y, -, S16MIN, S16MAX, clamp)
#define S16MUL(dst, x, y, clamp) \
  SMALL_BINOP_CLAMP(dst, x, y, *, S16MIN, S16MAX, clamp)
#define S16AND(dst, x, y) SMALL_BINOP(dst, x, y, &)
#define S16IOR(dst, x, y) SMALL_BINOP(dst, x, y, |)
#define S16XOR(dst, x, y) SMALL_BINOP(dst, x, y, ^)

#define S16ADDOBJ(dst, x, y, clamp) \
  dst = saddobj_small(x, y, S16MIN, S16MAX, clamp)
#define S16SUBOBJ(dst, x, y, clamp) \
  dst = ssubobj_small(x, y, S16MIN, S16MAX, clamp)
#define S16MULOBJ(dst, x, y, clamp) \
  dst = smulobj_small(x, y, S16MIN, S16MAX, clamp)

/*
 * U16Vector
 */
#define U16ELTTYPE unsigned short
#define U16MIN     0
#define U16MAX     65535
#define U16BOX(obj, elt)    (obj) = SCM_MAKE_INT(elt)
#define U16UNBOX(elt, obj, clamp)                                       \
  do {                                                                  \
    if (SCM_BIGNUMP(obj)) CLAMP_BIG(elt, obj, U16MIN, U16MAX, clamp);   \
    else if (!SCM_INTP(obj)) BADOBJ(obj);                               \
    else {                                                              \
      int V__ = SCM_INT_VALUE(obj);                                     \
      CLAMP_INT(V__, U16MIN, U16MAX, clamp);                            \
      elt = (U16ELTTYPE)V__;                                            \
    }                                                                   \
  } while (0)
#define U16ELTPRINT(out, elt)    Scm_Printf(out, "%u", elt)
#define U16ELTEQ(x, y)           ((x)==(y))

#define U16ADD(dst, x, y, clamp) \
  SMALL_BINOP_CLAMP(dst, x, y, +, U16MIN, U16MAX, clamp)
#define U16SUB(dst, x, y, clamp) \
  SMALL_BINOP_CLAMP(dst, x, y, -, U16MIN, U16MAX, clamp)
#define U16MUL(dst, x, y, clamp) \
  SMALL_BINOP_CLAMP(dst, x, y, *, U16MIN, U16MAX, clamp)
#define U16AND(dst, x, y) SMALL_BINOP(dst, x, y, &)
#define U16IOR(dst, x, y) SMALL_BINOP(dst, x, y, |)
#define U16XOR(dst, x, y) SMALL_BINOP(dst, x, y, ^)

#define U16ADDOBJ(dst, x, y, clamp) \
  dst = uaddobj_small(x, y, U16MIN, U16MAX, clamp)
#define U16SUBOBJ(dst, x, y, clamp) \
  dst = usubobj_small(x, y, U16MIN, U16MAX, clamp)
#define U16MULOBJ(dst, x, y, clamp) \
  dst = umulobj_small(x, y, U16MIN, U16MAX, clamp)

/*
 * S32Vector
 */
#define S32ELTTYPE SCM_UVECTOR_INT32
#define S32MAX     2147483647L
#define S32MIN     (-(S32MAX)-1)
#define S32BOX(obj, elt)    (obj) = Scm_MakeInteger(elt)
#if SIZEOF_LONG == 4
/* 32bit architecture */
#define S32UNBOX(elt, obj, clamp)                                       \
  do {                                                                  \
    if (SCM_INTP(obj)) {                                                \
      (elt) = SCM_INT_VALUE(obj);                                       \
    } else if (SCM_BIGNUMP(obj)) {                                      \
      if (Scm_NumCmp(obj, Scm_UvectorS32Min) < 0) {                     \
        if (CLAMP_LO_P(clamp)) (elt) = S32MIN;                          \
        else TOOSMALLOBJ(obj);                                          \
      } else if (Scm_NumCmp(obj, Scm_UvectorS32Max) > 0) {              \
        if (CLAMP_HI_P(clamp)) (elt) = S32MAX;                          \
        else TOOLARGEOBJ(obj);                                          \
      } else {                                                          \
        (elt) = (SCM_UVECTOR_INT32)Scm_BignumToSI(SCM_BIGNUM(obj));     \
      }                                                                 \
    } else BADOBJ(obj);                                                 \
  } while (0)
#define S32ADD(dst, x, y, clamp) dst = sadd(x, y, clamp)
#define S32SUB(dst, x, y, clamp) dst = ssub(x, y, clamp)
#define S32MUL(dst, x, y, clamp) dst = smul(x, y, clamp)
#else  /* SIZEOF_LONG >= 8 */
#define S32UNBOX(elt, obj, clamp)                                       \
  do {                                                                  \
    if (SCM_INTP(obj)) {                                                \
      long V__ = SCM_INT_VALUE(obj);                                    \
      CLAMP_INT(V__, S32MIN, S32MAX, clamp);                            \
      (elt) = (SCM_UVECTOR_INT32)V__;                                   \
    } else if (SCM_BIGNUMP(obj)) {                                      \
      if (Scm_NumCmp(obj, Scm_UvectorS32Min) < 0) {                     \
        if (CLAMP_LO_P(clamp)) (elt) = S32MIN;                          \
        else TOOSMALLOBJ(obj);                                          \
      } else if (Scm_NumCmp(obj, Scm_UvectorS32Max) > 0) {              \
        if (CLAMP_HI_P(clamp)) (elt) = S32MAX;                          \
        else TOOLARGEOBJ(obj);                                          \
      } else {                                                          \
        (elt) = (SCM_UVECTOR_INT32)Scm_BignumToSI(SCM_BIGNUM(obj));     \
      }                                                                 \
    } else BADOBJ(obj);                                                 \
  } while (0)
#define S32ADD(dst, x, y, clamp) \
  SMALL_BINOP_CLAMP(dst, x, y, +, S32MIN, S32MAX, clamp)
#define S32SUB(dst, x, y, clamp) \
  SMALL_BINOP_CLAMP(dst, x, y, -, S32MIN, S32MAX, clamp)
#define S32MUL(dst, x, y, clamp) \
  SMALL_BINOP_CLAMP(dst, x, y, *, S32MIN, S32MAX, clamp)
#endif /* SIZEOF_LONG >= 8 */
#define S32ELTPRINT(out, elt)  Scm_Printf(out, "%d", elt)
#define S32ELTEQ(x, y)         ((x)==(y))
#define S32AND(dst, x, y) SMALL_BINOP(dst, x, y, &)
#define S32IOR(dst, x, y) SMALL_BINOP(dst, x, y, |)
#define S32XOR(dst, x, y) SMALL_BINOP(dst, x, y, ^)

#define S32ADDOBJ(dst, x, y, clamp) \
  dst = saddobj(x, y, Scm_UvectorS32Min, Scm_UvectorS32Max, clamp)
#define S32SUBOBJ(dst, x, y, clamp) \
  dst = ssubobj(x, y, Scm_UvectorS32Min, Scm_UvectorS32Max, clamp)
#define S32MULOBJ(dst, x, y, clamp) \
  dst = smulobj(x, y, Scm_UvectorS32Min, Scm_UvectorS32Max, clamp)

/*
 * U32Vector
 */
#define U32ELTTYPE SCM_UVECTOR_UINT32
#define U32MIN     0
#define U32MAX     4294967295UL
#define U32BOX(obj, elt)    (obj) = Scm_MakeIntegerFromUI(elt)
#if SIZEOF_LONG == 4
/* 32bit architecture */
#define U32UNBOX(elt, obj, clamp)                                       \
  do {                                                                  \
    if (SCM_INTP(obj)) {                                                \
      if (SCM_INT_VALUE(obj) < 0) {                                     \
        if (CLAMP_LO_P(clamp)) (elt) = U32MIN;                          \
        else TOOSMALLOBJ(obj);                                          \
      } else {                                                          \
        (elt) = SCM_INT_VALUE(obj);                                     \
      }                                                                 \
    } else if (SCM_BIGNUMP(obj)) {                                      \
      if (Scm_NumCmp(obj, Scm_UvectorU32Min) < 0) {                     \
        if (CLAMP_LO_P(clamp)) (elt) = U32MIN;                          \
        else TOOSMALLOBJ(obj);                                          \
      } else if (Scm_NumCmp(obj, Scm_UvectorU32Max) > 0) {              \
        if (CLAMP_HI_P(clamp)) (elt) = U32MAX;                          \
        else TOOLARGEOBJ(obj);                                          \
      } else {                                                          \
        (elt) = (SCM_UVECTOR_UINT32)Scm_BignumToUI(SCM_BIGNUM(obj));    \
      }                                                                 \
    } else BADOBJ(obj);                                                 \
  } while (0)
#define U32ADD(dst, x, y, clamp) dst = uadd(x, y, clamp)
#define U32SUB(dst, x, y, clamp) dst = usub(x, y, clamp)
#define U32MUL(dst, x, y, clamp) dst = umul(x, y, CLAMP_HI_P(clamp))
#else  /* SIZEOF_LONG >= 8 */
#define U32UNBOX(elt, obj, clamp)                                       \
  do {                                                                  \
    if (SCM_INTP(obj)) {                                                \
      long V__ = SCM_INT_VALUE(obj);                                    \
      CLAMP_INT(V__, U32MIN, U32MAX, clamp);                            \
      (elt) = (SCM_UVECTOR_UINT32)V__;                                  \
    } else if (SCM_BIGNUMP(obj)) {                                      \
      if (Scm_NumCmp(obj, Scm_UvectorU32Min) < 0) {                     \
        if (CLAMP_LO_P(clamp)) (elt) = U32MIN;                          \
        else TOOSMALLOBJ(obj);                                          \
      } else if (Scm_NumCmp(obj, Scm_UvectorU32Max) > 0) {              \
        if (CLAMP_HI_P(clamp)) (elt) = U32MAX;                          \
        else TOOLARGEOBJ(obj);                                          \
      } else {                                                          \
        (elt) = (SCM_UVECTOR_UINT32)Scm_BignumToSI(SCM_BIGNUM(obj));    \
      }                                                                 \
    } else BADOBJ(obj);                                                 \
  } while (0)
#define U32ADD(dst, x, y, clamp) \
  SMALL_BINOP_CLAMP(dst, x, y, +, U32MIN, U32MAX, clamp)
#define U32SUB(dst, x, y, clamp) \
  SMALL_BINOP_CLAMP(dst, x, y, -, U32MIN, U32MAX, clamp)
#define U32MUL(dst, x, y, clamp) \
  SMALL_BINOP_CLAMP(dst, x, y, *, U32MIN, U32MAX, clamp)
#endif /* SIZEOF_LONG >= 8 */
#define U32ELTPRINT(out, elt)  Scm_Printf(out, "%u", elt)
#define U32ELTEQ(x, y)         ((x)==(y))
#define U32AND(dst, x, y) SMALL_BINOP(dst, x, y, &)
#define U32IOR(dst, x, y) SMALL_BINOP(dst, x, y, |)
#define U32XOR(dst, x, y) SMALL_BINOP(dst, x, y, ^)

#define U32ADDOBJ(dst, x, y, clamp) \
  dst = uaddobj(x, y, Scm_UvectorU32Min, Scm_UvectorU32Max, clamp)
#define U32SUBOBJ(dst, x, y, clamp) \
  dst = usubobj(x, y, Scm_UvectorU32Min, Scm_UvectorU32Max, clamp)
#define U32MULOBJ(dst, x, y, clamp) \
  dst = umulobj(x, y, Scm_UvectorU32Min, Scm_UvectorU32Max, clamp)

/*
 * S64Vector
 */
#define S64ELTTYPE SCM_UVECTOR_INT64
#if SIZEOF_LONG == 4
#define S64MIN  Scm_UvectorS64Min
#define S64MAX  Scm_UvectorS64Max
#define S64BOX(obj, elt)  (obj) = (elt)
#define S64UNBOX(elt, obj, clamp)               \
  do {                                          \
    if (SCM_INTP(obj)) (elt) = (obj);           \
    else if (SCM_BIGNUMP(obj)) {                \
      if (Scm_NumCmp(obj, S64MIN) < 0) {        \
        if (CLAMP_LO_P(clamp)) (elt) = S64MIN;  \
        else TOOSMALLOBJ(obj);                  \
      } else if (Scm_NumCmp(obj, S64MAX) > 0) { \
        if (CLAMP_HI_P(clamp)) (elt) = S64MAX;  \
        else TOOLARGEOBJ(obj);                  \
      } else {                                  \
        (elt) = (obj);                          \
      }                                         \
    } else BADOBJ(obj);                         \
  } while (0)
#define S64ELTPRINT(out, elt)  Scm_Printf(out, "%S", elt)
#define S64ELTEQ(x, y)         Scm_NumEq(x, y)

#define S64ADD(dst, x, y, clamp)                                        \
    BIG_BINOP_CLAMP(dst, x, y, Scm_Add2,                                \
                    Scm_UvectorS64Min, Scm_UvectorS64Max, clamp)
#define S64SUB(dst, x, y, clamp)                                        \
    BIG_BINOP_CLAMP(dst, x, y, Scm_Subtract2,                           \
                    Scm_UvectorS64Min, Scm_UvectorS64Max, clamp)
#define S64MUL(dst, x, y, clamp)                                        \
    BIG_BINOP_CLAMP(dst, x, y, Scm_Multiply2,                           \
                    Scm_UvectorS64Min, Scm_UvectorS64Max, clamp)
#define S64AND(dst, x, y)  dst = Scm_LogAnd(x, y)
#define S64IOR(dst, x, y)  dst = Scm_LogIor(x, y)
#define S64XOR(dst, x, y)  dst = Scm_LogXor(x, y)

#define S64ADDOBJ(dst, x, y, clamp) S64ADD(dst, x, y, clamp)
#define S64SUBOBJ(dst, x, y, clamp) S64SUB(dst, x, y, clamp)
#define S64MULOBJ(dst, x, y, clamp) S64MUL(dst, x, y, clamp)

#else /* SIZEOF_LONG >= 8 */
#define S64MIN -9223372036854775808L
#define S64MAX  9223372036854775807L
#define S64BOX(obj, elt)  (obj) = Scm_MakeInteger(elt)
#define S64UNBOX(elt, obj, clamp) 
  do {                                                                  \
    if (SCM_INTP(obj)) {                                                \
      (elt) = SCM_INT_VALUE(obj);                                       \
    } else if (SCM_BIGNUMP(obj)) {                                      \
      if (Scm_NumCmp(obj, Scm_UvectorS64Min) < 0) {                     \
        if (CLAMP_LO_P(clamp)) (elt) = S64MIN;                          \
        else TOOSMALLOBJ(obj);                                          \
      } else if (Scm_NumCmp(obj, Scm_UvectorS64Max) > 0) {              \
        if (CLAMP_HI_P(clamp)) (elt) = S64MAX;                          \
        else TOOLARGEOBJ(obj);                                          \
      } else {                                                          \
        (elt) = (SCM_UVECTOR_INT64)Scm_BignumToSI(SCM_BIGNUM(obj));     \
      }                                                                 \
    } else BADOBJ(obj);                                                 \
  } while (0)
#define S64ELTPRINT(out, elt)  Scm_Printf(out, "%ld", elt)
#define S64ELTEQ(x, y)         ((x)==(y))

#define S64ADD(dst, x, y, clamp) dst = sadd(x, y, clamp)
#define S64SUB(dst, x, y, clamp) dst = ssub(x, y, clamp)
#define S64MUL(dst, x, y, clamp) dst = smul(x, y, clamp)
#define S64AND(dst, x, y) SMALL_BINOP(dst, x, y, &)
#define S64IOR(dst, x, y) SMALL_BINOP(dst, x, y, |)
#define S64XOR(dst, x, y) SMALL_BINOP(dst, x, y, ^)

#define S64ADDOBJ(dst, x, y, clamp) \
    dst = saddobj(x, y, Scm_UvectorS64Min, Scm_UvectorS64Max, clamp)
#define S64SUBOBJ(dst, x, y, clamp) \
    dst = ssubobj(x, y, Scm_UvectorS64Min, Scm_UvectorS64Max, clamp)
#define S64MULOBJ(dst, x, y, clamp) \
    dst = smulobj(x, y, Scm_UvectorS64Min, Scm_UvectorS64Max, clamp)
#endif /* SIZEOF_LONG >= 8 */


/*
 * U64Vector
 */
#define U64ELTTYPE SCM_UVECTOR_INT64
#if SIZEOF_LONG == 4
#define U64MIN  Scm_UvectorU64Min
#define U64MAX  Scm_UvectorU64Max
#define U64BOX(obj, elt)  (obj) = (elt)
#define U64UNBOX(elt, obj, clamp)               \
  do {                                          \
    if (SCM_INTP(obj)) {                        \
      if (SCM_INT_VALUE(obj) < 0) {             \
        if (CLAMP_LO_P(clamp)) (elt) = U64MIN;  \
        else TOOSMALLOBJ(obj);                  \
      } else {                                  \
          (elt) = (obj);                        \
      }                                         \
    } else if (SCM_BIGNUMP(obj)) {              \
      if (Scm_NumCmp(obj, U64MIN) < 0) {        \
        if (CLAMP_LO_P(clamp)) (elt) = U64MIN;  \
        else TOOSMALLOBJ(obj);                  \
      } else if (Scm_NumCmp(obj, U64MAX) > 0) { \
        if (CLAMP_HI_P(clamp)) (elt) = U64MAX;  \
        else TOOLARGEOBJ(obj);                  \
      } else {                                  \
        (elt) = (obj);                          \
      }                                         \
    } else BADOBJ(obj);                         \
  } while (0)
#define U64ELTPRINT(out, elt)  Scm_Printf(out, "%S", elt)
#define U64ELTEQ(x, y)         Scm_NumEq(x, y)

#define U64ADD(dst, x, y, clamp)                                        \
    BIG_BINOP_CLAMP(dst, x, y, Scm_Add2,                                \
                    Scm_UvectorU64Min, Scm_UvectorU64Max, clamp)
#define U64SUB(dst, x, y, clamp)                                        \
    BIG_BINOP_CLAMP(dst, x, y, Scm_Subtract2,                           \
                    Scm_UvectorU64Min, Scm_UvectorU64Max, clamp)
#define U64MUL(dst, x, y, clamp)                                        \
    BIG_BINOP_CLAMP(dst, x, y, Scm_Multiply2,                           \
                    Scm_UvectorU64Min, Scm_UvectorU64Max, clamp)
#define U64AND(dst, x, y)  dst = Scm_LogAnd(x, y)
#define U64IOR(dst, x, y)  dst = Scm_LogIor(x, y)
#define U64XOR(dst, x, y)  dst = Scm_LogXor(x, y)

#define U64ADDOBJ(dst, x, y, clamp) U64ADD(dst, x, y, clamp)
#define U64SUBOBJ(dst, x, y, clamp) U64SUB(dst, x, y, clamp)
#define U64MULOBJ(dst, x, y, clamp) U64MUL(dst, x, y, clamp)

#else /* SIZEOF_LONG >= 8 */
#define U64MIN  0UL
#define U64MAX  18446744073709551616UL
#define U64BOX(obj, elt)  (obj) = Scm_MakeIntegerFromUI(elt)
#define U64UNBOX(elt, obj, clamp)                                       \
  do {                                                                  \
    if (SCM_INTP(obj)) {                                                \
      long V__ = SCM_INT_VALUE(obj);                                    \
      if (V__ < 0) {                                                    \
        if (CLAMP_LO_P(clamp)) (elt) = U64MIN;                          \
        else TOOSMALLOBJ(obj);                                          \
      } else {                                                          \
        (elt) = V__;                                                    \
      }                                                                 \
    } else if (SCM_BIGNUMP(obj)) {                                      \
      if (Scm_NumCmp(obj, Scm_UvectorU64Min) < 0) {                     \
        if (CLAMP_LO_P(clamp)) (elt) = U64MIN;                          \
        else TOOSMALLOBJ(obj);                                          \
      } else if (Scm_NumCmp(obj, Scm_UvectorU64Max) > 0) {              \
        if (CLAMP_HI_P(clamp)) (elt) = U64MAX;                          \
        else TOOLARGEOBJ(obj);                                          \
      } else {                                                          \
        (elt) = (SCM_UVECTOR_INT64)Scm_BignumToUI(SCM_BIGNUM(obj));     \
      }                                                                 \
    } else BADOBJ(obj);                                                 \
  } while (0)
#define U64ELTPRINT(out, elt)  Scm_Printf(out, "%lu", elt)
#define U64ELTEQ(x, y)         ((x)==(y))

#define U64ADD(dst, x, y, clamp) dst = uadd(x, y, clamp)
#define U64SUB(dst, x, y, clamp) dst = usub(x, y, clamp)
#define U64MUL(dst, x, y, clamp) dst = umul(x, y, clamp)
#define U64AND(dst, x, y) SMALL_BINOP(dst, x, y, &)
#define U64IOR(dst, x, y) SMALL_BINOP(dst, x, y, |)
#define U64XOR(dst, x, y) SMALL_BINOP(dst, x, y, ^)

#define U64ADDOBJ(dst, x, y, clamp) \
    dst = saddobj(x, y, Scm_UvectorU64Min, Scm_UvectorU64Max, clamp)
#define U64SUBOBJ(dst, x, y, clamp) \
    dst = ssubobj(x, y, Scm_UvectorU64Min, Scm_UvectorU64Max, clamp)
#define U64MULOBJ(dst, x, y, clamp) \
    dst = smulobj(x, y, Scm_UvectorU64Min, Scm_UvectorU64Max, clamp)
#endif /* SIZEOF_LONG >= 8 */


/*
 * F32Vector
 */
#define F32ELTTYPE  float
#define F32MIN      FLT_MIN
#define F32MAX      FLT_MAX
#define F32BOX(obj, elt)  (obj) = Scm_MakeFlonum((double)elt)
#define F32UNBOX(elt, obj, clamp)                                           \
    do {                                                                    \
        float v;                                                            \
        if (SCM_FLONUMP(obj)) v = (float)SCM_FLONUM_VALUE(obj);             \
        else if (SCM_INTP(obj)) v = (float)SCM_INT_VALUE(obj);              \
        else if (SCM_BIGNUMP(obj)) v = Scm_BignumToDouble(SCM_BIGNUM(obj)); \
        else BADOBJ(obj);                                                   \
        CLAMP_FLT(v, F32MIN, F32MAX, clamp);                                \
        elt = v;                                                            \
    } while (0)
#define F32ELTPRINT(out, elt) Scm_Printf(out, "%f", elt)
#define F32ELTEQ(x, y)        ((x)==(y))


/*
 * F64Vector
 */
#define F64ELTTYPE  double
#define F64MIN      DBL_MIN
#define F64MAX      DBL_MAX
#define F64BOX(obj, elt)  (obj) = Scm_MakeFlonum((double)elt)
#define F64UNBOX(elt, obj, clamp)                                           \
    do {                                                                    \
        double v;                                                           \
        if (SCM_FLONUMP(obj)) v = SCM_FLONUM_VALUE(obj);                    \
        else if (SCM_INTP(obj)) v = SCM_INT_VALUE(obj);                     \
        else if (SCM_BIGNUMP(obj)) v = Scm_BignumToDouble(SCM_BIGNUM(obj)); \
        else BADOBJ(obj);                                                   \
        CLAMP_FLT(v, F64MIN, F64MAX, clamp);                                \
        elt = v;                                                            \
    } while (0)
#define F64ELTPRINT(out, elt) Scm_Printf(out, "%f", elt)
#define F64ELTEQ(x, y)        ((x)==(y))

#endif /* GAUCHE_UVECTOR_P_H */
