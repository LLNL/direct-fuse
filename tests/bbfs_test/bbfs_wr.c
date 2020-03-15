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
 * Write 1 GB data with different transfer sizes (input size KB)
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


int main(int argc, char * const argv[])
{
	int	i;
	int	err;
	const char *fpath = "bbfs:/b_fuse/test.txt";
	struct timeval start, end;
	double time;
  struct stat statbuf;

	if (argc < 2) {
		printf("./bbfs_wr <size> <tot_size>\n");
		return 0;
	}

	err = _sysio_all_startup();
	if (err) {
		fprintf(stderr, "sysio startup\n");
		return 0;
	}
	

	int size = atoi(argv[1]);
  long tot_size = atol(argv[2]);
	int num = tot_size/size;

	char *data;
	char *read_data;
	int fd;

  printf("total data size: %ld, transfer size: %d\n", size, tot_size);
	data = malloc(sizeof(char) * size);
	init_buffer(data, size, 0);

	read_data = malloc(sizeof(char) * size);
	memset(read_data, 0, size);
	err = SYSIO_INTERFACE_NAME(mount)("/data/yzhu/bbfs", "/b_fuse/", "bbfs", 2, NULL);
	if (err) {
		fprintf(stderr, "mount bbfs failed\n");
		return 0;
	}

	int rc;
	fd = SYSIO_INTERFACE_NAME(open)(fpath, O_CREAT|O_WRONLY|O_SYNC|O_TRUNC, 0644);
	if (fd < 0)
		fprintf(stderr, "bbfs open failed\n");	
	int offset = 0;
  
  printf("file %s has created, fd: %d\n", fpath, fd);

  err = SYSIO_INTERFACE_NAME(ftruncate)(fd, tot_size);
  if (err < 0) {
    fprintf(stderr, "ftrunate failed\n");
    return 0;
  } else {
    fprintf("truncated file to %zu\n", size);
  }

  SYSIO_INTERFACE_NAME(close)(fd);

	fd = SYSIO_INTERFACE_NAME(open)(fpath, O_WRONLY);
	if (fd < 0)
		fprintf(stderr, "bbfs open failed\n");	

	gettimeofday(&start, NULL);
	for(i = 0; i < num; i++) {
		rc = SYSIO_INTERFACE_NAME(pwrite)(fd, data, size, offset);
		offset += size;
	
	}
	err = SYSIO_INTERFACE_NAME(fsync)(fd);
	gettimeofday(&end, NULL);	
        time = end.tv_sec + (end.tv_usec/1000000.0) - start.tv_sec - (start.tv_usec/1000000.0);
  
	printf("bbfs %d times write %d bytes %lf seconds\n", i, rc, time);
	
	SYSIO_INTERFACE_NAME(close)(fd);

  err = SYSIO_INTERFACE_NAME(stat)(fpath, &statbuf);
  if (err < 0) {
    fprintf(stderr, "stat %s error: %d\n", fpath, errno);
  } 

  printf("stat file size: %zu\n", statbuf.st_size);

	fd = SYSIO_INTERFACE_NAME(open)(fpath, O_RDONLY);
	if (fd < 0)
		fprintf(stderr, "bbfs read open failed\n");	
	else printf("bbfs read open correct\n");

	
	offset = 0;
	gettimeofday(&start, NULL);	
	for (i = 0; i < num; i++){
		err = SYSIO_INTERFACE_NAME(pread)(fd, read_data, size, offset);	
		offset += size;
	}
	gettimeofday(&end, NULL);	
	time = end.tv_sec + (end.tv_usec/1000000.0) - start.tv_sec - (start.tv_usec/1000000.0);
	printf("bbfs %d times read %d bytes %lf seconds\n", i, err, time);

	
	SYSIO_INTERFACE_NAME(close)(fd);
	SYSIO_INTERFACE_NAME(unlink)(fpath);

	SYSIO_INTERFACE_NAME(umount)("bbfs:/b_fuse");

	if (read_data)
		free(read_data);
	if (data)
		free(data);

	return 0;

}

