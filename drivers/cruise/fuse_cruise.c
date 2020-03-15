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
#include <dirent.h>

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
#include "fuse.h"
#endif
#ifndef FUSE_H
#define FUSE_H
#include "fuse.h"
#endif

#include "fs.h"
#include "mount.h"
#include "dev.h"

#include "cruise.h"
#include "cruise-internal.h"
#include "sysio_fuse.h"

#define CRUISE_PATH_MAX 128

static char cruise_rootdir[CRUISE_PATH_MAX];

static int
_sysio_cruise_fsswop_mount(const char *source,
                                    unsigned flags,
                                    const void *data __IS_UNUSED,
				    const char *fstype, 
                                    struct pnode *tocover,
                                    struct mount **mntp);

static struct fssw_ops cruise_fssw_ops = {
        _sysio_cruise_fsswop_mount
};


static void cruise_fullpath(char fpath[CRUISE_PATH_MAX], const char *path)
{
    strcpy(fpath, cruise_rootdir);
    strncat(fpath, path, PATH_MAX); 
}


int fuse_cruise_getattr(const char *path, struct stat *statbuf)
{
    int ret;
    char fpath[CRUISE_PATH_MAX];
    cruise_fullpath(fpath, path);    

    ret = CRUISE_WRAP(stat)(fpath, statbuf);

    return ret;
}

int fuse_cruise_readlink(const char *path, char *link, size_t size)
{
    int retstat;
    char fpath[PATH_MAX];
    
    printf("readlink cruise not implemented\n");
    return retstat;
}

int fuse_cruise_mknod(const char *path, mode_t mode, dev_t dev)
{
    int retstat;
    char fpath[CRUISE_PATH_MAX];
    cruise_fullpath(fpath, path);

	
    retstat = CRUISE_WRAP(creat)(path, mode);
    retstat = CRUISE_WRAP(close)(retstat);
   return retstat;
}

/** Create a directory */
int fuse_cruise_mkdir(const char *path, mode_t mode)
{
    int ret;
    char fpath[CRUISE_PATH_MAX];

    cruise_fullpath(fpath, path);
    ret = CRUISE_WRAP(mkdir)(fpath, mode);    

    return ret;
}

/** Remove a file */
int fuse_cruise_unlink(const char *path)
{
    int ret;
    char fpath[CRUISE_PATH_MAX];

    cruise_fullpath(fpath, path);
    ret = CRUISE_WRAP(unlink)(fpath);

    return ret;
}

/** Remove a directory */
int fuse_cruise_rmdir(const char *path)
{
    int ret;
    char fpath[CRUISE_PATH_MAX];
 
    cruise_fullpath(fpath, path);
    ret = CRUISE_WRAP(rmdir)(fpath);

    return ret;
}

/** Create a symbolic link */
int fuse_cruise_symlink(const char *path, const char *link)
{
    printf("symlink cruise not implemented\n");

    return 0;
}

/** Rename a file */
int fuse_cruise_rename(const char *path, const char *newpath)
{
    int ret;
    char fpath[CRUISE_PATH_MAX];

    cruise_fullpath(fpath, path);
    ret = CRUISE_WRAP(rename)(fpath, newpath);

    return ret;
}

/** Create a hard link to a file */
int fuse_cruise_link(const char *path, const char *newpath)
{
    
    printf("link not implemented in cruise\n");

    return 0;
}

/** Change the permission bits of a file */
int fuse_cruise_chmod(const char *path, mode_t mode)
{
    printf("chmod not implemented in cruise\n");
    
    return 0;
}

/** Change the owner and group of a file */
int fuse_cruise_chown(const char *path, uid_t uid, gid_t gid)
  
{
    printf("chown not implemented in cruise\n");

    return 0;
}

/** Change the size of a file */
int fuse_cruise_truncate(const char *path, off_t newsize)
{
    int ret;
    char fpath[CRUISE_PATH_MAX];
    
    cruise_fullpath(fpath, path);

    ret = CRUISE_WRAP(truncate)(fpath, newsize);

    return 0;
}

/** Change the access and/or modification times of a file */
int fuse_cruise_utime(const char *path, struct utimbuf *ubuf)
{
    printf("utime not implemented in cruise\n");

    return 0;
}

/** File open operation
 */
int fuse_cruise_open(const char *path, struct fuse_file_info *fi)
{
    	int fd;
	char fpath[CRUISE_PATH_MAX];

	cruise_fullpath(fpath, path);
	fd = CRUISE_WRAP(open)(fpath,fi->flags);
    	fi->fh = fd;
    return fd;
}

/** Read data from an open file
 */
int fuse_cruise_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    size_t ret;

    if (offset > 0)
	ret = CRUISE_WRAP(pread)(fi->fh, buf, size, offset);
    else
    	ret = CRUISE_WRAP(read)(fi->fh, buf, size);
 
    return ret;
}

/** Write data to an open file
 */
int fuse_cruise_write(const char *path, const char *buf, size_t size, off_t offset,
	     struct fuse_file_info *fi)
{
    int ret;
 
    if (offset > 0)
	ret = CRUISE_WRAP(pwrite)(fi->fh, buf, size, offset);
    else
	ret = CRUISE_WRAP(write)(fi->fh, buf, size);

    return ret;
}

/** Get file system statistics
 */
int fuse_cruise_statfs(const char *path, struct statvfs *statv)
{
    printf("statfs not implemented in cruise\n");
    return 0;
}

/** Possibly flush cached data
 */
int fuse_cruise_flush(const char *path, struct fuse_file_info *fi)
{
    printf("flush not supported in libsysio\n");
    return 0;
}

/** Release an open file
 */
int fuse_cruise_release(const char *path, struct fuse_file_info *fi)
{
    int ret;

  //  ret = CRUISE_WRAP(close)(fi->fh);

    return ret;
}

/** Synchronize file contents
 */
int fuse_cruise_fsync(const char *path, int datasync, struct fuse_file_info *fi)
{
    
    // some unix-like systems (notably freebsd) don't have a datasync call
    if (!datasync)
	return CRUISE_WRAP(fsync)(fi->fh);
    else
	return CRUISE_WRAP(fdatasync)(fi->fh);
}

#ifdef HAVE_SYS_XATTR_H
/** Set extended attributes */
int fuse_cruise_setxattr(const char *path, const char *name, const char *value, size_t size, int flags)
{
	printf("setxattr not implemented in cruise\n");

	return 0;
}

/** Get extended attributes */
int fuse_cruise_getxattr(const char *path, const char *name, char *value, size_t size)
{
    int retstat = 0;

    printf("getxattr is not implemented in cruise\n");
    
    return retstat;
}


/** List extended attributes */
int fuse_cruise_listxattr(const char *path, char *list, size_t size)
{
    int retstat = 0;

    printf("listxattr is not implemented in cruise\n");

    return retstat;
}

/** Remove extended attributes */
int fuse_cruise_removexattr(const char *path, const char *name)
{
    printf("removexattr is not implemented in cruise\n");
    return 0;
}
#endif

/** Open directory
 */
int fuse_cruise_opendir(const char *path, struct fuse_file_info *fi)
{
    printf("opendir is not implemented in cruise\n");
    return 0;
}

/** Read directory
 */
int fuse_cruise_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
	       struct fuse_file_info *fi)
{
    printf("readdir is not implemented in cruise\n");    

    return 0;
}

/** Release directory
 */
int fuse_cruise_releasedir(const char *path, struct fuse_file_info *fi)
{
    printf("releasedir is not implemented in cruise\n");
    return 0;
}

/** Synchronize directory contents
 */
int fuse_cruise_fsyncdir(const char *path, int datasync, struct fuse_file_info *fi)
{
    
    printf("fsyncdir is not implemented in cruise\n");
    return 0;
}

/**
 * Initialize filesystem
 */
void *fuse_cruise_init(struct fuse_conn_info *conn)
{
    return 0;
}

void fuse_cruise_destroy(void *userdata)
{
   return 0;
}

int fuse_cruise_access(const char *path, int mask)
{
    int ret;
    char fpath[CRUISE_PATH_MAX];
    
    cruise_fullpath(fpath, path);
   
    ret = CRUISE_WRAP(access)(fpath, mask);
  
    return ret;
}

int fuse_cruise_ftruncate(const char *path, off_t offset, struct fuse_file_info *fi)
{
    int ret;
    
    ret = CRUISE_WRAP(ftruncate)(fi->fh, offset);
    
    return ret;
}

/**
 * Get attributes from an open file
 */
int fuse_cruise_fgetattr(const char *path, struct stat *statbuf, struct fuse_file_info *fi)
{
    int ret;

    ret = CRUISE_WRAP(stat)(fi->fh, statbuf);

    return ret;
}

extern struct fuse_operations fuse_cruise_oper = {
  .getattr = fuse_cruise_getattr,
  .readlink = fuse_cruise_readlink,
  // no .getdir -- that's deprecated
  .getdir = NULL,
  .mknod = fuse_cruise_mknod,
  .mkdir = fuse_cruise_mkdir,
  .unlink = fuse_cruise_unlink,
  .rmdir = fuse_cruise_rmdir,
  .symlink = fuse_cruise_symlink,
  .rename = fuse_cruise_rename,
  .link = fuse_cruise_link,
  .chmod = fuse_cruise_chmod,
  .chown = fuse_cruise_chown,
  .truncate = fuse_cruise_truncate,
  .utime = fuse_cruise_utime,
  .open = fuse_cruise_open,
  .read = fuse_cruise_read,
  .write = fuse_cruise_write,
  /** Just a placeholder, don't set */ // huh???
  .statfs = fuse_cruise_statfs,
  .flush = fuse_cruise_flush,
  .release = fuse_cruise_release,
  .fsync = fuse_cruise_fsync,
  
#ifdef HAVE_SYS_XATTR_H
  .setxattr = fuse_cruise_setxattr,
  .getxattr = fuse_cruise_getxattr,
  .listxattr = fuse_cruise_listxattr,
  .removexattr = fuse_cruise_removexattr,
#endif
  
  .opendir = fuse_cruise_opendir,
  .readdir = fuse_cruise_readdir,
  .releasedir = fuse_cruise_releasedir,
  .fsyncdir = fuse_cruise_fsyncdir,
  .init = fuse_cruise_init,
  .destroy = fuse_cruise_destroy,
  .access = fuse_cruise_access,
  .ftruncate = fuse_cruise_ftruncate,
  .fgetattr = fuse_cruise_fgetattr
};

static int
_sysio_cruise_fsswop_mount(const char *source,
                                    unsigned flags,
                                    const void *data,
                                    const char *fstype,
                                    struct pnode *tocover,
                                    struct mount **mntp)
{
        int err;
        struct mount *tmnt;
	int size;
	size = (int *)data;

	err = _sysio_fuse_mount_init(source, flags, data, fstype, tocover, &tmnt, fuse_cruise_oper);
	if (err) {
		fprintf(stderr, "mount init failed\n");
		return err;
	}

	sprintf(cruise_rootdir, "/%s", tocover->p_base->pb_name.name);
	cruise_mount(cruise_rootdir, 1024 * 1024, 0);
	printf("mount path:%s, size:%d\n", cruise_rootdir, size);
	
	return err;
}



/* Init the dirver */
int
_sysio_cruise_init()
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
         * Move to entry for `..'
         */
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

        return _sysio_fssw_register("cruise", &cruise_fssw_ops);


}


