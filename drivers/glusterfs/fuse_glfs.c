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
#include "sys/stat.h"
#include "inode.h"
//#include "fuse.h"
#endif
#ifndef FUSE_H
#define FUSE_H
#include "fuse.h"
#endif

#include "fs.h"
#include "mount.h"
#include "dev.h"


#include "sysio_fuse.h"
#include "fuse_glfs.h"
#include "xglfs.h"

static int 
_sysio_glfs_fsswop_mount(const char *source,
				    unsigned flags,
				    const void *data __IS_UNUSED,
				    const char *fstype, 
				    struct pnode *tocover,
				    struct mount **mntp);

static struct fssw_ops glfs_fssw_ops = {
	_sysio_glfs_fsswop_mount
};


static int 
_sysio_glfs_fsswop_mount(const char *source,
				    unsigned flags,
				    const void *data __IS_UNUSED,
				    const char *fstype,
				    struct pnode *tocover,
				    struct mount **mntp)
{
	int err;
	struct mount *tmnt;

	printf("glfs mount\n");

	err = _sysio_fuse_mount_init(source, flags, data, fstype, tocover, &tmnt, xglfs_ops);
	if (err)
		return err;	
	
//	char *v2 = "/glfs";	
//	char *v2 = tocover->p_base->pb_name.name;
//	glfs_v[2] = v2;

	    int argc = 7;
	char *argv[7];

	argv[0] = "./xglfs";
//	argv[1] = "--server=inv09";
//	argv[1] = source;
	argv[1] = (char *)malloc(sizeof(char) * 128);
	sprintf(argv[1], "--server=%s", source);
	argv[2] = "--volume=gv0";
//	argv[3] = "--mountpoint=/home/yzhu/my_codes/fs/2017/04_03/xglfs/build/glusterfs";
//	argv[3] = tocover->p_base->pb_name.name;
	argv[3] = (char *)malloc(sizeof(char) * 128);
	sprintf(argv[3], "--mountpoint=%s", tocover->p_base->pb_name.name);
	argv[4] = "-o";
	argv[5] = "stderr";
	argv[6] = "-f";

	err = xglfs_mnt_init(argc, argv);

	return err;

}

/* Init the dirver */
int
_sysio_glfs_init()
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

        return _sysio_fssw_register("glfs", &glfs_fssw_ops);


}


