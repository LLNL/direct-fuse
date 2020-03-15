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
 *    Cplant(TM) Copyright 1998-2003 Sandia Corporation. 
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
 * File system or volume support.
 */
#ifndef QUEUE
#define QUEUE
#include <sys/queue.h>
#endif

struct filesys;

struct pnode;
struct mount;

/*
 * File system switch operations.
 */
struct fssw_ops {
	int	(*fsswop_mount)(const char *source,
				unsigned flags,
				const void *data,
				const char *fstype,
				struct pnode *tocover,
				struct mount **mntp);
};

/*
 * File system switch entry record.
 *
 * Each available file system or volume access driver is represented by
 * one of these switch entries in the switch.
 */
struct fsswent {
	const char *fssw_name;				/* entry name */
	LIST_ENTRY(fsswent) fssw_link;			/* link to next */
	struct fssw_ops fssw_ops;			/* operations */
};

/*
 * Init file system switch entry record.
 */
#define FSSWENT_INIT(fsswent, name, ops) \
	do { \
		(fsswent)->fssw_name = (name); \
		(fsswent)->fssw_ops = (ops); \
	} while (0)

struct inode;

/*
 * File system operations.
 */
struct filesys_ops {
	void	(*fsop_gone)(struct filesys *);
};

/*
 * Define the desired size of the file system record's inode table. This should
 * probably be something fancy that tries to use up a system page, or the
 * like. I'm not feeling adventurous right now though. It is prime though.
 * That should help out the hash.
 */
#ifndef FS_ITBLSIZ
#define FS_ITBLSIZ	503
#endif


/*
 * Inode list head record.
 */
LIST_HEAD(itable_entry, inode);

/*
 * A filesys record is maintained for each active file system or volume.
 */
struct filesys {
	dev_t	fs_dev;					/* device ID */
	unsigned fs_ref;				/* soft ref count */
	unsigned fs_flags;				/* flags (see below) */
	struct filesys_ops fs_ops;			/* operations */
	void	*fs_private;				/* driver data */
	struct itable_entry fs_itbl[FS_ITBLSIZ];	/* inodes hash */
	unsigned long fs_id;				/* ID */
	size_t	fs_bsize;				/* block size */
	char *fs_name;					/* fs name */
	size_t name_len;				/* length of the name */
};

#define FS_F_RO			0x01			/* read-only */

/*
 * Init file system record.
 */
#define FS_INIT(fs, flags, ops, fsname, name_len, private) \
	do { \
		size_t	__i; \
		struct itable_entry *__head; \
 \
		(fs)->fs_ref = 1; \
		(fs)->fs_flags = (flags); \
		(fs)->fs_ops = *(ops); \
		(fs)->fs_name = malloc(sizeof(char) * name_len); \
		strncpy((fs)->fs_name, fsname, name_len); \
		(fs)->name_len = name_len; \
		(fs)->fs_private = (private); \
		__i = FS_ITBLSIZ; \
		__head = (fs)->fs_itbl; \
		do { \
			LIST_INIT(__head); \
			__head++; \
		} while (--__i); \
	} while (0)

/*
 * Reference file system record.
 */
#define FS_REF(fs) \
	do { \
		++(fs)->fs_ref; \
		assert((fs)->fs_ref); \
	} while (0)

/*
 * Release reference to file system record.
 */
#define FS_RELE(fs) \
	do { \
		assert((fs)->fs_ref); \
		if (!--(fs)->fs_ref) \
			_sysio_fs_gone(fs); \
	} while (0)

extern struct fsswent *_sysio_fssw_lookup(const char *name);
extern int _sysio_fssw_register(const char *name, struct fssw_ops *ops);
extern struct filesys * _sysio_fs_new(struct filesys_ops *ops,
				      unsigned mask,
				      char *fs_name,
				      size_t name_len, 
				      void *private);
extern void _sysio_fs_gone(struct filesys *fs);
#ifdef ZERO_SUM_MEMORY
extern void _sysio_fssw_shutdown(void);
#endif
