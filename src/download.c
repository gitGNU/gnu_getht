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

extern proxytype proxy_type;
extern char proxy_addr[STR_MAX];
extern long proxy_port;
extern proxyauth proxy_auth;
extern char proxy_user[STR_MAX];
extern char proxy_pass[STR_MAX];
extern CURL *main_curl_handle;

int save_file(CURL *curl_handle, char *url, char *filepath, long resume_offset)
/*	Save the file *url to *filepath */
{
	printf("Downloading %s\n",url);

	if(!curl_handle)
		curl_handle = main_curl_handle;

	if(curl_handle) {
		FILE *file;
		file = fopen(filepath, "a");
		if(!file)
		{
			fprintf(stderr,"Error: cannot open file %s for writing.\n",filepath);
			return 1;
		}

		curl_easy_setopt(curl_handle, CURLOPT_URL, url);
		curl_easy_setopt(curl_handle, CURLOPT_READFUNCTION, read_func);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_func);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, file);

		if(proxy_type != NONE)
		{
			if(proxy_type == HTTP)
				curl_easy_setopt(curl_handle, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
			else if(proxy_type == SOCKS4)
				curl_easy_setopt(curl_handle, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4);
			else if(proxy_type == SOCKS5)
				curl_easy_setopt(curl_handle, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);

			curl_easy_setopt(curl_handle, CURLOPT_PROXY, proxy_addr);

			if(proxy_port)
				curl_easy_setopt(curl_handle, CURLOPT_PROXYPORT, proxy_port);

			if(proxy_auth != NOAUTH)
			{
				if(proxy_auth == BASIC)
					curl_easy_setopt(curl_handle, CURLOPT_PROXYAUTH, CURLAUTH_BASIC);
				else if(proxy_auth == DIGEST)
					curl_easy_setopt(curl_handle, CURLOPT_PROXYAUTH, CURLAUTH_DIGEST);
				else if(proxy_auth == NTLM)
					curl_easy_setopt(curl_handle, CURLOPT_PROXYAUTH, CURLAUTH_NTLM);

				if(proxy_user[0] && proxy_pass[0])
				{
					char userpass[STR_MAX];
					snprintf(userpass, STR_MAX, "%s:%s", proxy_user, proxy_pass);
					curl_easy_setopt(curl_handle, CURLOPT_PROXYUSERPWD, userpass);
				}
			}
		}

		curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0);
		curl_easy_setopt(curl_handle, CURLOPT_PROGRESSFUNCTION, update_progress);
		curl_easy_setopt(curl_handle, CURLOPT_PROGRESSDATA, &resume_offset);

		curl_easy_setopt(curl_handle, CURLOPT_RESUME_FROM, resume_offset);

		/* create a buffer to hold any curl errors */
		char errorinfo[CURL_ERROR_SIZE];
		curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, errorinfo);

		if(curl_easy_perform(curl_handle))
		{
			remove(filepath);
			fprintf(stderr,"\nError, could not download %s: %s\n",url, errorinfo);
			return 1;
		}

		fclose(file);

		printf("\n");
	}
	else {
		fprintf(stderr,"Error: curl failed to initialise.\n");
		printf("Could not download %s\n",url);
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

	printf("\rDownload progress: %3.0lf%% ", frac);
	fflush(stdout);

	return 0;
}

double getremotefilesize(CURL *curl_handle, char *url)
{
	double filesize;

	if(!curl_handle)
		curl_handle = main_curl_handle;

	if(curl_handle) {

		curl_easy_setopt(curl_handle, CURLOPT_URL, url);
		curl_easy_setopt(curl_handle, CURLOPT_READFUNCTION, read_func);

		/* don't download or return either body or header */
		curl_easy_setopt(curl_handle, CURLOPT_NOBODY, 1);
		curl_easy_setopt(curl_handle, CURLOPT_HEADER, 0);

		if(proxy_type != NONE)
		{
			if(proxy_type == HTTP)
				curl_easy_setopt(curl_handle, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
			else if(proxy_type == SOCKS4)
				curl_easy_setopt(curl_handle, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4);
			else if(proxy_type == SOCKS5)
				curl_easy_setopt(curl_handle, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);

			curl_easy_setopt(curl_handle, CURLOPT_PROXY, proxy_addr);

			if(proxy_port)
				curl_easy_setopt(curl_handle, CURLOPT_PROXYPORT, proxy_port);

			if(proxy_auth != NOAUTH)
			{
				if(proxy_auth == BASIC)
					curl_easy_setopt(curl_handle, CURLOPT_PROXYAUTH, CURLAUTH_BASIC);
				else if(proxy_auth == DIGEST)
					curl_easy_setopt(curl_handle, CURLOPT_PROXYAUTH, CURLAUTH_DIGEST);
				else if(proxy_auth == NTLM)
					curl_easy_setopt(curl_handle, CURLOPT_PROXYAUTH, CURLAUTH_NTLM);

				if(proxy_user[0] && proxy_pass[0])
				{
					char userpass[STR_MAX];
					snprintf(userpass, STR_MAX, "%s:%s", proxy_user, proxy_pass);
					curl_easy_setopt(curl_handle, CURLOPT_PROXYUSERPWD, userpass);
				}
			}
		}

		curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1);

		if(curl_easy_perform(curl_handle))
			filesize = -1;

		curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &filesize);

	}
	else
		filesize = -1;

	curl_easy_reset(curl_handle);
	return filesize;
}

void downloadissue(CURL *curl_handle, char * directory, iss * issue, int force)
/*	Download issue pointed to */
{
	sec * cur_section;
	char newdir[STR_MAX];
	char filename[STR_MAX];
	FILE * testfile;

	snprintf(newdir,STR_MAX,"%s/%i_%i-%i",directory,
		issue->date.year,issue->date.firstmonth,issue->date.lastmonth);

	printf("Downloading issue entitled '%s' to '%s'\n",issue->title,newdir);

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

		snprintf(filename,STR_MAX,"%s/section_%i.pdf", newdir, cur_section->number);
		if(!force){
			struct stat fileinfo;
			/* see if local file exists */
			if(stat(filename, &fileinfo))
				save_file(curl_handle, cur_section->uri, filename, 0);
			else
			{
				/* get size of local file */
				long localsize = 0;
				localsize = (long) fileinfo.st_size;

				/* get size of remote file */
				long remotesize = 0;
				remotesize = (long) getremotefilesize(curl_handle, cur_section->uri);

				/* if size of local file != size of remote file, resume */
				if(remotesize > 0 && localsize < remotesize)
					save_file(curl_handle, cur_section->uri, filename, localsize);
				else
					printf("Skipping download of completed section %i\n", cur_section->number);
			}
		}
		else
			save_file(curl_handle, cur_section->uri, filename, 0);
	}
}
