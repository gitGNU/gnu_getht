/*
 * This file is part of GetHT
 *
 * See COPYING file for copyright, license and warranty details.
 *
 */

#define XML_TOC_URI		"http://www.hinduismtoday.com/digital/htde_toc.xml"

#define ISS_XML_FILE	"htde_toc.xml"

#define STR_MAX		512

#include <curl/curl.h>
#include <libxml/xmlmemory.h>

struct proxy_options {
	char type;
	char auth;
	char address[STR_MAX];
	long port;
	char user[STR_MAX];
	char pass[STR_MAX];
};

struct config {
	char toc_xml[STR_MAX];
	char issue_uri[STR_MAX];

	char save_path[STR_MAX];

	struct proxy_options proxy;

	int startup_check;

	int verbose;
	int quiet;

	CURL *curl_handle;
};

typedef struct
{
	int year;
	int firstmonth;
	int lastmonth;
} issdates;

typedef struct
{
	int firstpage;
	int lastpage;
	char * title;
} it;

typedef struct
{
	char uri[512];
	char title[512];
	int number;
	int size;
	it ** item;
	int no_of_items;
} sec;

typedef struct
{
	char preview_uri[512];
	char title[512];
	int size;
	issdates date;
	sec ** section;
	int no_of_sections;
} iss;

iss ** parsetoc(char *filepath, int * iss_no);

iss ** assignnew_iss(iss ** issue, int *no_of_issues);
sec ** assignnew_sec(sec ** section, int * no_of_sections);
it ** assignnew_it(it ** item, int * no_of_items);

int updateconfig(char * getht_path, struct config * options);
int loadconfig(char * getht_path, struct config * options);
int writefreshconfig(char * getht_path, struct config * options);
void showusage();
void downloadissue(struct config * options, iss * issue, int force);
char * getissuedir(struct config * options, iss * issue);
void downloadsection(struct config * options, sec * section, char * downdir, int force);
void list_issues(iss ** issue, int no_of_issues, int verbose);
int save_file(char *uri, char *filepath, char *filetitle, long resume_offset, struct config * options);
int ready_xml(char * filepath, char * rootnode, xmlDocPtr * file, xmlNodePtr * node);
int issuesort(iss ** issue, int no_of_issues);
