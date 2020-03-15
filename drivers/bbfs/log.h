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
  Copyright (C) 2012 Joseph J. Pfeiffer, Jr., Ph.D. <pfeiffer@cs.nmsu.edu>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.
*/

#ifndef _LOG_H_
#define _LOG_H_
#include <stdio.h>

//  macro to log fields in structs.
#define log_struct(st, field, format, typecast) \
  log_msg("    " #field " = " #format "\n", typecast st->field)

FILE *log_open(void);
void log_msg(const char *format, ...);
void log_conn(struct fuse_conn_info *conn);
int log_error(char *func);
void log_fi(struct fuse_file_info *fi);
void log_fuse_context(struct fuse_context *context);
void log_retstat(char *func, int retstat);
void log_stat(struct stat *si);
void log_statvfs(struct statvfs *sv);
int  log_syscall(char *func, int retstat, int min_ret);
void log_utime(struct utimbuf *buf);

#endif
