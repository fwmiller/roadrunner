/* 
 *  Roadrunner/pk
 *    Copyright (C) 1989-2001  Cornfed Systems, Inc.
 *
 *  The Roadrunner/pk operating system is free software; you can
 *  redistribute and/or modify it under the terms of the GNU General
 *  Public License, version 2, as published by the Free Software
 *  Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this program; if not, write to the Free
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA 02111-1307 USA
 *
 *  More information about the Roadrunner/pk operating system of
 *  which this file is a part is available on the World-Wide Web
 *  at: http://www.cornfed.com.
 *
 */

#ifndef __CTYPE_H
#define __CTYPE_H

#define iscntrl(C) ((C) < ' ')
#define isspace(C) ((C) == ' ' || (C) == '\t')
#define isupper(C) (((C) >= 'A' && (C) <= 'Z') ? 1 : 0)
#define islower(C) (((C) >= 'a' && (C) <= 'z') ? 1 : 0)
#define isdigit(C) (((C) >= '0' && (C) <= '9') ? 1 : 0)
#define isalpha(C) ((islower(C) || isupper(C)) ? 1 : 0)
#define isalnum(C) (isalpha(C) || isdigit(C))
#define isprint(C) (isspace(C) || ((C) >= ' ' && (C) <= '~') ? 1 : 0)
#define isgraph(C) (isprint(C) && (C) != ' ')
#define ispunct(C) (isprint(C) && !((C) != ' ' || isalnum(C)))

#define isxdigit(C)							\
    ((isdigit(C) ||							\
      ((C) >= 'a' && (C) <= 'f') ||					\
      ((C) >= 'A' && (C) <= 'F')) ? 1 : 0)

#define toupper(C) (islower(C) ? (C) - 32 : (C))
#define tolower(C) (isupper(C) ? (C) + 32 : (C))

#endif
