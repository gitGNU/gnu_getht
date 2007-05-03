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
#include <string.h>
#include <unistd.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "getht.h"
#include "issue.h"

int ready_xml(char * filepath, char * rootnode, xmlDocPtr * file, xmlNodePtr * node)
/*	Opens filepath, filling nec pointers with essential information. */
{
	if(access(filepath, R_OK) != 0)
		return 1;

	if((*file = xmlParseFile(filepath)) == NULL)
	{
		fprintf(stderr, "Error: cannot open xml file %s\n", filepath);
		return 1;
	}

	*node = xmlDocGetRootElement(*file);

	if(*node == NULL)
	{
		fprintf(stderr,"Error: xml file %s has no root element",filepath);
		xmlFreeDoc(*file);
		return 1;
	}

	char * test;
	test = (char *) (*node)->name;

	if(xmlStrcmp((*node)->name, (const xmlChar *) rootnode))
	{
		fprintf(stderr,"Document of the wrong type, root node is '%s' (expected '%s').\n",(char *) (*node)->name, rootnode);
		fprintf(stderr,"Continuing regardless...\n");
	}

	return 0;
}
