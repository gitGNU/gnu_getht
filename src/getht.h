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

#define ISS_XML_FILE	"htde_toc.xml"

#define STR_MAX		512

#include <curl/curl.h>

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
