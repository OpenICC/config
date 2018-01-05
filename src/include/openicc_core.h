/**
 *  @file openicc_core.h
 *
 *  libOpenICC - OpenICC Colour Management Tools
 *
 *  @par Copyright:
 *            2011-2018 (C) Kai-Uwe Behrmann
 *
 *  @brief    OpenICC Colour Management core types
 *  @internal
 *  @author   Kai-Uwe Behrmann <ku.b@gmx.de>
 *  @par License:
 *            MIT <http://www.opensource.org/licenses/mit-license.php>
 *  @since    2011/10/21
 */

#ifndef __OPENICC_CORE_H__
#define __OPENICC_CORE_H__

#include <stdio.h>

typedef void * (*openiccAlloc_f)     ( size_t              size );
typedef void   (*openiccDeAlloc_f)   ( void              * data );


#if   defined(__clang__)
#define OI_FALLTHROUGH
#elif __GNUC__ >= 7 
#define OI_FALLTHROUGH                 __attribute__ ((fallthrough));
#else
#define OI_FALLTHROUGH
#endif

#if   __GNUC__ >= 7
#define OI_DEPRECATED                  __attribute__ ((deprecated))
#elif defined(_MSC_VER)
#define OI_DEPRECATED                  __declspec(deprecated)
#else
#define OI_DEPRECATED
#endif

#if   (__GNUC__*100 + __GNUC_MINOR__) >= 406
#define OI_UNUSED                      __attribute__ ((unused))
#elif defined(_MSC_VER)
#define OI_UNUSED                      __declspec(unused)
#else
#define OI_UNUSED
#endif

#endif /* __OPENICC_CORE_H__ */
