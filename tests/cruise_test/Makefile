CC=mpicc
SYSIO_HOME=../..
CRUISE_HOME=../../../cruise
FUSE_HOME=/data/yzhu/fuse-2.9.7
GLFS_HOME=../../../xglfs

GLFS_LIBS=-L$(GLFS_HOME)/test-build -lxglfs -lgfapi -lrt
CRUISE_LIBS=-L$(CRUISE_HOME)/install/lib -lcruise-posix -pthread

INCLUDES=-I$(SYSIO_HOME)/include  \
	-I$(SYSIO_HOME)/dev/stdfd \
	-I$(SYSIO_HOME)/drivers/bbfs \
	-I$(SYSIO_HOME)/drivers/sshfs \
	-I$(SYSIO_HOME)/drivers/fuse \
	-I$(SYSIO_HOME)/drivers/curlftpfs\
	-I$(SYSIO_HOME)/drivers/glusterfs \
	-I$(SYSIO_HOME)/drivers/cruise\
	-DSTDC_HEADERS=1 \
	-DTIME_WITH_SYS_TIME=1 \
	-D_XOPEN_SOURCE=600 \
	-DHAVE_POSIX_1003_READLINK=1 \
	-D_LARGEFILE64_SOURCE=1 \
	-DHAVE_POSIX2008_PREADV=1 \
	-DHAVE_ASM_WEAK_DIRECTIVE=1 \
	-I. \
	-DSYSIO_TRACING=1 \
	-DSTDFD_DEV=1 \
	-DHAVE_POSIX2008_SCANDIR=1 \
	-I$(CRUISE_HOME)/src
	
FUSE_LIBS=-L$(FUSE_HOME)/install/lib -lfuse  -lglib-2.0 
FUSE_INCLUDES=-I$(FUSE_HOME)/include \
        -D_FILE_OFFSET_BITS=64 \
        -DFUSE_USE_VERSION=26 \
	
SYSIO_INCLUDE=$(SYSIO_HOME)/include

LIBS=-L$(SYSIO_HOME) -lsysio -lpthread -lgthread-2.0 -lcurl $(CRUISE_LIBS) $(FUSE_LIBS) $(GLFS_LIBS)

cruise_wr.o:cruise_wr.c
	$(CC) $(FUSE_INCLUDES) $(INCLUDES) cruise_wr.c -o cruise_wr $(LIBS) $(FUSE_LIBS)

clean:
	rm cruise_wr
test: 
	make clean
	make
