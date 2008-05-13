/*
 * Copyright 2006,2008 Nick White
 *
 * This file is part of GetHT
 *
 * GetHT is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GetHT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GetHT.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

#include "getht.h"
#include "issue.h"

int read_func(void *ptr, size_t size, size_t nmemb, FILE *stream)
	{ return fread(ptr, size, nmemb, stream); }
int write_func(void *ptr, size_t size, size_t nmemb, FILE *stream)
	{ return fwrite(ptr, size, nmemb, stream); }
int update_progress(void *data, double dltotal, double dlnow,
					double ultotal, double ulnow);

int save_file(char *uri, char *filepath, char *filetitle, long resume_offset, struct config * options)
/*	Save the file *uri to *filepath */
{
	if(!options->quiet)
	{
		printf("Downloading %s       ",filetitle);
		fflush(stdout);
	}

	if(options->curl_handle) {
		FILE *file;
		file = fopen(filepath, resume_offset?"a":"w");
		if(!file)
		{
			fprintf(stderr,"Error: cannot open file %s for writing.\n",filepath);
			return 1;
		}

		curl_easy_setopt(options->curl_handle, CURLOPT_URL, uri);
		curl_easy_setopt(options->curl_handle, CURLOPT_READFUNCTION, read_func);
		curl_easy_setopt(options->curl_handle, CURLOPT_WRITEFUNCTION, write_func);
		curl_easy_setopt(options->curl_handle, CURLOPT_WRITEDATA, file);

		if(options->proxy.type)
		{
			curl_easy_setopt(options->curl_handle, CURLOPT_PROXYTYPE, options->proxy.type);
			curl_easy_setopt(options->curl_handle, CURLOPT_PROXY, options->proxy.address);
			if(options->proxy.port)
				curl_easy_setopt(options->curl_handle, CURLOPT_PROXYPORT, options->proxy.port);
			if(options->proxy.auth)
				curl_easy_setopt(options->curl_handle, CURLOPT_PROXYAUTH, options->proxy.auth);
			if(options->proxy.user[0] && options->proxy.pass[0])
			{
				char userpass[STR_MAX];
				snprintf(userpass, STR_MAX, "%s:%s", options->proxy.user, options->proxy.pass);
				curl_easy_setopt(options->curl_handle, CURLOPT_PROXYUSERPWD, userpass);
			}
		}

		if(!options->quiet)
		{
			curl_easy_setopt(options->curl_handle, CURLOPT_NOPROGRESS, 0);
			curl_easy_setopt(options->curl_handle, CURLOPT_PROGRESSFUNCTION, update_progress);
			curl_easy_setopt(options->curl_handle, CURLOPT_PROGRESSDATA, &resume_offset);
		}
		else
			curl_easy_setopt(options->curl_handle, CURLOPT_NOPROGRESS, 1);

		curl_easy_setopt(options->curl_handle, CURLOPT_RESUME_FROM, resume_offset);

		/* create a buffer to hold any curl errors */
		char errorinfo[CURL_ERROR_SIZE];
		curl_easy_setopt(options->curl_handle, CURLOPT_ERRORBUFFER, errorinfo);

		if(curl_easy_perform(options->curl_handle))
		{
			remove(filepath);
			fprintf(stderr,"\nError, could not download %s: %s\n",uri, errorinfo);
			return 1;
		}

		fclose(file);

		if(!options->quiet)
			printf("\rDownloaded %s       \n",filetitle);
	}
	else {
		fprintf(stderr,"Error: curl failed to initialise.\n");
		printf("Could not download %s\n",uri);
		return 1;
	}
	return 0;
}

int update_progress(void *data, double dltotal, double dlnow,
				double ultotal, double ulnow)
/*	Print status information */
{
	double frac;
	long *startsize = NULL;

	startsize = (long *)data;
	long cur = (long) dlnow + *startsize;
	long total = (long) dltotal + *startsize;

	if(cur > 0)
		frac = 100 * (double) cur / (double) total;
	else
		frac = 0;

	printf("\b\b\b\b\b\b\b: %3.0lf%% ", frac);
	fflush(stdout);

	return 0;
}

double getremotefilesize(char *uri, struct config * options)
{
	double filesize;

	if(options->curl_handle) {

		curl_easy_setopt(options->curl_handle, CURLOPT_URL, uri);
		curl_easy_setopt(options->curl_handle, CURLOPT_READFUNCTION, read_func);

		/* don't download or return either body or header */
		curl_easy_setopt(options->curl_handle, CURLOPT_NOBODY, 1);
		curl_easy_setopt(options->curl_handle, CURLOPT_HEADER, 0);

		if(options->proxy.type)
		{
			curl_easy_setopt(options->curl_handle, CURLOPT_PROXYTYPE, options->proxy.type);
			curl_easy_setopt(options->curl_handle, CURLOPT_PROXY, options->proxy.address);
			if(options->proxy.port)
				curl_easy_setopt(options->curl_handle, CURLOPT_PROXYPORT, options->proxy.port);
			if(options->proxy.auth)
				curl_easy_setopt(options->curl_handle, CURLOPT_PROXYAUTH, options->proxy.auth);
			if(options->proxy.user[0] && options->proxy.pass[0])
			{
				char userpass[STR_MAX];
				snprintf(userpass, STR_MAX, "%s:%s", options->proxy.user, options->proxy.pass);
				curl_easy_setopt(options->curl_handle, CURLOPT_PROXYUSERPWD, userpass);
			}
		}

		curl_easy_setopt(options->curl_handle, CURLOPT_NOPROGRESS, 1);

		if(curl_easy_perform(options->curl_handle))
			filesize = -1;

		curl_easy_getinfo(options->curl_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &filesize);

	}
	else
		filesize = -1;

	curl_easy_reset(options->curl_handle);
	return filesize;
}

void downloadissue(struct config * options, iss * issue, int force)
/*	Download issue pointed to */
{
	sec * cur_section;
	char newdir[STR_MAX];
	char filename[STR_MAX];
	char filepath[STR_MAX];

	snprintf(newdir,STR_MAX,"%s/%i_%i-%i",options->save_path,
		issue->date.year,issue->date.firstmonth,issue->date.lastmonth);

	printf("Downloading %s to %s\n",issue->title, newdir);

	if(!opendir(newdir))
		if(mkdir(newdir, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))
		{
			printf("Please enter the path of a directory to save issues in: ");
			scanf("%s", newdir);
		}

	int count;
	for(count = 0; count <= issue->no_of_sections; count++)
	{
		cur_section = issue->section[count];

		snprintf(filename,STR_MAX,"section_%i.pdf", cur_section->number);
		snprintf(filepath,STR_MAX,"%s/%s", newdir, filename);
		if(!force){
			struct stat fileinfo;
			/* see if local file exists */
			if(stat(filepath, &fileinfo))
				save_file(cur_section->uri, filepath, filename, 0, options);
			else
			{
				/* get size of local file */
				long localsize = 0;
				localsize = (long) fileinfo.st_size;

				/* get size of remote file */
				long remotesize = 0;
				remotesize = (long) getremotefilesize(cur_section->uri, options);

				/* if size of local file != size of remote file, resume */
				if(remotesize > 0 && localsize < remotesize)
					save_file(cur_section->uri, filepath, filename, localsize, options);
				else
				{
					if(!options->quiet)
						printf("Skipping download of completed section %i\n", cur_section->number);
				}
			}
		}
		else
			save_file(cur_section->uri, filepath, filename, 0, options);
	}
}
