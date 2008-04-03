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
#include <stdlib.h>

#include "issue.h"
#include "getht.h"

void list_issues(iss ** issue, int no_of_issues, int verbose)
{
	int iss_no, sec_no, it_no;
	for(iss_no=0;iss_no<=no_of_issues;iss_no++)
	{
		printf("[%i]\t", iss_no);
		printf("%s\n", issue[iss_no]->title);
		if(verbose >= 2)
			printf("Year: %i; Months: %i - %i\n",issue[iss_no]->date.year,issue[iss_no]->date.firstmonth,issue[iss_no]->date.lastmonth);
		if(verbose >= 1)
		{
			for(sec_no=0; sec_no<=issue[iss_no]->no_of_sections; sec_no++)
			{
				printf("\t%i: %s\n", issue[iss_no]->section[sec_no]->number,
					issue[iss_no]->section[sec_no]->title);
				printf("\t%s\n", issue[iss_no]->section[sec_no]->uri);
				if(verbose >= 2)
				{
					for(it_no=0;
						it_no<=issue[iss_no]->section[sec_no]->no_of_items;
						it_no++)
					{
						printf("\t\tpp %i - %i: %s\n",
							issue[iss_no]->section[sec_no]->item[it_no]->firstpage,
							issue[iss_no]->section[sec_no]->item[it_no]->lastpage,
							issue[iss_no]->section[sec_no]->item[it_no]->title);
					}
				}
			}
		}
	}
}

void showusage()
{
	printf("Usage: getht [-ualmofhv] [-d issno] [-n medno] [-t tocfile]\n");
	printf("-u | --update                 Update contents files\n");
	printf("-a | --download-all           Download all issues\n");
	printf("-d | --download-issue issno   Download issue number issno\n");
	printf("-f | --force                  Force re-download of existing files\n");
	printf("-l | --list-issues            List available issues\n");
	printf("-t | --tocfile file           Use alternative contents xml file\n");
	printf("-h | --help                   Print this help message\n");
	printf("-v | --verbose                Make output more verbose\n");
	printf("-V | --version                Print version information\n");
}
