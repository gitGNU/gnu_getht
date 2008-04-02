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

#define XML_TOC_URL		"http://www.hinduismtoday.com/digital/htde_toc.xml"
#define MEDIA_TOC_URL	"http://www.hinduismtoday.com/digital/htde_media-player.rev.gz"

#define MED_XML_FILE	"media_toc.xml"
#define ISS_XML_FILE	"htde_toc.xml"
#define MED_REVGZ_FILE	"htde_media-player.rev.gz"

#define STR_MAX		1024

#define MAX_ISS		10

typedef enum
{
	NONE,
	HTTP,
	SOCKS4,
	SOCKS5
} proxytype;

typedef enum
{
	NOAUTH,
	BASIC,
	DIGEST,
	NTLM,
} proxyauth;
