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

int save_file(CURL *curl_handle, char *url, char *filepath)
/*	Save the file *url to *filepath */
{
	printf("Downloading %s\n",url);

	if(curl_handle == NULL)
		curl_handle = main_curl_handle;

	if(curl_handle) {
		FILE *file;
		if((file = fopen(filepath,"w")) == NULL)
		{
			fprintf(stderr,"Error: cannot open file %s for writing.\n",filepath);
			return 1;
		}

		curl_easy_setopt(curl_handle, CURLOPT_URL, url);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_func);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, file);
		curl_easy_setopt(curl_handle, CURLOPT_READFUNCTION, read_func);

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

		if(curl_easy_perform(curl_handle))
		{
			remove(filepath);
			fprintf(stderr,"Error, could not download %s\n",url);
			return 1;
		}

/*		double d;
		curl_easy_getinfo(curl_handle, CURLINFO_SIZE_DOWNLOAD, &d);
		printf("Total downloaded: %lf\n",d);

		curl_easy_getinfo(curl_handle, CURLINFO_SPEED_DOWNLOAD, &d);
		printf("Average speed downloaded: %lf\n",d);

		curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &d);
		printf("Content Length: %lf\n",d);
*/
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
	if(dlnow > 0)
		frac = 100 * dlnow / dltotal;
	else
		frac = 0;
	printf("\rDownload progress: %3.0lf%% ", frac);
	fflush(stdout);

	return 0;
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
			scanf("%s", newdir); /* TODO: incorporate tab-completion */
		}

	int count;
	for(count = 0; count <= issue->no_of_sections; count++)
	{
		cur_section = issue->section[count];

		snprintf(filename,STR_MAX,"%s/section_%i.pdf", newdir, cur_section->number);
		if(!force){
			testfile = fopen(filename, "r");
			if(!testfile)
				save_file(curl_handle, cur_section->uri, filename);
			else
			{
				fclose(testfile);
				printf("Skipping download of section %i\n", cur_section->number);
			}
		}
		else
			save_file(curl_handle, cur_section->uri, filename);
	}
}
