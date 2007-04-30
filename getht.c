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

void show_iss_struct(iss ** issue, int no_of_issues);

void showusage();

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
			fprintf(stderr,"Cannot open/create directory %s",getht_path);
			printf("Please enter the path of a directory to save settings in: ");
			scanf("%s", getht_path);
		}

	snprintf(issue_xml,STR_MAX,"%s/%s",getht_path,ISS_XML_FILE);
	snprintf(media_xml,STR_MAX, "%s/%s", getht_path, MED_XML_FILE);
	snprintf(media_rev,STR_MAX,"%s/%s",getht_path,MED_REVGZ_FILE);

	snprintf(save_path,STR_MAX,"%s/hinduism_today",getenv("HOME"));

	int downall = 0, downlatest = 0;
	int downallmedia = 0, downlatestmedia = 0;
	int downissue = 0, downmedia = 0;
	int force = 0, update = 0, showstr = 0;
	int option = 0;

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
		{"download-latest", no_argument, 0, 'd'},
		{"download-all-media", no_argument, 0, 'm'},
		{"download-latest-media", no_argument, 0, 'n'},
		{"force", no_argument, 0, 'f'},
		{"update", no_argument, 0, 'u'},
		{"tocfile", required_argument, 0, 't'},
		{"mediatocfile", required_argument, 0, 'x'},
		{"help", no_argument, 0, 'h'},
		{"version", no_argument, 0, 'v'},
		{0, 0, 0, 0}
	};
	while((c = getopt_long(argc, argv, "adfhmnsuvx:t:", long_opts, NULL)) != -1) {
		switch(c) {
			case 'a':
				downall = 1;
				downissue = 1;
				option = 1;
				break;
			case 'd':
				downlatest = 1;
				downissue = 1;
				option = 1;
				break;
			case 'm':
				downallmedia = 1;
				downmedia = 1;
				option = 1;
				break;
			case 'n':
				downlatestmedia = 1;
				downmedia = 1;
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
			case 's':
				showstr = 1;
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
	int latest_index = -1;
	int i;

	if(downissue || showstr)
	{
		issue = parsetoc(issue_xml, &no_of_issues, &latest_index);

		if(!issue)
		{
			if(!update)
			{
				printf("Cannot open contents file, trying to update contents\n");
				if(update_contents_files())
					return 1;
				issue = parsetoc(issue_xml, &no_of_issues, &latest_index);
			}
			else
			{
				printf("Cannot open contents file, try running `getht --update`\n");
				return 1;
			}
		}

		if(latest_index == -1)
		{
			fprintf(stderr, "Error: Cannot ascertain latest issue. ");
			fprintf(stderr, "Defaulting to first issue in contents file\n");
			latest_index = 0;
		}

		if(downall)
		{
			for(i = 0; i < no_of_issues; i++)
				downloadissue(NULL, save_path, issue[i], force);
		}
	
		if(downlatest)
			downloadissue(NULL, save_path, issue[latest_index], force);
	}
	
	if(downmedia || showstr)
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

		if(downlatestmedia)
		{
			newest = findnewestiss(issue, no_of_issues);
			for(i = 0; i <= issue[newest]->no_of_media; i++)
				downloadmedia(NULL, save_path, issue[newest]->media[i], force);
		}

		if(downallmedia)
		{
			for(i = 0; i <= no_of_issues; i++)
			{
				for(newest = 0; newest <= issue[i]->no_of_media; newest++)
					downloadmedia(NULL, save_path, issue[i]->media[newest], force);
			}
		}
	}

	if(showstr)
		show_iss_struct(issue, no_of_issues);

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

void show_iss_struct(iss ** issue, int no_of_issues)
/*	Prints issue information */
{
	int iss_no, sec_no, med_no, it_no;
	printf("%i Issues\n",no_of_issues);
	for(iss_no=0;iss_no<=no_of_issues;iss_no++)
	{
		printf("-Issue %i-\n", (iss_no+1));
		printf("Title:\t'%s'\n", issue[iss_no]->title);
		printf("Preview URI:\t'%s'\n", issue[iss_no]->preview_uri);
		printf("Months:\t'%i' - '%i'\n",issue[iss_no]->date.firstmonth,issue[iss_no]->date.lastmonth);
		printf("Year:\t'%i'\n",issue[iss_no]->date.year);
		printf("Number of Sections:\t'%i'\n",issue[iss_no]->no_of_sections);

		for(sec_no=0; sec_no <= (issue[iss_no]->no_of_sections); sec_no++)
		{
			printf("\t-Section %i-\n", (sec_no));
			printf("\tTitle:\t'%s'\n", issue[iss_no]->section[sec_no]->title);
			printf("\tURI:\t'%s'\n", issue[iss_no]->section[sec_no]->uri);
			printf("\tNo. of Items:\t'%i'\n", issue[iss_no]->section[sec_no]->no_of_items);

			for(it_no=0; it_no <= issue[iss_no]->section[sec_no]->no_of_items; it_no++)
			{
				printf("\t\t-Item-\n");
				printf("\t\tTitle:\t'%s'\n",issue[iss_no]->section[sec_no]->item[it_no]->title);
				printf("\t\tFirst page:\t'%i'",issue[iss_no]->section[sec_no]->item[it_no]->firstpage);
				printf("\tLast page:\t'%i'\n",issue[iss_no]->section[sec_no]->item[it_no]->lastpage);
			}
			it_no = 0;
		}
		sec_no = 0;

		printf("Number of Media:\t'%i'\n",issue[iss_no]->no_of_media);

		for(med_no=0; med_no <= (issue[iss_no]->no_of_media); med_no++)
		{
			printf("\t-Media %i-\n", (med_no));
			printf("\tTitle:\t'%s'\n", issue[iss_no]->media[med_no]->title);
			printf("\tURI:\t'%s'\n", issue[iss_no]->media[med_no]->uri);
			printf("\tComment:\t'%s'\n", issue[iss_no]->media[med_no]->comment);
			printf("\tPreview URI:\t'%s'\n", issue[iss_no]->media[med_no]->preview_uri);
		}
		med_no = 0;
	}
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

void showusage()
{
	printf("Usage: getht -u -a -d -m -n -f [-t tocfile] -h -v\n");
	printf("-u | --update                 Update contents files\n");
	printf("-a | --download-all           Download all issues\n");
	printf("-d | --download-latest        Download latest issue\n");
	printf("-m | --download-all-media     Download all media\n");
	printf("-n | --download-latest-media  Download latest issue's media\n");
	printf("-f | --force                  Force re-download of existing files\n");
	printf("-t | --tocfile file           Use alternative contents xml file\n");
	printf("-x | --mediatocfile file      Use alternative media contents xml file\n");
	printf("-h | --help                   Print this help message\n");
	printf("-v | --version                Print version information\n");
}
