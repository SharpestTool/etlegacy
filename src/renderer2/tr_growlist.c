/**
 * Wolfenstein: Enemy Territory GPL Source Code
 * Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
 * Copyright (C) 2010-2011 Robert Beckebans <trebor_7@users.sourceforge.net>
 *
 * ET: Legacy
 * Copyright (C) 2012 Jan Simek <mail@etlegacy.com>
 *
 * This file is part of ET: Legacy - http://www.etlegacy.com
 *
 * ET: Legacy is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ET: Legacy is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ET: Legacy. If not, see <http://www.gnu.org/licenses/>.
 *
 * In addition, Wolfenstein: Enemy Territory GPL Source Code is also
 * subject to certain additional terms. You should have received a copy
 * of these additional terms immediately following the terms and conditions
 * of the GNU General Public License which accompanied the source code.
 * If not, please request a copy in writing from id Software at the address below.
 *
 * id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
 *
 * @file tr_growlist.c
 */

#include "tr_local.h"

/*
============================================================================
GROWLISTS
============================================================================
*/

// malloc / free all in one place for debugging

void Com_InitGrowList(growList_t *list, int maxElements)
{
	list->maxElements     = maxElements;
	list->currentElements = 0;
	list->elements        = (void **)Com_Allocate(list->maxElements * sizeof(void *));
}

void Com_DestroyGrowList(growList_t *list)
{
	Com_Dealloc(list->elements);
	memset(list, 0, sizeof(*list));
}

int Com_AddToGrowList(growList_t *list, void *data)
{
	void **old;

	if (list->currentElements != list->maxElements)
	{
		list->elements[list->currentElements] = data;
		return list->currentElements++;
	}

	// grow, reallocate and move
	old = list->elements;

	if (list->maxElements < 0)
	{
		Com_Error(ERR_FATAL, "Com_AddToGrowList: maxElements = %i", list->maxElements);
	}

	if (list->maxElements == 0)
	{
		// initialize the list to hold 100 elements
		Com_InitGrowList(list, 100);
		return Com_AddToGrowList(list, data);
	}

	list->maxElements *= 2;

	//Com_DPrintf("Resizing growlist to %i maxElements\n", list->maxElements);

	list->elements = (void **)Com_Allocate(list->maxElements * sizeof(void *));

	if (!list->elements)
	{
		Com_Error(ERR_DROP, "Growlist alloc failed");
	}

	Com_Memcpy(list->elements, old, list->currentElements * sizeof(void *));

	Com_Dealloc(old);

	return Com_AddToGrowList(list, data);
}

void *Com_GrowListElement(const growList_t *list, int index)
{
	if (index < 0 || index >= list->currentElements)
	{
		Com_Error(ERR_DROP, "Com_GrowListElement: %i out of range of %i", index, list->currentElements);
	}
	return list->elements[index];
}

int Com_IndexForGrowListElement(const growList_t *list, const void *element)
{
	int i;

	for (i = 0; i < list->currentElements; i++)
	{
		if (list->elements[i] == element)
		{
			return i;
		}
	}
	return -1;
}
