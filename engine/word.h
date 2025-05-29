/*
 * Asp engine word definitions.
 */

#ifndef ASP_WORD_H
#define ASP_WORD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Word constants. */
#define AspWordBitSize 28U
#define AspWordMax ((1U << (AspWordBitSize)) - 1U)
#define AspSignedWordMin (-(1 << (AspWordBitSize) - 1))
#define AspSignedWordMax ((1 << (AspWordBitSize) - 1) - 1)

#ifdef __cplusplus
}
#endif

#endif
