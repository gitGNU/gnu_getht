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
#include <string.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "issue.h"
#include "getht.h"

iss ** parsetoc(char *filepath, int * iss_no, int * latest);
iss ** parseyear(xmlDocPtr file, xmlNodePtr node, iss ** issue, int * latest);
int parseissue(xmlDocPtr file, xmlNodePtr node, iss * cur_issue, int * latest);
void parsesection(xmlDocPtr file, xmlNodePtr node, sec * cur_section);

void tokenise_hyphons(char to_token[10], int * first, int * last);

int no_of_issues;

void nogo_mem()
/*	called if memory assignation fails
	TODO: handle freeing of memory to avoid leaks */
{
	fprintf(stderr, "Could not assign memory, exitting\n");
	exit(1);
}

iss ** parsetoc(char *filepath, int * iss_no, int * latest)
/*	starts parsing of xml to issue structure
	TODO: combine with parseyear */
{
	xmlDocPtr file;
	xmlNodePtr node;

	if(ready_xml(filepath, "issues", &file, &node))
		return NULL;

	*node = *node->xmlChildrenNode;

	no_of_issues = -1;

	iss ** issue;

	int year;
	iss ** tmp;

	xmlNodePtr cnode;

	while(node != NULL)
	{
		if(!xmlStrncmp(node->name,(char *) "year",4))
		{
			cnode = node->children;
			while(cnode != NULL)
			{
    				if(!xmlStrncmp(cnode->name,(char *) "issue",5))
				{
					if(no_of_issues < 0)
					{	/* make issue** a new array of issue pointers */
						if( (tmp = malloc(sizeof(iss *))) == NULL )
							nogo_mem();
					}
					else
					{	/* add a new pointer to issue pointer list */
						if( (tmp = realloc(issue, sizeof(iss *) + ((no_of_issues+1) * sizeof(iss *)))) == NULL )
							nogo_mem();
					}

					no_of_issues++;

					/* make new array item a pointer to issue */
					if( (tmp[no_of_issues] = malloc(sizeof(iss))) == NULL )
						nogo_mem();

					issue = tmp;

					issue[no_of_issues]->no_of_media = -1; /* set default no of media */
					issue[no_of_issues]->date.year = atoi( (const char *)(xmlStrsub(node->name,5,4)) );
					tokenise_hyphons(xmlStrsub(cnode->name,6,5), &(issue[no_of_issues]->date.firstmonth), &(issue[no_of_issues]->date.lastmonth));
					issue[no_of_issues]->no_of_sections = parseissue(file, cnode, issue[no_of_issues], latest);
				}
				cnode = cnode->next;
			}
		}
		node = node->next;
	}

	xmlFreeDoc(file);

	*iss_no = no_of_issues;

	return issue;
}

int parseissue(xmlDocPtr file, xmlNodePtr node, iss * cur_issue, int * latest)
/*	parses issue from xml, saving in cur_issue structure */
{
	int no_of_sections = -1;

	strncpy(cur_issue->title, (char *) xmlGetProp(node, "title"), STR_MAX);
	strncpy(cur_issue->preview_uri, (char *) xmlGetProp(node, "coverlink"), STR_MAX);

	if(xmlGetProp(node, "current") && *latest==-1)
		*latest = no_of_issues;

	sec * cur_section = NULL;

	node = node->xmlChildrenNode;

	while(node != NULL){
		if(!xmlStrncmp(node->name, (char *) "section",7) ||
			!xmlStrcmp(node->name, (const xmlChar *) "cover"))
		{
			no_of_sections++;
			cur_section = &(cur_issue->section[no_of_sections]);

			parsesection(file, node, cur_section);
		}
		node = node->next;
	}

	return no_of_sections;
}

void parsesection(xmlDocPtr file, xmlNodePtr node, sec * cur_section)
/*	parses section xml, filling cur_section structure */
{
	it * cur_item;

	strncpy(cur_section->uri, (char *) xmlGetProp(node, "pdflink"), STR_MAX);
	strncpy(cur_section->title, (char *) xmlGetProp(node, "title"), STR_MAX);

	if(!xmlStrcmp(node->name, (const xmlChar *) "cover"))
		cur_section->number = 0;
	else
		cur_section->number = atoi( (const char *)(xmlStrsub(node->name,8,1)) );

	cur_item = cur_section->item;
	cur_section->no_of_items = 0;

	node = node->xmlChildrenNode;

	char * pagenums;

	while(node != NULL)
	{
		if(!xmlStrcmp(node->name, (const xmlChar *) "item"))
		{
			cur_section->no_of_items++;
			cur_item->title = xmlNodeListGetString(file, node->xmlChildrenNode, 1);
			if(pagenums = (char *) xmlGetProp(node, "pages"))
				tokenise_hyphons(pagenums, &(cur_item->firstpage), &(cur_item->lastpage));
			else
			{
				cur_item->firstpage = 0;
				cur_item->lastpage = 0;
			}
			cur_item++;
		}
	node = node->next;	
	}
	cur_item = 0;
}

void tokenise_hyphons(char to_token[10], int * first, int * last)
/*	splits string to_token, filling positions passed */
{
	char token[10];
	char * tmp;

	tmp = token;

	while(*to_token != '-' && *to_token)
	{
		*tmp = *to_token;
		to_token++; tmp++;
	}

	*first = atoi(token);

	if(!*to_token)
		*last = *first;
	else
	{
		to_token++; /* advance past '-' */
		tmp = token; /* reset tmp */
		while(*to_token)
		{
			*tmp = *to_token;
			tmp++; to_token++;
		}
		*last = atoi(token);
	}
}

int cur_identifiers(char * filepath, char * title, issdates * date)
/*	parses xml file to ascertain current issue title and date */
{
	xmlDocPtr file;

	if((file = xmlParseFile(filepath)) == NULL)
	{
		return 1;
	}

	xmlNodePtr node,cnode;

	node = xmlDocGetRootElement(file);

	if(node == NULL)
	{
		fprintf(stderr,"Error: xml file %s has no root element",filepath);
		xmlFreeDoc(file);
		return 1;
	}

	if(xmlStrcmp(node->name, (const xmlChar *) "issues"))
	{
		fprintf(stderr,"Document of the wrong type, root node is '%s' (expected 'issues').\n",(char *) node->name);
		fprintf(stderr,"Continuing regardless...\n");
	}

	/* Now that's all sorted, let's do some work */

	node = node->xmlChildrenNode;

	xmlChar *temp;
	while(node != NULL)
	{
		if(!xmlStrncmp(node->name,(char *) "year",4))
		{
			cnode = node->children;
			while(cnode != NULL)
			{
	   		    if(!xmlStrncmp(cnode->name,(char *) "issue",5))
				{
					temp = xmlGetProp(cnode, "current");
        			if(temp)
        			{
        			    strncpy(title, (char *) xmlGetProp(cnode, "title"), STR_MAX);
						date->year = atoi( (const char *)(xmlStrsub(node->name,5,4)) );
						tokenise_hyphons(xmlStrsub(cnode->name,6,5), &(date->firstmonth), &(date->lastmonth));
						return 0;
        			}
				}
				cnode = cnode->next;
			}
		}
		node = node->next;
	}

	return 0;
}
