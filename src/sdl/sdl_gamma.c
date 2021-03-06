/*
 * Wolfenstein: Enemy Territory GPL Source Code
 * Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
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
 */
/**
 * @file sdl_gamma.c
 */

#if defined(FEATURE_RENDERER2)
#include "../renderer2/tr_local.h"
#elif defined(FEATURE_RENDERER_GLES)
#include "../rendererGLES/tr_local.h"
#else
#include "../renderer/tr_local.h"
#endif

#include "../qcommon/qcommon.h"

#include "sdl_defs.h"

#ifdef _WIN32
#include <windows.h>
#endif

/*
=================
GLimp_SetGamma
=================
*/
void GLimp_SetGamma(unsigned char red[256], unsigned char green[256], unsigned char blue[256])
{
	Uint16 table[3][256];
	int    i, j;

	if (!glConfig.deviceSupportsGamma || r_ignorehwgamma->integer > 0)
	{
		Ren_Warning("Device doesn't support gamma or r_ignorehwgamma is set.\n");
		return;
	}

	for (i = 0; i < 256; i++)
	{
		table[0][i] = ((( Uint16 ) red[i]) << 8) | red[i];
		table[1][i] = ((( Uint16 ) green[i]) << 8) | green[i];
		table[2][i] = ((( Uint16 ) blue[i]) << 8) | blue[i];
	}

#ifdef _WIN32

	// Win2K and newer put this odd restriction on gamma ramps...
	{
		OSVERSIONINFO vinfo;

		vinfo.dwOSVersionInfoSize = sizeof(vinfo);
		GetVersionEx(&vinfo);
		if (vinfo.dwMajorVersion >= 5 && vinfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
		{
			Ren_Developer("performing gamma clamp.\n");
			for (j = 0 ; j < 3 ; j++)
			{
				for (i = 0 ; i < 128 ; i++)
				{
					if (table[j][i] > ((128 + i) << 8))
					{
						table[j][i] = (128 + i) << 8;
					}
				}

				if (table[j][127] > 254 << 8)
				{
					table[j][127] = 254 << 8;
				}
			}
		}
	}
#endif

	// enforce constantly increasing
	for (j = 0; j < 3; j++)
	{
		for (i = 1; i < 256; i++)
		{
			if (table[j][i] < table[j][i - 1])
			{
				table[j][i] = table[j][i - 1];
			}
		}
	}

	SDL_SetWindowGammaRamp(GLimp_MainWindow(), table[0], table[1], table[2]);
}
