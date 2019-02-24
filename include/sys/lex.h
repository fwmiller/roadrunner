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

#ifndef __LEX_H
#define __LEX_H

typedef struct lex {
#define LEX_NULL	0
#define LEX_CR		1
#define LEX_LF		2
#define LEX_EOL		3
#define LEX_ID		4
#define LEX_NUM		5
#define LEX_COLON	6
#define LEX_DASH	7
#define LEX_QUESTION	8
#define LEX_SLASH	9
#define LEX_PERIOD	10
    int type;
    int val;
    char s[80];
} *lex_t;

void nextarg(char *ln, int *pos, char *arg);
void nextlex(char *ln, int *pos, lex_t l);

#endif
