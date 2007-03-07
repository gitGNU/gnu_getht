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
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "getht.h"
#include "issue.h"

int media_accounted_for(char * filepath, issdates * date)
/*	checks if media for issue is found */
{
	xmlDocPtr media_file;
	xmlNodePtr node;

	if(ready_xml(filepath, "media", &media_file, &node))
		return 1;

	*node = *node->xmlChildrenNode;

	issdates curdate;
	int found = 1;

	while (node != NULL)
	{
		if(!xmlStrcmp(node->name,(char *) "issue"))
		{
			curdate.year = atoi( (char *) xmlGetProp(node, "year"));
			curdate.firstmonth = atoi( (char *) xmlGetProp(node, "firstmonth"));
			curdate.lastmonth = atoi( (char *) xmlGetProp(node, "lastmonth"));
		}

		if( curdate.year == date->year &&
			curdate.firstmonth == date->firstmonth &&
			curdate.lastmonth == date->lastmonth )
		{
			found = 0;
			break;
		}

		node = node->next;
	}

	xmlFreeDoc(media_file);

	return found;
}

int addmediaissue(char * filepath, char * title, issdates * date, med * media, int med_no)
/*	Appends data from media structures to xml file. */
{
	xmlDocPtr media_file;
	xmlNodePtr node;

	/* if xml file doesn't exist */
	if(ready_xml(filepath, "media", &media_file, &node))
	{
		/* set up fresh xml file */
		media_file = xmlNewDoc(NULL);
		node = xmlNewNode(NULL, "media");
		xmlDocSetRootElement(media_file, node);
	}

	xmlNodePtr curissue;
	char tmp[5];

	/* set up issue node */
	curissue = xmlNewTextChild(node, NULL, "issue", NULL);

	xmlNewProp(curissue, "title", title);

	snprintf(tmp,5,"%i", date->year);
	xmlNewProp(curissue, "year", tmp);

	snprintf(tmp,5,"%i",date->firstmonth);
	xmlNewProp(curissue, "firstmonth", tmp);

	snprintf(tmp,5,"%i",date->lastmonth);
	xmlNewProp(curissue, "lastmonth", tmp);

	xmlNodePtr curitem;
	int i;
	for(i = 0; i <= med_no; i++)
	{
		curitem = xmlNewTextChild(curissue, NULL, "item", media[i].title);

		xmlNewProp(curitem, "uri", media[i].uri);

		if(media[i].comment)
			xmlNewProp(curitem, "comment", media[i].comment);
		if(media[i].preview_uri)
			xmlNewProp(curitem, "preview_uri", media[i].preview_uri);
	}

	xmlKeepBlanksDefault(0);

	xmlSaveFormatFile(filepath, media_file, 1);

	xmlFreeDoc(media_file);

	return 0;
}

iss ** parsemedia(char * filepath, iss ** issue, int * no_of_issues)
/*	Fills issues with relevant info from media xml, creating new
	ones where necessary. */
{
	issdates tmpdate;

	iss * cur_issue; med * cur_media;

	xmlDocPtr media_file;
	xmlNodePtr node, itnode;

	if(ready_xml(filepath, "media", &media_file, &node))
		return NULL;

	*node = *node->xmlChildrenNode;

	int issue_there = 0;

	char title[STR_MAX];
	issdates curdate;
	int tmp;

	while (node != NULL)
	{
		if(!xmlStrcmp(node->name,(char *) "issue"))
		{
			/* check if issue with title already exists */
			for(tmp = 0; tmp < *no_of_issues; tmp++)
			{
				curdate.year = atoi( (char *) xmlGetProp(node, "year"));
				curdate.firstmonth = atoi( (char *) xmlGetProp(node, "firstmonth"));
				curdate.lastmonth = atoi( (char *) xmlGetProp(node, "lastmonth"));

				if( curdate.year == issue[tmp]->date.year &&
				curdate.firstmonth == issue[tmp]->date.firstmonth &&
				curdate.lastmonth == issue[tmp]->date.lastmonth )
				{
					issue_there = 1;
					break;
				}
			}

			if(!issue_there)
			{	/* advance to the next free issue */
				iss ** tmpiss;
				if(*no_of_issues == -1)
				{	/* make issue** a new array of issue pointers */
					if( (tmpiss = malloc(sizeof(iss *))) == NULL )
						nogo_mem();
				}
				else
				{	/* add a new pointer to issue pointer list */
					if( (tmpiss = realloc(issue, sizeof(iss *) + (*no_of_issues * sizeof(iss *)))) == NULL )
					nogo_mem();
				}

				(*no_of_issues)++;

				/* make new array item a pointer to issue */
				if( (tmpiss[*no_of_issues] = malloc(sizeof(iss))) == NULL )
					nogo_mem();

				issue = tmpiss;

				issue[*no_of_issues]->date.year = atoi( (char *) xmlGetProp(node, "year"));
				issue[*no_of_issues]->date.firstmonth = atoi( (char *) xmlGetProp(node, "firstmonth"));
				issue[*no_of_issues]->date.lastmonth = atoi( (char *) xmlGetProp(node, "lastmonth"));

				strncpy(issue[*no_of_issues]->title, (char *) xmlGetProp(node, "title"), STR_MAX);

				tmp = *no_of_issues;
			}

			issue[tmp]->no_of_media = -1;
			cur_media = issue[tmp]->media;


			itnode = node->xmlChildrenNode;

			while (itnode != NULL)
			{

				if(!xmlStrcmp(itnode->name,(char *) "item"))
				{
					/* add media info to cur_media */
					if(xmlGetProp(itnode, "uri"))
						strncpy(cur_media->uri, (char *) xmlGetProp(itnode, "uri"), STR_MAX);

					if(xmlGetProp(itnode, "comment"))
						strncpy(cur_media->comment, (char *) xmlGetProp(itnode, "comment"), STR_MAX);

					if(xmlGetProp(itnode, "preview"))
						strncpy(cur_media->preview_uri, (char *) xmlGetProp(itnode, "preview_uri"), STR_MAX);

					strncpy(cur_media->title, (char *) xmlNodeListGetString(media_file, itnode->xmlChildrenNode, 1), STR_MAX);

					issue[tmp]->no_of_media++;

					cur_media++;
				}
				
				itnode = itnode->next;
			}
		}

		node = node->next;
	}

	xmlFreeDoc(media_file);

	return issue;
}
