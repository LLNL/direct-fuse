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



#ifndef __CURLFTPFS_FTPFS_LS_H__
#define __CURLFTPFS_FTPFS_LS_H__

/*
    FTP file system
    Copyright (C) 2006 Robson Braga Araujo <robsonbraga@gmail.com>

    This program can be distributed under the terms of the GNU GPL.
    See the file COPYING.
*/

#include "curlftpfs_cache.h"

int parse_dir(const char* list, const char* dir,
              const char* name, struct stat* sbuf,
              char* linkbuf, int linklen,
              fuse_cache_dirh_t h, fuse_cache_dirfil_t filler);

#endif  /* __CURLFTPFS_FTPFS_LS_H__ */
