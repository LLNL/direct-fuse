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
 *    This Cplant(TM) source code is the property of Sandia National
 *    Laboratories.
 *
 *    This Cplant(TM) source code is copyrighted by Sandia National
 *    Laboratories.
 *
 *    The redistribution of this Cplant(TM) source code is subject to the
 *    terms of the GNU Lesser General Public License
 *    (see cit/LGPL or http://www.gnu.org/licenses/lgpl.html)
 *
 *    Cplant(TM) Copyright 1998-2004 Sandia Corporation. 
 *    Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
 *    license for use of this work by or on behalf of the US Government.
 *    Export of this program may require a license from the United States
 *    Government.
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
 * Questions or comments about this library should be sent to:
 *
 * Lee Ward
 * Sandia National Laboratories, New Mexico
 * P.O. Box 5800
 * Albuquerque, NM 87185-1110
 *
 * lee@sandia.gov
 */

#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/queue.h>

#include "sysio.h"
#include "inode.h"
#include "sysio-symbols.h"

#ifdef HAVE_POSIX_1003_READLINK
ssize_t
#else
int
#endif
SYSIO_INTERFACE_NAME(readlink)(const char *path, char *buf, size_t bufsiz)
{
	struct intent intent;
	int	err;
	struct pnode *pno;
	struct inode *ino;
	SYSIO_INTERFACE_DISPLAY_BLOCK;

	SYSIO_INTERFACE_ENTER;
	INTENT_INIT(&intent, INT_GETATTR, NULL, NULL);
	err = _sysio_namei(_sysio_cwd, path, ND_NOFOLLOW, &intent, &pno);
	if (err)
		goto out;
	ino = pno->p_base->pb_ino;
	if (!S_ISLNK(ino->i_stbuf.st_mode)) {
		err = -EINVAL;
		goto error;
	}
	err = (*ino->i_ops.inop_readlink)(pno, buf, bufsiz);
error:
	P_RELE(pno);
out:
	SYSIO_INTERFACE_RETURN(err < 0 ? -1 : err, err >= 0 ? 0 : err);
}

#ifdef REDSTORM
#undef __readlink
sysio_sym_weak_alias(SYSIO_INTERFACE_NAME(readlink),
		     PREPEND(__, SYSIO_INTERFACE_NAME(readlink)))
#endif
