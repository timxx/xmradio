/** 
 * radioinfo.c
 * This file is part of xmradio
 *
 * Copyright (C) 2012  Weitian Leung (weitianleung@gmail.com)

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "radioinfo.h"

RadioInfo *
radio_info_new()
{
	return g_new0(RadioInfo, 1);
}

void
radio_info_free(RadioInfo *ri)
{
	if (ri)
	{
		g_free(ri->id);
		g_free(ri->name);
		g_free(ri->logo);
		g_free(ri->url);

		g_free(ri);
	}
}
