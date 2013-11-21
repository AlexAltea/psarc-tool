/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS System libc */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <stdlib.h>
#include <string.h>
#include "libgen.h"

// basename
char * basename(char * path)
{
	char *base;
	base = strrchr(path, '/');
	if (base) return base+1;
	base = strrchr(path, '\\');
	if (base) return base+1;
    return path;
}

// dirname
char * dirname(char * path)
{
	char *slash1 = strrchr(path, '/');
    char *slash2 = strrchr(path, '\\');
	
	if (slash1 > slash2 && slash1)
	{
		char *dir = (char*)malloc(slash1-path+1);
		memcpy(dir, path, slash1-path);
		dir[slash1-path] = 0x0;
		return dir;
	}
	if (slash2 > slash1 && slash2)
	{
		char *dir = (char*)malloc(slash2-path+1);
		memcpy(dir, path, slash2-path);
		dir[slash2-path] = 0x0;
		return dir;
	}

    return ".\x00";
}