/*
 * Copyright (c) 2020, Lawrence Livermore National Security, LLC.
 * Produced at the Lawrence Livermore National Laboratory.
 * Copyright (c) 2020, Florida State University. Contributions from
 * the Computer Architecture and Systems Research Laboratory (CASTL)
 * at the Department of Computer Science.
 *
 * LLNL-CODE-805021. All rights reserved.
 * 
 * This is the license for Direct-FUSE.
 * For details, see https://github.com/llnl/direct-fuse
 * Please read https://github.com/llnl/direct-fuse/LICENSE for full license text.
 */


#if defined(HAVE_ASM_WEAK_DIRECTIVE) || defined(HAVE_ASM_WEAKEXT_DIRECTIVE)
#define HAVE_WEAK_SYMBOLS
#endif

#define STRINGOF(x) #x

/*
 * Define alias, asym, as a strong alias for symbol, sym.
 */
#define sysio_sym_strong_alias(sym, asym) \
  extern __typeof(sym) asym __attribute__((alias(STRINGOF(sym))));

#ifdef HAVE_WEAK_SYMBOLS

/*
 * Define alias, asym, as a strong alias for symbol, sym.
 */
#define sysio_sym_weak_alias(sym, asym) \
  extern __typeof(sym) asym __attribute__((weak, alias(STRINGOF(sym))));
#else /* !defined(HAVE_ASM_WEAK_DIRECTIVE) */

/*
 * Weak symbols not supported. Make it a strong alias then.
 */
#define sysio_sym_weak_alias(sym, asym) sysio_sym_strong_alias(sym, asym)
#endif
