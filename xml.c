/*
 * This file is part of GetHT
 *
 * See COPYING file for copyright, license and warranty details.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "getht.h"

int ready_xml(char * filepath, char * rootnode, xmlDocPtr * file, xmlNodePtr * node)
/*	Opens filepath, filling nec pointers with essential information. */
{
	if(access(filepath, R_OK) != 0)
		return 1;

	if((*file = xmlReadFile(filepath, "ISO-8859-1", 0)) == NULL)
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
