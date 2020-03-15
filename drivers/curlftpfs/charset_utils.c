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



/*
    FTP file system
    Copyright (C) 2007 Robson Braga Araujo <robsonbraga@gmail.com>

    This program can be distributed under the terms of the GNU GPL.
    See the file COPYING.
*/

#include "config.h"

#include <string.h>
#include <limits.h>
#include <iconv.h>
#include <errno.h>
#include <stdlib.h>

#include "ftpfs.h"

int convert_charsets(const char* from, const char* to, char** str) {
  iconv_t cd;
  char* s = *str;

  if (!s || !*s)
    return 0;

  if (to && from && (cd = iconv_open(to, from)) != (iconv_t)-1) {
    ICONV_CONST char* ib;
    char* buf;
    char* ob;
    size_t ibl, obl;

    ibl = strlen(s) + 1;
    ib = s;
    obl = MB_LEN_MAX * ibl;
    buf = (char*)malloc(obl * sizeof(char));
    ob = buf;

    do {
      if (iconv(cd, &ib, &ibl, &ob, &obl) == (size_t)-1) {
        DEBUG(2, "iconv return error %d\n", errno);
        if (obl) {
          *ob++ = *ib++;
          ibl--;
          obl--;
        }
      }
    } while (ibl && obl);
    *ob = 0;
    DEBUG(2, "iconv return %s\n", buf);
    iconv_close (cd);
    free(*str);
    *str = buf;
  } else {
    DEBUG(2, "iconv_open return error %d\n", errno);
  }
  
  return 0;
}
