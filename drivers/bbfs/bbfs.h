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


//#ifndef BBFS_HEADER
//#define BBFS_HEADER
//#include <stdio.h>
#include <fuse.h>
int bb_open(const char *path, struct fuse_file_info *fi);
int bb_mknod(const char *path, mode_t mode, dev_t dev);
int bb_mkdir(const char *path, mode_t mode);

//#endif
