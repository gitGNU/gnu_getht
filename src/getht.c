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
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <sys/stat.h>
#include <curl/curl.h>

#include "version.h"
#include "issue.h"
#include "getht.h"

int update_contents_files();

med * findnewestmed(iss ** issue, int no_of_issues);

proxytype proxy_type; char proxy_addr[STR_MAX]; long proxy_port;
proxyauth proxy_auth; 
char proxy_user[STR_MAX]; char proxy_pass[STR_MAX];
char issue_xml[STR_MAX]; char media_xml[STR_MAX]; char media_rev[STR_MAX];
CURL *main_curl_handle; 

int main(int argc, char *argv[])
{
	char getht_path[STR_MAX];
	char save_path[STR_MAX];

	/* Define & set up paths */
	snprintf(getht_path,STR_MAX,"%s/.getht",getenv("HOME"));

	if(!opendir(getht_path))
		if(mkdir(getht_path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))
		{
			fprintf(stderr,"Cannot open/create directory %s\n",getht_path);
			printf("Please enter the path of a directory to save settings in: ");
			scanf("%s", getht_path);
		}

	snprintf(issue_xml,STR_MAX,"%s/%s",getht_path,ISS_XML_FILE);
	snprintf(media_xml,STR_MAX, "%s/%s", getht_path, MED_XML_FILE);
	snprintf(media_rev,STR_MAX,"%s/%s",getht_path,MED_REVGZ_FILE);

	snprintf(save_path,STR_MAX,"%s/hinduism_today",getenv("HOME"));

	int downall = 0, downallmedia = 0;
	int downissue = 0, downissueno = -1;
	int downmedia = 0, downmediano = -1;
	int listissues = 0, listmedia = 0;
	int force = 0, update = 0;
	int verbose = 0, option = 0;

	proxy_type = NONE;
	proxy_port = 0;
	proxy_addr[0] = '\0';
	proxy_auth = NOAUTH;
	proxy_user[0] = '\0';
	proxy_pass[0] = '\0';

	if(loadconfig(getht_path, &save_path, &update) != 0)
		writefreshconfig(getht_path, &save_path, &update);

	if(!opendir(save_path))
		if(mkdir(save_path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))
		{
			fprintf(stderr,"Cannot open/create directory %s",save_path);
			printf("Please enter the path of a directory to save issues in: ");
			scanf("%s", save_path);

			updateconfig(getht_path, &save_path, NULL);
		}

	/* Parse command line options */
	char c;
	static struct option long_opts[] =
	{
		{"download-all", no_argument, 0, 'a'},
		{"download-issue", required_argument, 0, 'd'},
		{"download-media", required_argument, 0, 'n'},
		{"download-all-media", no_argument, 0, 'o'},
		{"force", no_argument, 0, 'f'},
		{"list-issues", no_argument, 0, 'l'},
		{"list-media", no_argument, 0, 'm'},
		{"update", no_argument, 0, 'u'},
		{"tocfile", required_argument, 0, 't'},
		{"mediatocfile", required_argument, 0, 'x'},
		{"help", no_argument, 0, 'h'},
		{"verbose", no_argument, 0, 'v'},
		{"version", no_argument, 0, 'V'},
		{0, 0, 0, 0}
	};
	while((c = getopt_long(argc, argv, "ad:fhlmn:osuvVx:t:", long_opts, NULL)) != -1) {
		switch(c) {
			case 'a':
				downall = 1;
				downissue = 1;
				option = 1;
				break;
			case 'd':
				downissue = 1;
				downissueno = atoi(optarg);
				option = 1;
				break;
			case 'l':
				listissues = 1;
				option = 1;
				break;
			case 'm':
				listmedia = 1;
				option = 1;
				break;
			case 'o':
				downallmedia = 1;
				downmedia = 1;
				option = 1;
				break;
			case 'n':
				downmedia = 1;
				downmediano = atoi(optarg);
				option = 1;
				break;
			case 'f':
				force = 1;
				option = 1;
				break;
			case 'u':
				update = 1;
				option = 1;
				break;
			case 't':
				strcpy(issue_xml, strdup(optarg));
				option = 1;
				break;
			case 'x':
				strcpy(media_xml, strdup(optarg));
				option = 1;
				break;
			case 'h':
				showusage();
				return 0;
				break;
			case 'v':
				verbose++;
				option = 1;
				break;
			case 'V':
				printf("GetHT version: %s\n",VERSION);
				option = 1;
				return 0;
				break;
		}
	}

	if(!option)
	{
		showusage();
		return 0;
	}

	main_curl_handle = curl_easy_init();

	if(update)
	{
		if(update_contents_files())
			fprintf(stderr,"Could not update contents files\n");
	}

	/* Parse TOC, filling issue structure */

	iss **issue;
	int no_of_issues = -1;
	int i;

	if(downissue || listissues)
	{
		issue = parsetoc(issue_xml, &no_of_issues);

		if(!issue)
		{
			if(!update)
			{
				printf("Cannot open contents file, trying to update contents\n");
				if(update_contents_files())
					return 1;
				issue = parsetoc(issue_xml, &no_of_issues);
			}
			else
			{
				printf("Cannot open contents file, try running `getht --update`\n");
				return 1;
			}
		}

		if(downall)
		{
			for(i = 0; i < no_of_issues; i++)
				downloadissue(NULL, save_path, issue[i], force);
		}
		else if(downissueno >= 0 && downissueno <= no_of_issues)
			downloadissue(NULL, save_path, issue[downissueno], force);
	}
	
	if(downmedia || listmedia)
	{
		int newest;

		issue = parsemedia(media_xml, issue, &no_of_issues);

		if(!issue)
		{
			if(!update)
			{
				printf("Cannot open media contents file, trying to update contents\n");
				if(update_contents_files())
					return 1;
				issue = parsemedia(media_xml, issue, &no_of_issues);
			}
			else
			{
				printf("Cannot open contents file, try running `getht --update`\n");
				return 1;
			}
		}

		if(downallmedia)
		{
			for(i = 0; i <= no_of_issues; i++)
			{
				for(newest = 0; newest <= issue[i]->no_of_media; newest++)
					downloadmedia(NULL, save_path, issue[i]->media[newest], force);
			}
		}
		else if(downmediano >= 0)
		{
			int med_global, med_no;

			for(i=0,med_global=0; i<=no_of_issues; i++)
				if(issue[i]->no_of_media >= 0)
					for(med_no=0; med_no <= (issue[i]->no_of_media); med_no++,med_global++)
						if(med_global == downmediano)
							downloadmedia(NULL, save_path, issue[i]->media[med_no], force);
		}
	}

	if(listissues)
		list_issues(issue, no_of_issues, verbose);

	if(listmedia)
		list_media(issue, no_of_issues, verbose);

	/* Ensure curl cleans itself up */
	curl_easy_cleanup(main_curl_handle);

	return 0;
}

int update_contents_files()
/* Returns 0 on success, 1 on failure */
{
	if(save_file(NULL, XML_TOC_URL, issue_xml))
		return 1;
	
	char isstitle[STR_MAX];
	issdates date;

	/*	see if current issue's media toc has already
		been written to the xml, if not do so */
	if(access(issue_xml, R_OK) == 0)
	{
		if(cur_identifiers(issue_xml, isstitle, &date))
			return 1;
	}
	else
		return 1;

	if(media_accounted_for(media_xml, &date))
	{
		if(save_file(NULL, MEDIA_TOC_URL, media_rev))
			return 1;
	
		med ** temp_med;
		int med_no = -1;

		temp_med = parsemediagz(media_rev, &med_no);

		unlink(media_rev);

		addmediaissue(media_xml, isstitle, &date, temp_med, med_no);
	}

	return 0;
}

int findnewestiss(iss ** issue, int no_of_issues)
/*	returns newest issue indice */
{
	iss * tmp_issue; issdates newest;
	int new_iss;

	new_iss = -1;
	newest.year = 0; newest.firstmonth = 0; newest.lastmonth = 0;

	int i;

	for(i = 0; i <= no_of_issues; i++)
	{
		if(issue[i]->date.year > newest.year ||
			(issue[i]->date.year == newest.year && issue[i]->date.firstmonth > newest.firstmonth))
		{
			newest.year = issue[i]->date.year;
			newest.firstmonth = issue[i]->date.firstmonth;
			newest.lastmonth = issue[i]->date.lastmonth;
			new_iss = i;
		}
	}

	return new_iss;
}
