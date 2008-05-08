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

#include "getht.h"

#include <curl/curl.h>

int loadconfig(char * getht_path, struct config * options)
/*	Loads variables from config file to extern and passed
 *	variables. */
{
	FILE * config_file;
	char filepath[STR_MAX];

	snprintf(filepath, STR_MAX, "%s/config.ini", getht_path);

	if((config_file = fopen(filepath,"r")) == NULL)
	{
		fprintf(stderr,"Cannot open file %s for reading.\n",filepath);
		return 1;
	}

	char parameter[80], option[80];
	while(!feof(config_file))
	{
		fscanf(config_file, "%s = %s", option, parameter);

		if(option[0] == '#');	/* ignore lines beginning with a hash */
		else if(!strcmp(option, "savepath"))
			strncpy(options->save_path, parameter, STR_MAX);
		else if(!strcmp(option, "startup_check"))
			options->startup_check = atoi(parameter);
		else if(!strcmp(option, "toc_uri"))
			strncpy(options->issue_uri, parameter, STR_MAX);
		else if(!strcmp(option, "proxy_type"))
		{
			if(!strcmp(parameter, "http"))
				options->proxy.type = CURLPROXY_HTTP;
			else if(!strcmp(parameter, "socks4"))
				options->proxy.type = CURLPROXY_SOCKS4;
			else if(!strcmp(parameter, "socks5"))
				options->proxy.type = CURLPROXY_SOCKS5;
			else
				fprintf(stderr,
					"Proxy type %s not known, please use either http, socks4 or socks5\n",
					parameter);
		}
		else if(!strcmp(option, "proxy_address"))
			strncpy(options->proxy.address, parameter, STR_MAX);
		else if(!strcmp(option, "proxy_port"))
			options->proxy.port = (long) atoi(parameter);
		else if(!strcmp(option, "proxy_auth"))
		{
			if(!strcmp(parameter, "basic"))
				options->proxy.auth = CURLAUTH_BASIC;
			else if(!strcmp(parameter, "digest"))
				options->proxy.auth = CURLAUTH_DIGEST;
			else if(!strcmp(parameter, "ntlm"))
				options->proxy.auth = CURLAUTH_NTLM;
			else
				fprintf(stderr,
					"Proxy authentication method %s not known, please use basic, digest or ntlm",
					parameter);
		}
		else if(!strcmp(option, "proxy_user"))
			strncpy(options->proxy.user, parameter, STR_MAX);
		else if(!strcmp(option, "proxy_pass"))
			strncpy(options->proxy.pass, parameter, STR_MAX);
		else
			fprintf(stderr, "Option '%s' not recognised, ignoring\n", option);
	}

	return 0;
}

int writefreshconfig(char * getht_path, struct config * options)
/*	Write a new config file according to extern and passed variables. */
{
	FILE * config_file;
	char filepath[STR_MAX];

	snprintf(filepath, STR_MAX, "%s/config.ini", getht_path);

	if((config_file = fopen(filepath,"w")) == NULL)
	{
		fprintf(stderr,"Cannot open file %s for writing.\n",filepath);
		return 1;
	}
	else
		fprintf(stdout,"Writing a fresh config file to %s.\n",filepath);

	if(options->save_path[0])
		fprintf(config_file, "%s = %s\n", "savepath", options->save_path);
	if(options->startup_check)
		fprintf(config_file, "%s = %i\n", "startup_check", options->startup_check);
	if(options->issue_uri[0])
		fprintf(config_file, "%s = %s\n", "toc_uri", options->issue_uri);
	if(options->proxy.type)
	{
		if(options->proxy.type == CURLPROXY_HTTP)
			fprintf(config_file, "%s = %s\n", "proxy_type", "http");
		else if(options->proxy.type == CURLPROXY_SOCKS4)
			fprintf(config_file, "%s = %s\n", "proxy_type", "socks4");
		else if(options->proxy.type == CURLPROXY_SOCKS5)
			fprintf(config_file, "%s = %s\n", "proxy_type", "socks5");
	}
	if(options->proxy.address[0])
		fprintf(config_file, "%s = %s\n", "proxy_address", options->proxy.address);
	if(options->proxy.port)
		fprintf(config_file, "%s = %i\n", "proxy_port", options->proxy.port);
	if(options->proxy.auth)
	{
		if(options->proxy.auth = CURLAUTH_BASIC)
			fprintf(config_file, "%s = %s\n", "proxy_auth", "basic");
		else if(options->proxy.auth = CURLAUTH_DIGEST)
			fprintf(config_file, "%s = %s\n", "proxy_auth", "digest");
		else if(options->proxy.auth = CURLAUTH_NTLM)
			fprintf(config_file, "%s = %s\n", "proxy_auth", "ntlm");
	}
	if(options->proxy.user[0])
		fprintf(config_file, "%s = %s\n", "proxy_user", options->proxy.user);
	if(options->proxy.pass[0])
		fprintf(config_file, "%s = %s\n", "proxy_pass", options->proxy.pass);
		
	return 0;
}

int updateconfig(char * getht_path, struct config * options)
/*	Read existing config file, and rewrite any variables which differ
 *	in memory. */
{
	FILE * config_file;
	char filepath[STR_MAX];

	snprintf(filepath, STR_MAX, "%s/config.ini", getht_path);

	if((config_file = fopen(filepath,"rw")) == NULL)
	{
		fprintf(stderr,"Cannot open file %s for reading/writing.\n",filepath);
		return 1;
	}

	fprintf(stderr,"Not yet implemented.\n");

	return 1;
}
