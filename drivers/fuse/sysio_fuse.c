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
#include <dirent.h>
#include <errno.h>
#include <stdint.h>
#include <dirent.h>
#include <assert.h>
#include <sys/uio.h>
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
#include "init_fuse.h"
char fuse_dir_template[FUSE_D_RECLEN(1) + FUSE_D_RECLEN(2)];

/* pesudo-blocksize learn from incore, not sure yet*/
//#define FUSE_BLKSIZE (4096)

/*
 * Lookup data argument bundle record.
 */
struct lookup_data {
  struct qstr *name;                              /* desired entry name */
  struct intnl_dirent *de;                        /* last dirent */
  size_t  minsiz;                                 /* min hole needed */
  struct {
    void    *p;                             /* best hole */
    size_t  len;                            /* best hole len */
  } hole;
};

typedef void *(*probe_ty)(void *data, size_t len, void *arg);


/*
 *  * Initialize lookup data argument bundle.
 *   */
#define FUSE_LD_INIT(ld, minsz, qs) \
        do { \
                (ld)->name = (qs); \
                (ld)->de = NULL; \
                (ld)->minsiz = (minsz); \
                (ld)->hole.p = NULL; \
                (ld)->hole.len = 0; \
        } while (0)

# define DTTOIF(dirtype)        ((dirtype) << 12)

/* calculate size of a directory entry given length of the entry name  */
//#define FUSE_D_RECLEN(namelen) \
//	(((size_t )&((struct intnl_dirent *)0)->d_name + \
//          (namelen) + 1 + sizeof(void *)) & \
//         ~(sizeof(void *) - 1))


/* fuse dir template*/
//static char fuse_dir_template[FUSE_D_RECLEN(1) + FUSE_D_RECLEN(2)]; 

/* given pointer to filesys, return pointer to fuse filesys  */
#define FS2FFS(fs)	((struct fuse_filesys *)(fs)->fs_private)

///* given pointer to inode, return pointer to fuse_inode  */
//#define I2FI(ino)	((struct fuse_inode *)(ino)->i_private)


/*
 * Given mode bits, return directory entry type code.
 */
//#define FUSE_D_TYPEOF(m)      (((m) & S_IFMT) >> 12)


/* given pointer to inode, return pointer to fuse_inode  */
#define I2FI(ino)       ((struct fuse_inode *)(ino)->i_private)
#define P2FI(pno)	(I2FI(pno->p_base->pb_ino))

#define __u64 uint64_t

static int _sysio_fuse_fsswop_mount(const char *source,
                                    unsigned flags,
                                    const void *data,
                                    const char *fstype,
                                    struct pnode *tocover,
                                    struct mount **mntp);

static struct fssw_ops fuse_fssw_ops = {
  _sysio_fuse_fsswop_mount
};

static void _sysio_fuse_fsop_gone(struct filesys *fs);

struct filesys_ops fuse_fs_ops = {
  _sysio_fuse_fsop_gone,
};


static int fuse_inop_lookup(struct pnode *pno,
                            struct inode **inop,
                            struct intent *intnt,
                            const char *path);
static int fuse_inop_getattr(struct pnode *pno,
                             struct inode *ino,
                             struct intnl_stat *stbuf);
static int fuse_inop_setattr(struct pnode *pno,
                             struct inode *ino,
                             unsigned mask,
                             struct intnl_stat *stbuf);
static ssize_t fuse_filldirentries(struct inode *ino,
                                   _SYSIO_OFF_T *posp,
                                   char *buf,
                                   size_t nbytes);
static int fuse_inop_mkdir(struct pnode *pno, mode_t mode);
static int fuse_inop_rmdir(struct pnode *pno);
static int fuse_inop_symlink(struct pnode *pno, const char *data);
static int fuse_inop_readlink(struct pnode *pno, char *buf, size_t bufsiz);
static int fuse_inop_open(struct pnode *pno, int flags, mode_t mode);
static int fuse_inop_close(struct pnode *pno);
static int fuse_inop_link(struct pnode *old, struct pnode *new);
static int fuse_inop_unlink(struct pnode *pno);
static int fuse_inop_rename(struct pnode *old, struct pnode *new);
static int fuse_inop_read(void *buf, size_t nbytes, _SYSIO_OFF_T off, struct pnode *pno);
static int fuse_inop_write(void *buf, size_t nbytes, _SYSIO_OFF_T off, struct pnode *pno);
static _SYSIO_OFF_T fuse_inop_pos(struct inode *ino, _SYSIO_OFF_T off);
static int fuse_inop_iodone(struct ioctx *ioctx);
static int fuse_inop_fcntl(struct inode *ino, int cmd, va_list ap, int *rtn);
static int fuse_inop_sync(struct pnode *pno);
static int fuse_inop_datasync(struct pnode *pno);
static int fuse_inop_ioctl(struct inode *ino,
                           unsigned long int request,
                           va_list ap);
static int fuse_inop_mknod(struct pnode *pno, mode_t mode, dev_t dev);
#ifdef _HAVE_STATVFS
static int fuse_inop_statvfs(struct pnode *pno,
                             struct inode *ino,
                             struct intnl_statvfs *buf);
#endif
static int fuse_inop_ftruncate(struct pnode *pno, size_t size);
static void fuse_inop_gone(struct inode *ino);




struct inode_ops fuse_i_ops = {
  fuse_inop_lookup,
  fuse_inop_getattr,
  fuse_inop_setattr,
  fuse_filldirentries,
  fuse_inop_mkdir,
  fuse_inop_rmdir,
  fuse_inop_symlink,
  fuse_inop_readlink,
  fuse_inop_open,
  fuse_inop_close,
  fuse_inop_link,
  fuse_inop_unlink,
  fuse_inop_rename,
  fuse_inop_read,
  fuse_inop_write,
  fuse_inop_pos,
  fuse_inop_iodone,
  fuse_inop_fcntl,
  fuse_inop_sync,
  fuse_inop_datasync,
  fuse_inop_ioctl,
  fuse_inop_mknod,
#ifdef _HAVE_STATVFS
  fuse_inop_statvfs,
#endif
  fuse_inop_ftruncate, 
  fuse_inop_gone
};


ino_t fuse_inum_alloc()
{
	static ino_t nxtnum = 1;
	
	assert(nxtnum);
	return nxtnum++;
}

static struct fuse_inode *
fuse_i_alloc(struct fuse_filesys *ffs, 
	     struct intnl_stat *st, 
	     struct fuse_operations *ops)
{
	struct fuse_inode *fino;

	assert(st->st_ino);
	assert(!st->st_size);

	fino = malloc(sizeof(struct fuse_inode));
	if (!fino)
		return NULL;
	fino->fi_st = *st;
	fino->fi_fileid.fid_data = &fino->fi_st.st_ino;
	fino->fi_fileid.fid_len = sizeof(fino->fi_st.st_ino);
	fino->fi_data = NULL;
	fino->fi_file = NULL;
	if (ops)
		fino->fi_ops = *ops;

	LIST_INSERT_HEAD(&ffs->ffs_finodes, fino, fi_link);

	return fino;


}

static int 
fuse_trunc(struct fuse_inode *fino, 
    _SYSIO_OFF_T size, int clear)
{
	_SYSIO_OFF_T n;
	void *p;

	if (size < 0)
		return -EINVAL;
	n = size;

	if (!size) {

		if (fino->fi_data) {
			free(fino->fi_data);
			fino->fi_data = NULL;
		}

		if (fino->fi_file) {
			free(fino->fi_file);
			fino->fi_file = NULL;
		}

		n = 0;
		goto out;
	}

	p = realloc(fino->fi_data, (size_t)n);
	if (!p) 
		return -ENOSPC;

	fino->fi_data = p;
	if (clear && n > fino->fi_st.st_size) {
		(void)memset((char *)fino->fi_data + fino->fi_st.st_size,
			     0,
			     (size_t)(n - fino->fi_st.st_size));
	}
out:
        fino->fi_st.st_size = n;
        fino->fi_st.st_blocks =
                (n + fino->fi_st.st_blksize -1)/fino->fi_st.st_blksize;
        fino->fi_st.st_mtime = time(NULL);
        return 0;

}

void 
fuse_i_destory(struct fuse_inode *fino)
{
	LIST_REMOVE(fino, fi_link);
	(void)fuse_trunc(fino, 0 ,0);
	free(fino);
	
}

struct fuse_inode *
fuse_directory_new(struct fuse_filesys *ffs,
		   struct fuse_inode *parent,
		   struct intnl_stat *st)
{
	struct fuse_inode *fino;
	int err;
	struct intnl_dirent *de;

	if (parent) {
		fino = fuse_i_alloc(ffs, st, &(parent->fi_ops));
	} else {
		fino = fuse_i_alloc(ffs, st, NULL);
	}
	if (!fino)
		return NULL;

	if (!parent)
		parent = fino;		/* root */

	
	/* Allocate and init directory data */
	err = fuse_trunc(fino, sizeof(fuse_dir_template), 1);
	if (err) {
		fuse_i_destory(fino);
		return NULL;
	}
	
	(void)memcpy(fino->fi_data,
		     &fuse_dir_template, 
		     sizeof(fuse_dir_template));
	
	de = fino->fi_data;
	de->d_ino = st->st_ino;
	de = (struct intnl_dirent *)((char *)de + 
#ifdef _DIRENT_HAVE_D_OFF
				     de->d_off
#else
				     de->d_reclen
#endif
				     );
	de->d_ino = parent->fi_st.st_ino;

	/* set creation time to modify time set by truncate. */
	st->st_ctime = st->st_mtime;

	return fino;

}

static void 
_sysio_fuse_fsop_gone(struct filesys *fs)
{
	struct fuse_filesys *ffs;
	struct fuse_inode *fino, *ofino;
	
	ffs = FS2FFS(fs);

	fino = ffs->ffs_finodes.lh_first;
	while(fino) {
		ofino = fino;
		fino = fino->fi_link.le_next;
		fuse_i_destory(ofino);

	}

	/* free FS record */
	free(ffs);

}


static int 
_sysio_fuse_fsswop_mount(const char *source,
				                 unsigned flags,
				                 const void *data __IS_UNUSED,
				                 const char *fstype,
				                 struct pnode *tocover,
				                 struct mount **mntp)
{
	int err;
	struct mount *tmnt;
	struct fuse_operations null_ops;	

	err = _sysio_fuse_mount_init(source, flags, data, fstype, tocover, &tmnt, null_ops);
	if (err) 
		return err;
	
	*mntp = tmnt;

	return err;
}

int 
_sysio_fuse_mount_init(const char *source,
			                 unsigned flags, 
			                 const void *data __IS_UNUSED,
                       const char *fstype,
                       struct pnode *tocover,
                       struct mount **mntp, 
                       struct fuse_operations fuse_opers)
{
	char *cp;
	unsigned long ul;
	uid_t uid;
	gid_t gid;
	int err;
	mode_t mode;
	dev_t dev;
	ino_t inum;
	struct filesys *fs;
	struct mount *mnt;
	struct inode *rooti;
	struct pnode_base *rootpb;	
	struct fuse_filesys *ffs;
	struct intnl_stat stat;
	struct fuse_inode *fino;
	static struct qstr noname = {NULL, 0, 0};
	size_t name_len;


	uid = getuid();
	gid = getgid();

	fs = NULL;
	mnt = NULL;
	rooti = NULL;
	rootpb = NULL;
	ffs = NULL;
	fino = NULL;


	ffs = malloc(sizeof(struct fuse_filesys));
	if (!ffs) {
		err = -ENOMEM;
		goto error;	

	}
	
	err = 0;

	dev = _sysio_dev_alloc();


	/* create new fs */
	(void)memset(ffs, 0, sizeof(struct fuse_filesys));
	LIST_INIT(&ffs->ffs_finodes);

	/* create root i-node */
	(void)memset(&stat, 0, sizeof(stat));
	stat.st_dev = dev;
	inum = fuse_inum_alloc();
	stat.st_mode = S_IFDIR| S_IRUSR | S_IWUSR |S_IXUSR | (mode & 07777);
	stat.st_nlink = 2;
	stat.st_uid = uid;
	stat.st_gid = gid;
	stat.st_size = 0;
	stat.st_blksize = FUSE_BLKSIZE;
	stat.st_blocks = 0;
	stat.st_ctime = stat.st_mtime = stat.st_atime = 0;
	stat.st_ino = inum;

	fino = fuse_directory_new(ffs, NULL, &stat);
	if (!fino)
		return -ENOSPC;

//	fino->fi_ops = bb_oper;
	fino->fi_ops = fuse_opers;
	fino->fi_st.st_atime = fino->fi_st.st_mtime;

	name_len = strlen(fstype);
	fs = _sysio_fs_new(&fuse_fs_ops, 0, fstype, name_len, ffs);
	if (!fs) {
		err = -ENOMEM;
		goto error;
	}

	/* create root inode for system */
	rooti = _sysio_i_new(fs, &fino->fi_fileid,
			     &fino->fi_st, 
			     1,
			     &fuse_i_ops,
			     fino);
	if (!rooti) {
		err = -ENOMEM;
		goto error;
	}

	/* allocate and initialize a new base path node */
	rootpb = _sysio_pb_new(&noname, NULL, rooti);
	if (!rootpb) {
		err = -ENOMEM;
		goto error;
	}

	err = _sysio_do_mount(fs, rootpb, flags, tocover, &mnt);
	if (err) 
		goto error;
	
	*mntp = mnt;

	goto out;

error:
	if (mnt && _sysio_do_unmount(mnt) != 0)
		abort();

	if (rootpb) {
		_sysio_pb_gone(rootpb);
		rooti = NULL;
	}
	
	if (rooti)
		I_RELE(rooti);
		
	if (fs) {
		FS_RELE(fs);
		goto out;
	}

	if (fino) {
		fuse_i_destory(fino);
		goto out;
	}
	
	if (ffs) {
		free(ffs);
		goto out;
	}
out:
	return err;

}

/* Init the dirver */
int
_sysio_fuse_init()
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
  
  return _sysio_fssw_register("fuse", &fuse_fssw_ops);
}


static void *
fuse_directory_probe(void *data,
                     size_t size,
                     _SYSIO_OFF_T origin,
                     probe_ty entry,
                     probe_ty hole,
                     void *arg)
{
	struct intnl_dirent *de;
	void *p;
	size_t n;
	
	de = data;

	for (;;) {
    
    assert(de->d_reclen);
		if (entry && (p = (*entry)(de, de->d_reclen, arg))) {
			return p;
		}

		n = ((void *)de - data) + de->d_reclen;
	
		if (hole) {
			p = (*hole)((void *)de, de->d_reclen, arg);
			if (p) {
				return p;
			}
		}
	
		if (n >= size) {
			break;
		}
	
		de = (struct intnl_dirent *)((char *)data + n);

	}
	
	return NULL;

}

static struct intnl_dirent *
fuse_directory_match(struct intnl_dirent *de,
		                 size_t reclen, 
                     struct lookup_data *ld)
{
	size_t len;
		
#ifdef _DIRENT_HAVE_D_NAMLEN
  len = de->d_namlen;
#else
  {
    const char *cp, *end;
    
    cp = de->d_name;
    end = (const char *)de + reclen;
    while (cp < end && *cp != '\0')
      cp++;
    len = cp - de->d_name;
  }
#endif
  if (ld->name->len == len && 
      strncmp(de->d_name, 
        ld->name->name, ld->name->len) == 0)
    return de;
  
  ld->de = de;
  return NULL;

}

static struct intnl_dirent *
fuse_directory_best_fit(void *data, size_t len, struct lookup_data *ld)
{
	if (!ld->hole.len || len < ld->hole.len) {
		ld->hole.p = data;
		ld->hole.len = len;

	}

	return NULL;
}

	
	
static int 
fuse_inop_lookup(struct pnode *pno,
                 struct inode **inop, 
                 struct intent *intnt,
                 const char *path)
{
	struct inode *ino;
	struct intnl_dirent *de;
	struct fuse_inode *fino;
	struct file_identifier fileid;
	struct lookup_data lookup_data;	
	struct inode_ops *ops;
	struct filesys *fs;
	struct intnl_stat stat;

	if (*inop) {
		fino = I2FI(*inop);
		assert(fino);
		(*inop)->i_stbuf = fino->fi_st;
		return 0;

	}

	ino = pno->p_parent->p_base->pb_ino;
	
	fs = ino->i_fs;
	
	fino = I2FI(ino);
	
	FUSE_LD_INIT(&lookup_data,
		     ULONG_MAX,
		     &pno->p_base->pb_name);

	de = fuse_directory_probe(fino->fi_data, 
				  fino->fi_st.st_size, 
				  0,
				  (probe_ty )fuse_directory_match,
          NULL, 
          &lookup_data);

	if (!de) {
		return -ENOENT;
	}

	fileid.fid_data = &de->d_ino;
  fileid.fid_len = sizeof(de->d_ino);
	
  ino = _sysio_i_find(ino->i_fs, &fileid);
  if (ino)
    goto out;

  fino->fi_fileid.fid_data = &fino->fi_st.st_ino;
  fino->fi_fileid.fid_len = sizeof(fino->fi_st.st_ino);
  ops = &fuse_i_ops;
  if (!ops)
    abort();

	fino->fi_fileid.fid_data = &fino->fi_st.st_ino;
	fino->fi_fileid.fid_len = sizeof(fino->fi_st.st_ino);


  ino = _sysio_i_new(fs,&fino->fi_fileid, 
      &fino->fi_st, 1, ops, fino);
  
  if (!ino)
    return -ENOENT;

out:
  *inop = ino;
  return 0;

}

static int 
fuse_directory_insert(struct fuse_inode *parent,
                      struct qstr *name, 
                      ino_t inum,
                      unsigned char type)
{
	size_t reclen;
	struct lookup_data lookup_data;
	struct intnl_dirent *de;
	size_t xt;
	size_t n;
	size_t r;

	reclen = FUSE_D_RECLEN(name->len);
	FUSE_LD_INIT(&lookup_data, reclen, name);
	de = fuse_directory_probe(parent->fi_data, 
                            parent->fi_st.st_size,
                            0,
                            (probe_ty) fuse_directory_match,
                            (probe_ty) fuse_directory_best_fit,
                            &lookup_data);
	if (de)	{
		return -EEXIST;
	}

	de = lookup_data.de;
	xt = (char *)lookup_data.de - (char *)parent->fi_data;
#ifdef _DIRENT_HAVE_D_OFF
  de->d_off;
#else
  xt + de->d_reclen;
#endif
	r =
#ifdef _DIRENT_HAVE_D_OFF
      de->d_reclen;
#else
      FUSE_D_RECLEN(de->d_namlen);
#endif 
	
	if (!parent->fi_st.st_size || 
	    xt + r + reclen > (size_t) parent->fi_st.st_size) {
		int err;
	
		err = fuse_trunc(parent, xt + r+ reclen, 1);
		if (err)
			return err;
		
    de = (struct intnl_dirent *)((char *)parent->fi_data + xt);
		n = parent->fi_st.st_size;
	} 
	
#ifdef _DIRENT_HAVE_D_OFF
  de->d_off = xt + r; 	/* trim */
#else
  de->d_reclen = r;
#endif
  de = (struct intnl_dirent *)((char *)de + r);    	/* reposition */
  xt += r;

#ifndef _DIRENT_HAVE_D_OFF
  /*
   * Will we split this hole or use all of it?
   */
  if (lookup_data.hole.len - reclen &&
      lookup_data.hole.len - reclen <= FUSE_D_RECLEN(1))
    reclen = lookup_data.hole.len;
#endif
  
  /* 
   * Insert new
   */
  de->d_ino = inum;
#ifdef _DIRENT_HAVE_D_OFF
  de->d_off = n;
#endif
  
  de->d_reclen = reclen;
  de->d_type = type;
  (void )memcpy(de->d_name, name->name, name->len);

#ifdef _DIRENT_HAVE_D_NAMLEN
  de->d_namlen = name->len;
#endif

#ifndef _DIRENT_HAVE_D_OFF
  xt += reclen;
  if (n - xt) {
    /* 
     * White-out remaining part of the hole.
     */
    (void *)de += reclen;
    de->d_ino = 0;
    de->d_reclen = n - xt;
    de->d_type = DT_WHT;
    de->d_namlen = 0;
  }
#endif
  
  /* update attributes to new entry  */
	parent->fi_st.st_nlink++;
	assert(parent->fi_st.st_nlink);
  parent->fi_st.st_atime = parent->fi_st.st_mtime = time(NULL);
	
	return 0;

}


static int 
fuse_inop_getattr(struct pnode *pno,
                  struct inode *ino,
                  struct intnl_stat *stbuf)
{
	
	char *path;
  int err;
  struct fuse_inode *fino;
  
  path = _sysio_pb_path(pno->p_base, '/');

  fino = P2FI(pno);
  if (!fino) {
    fprintf(stderr, "no fuse inode found from pno\n");
    return -ENOENT;
  }
  
  err = (*fino->fi_ops.getattr)(path, stbuf);
  return err;

}


static int fuse_inop_setattr(struct pnode *pno, 
                             struct inode *ino,
                             unsigned mask, 
                             struct intnl_stat *stbuf){}

static ssize_t fuse_filldirentries(struct inode *ino,
                                   _SYSIO_OFF_T *posp,
                                   char *buf, 
                                   size_t nbytes){}


static int 
fuse_unlink_entry(struct fuse_inode *parent, 
                  struct qstr *name) 
{
	size_t reclen;
	struct lookup_data lookup_data;
	struct intnl_dirent *de;
	size_t off;

	reclen = FUSE_D_RECLEN(name->len);
	FUSE_LD_INIT(&lookup_data, 0, name);
	de = fuse_directory_probe(parent->fi_data, 
                            parent->fi_st.st_size,
                            0,
                            (probe_ty) fuse_directory_match, 
                            NULL, 
                            &lookup_data);
	if (!de)	{
		return -ENOENT;
	}

	reclen = de->d_reclen;
	memset(de, 0, reclen);
	
	de->d_reclen = reclen;
	
	assert(parent->fi_st.st_nlink > 2);
	parent->fi_st.st_nlink--;
	return 0;

}



static int 
fuse_inop_mkdir(struct pnode *pno, mode_t mode)
{
	struct intnl_stat stat;
	struct fuse_inode *fino, *parent;
	ino_t inum;
	int err;
	struct intnl_dirent *de = NULL;
	struct inode *ino;
	char *path;

	ino = pno->p_parent->p_base->pb_ino;
	parent = I2FI(ino);

	if (!S_ISDIR(parent->fi_st.st_mode))
		return -ENOTDIR;

	(void)memset(&stat, 0 , sizeof(stat));
	stat.st_dev = pno->p_parent->p_base->pb_ino->i_fs->fs_dev;
	inum = fuse_inum_alloc();

	stat.st_mode = S_IFDIR | (mode & 07777);
	stat.st_nlink = 2;
	stat.st_uid = getuid();
	stat.st_gid = getgid();
	stat.st_size = 0;
	stat.st_blksize = FUSE_BLKSIZE;
	stat.st_blocks = 0;
	stat.st_ctime = stat.st_mtime = stat.st_atime = 0;
	stat.st_ino = inum;
	fino = fuse_directory_new(FS2FFS(ino->i_fs), parent, &stat);
	if (!fino)
		return -ENOSPC;

	ino = _sysio_i_new(pno->p_parent->p_base->pb_ino->i_fs, 
                     &fino->fi_fileid, 
                     &stat, 
                     1, 
                     &fuse_i_ops, 
                     fino);
	if (!ino) {
		fuse_i_destory(fino);
		return -ENOMEM;
	}

	err = fuse_directory_insert(parent, 
                              &pno->p_base->pb_name, 
                              stat.st_ino,  
                              FUSE_D_TYPEOF(S_IFDIR));

	if (err) {
		de->d_ino = 0;
		I_RELE(ino);
		_sysio_i_gone(ino);
		return err;
	}
	

	path = _sysio_pb_path(pno->p_base, '/');
	if (!path)
		return -ENOMEM;


	err = (*fino->fi_ops.mkdir)(path, mode);
	if (err) {
		fprintf(stderr, "mkdir from fuse failed\n");
		de->d_ino = 0;
		fuse_i_destory(fino);
		I_RELE(ino);
		return err;
	}

	pno->p_base->pb_ino = ino;
		
	return err;

}


static int 
fuse_inop_rmdir(struct pnode *pno)
{
	int err;
	char *path;
	struct fuse_inode *fino;
	struct inode *ino;

	path = _sysio_pb_path(pno->p_base, '/');
	if (!path)
		return -ENOMEM;

	err = fuse_unlink_entry(I2FI(pno->p_parent->p_base->pb_ino), 
                          &pno->p_base->pb_name);

	fino = P2FI(pno);
	err = (*fino->fi_ops.rmdir)(path);	
	if (err) {
		fprintf(stderr, "rmdir from fuse failed\n");
		return err;
	}

	free(path);
	return err;
}

static int fuse_inop_symlink(struct pnode *pno, const char *data){}

static int fuse_inop_readlink(struct pnode *pno, char *buf, size_t bufsiz){}

static int 
fuse_create(struct pnode *pno, struct intnl_stat *stat, struct inode **inop)
{
	struct inode *dino, *ino;
	struct fuse_inode *fino;
	int err;

	dino = pno->p_parent->p_base->pb_ino;
	assert(dino);
	
	fino = fuse_i_alloc(FS2FFS(dino->i_fs), stat, &(I2FI(dino)->fi_ops));
	if (!fino)
		return -ENOSPC;

	/* add new inode to the system */
	ino = _sysio_i_new(dino->i_fs, 
                     &fino->fi_fileid, 
                     stat, 
                     1, 
                     &fuse_i_ops, 
                     fino);
	
  pno->p_base->pb_ino = ino;

	*inop = ino;

	return 0;
}


static int 
fuse_inop_open(struct pnode *pno, 
               int flags, 
               mode_t mode)
{
	struct fuse_file_info *fi_t = malloc(sizeof(struct fuse_file_info));
	int err;	
	struct intnl_stat stat;
	ino_t inum;
	struct fuse_inode *fino;

	/* file exists  */
	if (!pno->p_base->pb_ino) {
		(void)memset(&stat, 0, sizeof(stat));
		stat.st_dev = pno->p_parent->p_base->pb_ino->i_fs->fs_dev;
		inum = fuse_inum_alloc();
		stat.st_mode = S_IFREG | (mode & 07777);
		stat.st_nlink = 1;
		stat.st_uid = getuid();
		stat.st_gid = getgid();
		stat.st_rdev = 0;
		stat.st_size = 0;
		stat.st_blksize = FUSE_BLKSIZE;
		stat.st_blocks = 0;
		stat.st_ctime = stat.st_mtime = stat.st_atime = 0;
		stat.st_ino = inum;
		fuse_create(pno, &stat, &pno->p_base->pb_ino);
		fino = P2FI(pno);
		fino->fi_path  = _sysio_pb_path(pno->p_base, '/');
		if (!(fino->fi_path))
			return -ENOMEM;
		fino = P2FI(pno);
		if ((flags & O_CREAT) ) {
			if (*fino->fi_ops.mknod)
				err = (*fino->fi_ops.mknod)(fino->fi_path, mode, 0);
			else
				fprintf(stderr, "no mknod implemented in the backend\n");
		} 
	}
	else {
		fino = P2FI(pno);
		fino->fi_path  = _sysio_pb_path(pno->p_base, '/');
	}


	fi_t->flags = flags;
	
	err = (*fino->fi_ops.open)(fino->fi_path, fi_t);

	
	((struct fuse_inode *)(pno->p_base->pb_ino)->i_private)->fi_file = fi_t;
	
	return err;


}

static int 
fuse_inop_ftruncate(struct pnode *pno, size_t size)
{
	char *path;
	int err;
	struct fuse_inode *fino;
	struct fuse_file_info *fi;

	path = _sysio_pb_path(pno->p_base, '/');

	fino = P2FI(pno);
	if (!fino) {
		fprintf(stderr, "no fuse inode found from pno\n");
		return -ENOENT;
	}
	
	fi = fino->fi_file;

	err = (*fino->fi_ops.ftruncate)(path, size, fino->fi_file);
	
	return err;
}


static int 
fuse_inop_close(struct pnode *pno)
{
	char *path;
	int err;
	struct fuse_inode *fino;
	struct fuse_file_info *fi;

	path = _sysio_pb_path(pno->p_base, '/');

	fino = P2FI(pno);
	if (!fino) {
		fprintf(stderr, "no fuse inode found from pno\n");
		return -ENOENT;
	}
	
	fi = fino->fi_file;

	err = (*fino->fi_ops.release)(path, fi);

	return err;
}

static int fuse_inop_link(struct pnode *old, struct pnode *new){}

static int fuse_inop_unlink(struct pnode *pno){
	char *path;
	int err;
	struct fuse_inode *fino;

	path = _sysio_pb_path(pno->p_base, '/');

	fino = P2FI(pno);
	if (!fino) {
		fprintf(stderr, "no fuse inode found from pno\n");
		return -ENOENT;
	}

	err = (*fino->fi_ops.unlink)(path);
	
	free(path);
	return err;

}
static int fuse_inop_rename(struct pnode *old, struct pnode *new){}



static int
fuse_io(ssize_t (*f)(void *, size_t, _SYSIO_OFF_T, struct fuse_inode *), 
	struct pnode *pno,  
	struct ioctx *ioctx)
{
  struct iovec *iov;
	struct intnl_xtvec *xtv;
	int err;

	iov = ioctx->ioctx_iov;
	xtv = ioctx->ioctx_xtv;

	ioctx->ioctx_cc
		 = (*f)(iov->iov_base, iov->iov_len, 0, pno);
	ioctx->ioctx_done = 1;

	return 0;

}

static int
fuse_inop_read(void *buf, size_t nbytes, 
               _SYSIO_OFF_T off, 
               struct pnode *pno)
{
	struct fuse_inode *fino;

	fino = P2FI(pno);
	if (!fino) {
		fprintf(stderr, "no inode found from pno\n");
		return -ENOENT;
	}
	
	return (*fino->fi_ops.read)(fino->fi_path, buf, nbytes, off, fino->fi_file);
	
}

static int
fuse_inop_write(void *buf, size_t nbytes,
	   _SYSIO_OFF_T off, 
	   struct pnode *pno)
{
	struct fuse_inode *fino;
	
	fino = P2FI(pno);
	if (!fino) {
		fprintf(stderr, "no inode found from pno\n");
		return -ENOENT;
	}
	
	return (*fino->fi_ops.write)(fino->fi_path, buf, nbytes, off, fino->fi_file);

}

static _SYSIO_OFF_T fuse_inop_pos(struct inode *ino, _SYSIO_OFF_T off){}
static int fuse_inop_iodone(struct ioctx *ioctx){}
static int fuse_inop_fcntl(struct inode *ino, int cmd, va_list ap, int *rtn){}

static int fuse_inop_sync(struct pnode *pno)
{
	int err;
	char *path;
	struct fuse_file_info *fi;
	struct fuse_inode *fino;

	path = _sysio_pb_path(pno->p_base, '/');
	
	fino = P2FI(pno);

	fi = fino->fi_file;

	err = (*fino->fi_ops.fsync)(path, 0, fi);

	return err;	
}

static int fuse_inop_datasync(struct pnode *pno)
{
	int err;
	char *path;
	struct fuse_file_info *fi;
	struct fuse_inode *fino;

	path = _sysio_pb_path(pno->p_base, '/');
	
	fino = P2FI(pno);

	fi = fino->fi_file;

	err = (*fino->fi_ops.fsync)(path, 1, fi);

	return err;
}

	
static int fuse_inop_ioctl(struct inode *ino, 
                           unsigned long int request, 
                           va_list ap){}

static int fuse_inop_mknod(struct pnode *pno, mode_t mode, dev_t dev){}

#ifdef _HAVE_STATVFS
static int fuse_inop_statvfs(struct pnode *pno,
                             struct inode *ino,
                             struct intnl_statvfs *buf){}
#endif
static void 
fuse_inop_gone(struct inode *ino)
{
	struct fuse_inode *fino = I2FI(ino);
	
	fuse_i_destory(fino);
	
	return 0;
}

