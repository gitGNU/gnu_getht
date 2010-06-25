/*
 * This file is part of GetHT
 *
 * See COPYING file for copyright, license and warranty details.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "getht.h"

iss ** parsetoc(char *filepath, int * iss_no);
int parseissue(xmlDocPtr file, xmlNodePtr node, iss * cur_issue);
void parsesection(xmlDocPtr file, xmlNodePtr node, sec * cur_section);

void tokenise_hyphons(char to_token[10], int * first, int * last);

int no_of_issues;

iss ** parsetoc(char *filepath, int * iss_no)
/*	starts parsing of xml to issue structure	*/
{
	xmlDocPtr file;
	xmlNodePtr node;

	if(ready_xml(filepath, "issues", &file, &node))
		return NULL;

	*node = *node->xmlChildrenNode;

	no_of_issues = -1;

	iss ** issue = NULL;

	xmlNodePtr cnode;

	while(node != NULL)
	{
		if(!xmlStrncmp(node->name,(unsigned char *) "year",4))
		{
			cnode = node->children;
			while(cnode != NULL)
			{
    				if(!xmlStrncmp(cnode->name,(unsigned char *) "issue",5))
				{
					/* assign memory for the new issue */
					issue = assignnew_iss(issue, &no_of_issues);

					/* setup issue globals */
					issue[no_of_issues]->no_of_sections = -1;
					issue[no_of_issues]->date.year =
						atoi( (const char *)(xmlStrsub(node->name,5,4)) );
					tokenise_hyphons(
							(char *) xmlStrsub(cnode->name,6,5),
							&(issue[no_of_issues]->date.firstmonth),
							&(issue[no_of_issues]->date.lastmonth));

					/* parse the issue */
					parseissue(file, cnode, issue[no_of_issues]);
				}
				cnode = cnode->next;
			}
		}
		node = node->next;
	}

	xmlFreeDoc(file);

	*iss_no = no_of_issues;

	issuesort(issue, no_of_issues);

	return issue;
}

int parseissue(xmlDocPtr file, xmlNodePtr node, iss * cur_issue)
/*	parses issue from xml, saving in cur_issue structure	*/
{
	strncpy(cur_issue->title, (char *) xmlGetProp(node, (unsigned char *)"title"), STR_MAX);
	strncpy(cur_issue->preview_uri, (char *) xmlGetProp(node, (unsigned char *) "coverlink"), STR_MAX);

	node = node->xmlChildrenNode;

	while(node != NULL){
		if(!xmlStrncmp(node->name, (unsigned char *) "section",7) ||
			!xmlStrcmp(node->name, (const xmlChar *) "cover"))
		{
			/* assign memory for new section */
			cur_issue->section = 
				assignnew_sec(cur_issue->section, &(cur_issue->no_of_sections));

			/* setup section globals */
			cur_issue->section[cur_issue->no_of_sections]->no_of_items = -1;

			/* parse the section */
			parsesection(file, node, cur_issue->section[cur_issue->no_of_sections]);
		}
		node = node->next;
	}

	return 0;
}

void parsesection(xmlDocPtr file, xmlNodePtr node, sec * cur_section)
/*	parses section xml, filling cur_section structure */
{
	it * cur_item = NULL;
	char * pagenums = "";

	strncpy(cur_section->uri, (char *) xmlGetProp(node, (unsigned char *) "pdflink"), STR_MAX);
	strncpy(cur_section->title, (char *) xmlGetProp(node, (unsigned char *) "title"), STR_MAX);

	if(!xmlStrcmp(node->name, (const xmlChar *) "cover"))
		cur_section->number = 0;
	else
		cur_section->number = atoi( (const char *)(xmlStrsub(node->name,8,1)) );

	node = node->xmlChildrenNode;

	while(node != NULL)
	{
		if(!xmlStrcmp(node->name, (const xmlChar *) "item"))
		{
			if(xmlNodeListGetString(file, node->xmlChildrenNode, 1) != NULL)
			/* ignore items without titles */
			{
				/* assign memory for new item */
				cur_section->item =
					assignnew_it( cur_section->item, &(cur_section->no_of_items));

				cur_item = cur_section->item[cur_section->no_of_items];

				/* parse item */
				cur_item->title = (char *) xmlNodeListGetString(file, node->xmlChildrenNode, 1);
				if(pagenums == (char *) xmlGetProp(node, (unsigned char *) "pages"))
					tokenise_hyphons(pagenums, &(cur_item->firstpage), &(cur_item->lastpage));
				else
				{
					cur_item->firstpage = 0;
					cur_item->lastpage = 0;
				}
			}
		}
	node = node->next;	
	}
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

	if((file =  xmlReadFile(filepath, "ISO-8859-1", 0)) == NULL)
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
		if(!xmlStrncmp(node->name,(unsigned char *) "year",4))
		{
			cnode = node->children;
			while(cnode != NULL)
			{
	   		    if(!xmlStrncmp(cnode->name,(unsigned char *) "issue",5))
				{
					temp = xmlGetProp(cnode, (unsigned char *) "current");
        			if(temp)
        			{
        			    strncpy(title, (char *) xmlGetProp(cnode, (unsigned char *) "title"), STR_MAX);
						date->year = atoi( (const char *)(xmlStrsub(node->name,5,4)) );
						tokenise_hyphons((char *)xmlStrsub(cnode->name,6,5), &(date->firstmonth), &(date->lastmonth));
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
