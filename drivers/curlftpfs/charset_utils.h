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



#ifndef __CURLFTPFS_CHARSET_UTILS_H__
#define __CURLFTPFS_CHARSET_UTILS_H__

int convert_charsets(const char* from, const char* to, char** str);

#endif  /* __CURLFTPFS_CHARSET_UTILS_H__ */
