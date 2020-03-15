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



#ifndef __CURLFTPFS_PATH_UTILS_H__
#define __CURLFTPFS_PATH_UTILS_H__

char* get_file_name(const char* path);
char* get_full_path(const char* path);
char* get_fulldir_path(const char* path);
char* get_dir_path(const char* path);

#endif   /* __CURLFTPFS_PATH_UTILS_H__ */
