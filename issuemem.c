/*
 * Copyright 2006 Nick White
 *
 * This file is part of GetHT
 * 
 * GetHT is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GetHT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GetHT; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "issue.h"

void nogo_mem()
/*	called if memory assignation fails
 *	TODO: handle freeing of memory to avoid leaks */
{
	fprintf(stderr, "Could not assign memory, exitting\n");
	exit(1);
}

iss ** assignnew_iss(int *no_of_issues, iss ** issue)
/* assign memory for new issue */
{
	iss ** tmp = NULL;

	if(*no_of_issues < 0)
	{       /* make issue** a new array of issue pointers */
		if( (tmp = malloc(sizeof(iss *))) == NULL )
			nogo_mem();
	}
	else
	{       /* add a new pointer to issue pointer list */
		if( (tmp = realloc(issue, sizeof(iss *) + (((*no_of_issues)+1) * sizeof(iss *)))) == NULL )
			nogo_mem();
	}

	(*no_of_issues)++;
	
	/* make new array item a pointer to issue */
	if( (tmp[*no_of_issues] = malloc(sizeof(iss))) == NULL )
		nogo_mem();

	return tmp;
}

sec ** assignnew_sec(int *no_of_sections, sec ** section)
/* assign memory for new section */
{
	sec ** tmp = NULL;

	if(*no_of_sections < 0)
	{       /* make **section a new array of section pointers */
		if( (tmp = malloc(sizeof(sec *))) == NULL )
			nogo_mem();
	}
	else
	{       /* add a new pointer to section pointer list */
		if( (tmp = realloc(section, sizeof(sec *) + (((*no_of_sections)+1) * sizeof(sec *)))) == NULL )
			nogo_mem();
	}
	
	(*no_of_sections)++;

	/* make new array item a pointer to issue */
	if( (tmp[*no_of_sections] = malloc(sizeof(sec))) == NULL )
		nogo_mem();

	return tmp;
}

it ** assignnew_it(int * no_of_items, it ** item)
{
	it ** tmp = NULL;

	if(*no_of_items < 0)
	{       /* make **item a new array of item pointers */
		if( (tmp = malloc(sizeof(it *))) == NULL )
			nogo_mem();
										                        }
	else
	{       /* add a new pointer to item pointer list */
		if( (tmp = realloc(item, sizeof(it *) + (((*no_of_items)+1) * sizeof(it *)))) == NULL )
			nogo_mem();
	}

	(*no_of_items)++;

	/* make new array item a pointer to item */
	if( (tmp[*no_of_items] = malloc(sizeof(it))) == NULL )
		nogo_mem();

	return tmp;
}
