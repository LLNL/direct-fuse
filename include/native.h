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
 *    This Cplant(TM) source code is the property of Sandia National
 *    Laboratories.
 *
 *    This Cplant(TM) source code is copyrighted by Sandia National
 *    Laboratories.
 *
 *    The redistribution of this Cplant(TM) source code is subject to the
 *    terms of the GNU Lesser General Public License
 *    (see cit/LGPL or http://www.gnu.org/licenses/lgpl.html)
 *
 *    Cplant(TM) Copyright 1998-2004 Sandia Corporation. 
 *    Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
 *    license for use of this work by or on behalf of the US Government.
 *    Export of this program may require a license from the United States
 *    Government.
 */

/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Questions or comments about this library should be sent to:
 *
 * Lee Ward
 * Sandia National Laboratories, New Mexico
 * P.O. Box 5800
 * Albuquerque, NM 87185-1110
 *
 * lee@sandia.gov
 */

/*
 * Native file system support.
 */

#if ALPHA_LINUX

/*
 * stat struct from asm/stat.h, as returned 
 * by alpha linux kernel
 */
struct _sysio_native_stat {
	unsigned int    st_dev;
	unsigned int    st_ino;
	unsigned int    st_mode;
	unsigned int    st_nlink;
	unsigned int    st_uid;
	unsigned int    st_gid;
	unsigned int    st_rdev;
	long            st_size;
	unsigned long   st_atime;
	unsigned long   st_mtime;
	unsigned long   st_ctime;
	unsigned int    st_blksize;
	int             st_blocks;
	unsigned int    st_flags;
	unsigned int    st_gen;
};

#define SYSIO_COPY_STAT(src, dest)                    \
do {                                            \
	memset((dest), 0, sizeof((*dest)));	\
	(dest)->st_dev     = (src)->st_dev;     \
	(dest)->st_ino     = (src)->st_ino;     \
	(dest)->st_mode    = (src)->st_mode;    \
	(dest)->st_nlink   = (src)->st_nlink;   \
	(dest)->st_uid     = (src)->st_uid;     \
	(dest)->st_gid     = (src)->st_gid;     \
	(dest)->st_rdev    = (src)->st_rdev;    \
	(dest)->st_size    = (src)->st_size;    \
	(dest)->st_atime   = (src)->st_atime;   \
	(dest)->st_mtime   = (src)->st_mtime;   \
	(dest)->st_ctime   = (src)->st_ctime;   \
	(dest)->st_blksize = (src)->st_blksize; \
	(dest)->st_blocks  = (src)->st_blocks;  \
	(dest)->st_flags   = (src)->st_flags;   \
	(dest)->st_gen     = (src)->st_gen;     \
} while (0)

#else 
#define _sysio_native_stat intnl_stat
#define SYSIO_COPY_STAT(src, dest) *(dest) = *(src) 
#endif

/*
 * System calls.
 */
#if defined(_LARGEFILE64_SOURCE) && defined(SYS_lstat64)
#define SYSIO_SYS_stat		SYS_lstat64
#elif defined(SYS_lstat)
#define SYSIO_SYS_stat		SYS_lstat
#endif
#if defined(_LARGEFILE64_SOURCE) && defined(SYS_fstat64)
#define SYSIO_SYS_fstat		SYS_fstat64
#elif defined(SYS_fstat)
#define SYSIO_SYS_fstat		SYS_fstat
#endif
#if defined(_LARGEFILE64_SOURCE) && defined(SYS_truncate64)
#define SYSIO_SYS_truncate	SYS_truncate64
#elif defined(SYS_truncate)
#define SYSIO_SYS_truncate	SYS_truncate
#endif
#if defined(_LARGEFILE64_SOURCE) && defined(SYS_ftruncate64)
#define SYSIO_SYS_ftruncate	SYS_ftruncate64
#elif defined(SYS_ftruncate)
#define SYSIO_SYS_ftruncate	SYS_ftruncate
#endif
#if defined(SYS_open)
#define SYSIO_SYS_open		SYS_open
#endif
#if defined(SYS_close)
#define SYSIO_SYS_close		SYS_close
#endif
#if defined(_LARGEFILE64_SOURCE) && defined(SYS_lseek64)
#define SYSIO_SYS_lseek		SYS_lseek64
#elif defined(SYS_lseek)
#define SYSIO_SYS_lseek		SYS_lseek
#endif
#if defined(SYS__llseek)
# if defined (__mips64__)
#  define SYSIO_SYS__llseek     SYS_O32__llseek
# else
#  define SYSIO_SYS__llseek     SYS__llseek
# endif
#endif
#if defined(SYS_read)
#define SYSIO_SYS_read		SYS_read
#endif
#if defined(SYS_write)
#define SYSIO_SYS_write		SYS_write
#endif
#if defined(SYS_readv)
#define SYSIO_SYS_readv		SYS_readv
#endif
#if defined(SYS_writev)
#define SYSIO_SYS_writev	SYS_writev
#endif
#if defined(_LARGEFILE64_SOURCE) && defined(SYS_pread64)
#define SYSIO_SYS_pread		SYS_pread64
#elif defined(SYS_pread)
#define SYSIO_SYS_pread		SYS_pread
#endif
#if defined(_LARGEFILE64_SOURCE) && defined(SYS_pwrite64)
#define SYSIO_SYS_pwrite	SYS_pwrite64
#elif defined(SYS_pwrite)
#define SYSIO_SYS_pwrite	SYS_pwrite
#endif
#if defined(_LARGEFILE64_SOURCE) && defined(SYS_fcntl64)
#define SYSIO_SYS_fcntl		SYS_fcntl64
#elif defined(SYS_fcntl)
#define SYSIO_SYS_fcntl		SYS_fcntl
#endif
#if defined(SYS_fsync)
#define SYSIO_SYS_fsync		SYS_fsync
#endif
#if defined(ALPHA_LINUX) && defined(SYS_osf_fdatasync)
#define SYSIO_SYS_fdatasync	SYS_osf_fdatasync
#elif defined(SYS_fdatasync)
#define SYSIO_SYS_fdatasync	SYS_fdatasync
#endif
#if defined(SYS_chmod)
#define SYSIO_SYS_chmod		SYS_chmod
#endif
#if defined(SYS_fchmod)
#define SYSIO_SYS_fchmod	SYS_fchmod
#endif
#if defined(SYS_chown)
#define SYSIO_SYS_chown		SYS_chown
#endif
#if defined(SYS_fchown)
#define SYSIO_SYS_fchown	SYS_fchown
#endif
#if defined(SYS_umask)
#define SYSIO_SYS_umask		SYS_umask
#endif
#if defined(SYS_mkdir)
#define SYSIO_SYS_mkdir		SYS_mkdir
#endif
#if defined(SYS_rmdir)
#define SYSIO_SYS_rmdir		SYS_rmdir
#endif
#if defined(SYS_getdirentries)
#if defined(_LARGEFILE64_SOURCE) && defined(SYS_getdirentries64)
#define SYSIO_SYS_getdirentries	SYS_getdirentries64
#elif defined(SYS_getdirentries)
#define SYSIO_SYS_getdirentries	SYS_getdirentries
#endif
#endif
#if defined(_LARGEFILE64_SOURCE) && defined(SYS_getdents64)
# define SYSIO_SYS_getdents64	SYS_getdents64
#elif defined(SYS_getdents)
# if defined (__mips64__)
#  define SYSIO_SYS_getdents64  SYS_getdents
# else
#  define SYSIO_SYS_getdents    SYS_getdents
# endif
#endif
#if defined(SYS_link)
#define SYSIO_SYS_link		SYS_link
#endif
#if defined(SYS_unlink)
#define SYSIO_SYS_unlink	SYS_unlink
#endif
#if defined(SYS_symlink)
#define SYSIO_SYS_symlink	SYS_symlink
#endif
#if defined(SYS_rename)
#define SYSIO_SYS_rename	SYS_rename
#endif
#if defined(SYS_readlink)
#define SYSIO_SYS_readlink	SYS_readlink
#endif
#if defined(SYS_utimes)
#define SYSIO_SYS_utimes	SYS_utimes
#endif
#if defined(SYS_utime)
#define SYSIO_SYS_utime		SYS_utime
#endif
#if defined(SYS_socketcall)
# if defined (__mips64__)
#  define SYSIO_SYS_socketcall  SYS_O32_socketcall
# else
#  define SYSIO_SYS_socketcall  SYS_socketcall
# endif
#endif
#if defined(SYS_socket)
#define SYSIO_SYS_socket	SYS_socket
#endif
#if defined(SYS_accept)
#define SYSIO_SYS_accept	SYS_accept
#endif
#if defined(SYS_bind)
#define SYSIO_SYS_bind		SYS_bind
#endif
#if defined(SYS_listen)
#define SYSIO_SYS_listen	SYS_listen
#endif
#if defined(SYS_connect)
#define SYSIO_SYS_connect	SYS_connect
#endif
#if defined(SYS_ioctl)
#define SYSIO_SYS_ioctl		SYS_ioctl
#endif
