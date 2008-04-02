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

int gzgetstr(char * newstr, gzFile * gzfile);
int getquote(char * input, char * quote, int number);
int strcontains(char * source, char * comparison);

med ** parsemediagz(char * media_path, int * no_of_media)
/*	Parses gzipped adobe pagemaker files for media urls and metadata,
 *	filling media with the information. */
{
	char c;
	char cur_line[STR_MAX];
	gzFile mediagz;

	*no_of_media = -1;

	med ** media = NULL;

	strcpy(cur_line,""); /* reset cur_line */

	mediagz = gzopen(media_path, "r");

	med * cur_media;

	while(gzeof(mediagz)==0)
	{
		gzgetstr(cur_line, mediagz);

		if(strcontains(cur_line,"on mouseUp") == 0)
		{
			/* assign memory for the new media */
			media = assignnew_med(media, no_of_media);

			cur_media = media[*no_of_media];

			/* setup media globals */
			cur_media->uri[0] = '\0';
			cur_media->title[0] = '\0';
			cur_media->comment[0] = '\0';
			cur_media->preview_uri[0] = '\0';
			cur_media->size = 0;

			/* process rev file */
			while(strcontains(cur_line,"end mouseUp") && gzeof(mediagz)==0)
			{
				strcpy(cur_line,""); /* reset cur_line */
				gzgetstr(cur_line, mediagz);

				if(!strcontains(cur_line,"set the filename of player \"") && strcontains(cur_line,"empty"))
					getquote(cur_line, cur_media->uri, 2);
				else if(!strcontains(cur_line,"set the label of this stack to"))
					getquote(cur_line, cur_media->title, 1);
				else if(!strcontains(cur_line,"statusMsg\(\"") || !strcontains(cur_line,"StatusMsg \(\""))
					getquote(cur_line, cur_media->comment, 1);
			}

			/* if it turns out that there was nothing useful there, remove the new media */
			if(cur_media->uri[0] == '\0')
				(*no_of_media)--;
		}
		strcpy(cur_line,""); /* reset cur_line */
	}

	return media;
}

int getquote(char * input, char * quote, int number)
/*	assigns quote string from a line of the format:
 *	'something "quote" something' */
{
	int curnum;

	/* advance past earlier quotes */
	for(curnum=0; curnum < ((number*2)-2); curnum++)
	{
		/* advance until " character is reached */
		while(*input != '"' && *input)	
			input++;
		input++;
	}

	/* advance until " character is reached */
	while(*input != '"' && *input)	
		input++;

	input++;

	/* copy characters in until next '"' */
	while(*input != '"')
	{
		/* if end is found without closing bracket,
		 * exit with error */
		if(! *input)
			return 1;
		*quote = *input;
		quote++;
		input++;
	}

	*quote = '\0';

	return 0;
}

int strcontains(char * source, char * comparison)
{
	int srcpos = 0; /* position in source string */
	int compos = 0; /* position in comparison string */

	while(srcpos < strlen(source) && srcpos < STR_MAX)
	{
		/* if the chars match, move to the next in comparison,
		 * otherwise only advance the source */
		if(source[srcpos] == comparison[compos])
			compos++;
		else
			compos = 0;

		/* if we got to the end of comparison, there's a match */
		if(compos == strlen(comparison))
			return 0;

		srcpos++;
	}

	/* if we got all the way through the source, there's no match */
	return 1;
}

int gzgetstr(char * newstr, gzFile * gzfile)
/*	a reimplementation of gzgetstr, which doesn't choke at odd characters
 */
{
	strcpy(newstr,""); /* reset cur_line */
	char c;

	while((c = gzgetc(gzfile)) != -1)
	{
		if(c == '\n')
			break;

		/* append the char if there's room in the str */
		if(strlen(newstr)+1 < STR_MAX)
			strncat(newstr,&c,1);
		/* if the line is too long just break to pick up the 2nd half on next pass,
		 * not perfect (will miss strs cut into 2 sections), but good enough for now */
		else
			break;
	}

	return 0;
}
