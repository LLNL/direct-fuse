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
 *    Cplant(TM) Copyright 1998-2006 Sandia Corporation. 
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

#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/queue.h>

#include "sysio.h"
#include "inode.h"
#include "fs.h"
#include "mount.h"
#include "sysio-symbols.h"

int
_sysio_mkdir(struct pnode *pno, mode_t mode)
{
	int	err;
	struct inode *parenti;

	if (pno->p_base->pb_ino) 
		return -EEXIST;

	err = _sysio_permitted(pno->p_parent, W_OK);
	if (err)
		return err;

	parenti = pno->p_parent->p_base->pb_ino;
	assert(parenti);
	return (*parenti->i_ops.inop_mkdir)(pno, mode);
}

int
SYSIO_INTERFACE_NAME(mkdir)(const char *path, mode_t mode)
{
	int	err;
	struct intent intent;
	struct pnode *pno;
	struct mount *mnt;
	SYSIO_INTERFACE_DISPLAY_BLOCK;

	SYSIO_INTERFACE_ENTER;
	err = _sysio_find_mount(path, &mnt);
	if (err)
		goto out;
	path = path + mnt->mnt_fs->name_len + 1;

	INTENT_INIT(&intent, INT_CREAT, &mode, NULL);
//	err = _sysio_namei(_sysio_cwd, path, ND_NEGOK, &intent, &pno);
	err = _sysio_namei(mnt->mnt_root, path, ND_NEGOK, &intent, &pno);
	if (err)
		goto out;


	mode &= ~(_sysio_umask & 0777);			/* apply umask */
	err = _sysio_mkdir(pno, mode);
	P_RELE(pno);

out:
	SYSIO_INTERFACE_RETURN(err ? -1 : 0, err);
}

#ifdef REDSTORM
#undef __mkdir
sysio_sym_weak_alias(SYSIO_INTERFACE_NAME(mkdir),
		     PREPEND(__, SYSIO_INTERFACE_NAME(mkdir)))
#endif
