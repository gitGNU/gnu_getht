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

#define XML_TOC_URI		"http://www.hinduismtoday.com/digital/htde_toc.xml"

/* currently unused - see 0.1 patch-30 for latest version parsing media */
#define MEDIA_TOC_URI	"http://www.hinduismtoday.com/digital/htde_media-player.rev.gz"

#define ISS_XML_FILE	"htde_toc.xml"

#define STR_MAX		512

#include <curl/curl.h>

struct config {
	char * proxytype;
	char * proxyauth;
	char proxy_addr[STR_MAX];
	long proxy_port;
	char proxy_user[STR_MAX];
	char proxy_pass[STR_MAX];

	char toc_xml[STR_MAX];
	char issue_uri[STR_MAX];

	char save_path[STR_MAX];

	int startup_check;

	CURL *curl_handle;
};
