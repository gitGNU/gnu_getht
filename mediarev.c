/*
 * Copyright 2006 Nick White
 *
 * This mediagz is part of GetHT
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
#include <zlib.h>

#include "getht.h"
#include "issue.h"

void getquote(char * input, char * label);

int parsemediagz(char * media_path, med * cur_media, int * no_of_media)
/*	Parses gzipped adobe pagemaker files for media urls and metadata,
 *	filling cur_media with the information. */
{
	char cur_line[STR_MAX];
	gzFile mediagz;

	strcpy(cur_line,""); /* reset cur_line */

	mediagz = gzopen(media_path, "r");

	while(gzeof(mediagz)==0)
	{
		gzgets(mediagz, cur_line, STR_MAX);
		cur_line[strlen(cur_line)-1] = '\0'; /* get rid of trailing newline */

		if(!strcmp(cur_line,"on mouseUp"))
		{
			strcpy(cur_line,""); /* reset cur_line */
			while(strcmp(cur_line,"end mouseUp") && gzeof(mediagz)==0)
			{
				strcpy(cur_line,""); /* reset cur_line */
				gzgets(mediagz, cur_line, STR_MAX);
				cur_line[strlen(cur_line)-1] = '\0'; /* get rid of trailing newline */

				if(!strncmp(cur_line,"    set the filename of player \"player1\" to \"",45))
				{
					/* todo: if smil follow, otherwise load into media->url */
					sscanf(cur_line,"    set the filename of player \"player1\" to \"%s\"",cur_media->uri);
					cur_media->uri[strlen(cur_media->uri)-1] = '\0'; /* workaround extra final character */
				}
				else if(!strncmp(cur_line,"    set the label of this stack to \"",36))
				{
					getquote(cur_line,cur_media->title);
				}
				else if(!strncmp(cur_line,"    statusMsg \(\"",16))
				{
					getquote(cur_line,cur_media->comment);
				}
			/*	else
			 *		fprintf(stderr,"Interesting line to put in struct: '%s'\n",cur_line);
			 */
			 
			}
			*cur_media++;
			(*no_of_media)++;
			cur_media->uri[0] = '\0';
		}
		strcpy(cur_line,""); /* reset cur_line */
	}
	return 0;
}

int smilurl(char * smilurl, med * cur_media)
/*	Extracts url and other data from remote smil file, storing
 *	the info in the cur_media structure. */
{
	return 0;
}

void getquote(char * input, char * quote)
/*	sets quote from a line of the format:
 *	`something "quote" something' */
{
	char * cur_pos;
	cur_pos = quote;

	/* advance until " character is reached */
	while(*input != '"' && *input)	
		input++;

	input++;

	/* copy characters in until next '"' */
	while(*input != '"' && *input)
	{
		*cur_pos = *input;
		cur_pos++;
		input++;
	}

	*cur_pos = '\0';
}
