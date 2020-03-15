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
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/queue.h>
#include <sys/time.h>

#include "sysio.h"
/*
 * Write/read data to/from remote node(input size KB)
 *
 * Usage: ./bbfs_wr <size>
 *
 */
#include <time.h>

int init_buffer(char* buf, size_t size, int ckpt)
{
  size_t i;
  for(i=0; i < size; i++) {
    char c = 'a' + (char)((ckpt + i) & 32);
    buf[i] = c;
  }
  return 0;
}


int check_buffer(char* buf, size_t size, int ckpt)
{
  size_t i;
  for(i=0; i < size; i++) {
    char c = 'a' + (char)((ckpt + i) & 32);
    if (buf[i] != c) {
      printf("check failed at byte %d, should be %c is %c\n", (int)i, c, buf[i]);
      return 0;
    }
  }
  return 1;
}


int
main(int argc, char * const argv[])
{
	int	rc, err, size, fd;
	const char *fpath = "sshfs:/sshfs/test.txt";
	struct timeval start, end;
	double time;
	char *data;
	char *read_data;


	if (argc < 2) {
		printf("./sshfs_wr <size>\n");
		return 0;
	}

	err = _sysio_all_startup();
	if (err) {
		fprintf(stderr, "sysio startup\n");
		return 0;
	}
	

	size = atoi(argv[1]);

	size = size * 1024;
	data = malloc(sizeof(char) * size);
	init_buffer(data, size, 0);

	read_data = malloc(sizeof(char) * size);
	memset(read_data, 0, size);
	err = SYSIO_INTERFACE_NAME(mount)("yzhu@inv10:/data/yzhu/bbfs", "/sshfs/", "sshfs", 2, NULL);
	if (err) {
		fprintf(stderr, "mount bbfs failed\n");
		return 0;
	}

	fd = SYSIO_INTERFACE_NAME(open)(fpath, O_CREAT|O_WRONLY|O_SYNC|O_TRUNC, 0644);
	if (fd < 0)
		printf("bbfs open failed\n");	

	gettimeofday(&start, NULL);
	rc = SYSIO_INTERFACE_NAME(write)(fd, data, size);
	if (rc < 0) {
		fprintf(stderr, "sshfs write failed\n");
		return 0;
	}
	
	err = SYSIO_INTERFACE_NAME(fsync)(fd);
	if (err < 0) {
		fprintf(stderr, "sshfs fsync failed\n");
		return 0;
	}
	gettimeofday(&end, NULL);	
        time = end.tv_sec + (end.tv_usec/1000000.0) - start.tv_sec - (start.tv_usec/1000000.0);
  
	printf("bbfs write %d bytes %lf seconds\n", rc, time);
	
	err = SYSIO_INTERFACE_NAME(close)(fd);
	if (err < 0) {
		fprintf(stderr, "sshfs close failed\n");
		return 0;
	}
	
	fd = SYSIO_INTERFACE_NAME(open)(fpath, O_RDONLY);
	if (fd < 0) {
		printf("bbfs read open failed\n");	
		return 0;
	}

	gettimeofday(&start, NULL);	
	err = SYSIO_INTERFACE_NAME(read)(fd, read_data, size);	
	if (err < 0) {
		fprintf(stderr, "sshfs read failed\n");
		return 0;
	}
	check_buffer(read_data, size, 0);

	gettimeofday(&end, NULL);	
	time = end.tv_sec + (end.tv_usec/1000000.0) - start.tv_sec - (start.tv_usec/1000000.0);
	printf("bbfs read %d bytes %lf seconds\n", err, time);

	
	err = SYSIO_INTERFACE_NAME(close)(fd);
	if (err < 0) {
		fprintf(stderr, "sshfs close failed\n");
		return 0;
	}
	err = SYSIO_INTERFACE_NAME(unlink)(fpath);
	if (err < 0) {
		fprintf(stderr, "sshfs unlink failed\n");
		return 0;
	}
		
	SYSIO_INTERFACE_NAME(umount)("sshfs:/sshfs");

	if (read_data)
		free(read_data);
	if (data)
		free(data);

	return 0;

}

