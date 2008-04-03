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

/* all sizes are in bytes and are child-inclusive */

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
