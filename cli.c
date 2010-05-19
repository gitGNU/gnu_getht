/*
 * This file is part of GetHT
 *
 * See COPYING file for copyright, license and warranty details.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "getht.h"

void list_issues(iss ** issue, int no_of_issues, int verbose)
{
	int iss_no, sec_no, it_no;
	for(iss_no=0;iss_no<=no_of_issues;iss_no++)
	{
		printf("%2i: %s\n", iss_no, issue[iss_no]->title);
		if(verbose >= 2)
			printf("    Year: %i; Months: %i - %i\n",issue[iss_no]->date.year,issue[iss_no]->date.firstmonth,issue[iss_no]->date.lastmonth);
		if(verbose >= 1)
		{
			for(sec_no=0; sec_no<=issue[iss_no]->no_of_sections; sec_no++)
			{
				printf("   %2i: %s\n", issue[iss_no]->section[sec_no]->number,
					issue[iss_no]->section[sec_no]->title);
				printf("       %s\n", issue[iss_no]->section[sec_no]->uri);
				if(verbose >= 2)
				{
					for(it_no=0;
						it_no<=issue[iss_no]->section[sec_no]->no_of_items;
						it_no++)
					{
						printf("         pp %2i - %2i: %s\n",
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
	printf("Usage: getht [options] -a\n");
	printf("  or: getht [options] -d issno [-s secno]\n");
	printf("  or: getht [options] -l\n");
	printf("Downloads issue(s) of Hinduism Today\n\n");
	printf("Download options:\n");
	printf("  -a, --download-all            Download all issues\n");
	printf("  -d, --download-issue issno    Download issue number issno\n");
	printf("  -s, --download-section secno  Download section number secno\n");
	printf("  -f, --force                   Force re-download of existing files\n");
	printf("Discovery options:\n");
	printf("  -l, --list-issues             List available issues\n");
	printf("  -u, --update                  Update contents files\n");
	printf("Global options:\n");
	printf("  -q, --quiet                   Make output less verbose\n");
	printf("  -v, --verbose                 Make output more verbose\n");
	printf("Other options:\n");
	printf("  -h, --help                    Print this help message\n");
	printf("  -V, --version                 Print version information\n");
}
