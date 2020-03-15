CC=gcc
#CFLAGS=-fpic -g -O2 
SYSIO_HOME=.
FUSE_HOME=/data/yzhu/fuse-2.9.7#replace to your FUSE_HOME (this path is for inv09)
CRUISE_HOME=../cruise#your cruise path
GLFS_HOME=../xglfs#your xglfs path

SYSIO_SRC=$(SYSIO_HOME)/src
SYSIO_DEV=$(SYSIO_HOME)/dev/stdfd
SYSIO_BBFS=$(SYSIO_HOME)/drivers/bbfs
SYSIO_SSHFS=$(SYSIO_HOME)/drivers/sshfs
SYSIO_CRUISE=$(SYSIO_HOME)/drivers/cruise
SYSIO_FUSE=$(SYSIO_HOME)/drivers/fuse
SYSIO_FTPFS=$(SYSIO_HOME)/drivers/curlftpfs
SYSIO_GLFS=$(SYSIO_HOME)/drivers/glusterfs

CRUISE_LIBS=-L$(CRUISE_HOME)/install/lib -lcruise-posix -pthread
CRUISE_INCLUDES=-I$(CRUISE_HOME)/src \
		-I$(CRUISE_HOME)

FUSE_LIBS=-lglib-2.0  -L$(FUSE_HOME)/install/lib -lfuse -lgthread-2.0
FUSE_INCLUDES=-I$(FUSE_HOME)/include \
	-D_FILE_OFFSET_BITS=64 \
	-DFUSE_USE_VERSION=26 \
	-I/usr/include/glib-2.0 \
	-I/usr/lib64/glib-2.0/include  

FTPFS_LIBS=-lcurl

GLFS_LIBS=-L$(GLFS_HOME)/test-build  -lxglfs -lgfapi -lrt

LIBS=$(FUSE_LIBS) $(CRUISE_LIBS) $(FTPFS_LIBS) $(GLFS_LIBS)

INCLUDES=-I$(SYSIO_HOME)/include  \
        -I$(SYSIO_DEV) \
	-I$(SYSIO_BBFS) \
	-I$(SYSIO_SSHFS) \
	-I$(SYSIO_FUSE) \
	-I$(SYSIO_FTPFS) \
        -I$(SYSIO_CRUISE) \
	-I$(SYSIO_GLFS) \
	-I$(GLFS_HOME) \
	-DSTDC_HEADERS=1 \
        -DTIME_WITH_SYS_TIME=1 \
        -D_XOPEN_SOURCE=600 \
        -DHAVE_POSIX_1003_READLINK=1 \
        -D_LARGEFILE64_SOURCE=1 \
        -DHAVE_POSIX2008_PREADV=1 \
        -DHAVE_POSIX2008_SCANDIR=1 \
	-DHAVE_ASM_WEAK_DIRECTIVE=1 \
	$(FUSE_INCLUDES) \
	$(CRUISE_INCLUDES) \
        -DSYSIO_TRACING=1 \
        -DSTDFD_DEV=1 


CFLAGS=-fpic -g -O2 $(INCLUDES)

all: libsysio.a 
#all: libsysio.so

SYSIO_OBJS=	\
	$(SYSIO_SRC)/access.o \
	$(SYSIO_SRC)/chdir.o \
	$(SYSIO_SRC)/chmod.o \
	$(SYSIO_SRC)/chown.o \
	$(SYSIO_SRC)/dev.o \
	$(SYSIO_SRC)/dup.o \
        $(SYSIO_SRC)/fcntl.o \
        $(SYSIO_SRC)/fs.o \
	$(SYSIO_SRC)/fsync.o \
        $(SYSIO_SRC)/getdirentries.o \
        $(SYSIO_SRC)/init.o \
        $(SYSIO_SRC)/inode.o \
        $(SYSIO_SRC)/ioctl.o \
	$(SYSIO_SRC)/ioctx.o \
        $(SYSIO_SRC)/iowait.o \
	$(SYSIO_SRC)/link.o \
	$(SYSIO_SRC)/lseek.o \
	$(SYSIO_SRC)/mkdir.o \
	$(SYSIO_SRC)/mknod.o \
	$(SYSIO_SRC)/mount.o \
	$(SYSIO_SRC)/namei.o \
	$(SYSIO_SRC)/open.o \
	$(SYSIO_SRC)/rw.o \
	$(SYSIO_SRC)/reconcile.o \
	$(SYSIO_SRC)/rename.o \
	$(SYSIO_SRC)/rmdir.o \
	$(SYSIO_SRC)/stat64.o \
	$(SYSIO_SRC)/stat.o \
	$(SYSIO_SRC)/stddir.o \
	$(SYSIO_SRC)/readdir.o \
	$(SYSIO_SRC)/readdir64.o \
	$(SYSIO_SRC)/symlink.o \
	$(SYSIO_SRC)/readlink.o \
	$(SYSIO_SRC)/truncate.o \
	$(SYSIO_SRC)/unlink.o \
	$(SYSIO_SRC)/utime.o \
	$(SYSIO_SRC)/file.o \
	$(SYSIO_DEV)/stdfd.o 

SYSIO_FUSE_OBJS = $(SYSIO_FUSE)/sysio_fuse.o

BBFS_OBJS= \
	$(SYSIO_BBFS)/fuse_bbfs.o \
	$(SYSIO_BBFS)/bbfs.o \
	$(SYSIO_BBFS)/log.o 

SSHFS_OBJS = \
	$(SYSIO_SSHFS)/fuse_sshfs.o \
	$(SYSIO_SSHFS)/sshfs.o \
	$(SYSIO_SSHFS)/cache.o \

CRUISE_OBJS = $(SYSIO_CRUISE)/fuse_cruise.o

FTPFS_OBJS = \
	$(SYSIO_FTPFS)/fuse_ftpfs.o \
	$(SYSIO_FTPFS)/charset_utils.o \
	$(SYSIO_FTPFS)/ftpfs.o \
	$(SYSIO_FTPFS)/ftpfs-ls.o \
	$(SYSIO_FTPFS)/path_utils.o \
	$(SYSIO_FTPFS)/curlftpfs_cache.o


GLFS_OBJS = \
	$(SYSIO_GLFS)/fuse_glfs.o

OBJ = $(SYSIO_OBJS) $(SYSIO_FUSE_OBJS) $(BBFS_OBJS) $(SSHFS_OBJS) $(CRUISE_OBJS) $(FTPFS_OBJS) $(GLFS_OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LIBS)

libsysio.a: $(OBJ) 
	ar rcs $@ $^

libsysio.so: $(OBJS) $(SYSIO_FUSE_OBJS) $(BBFS_OBJS) $(SSHFS_OBJS) $(CRUISE_OBJS) $(GLFS_OBJS)
	$(CC) -shared $^ -o $@
clean:
	rm $(SYSIO_SRC)/*.o
	rm $(SYSIO_BBFS)/*.o
	rm $(SYSIO_FUSE)/*.o
	rm $(SYSIO_FTPFS)/*.o
	rm $(SYSIO_SSHFS)/*.o
	rm $(SYSIO_CRUISE)/*.o 
	rm $(SYSIO_DEV)/*.o
	rm libsysio.a
#	rm libsysio.so

cleanall:
	rm $(SYSIO_SRC)/*.o
	rm $(SYSIO_BBFS)/*.o
	rm $(SYSIO_FUSE)/*.o
	rm $(SYSIO_FTPFS)/*.o
	rm $(SYSIO_SSHFS)/*.o
	rm $(SYSIO_CRUISE)/*.o 
	rm $(SYSIO_DEV)/*.o
	rm libsysio.a
	rm ./tests/bbfs_test/*.o
	rm ./tests/sshfs_test/*.o
	rm ./tests/ftpfs_test/*.o
	rm ./tests/cruise_test/*.o
	rm ./tests/glfs_test/*.o
#	rm libsysio.so




















