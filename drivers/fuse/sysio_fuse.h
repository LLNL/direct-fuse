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



#ifndef QUEUE
#define QUEUE
#include <sys/queue.h>
#endif

#ifndef SYSIO_HEADER
#define SYSIO_HEADER
#include <sys/types.h>
#include "sysio.h"
#include <sys/stat.h>
#include "inode.h"
#include <dirent.h>
#include "fuse.h"
#endif
/*
 * fuse file system driver support.
 */
#define FUSE_BLKSIZE (4096)
struct fuse_inode {
        LIST_ENTRY(fuse_inode) fi_link;         /* i-nodes list link */
       struct intnl_stat fi_st;                /* attrs */
        struct file_identifier fi_fileid;       /* file ID */
        void *fi_data;                          /* file data */
        struct fuse_file_info *fi_file;         /* fuse file info */
	struct fuse_operations fi_ops;		/* fuse operations */
	char *fi_path;				/* file path */
};


struct fuse_filesys{
	LIST_HEAD(, fuse_inode) ffs_finodes; 	/* all i-nodes list */
};

/*
 * Given mode bits, return directory entry type code.
 */
#define FUSE_D_TYPEOF(m)      (((m) & S_IFMT) >> 12)

/* calculate size of a directory entry given length of the entry name  */
#define FUSE_D_RECLEN(namelen) \
	(((size_t )&((struct intnl_dirent *)0)->d_name + \
          (namelen) + 1 + sizeof(void *)) & \
         ~(sizeof(void *) - 1))


/* fuse dir template*/
extern char fuse_dir_template[FUSE_D_RECLEN(1) + FUSE_D_RECLEN(2)]; 
extern int _sysio_fuse_mount_init(const char *source,
                        	  unsigned flags,
                        	  const void *data __IS_UNUSED,
                        	  const char *fstype,
                        	  struct pnode *tocover,
                        	  struct mount **mntp,
                        	  struct fuse_operations fuse_opers);

//extern int _sysio_fuse_init(void);
extern ino_t fuse_inum_alloc();
extern struct fuse_inode *fuse_directory_new(struct fuse_filesys *ffs,
						    struct fuse_inode *parent, 
						    struct intnl_stat *st);
extern void fuse_i_destory(struct fuse_inode *fino);
extern struct filesys_ops fuse_fs_ops;   
extern struct inode_ops fuse_i_ops; 
