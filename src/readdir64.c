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



#ifdef _LARGEFILE64_SOURCE
#define _SCANDIR SYSIO_INTERFACE_NAME(scandir64)
#define _READDIR SYSIO_INTERFACE_NAME(readdir64)
#define _GETDIRENTRIES SYSIO_INTERFACE_NAME(getdirentries64)
#define _DIRENT_T struct dirent64
#define _OFF_T _SYSIO_OFF_T

#include "readdir.c"

#endif
