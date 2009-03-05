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

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <sys/stat.h>
#include <curl/curl.h>

#include "issue.h"
#include "getht.h"

int update_contents_files(struct config * options);

int main(int argc, char *argv[])
{
	struct config options;
	char getht_path[STR_MAX];

	/* initialise appropriate options vars */
	options.startup_check = 0;
	options.quiet = 0;
	options.verbose = 0;
	options.proxy.type = 0;
	options.proxy.auth = 0;
	options.proxy.address[0] = '\0';
	options.proxy.port = 0;
	options.proxy.user[0] = '\0';
	options.proxy.pass[0] = '\0';

	/* Define & set up paths */
	snprintf(getht_path,STR_MAX,"%s/.getht",getenv("HOME"));

	if(!opendir(getht_path))
		if(mkdir(getht_path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))
		{
			fprintf(stderr,"Cannot open/create directory %s\n",getht_path);
			printf("Please enter the path of a directory to save settings in: ");
			scanf("%s", getht_path);
		}

	snprintf(options.toc_xml,STR_MAX,"%s/%s",getht_path,ISS_XML_FILE);

	strncpy(options.issue_uri,XML_TOC_URI,STR_MAX);

	snprintf(options.save_path,STR_MAX,"%s/hinduism_today",getenv("HOME"));

	int downall = 0;
	int downissue = 0, downissueno = -1;
	int downsection = 0, downsectionno = -1;
	int listissues = 0;
	int force = 0;
	int option = 0;

	if(loadconfig(getht_path, &options) != 0)
		writefreshconfig(getht_path, &options);

	/* Parse command line options */
	char c;
	static struct option long_opts[] =
	{
		{"download-all", no_argument, 0, 'a'},
		{"download-issue", required_argument, 0, 'd'},
		{"download-section", required_argument, 0, 's'},
		{"force", no_argument, 0, 'f'},
		{"list-issues", no_argument, 0, 'l'},
		{"quiet", no_argument, 0, 'q'},
		{"update", no_argument, 0, 'u'},
		{"help", no_argument, 0, 'h'},
		{"verbose", no_argument, 0, 'v'},
		{"version", no_argument, 0, 'V'},
		{0, 0, 0, 0}
	};
	while((c = getopt_long(argc, argv, "ad:fs:hluqvV", long_opts, NULL)) != -1) {
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
			case 's':
				downsection = 1;
				downsectionno = atoi(optarg);
				option = 1;
				break;
			case 'l':
				listissues = 1;
				option = 1;
				break;
			case 'f':
				force = 1;
				option = 1;
				break;
			case 'u':
				options.startup_check = 1;
				option = 1;
				break;
			case 'h':
				showusage();
				return 0;
				break;
			case 'q':
				options.quiet = 1;
				option = 1;
				break;
			case 'v':
				options.verbose++;
				option = 1;
				break;
			case 'V':
				printf("GetHT version: %s\n",PACKAGE_VERSION);
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

	options.curl_handle = curl_easy_init();

	if(options.startup_check)
	{
		if(update_contents_files(&options))
			fprintf(stderr,"Could not update contents files\n");
	}

	/* Parse TOC, filling issue structure */

	iss **issue;
	int no_of_issues = -1;
	int i;

	if(downissue || listissues)
	{
		issue = parsetoc(options.toc_xml, &no_of_issues);

		if(!issue)
		{
			if(!options.startup_check)
			{
				printf("Cannot open contents file, trying to update contents\n");
				if(update_contents_files(&options))
					return 1;
				issue = parsetoc(options.toc_xml, &no_of_issues);
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
				downloadissue(&options, issue[i], force);
		}
		else if(downissueno >= 0 && downissueno <= no_of_issues)
		{
			if(downsection && downsectionno >= 0 && downsectionno <= issue[downissueno]->no_of_sections)
			{
				char downdir[STR_MAX];
				sec * cursec;
				strncpy(downdir, (char *) getissuedir(&options, issue[downissueno]), STR_MAX);
				cursec = issue[downissueno]->section[downsectionno];

				printf("Downloading %s to %s\n", cursec->title, downdir);

				downloadsection(&options, cursec, &downdir, force);
			}
			else
				downloadissue(&options, issue[downissueno], force);
		}
	}
	

	if(listissues)
		list_issues(issue, no_of_issues, options.verbose);

	/* Ensure curl cleans itself up */
	curl_easy_cleanup(options.curl_handle);

	return 0;
}

int update_contents_files(struct config * options)
/* Returns 0 on success, 1 on failure */
{
	if(save_file(options->issue_uri, options->toc_xml, "contents", 0, options))
		return 1;
	else
		return 0;
}
