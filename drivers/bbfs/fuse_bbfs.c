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



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>
#include <errno.h>
#include <stdint.h>
#include <assert.h>
#include <sys/uio.h>
//#include <sys/types.h>
#include <dirent.h>
//#include <sys/stat.h>
#ifdef _HAVE_STATVFS
#include <sys/statvfs.h>
#endif
#ifndef QUEUE
#define QUEUE
#include <sys/queue.h>
#endif

#include <fuse_lowlevel.h>
#include "xtio.h"
#ifndef SYSIO_HEADER
#define SYSIO_HEADER
#include <sys/types.h>
#include "sysio.h"
#include <sys/stat.h>
#include "inode.h"
#include "fuse.h"
#endif
#include "fs.h"
#include "mount.h"
#include "dev.h"
#include "file.h"

#include "sysio_fuse.h"
#include "fuse_bbfs.h"
#include "log.h"
#include "bbfs.h"
#include "params.h"
/* bbfs stat */
struct bb_state *bb_data;
struct fuse_operations bb_oper;


static int _sysio_bbfs_fsswop_mount(const char *source,
                                      unsigned flags,
                                      const void *data,
				                              const char *fstype,
                                      struct pnode *tocover,
                                      struct mount **mntp);

static struct fssw_ops bbfs_fssw_ops = {
                _sysio_bbfs_fsswop_mount
};

static int bbfs_init(char *source)
{
	BB_DATA = malloc(sizeof(struct bb_state));
	if (BB_DATA == NULL) {
		perror("bbfs alloc");
		abort();
	}

	BB_DATA->rootdir = realpath(source, NULL);
	BB_DATA->logfile = log_open();
	return 0;
}

static int mount_times = 0;

static int 
_sysio_bbfs_fsswop_mount(const char *source,
				    unsigned flags,
				    const void *data __IS_UNUSED,
				    const char *fstype, 
				    struct pnode *tocover,
				    struct mount **mntp)
{
	int err;
	struct mount *tmnt;

	err = _sysio_fuse_mount_init(source, flags, data, fstype, tocover, &tmnt, bb_oper);
	if (err)
		return err;

	bbfs_init(source);
	
	return err;
}

/* Init the dirver */
int
_sysio_bbfs_init()
{
	struct intnl_dirent *de;
	off_t off;

	/* Fill in the dir template */
	de = (struct intnl_dirent *)fuse_dir_template;
#ifdef _DIRENT_HAVE_D_OFF
        de->d_off =
#endif
            off = de->d_reclen = FUSE_D_RECLEN(1);
        de->d_type = FUSE_D_TYPEOF(S_IFDIR);
        de->d_name[0] = '.';
#ifdef _DIRENT_HAVE_D_NAMLEN
        de->d_namlen = 1;
#endif
        /*
 *          * Move to entry for `..'
 *                   */
        de = (struct intnl_dirent *)((char *)de + off);
        de->d_reclen = FUSE_D_RECLEN(2);
#ifdef _DIRENT_HAVE_D_NAMLEN
        de->d_namlen = 2;
#endif
#ifdef _DIRENT_HAVE_D_OFF
        de->d_off =
#endif
            off += de->d_reclen;
        de->d_type = FUSE_D_TYPEOF(S_IFDIR);
        de->d_name[0] = de->d_name[1] = '.';
        de->d_name[2] = ' ';

        return _sysio_fssw_register("bbfs", &bbfs_fssw_ops);


}

