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
 */

#ifdef __linux__
#include <features.h>
#if defined(__GLIBC__) && !defined(REDSTORM) 

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <sysio.h>

#include "sysio-symbols.h"

#ifndef _READDIR
#define _READDIR SYSIO_INTERFACE_NAME(readdir)
#define _SCANDIR SYSIO_INTERFACE_NAME(scandir)
#define _GETDIRENTRIES SYSIO_INTERFACE_NAME(getdirentries)
#define _DIRENT_T struct dirent
#define _OFF_T off_t
#endif

#include "stddir.h"

_DIRENT_T *
_READDIR(DIR *dir)
{
	_DIRENT_T *dp = NULL;
	_OFF_T dbase;

#ifndef BSD
	ssize_t rc;
#else
	int rc;
#endif
	SYSIO_INTERFACE_DISPLAY_BLOCK;

	SYSIO_INTERFACE_ENTER;

	/* need to read new data? */
	rc = 0;
	if (dir->cur >= dir->effective) {
		dir->cur = 0;
		dbase = (_OFF_T )dir->base;
		if (sizeof(dbase) != sizeof(dir->base) &&
		    dbase != dir->base) {
			dir->effective = 0;
			SYSIO_INTERFACE_RETURN(NULL, -EOVERFLOW);
		}
		rc = _GETDIRENTRIES(dir->fd, 
				    dir->buf, 
#ifndef BSD
				    (size_t )BUFSIZE, 
				    (_OFF_T *) &dbase);
#else
				    (int )BUFSIZE, 
				    (long *) __restrict dbase);
#endif
		dir->base = (_SYSIO_OFF_T )dbase;

		/* error or end-of-file */
		if (rc == -ENOENT)
			rc = 0;
		if (rc <= 0) {
			dir->effective = 0;
			SYSIO_INTERFACE_RETURN(NULL, rc);
		}
		dir->effective = rc;
	}
	dp = (_DIRENT_T *)(dir->buf + dir->cur);

#ifdef _DIRENT_HAVE_D_RECLEN
	dir->cur += dp->d_reclen;
#else
	dir->cur += sizeof(_DIRENT_T);
#endif
#ifdef _DIRENT_HAVE_D_OFF
	dir->filepos = dp->d_off;
#else
	dir->filepos = dir->cur;
#endif

	SYSIO_INTERFACE_RETURN(dp, 0);
}

sysio_sym_weak_alias(_READDIR, PREPEND(__,_READDIR))

int
_SCANDIR(const char *dirname, 
	 _DIRENT_T ***namelist, 
	 int (*filter) (const _DIRENT_T *), 
#ifdef HAVE_POSIX2008_SCANDIR
	 int(*compar)(const _DIRENT_T **, const  _DIRENT_T **)
#else
	 int(*compar)(const void *, const void *)
#endif
	)
{
	DIR *dir = NULL;
	_DIRENT_T *de 	  = NULL,
		  *nextde = NULL,
		  **s 	  = NULL;
	int n = 32, i = 0;
	size_t desize;
	SYSIO_INTERFACE_DISPLAY_BLOCK;

	SYSIO_INTERFACE_ENTER;

	if ((dir = SYSIO_INTERFACE_NAME(opendir)(dirname)) == NULL)
		SYSIO_INTERFACE_RETURN(-1, -errno);

	while ((de = _READDIR(dir)) != NULL) {
		if ((filter == NULL) || filter(de)) {
			if (i == 0 || i >= n) {
			    n = MAX(n, 2*i);
			    s = (_DIRENT_T **)realloc(s, 
			    	(size_t )(n * sizeof(_DIRENT_T *)));
			    if (!s) 
				SYSIO_INTERFACE_RETURN(-1, -ENOMEM);
			}
			desize = &de->d_name[_D_ALLOC_NAMLEN(de)] - (char * )de;
			nextde = (_DIRENT_T *)malloc(desize); 
			if (!nextde)
				SYSIO_INTERFACE_RETURN(-1, -ENOMEM);

			s[i++] = (_DIRENT_T *)memcpy(nextde, de, desize);
		}
	}
	if (compar)
		qsort (s,
		       i,
		       sizeof (*s),
		       (int (*)(const void *, const void *))compar);

	*namelist = s;

	SYSIO_INTERFACE_NAME(closedir)(dir);

	SYSIO_INTERFACE_RETURN(i, 0);
}

sysio_sym_weak_alias(_SCANDIR, PREPEND(__,_SCANDIR))

#endif
#endif
