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
 * @file cl_console.c
 * @brief Ingame console
 * Must hold SHIFT + ~ to get console. CTRL + SHIFT + '~' opens a small console.
 */

#include "client.h"
#include "../qcommon/q_unicode.h"

#define CONSOLE_COLOR  COLOR_WHITE
#define DEFAULT_CONSOLE_WIDTH   99

int g_console_field_width = DEFAULT_CONSOLE_WIDTH;

console_t con;

cvar_t *con_conspeed;
cvar_t *con_autoclear;

vec4_t console_color = { 1.0, 1.0, 1.0, 1.0 };
vec4_t console_highlightcolor = { 0.5, 0.5, 0.2, 0.45 };

/**
 * @brief Toggle console
 */
void Con_ToggleConsole_f(void)
{
	con.highlightOffset = 0;

	// persistent console input is more useful (added cvar)
	if (con_autoclear->integer)
	{
		Field_Clear(&g_consoleField);
	}

	g_consoleField.widthInChars = g_console_field_width;

	// multiple console size support
	if (cls.keyCatchers & KEYCATCH_CONSOLE)
	{
		cls.keyCatchers &= ~KEYCATCH_CONSOLE;
		con.desiredFrac  = 0.0f;
	}
	else
	{
		cls.keyCatchers |= KEYCATCH_CONSOLE;

		// short console
		if (keys[K_CTRL].down)
		{
			con.desiredFrac = (4.0 * SMALLCHAR_HEIGHT) / cls.glconfig.vidHeight;
		}
		// full console
		else if (keys[K_ALT].down)
		{
			con.desiredFrac = 1.0f;
		}
		// normal half-screen console
		else
		{
			con.desiredFrac = 0.5f;
		}
	}
}

/**
 * @brief Clear console
 */
void Con_Clear_f(void)
{
	int i;

	for (i = 0 ; i < CON_TEXTSIZE ; i++)
	{
		con.text[i]      = ' ';
		con.textColor[i] = ColorIndex(CONSOLE_COLOR);
	}

	con.totallines = 0;

	Con_Bottom();
}

/**
 * @brief Save content to a file
 */
void Con_Dump_f(void)
{
	int          l, x, i;
	unsigned int *line;
	fileHandle_t f;
	int          bufferlen;
	char         *buffer;
	char         filename[MAX_QPATH];

	if (Cmd_Argc() != 2)
	{
		Com_Printf("usage: condump <filename>\n");
		return;
	}

	Q_strncpyz(filename, Cmd_Argv(1), sizeof(filename));
	COM_DefaultExtension(filename, sizeof(filename), ".txt");

	f = FS_FOpenFileWrite(filename);
	if (!f)
	{
		Com_Printf("ERROR: couldn't open %s.\n", filename);
		return;
	}

	Com_Printf("Dumped console text to %s.\n", filename);

	// skip empty lines
	for (l = con.current - con.maxtotallines + 1 ; l <= con.current ; l++)
	{
		line = con.text + (l % con.maxtotallines) * con.linewidth;
		for (x = 0 ; x < con.linewidth ; x++)
			if (line[x] != ' ')
			{
				break;
			}
		if (x != con.linewidth)
		{
			break;
		}
	}

#ifdef _WIN32
	bufferlen = con.linewidth + 3 * sizeof(char);
#else
	bufferlen = con.linewidth + 2 * sizeof(char);
#endif

	buffer = Hunk_AllocateTempMemory(bufferlen);

	// write the remaining lines
	buffer[bufferlen - 1] = 0;
	for ( ; l <= con.current ; l++)
	{
		line = con.text + (l % con.maxtotallines) * con.linewidth;
		for (i = 0; i < con.linewidth; i++)
			buffer[i] = line[i] & 0xff;
		for (x = con.linewidth - 1 ; x >= 0 ; x--)
		{
			if (buffer[x] == ' ')
			{
				buffer[x] = 0;
			}
			else
			{
				break;
			}
		}
#ifdef _WIN32
		Q_strcat(buffer, bufferlen, "\r\n");
#else
		Q_strcat(buffer, bufferlen, "\n");
#endif
		FS_Write(buffer, strlen(buffer), f);
	}

	Hunk_FreeTempMemory(buffer);
	FS_FCloseFile(f);
}

/**
 * @brief Reformat the buffer if the line width has changed
 */
void Con_CheckResize(void)
{
	int             i, width;
	MAC_STATIC int  tbuf[CON_TEXTSIZE];
	MAC_STATIC byte tbuff[CON_TEXTSIZE];

	// wasn't allowing for larger consoles
	// width = (SCREEN_WIDTH / SMALLCHAR_WIDTH) - 2;
	width = (cls.glconfig.vidWidth / SMALLCHAR_WIDTH) - 2;

	if (width == con.linewidth)
	{
		return;
	}

	if (width < 1)            // video hasn't been initialized yet
	{
		con.linewidth     = DEFAULT_CONSOLE_WIDTH;
		con.maxtotallines = CON_TEXTSIZE / con.linewidth;
		for (i = 0; i < CON_TEXTSIZE; i++)
		{
			con.text[i]      = ' ';
			con.textColor[i] = ColorIndex(CONSOLE_COLOR);
		}
		con.totallines = 0;
	}
	else
	{
		int j;
		int oldtotallines, numlines, numchars;
		int oldwidth = con.linewidth;

		con.linewidth     = width;
		oldtotallines     = con.maxtotallines;
		con.maxtotallines = CON_TEXTSIZE / con.linewidth;
		numlines          = oldtotallines;

		if (con.maxtotallines < numlines)
		{
			numlines = con.maxtotallines;
		}

		numchars = oldwidth;

		if (con.linewidth < numchars)
		{
			numchars = con.linewidth;
		}

		memcpy(tbuf, con.text, CON_TEXTSIZE * sizeof(int));
		memcpy(tbuff, con.textColor, CON_TEXTSIZE * sizeof(byte));
		for (i = 0; i < CON_TEXTSIZE; i++)
		{
			con.text[i]      = ' ';
			con.textColor[i] = ColorIndex(CONSOLE_COLOR);
		}

		//con.totallines = 0;

		for (i = 0 ; i < numlines ; i++)
		{
			for (j = 0 ; j < numchars ; j++)
			{
				con.text[(con.maxtotallines - 1 - i) * con.linewidth + j] =
				    tbuf[((con.current - i + oldtotallines) %
				          oldtotallines) * oldwidth + j];
				con.textColor[(con.maxtotallines - 1 - i) * con.linewidth + j] =
				    tbuff[((con.current - i + oldtotallines) %
				           oldtotallines) * oldwidth + j];
			}
		}
		//con.totallines = numlines;
	}

	con.current = con.maxtotallines - 1;
	con.display = con.current;
	con.scrollline = con.current;
}

/**
 * @brief Complete file text name
 */
void Cmd_CompleteTxtName(char *args, int argNum)
{
	if (argNum == 2)
	{
		Field_CompleteFilename("", "txt", qfalse, qtrue);
	}
}

/**
 * @brief Initialize console
 */
void Con_Init(void)
{
	int i;

	con_conspeed   = Cvar_Get("scr_conspeed", "3", 0);
	con_autoclear  = Cvar_Get("con_autoclear", "1", CVAR_ARCHIVE);

	Field_Clear(&g_consoleField);
	g_consoleField.widthInChars = g_console_field_width;
	for (i = 0 ; i < COMMAND_HISTORY ; i++)
	{
		Field_Clear(&historyEditLines[i]);
		historyEditLines[i].widthInChars = g_console_field_width;
	}

	Cmd_AddCommand("toggleconsole", Con_ToggleConsole_f);
	Cmd_AddCommand("clear", Con_Clear_f);
	Cmd_AddCommand("condump", Con_Dump_f);
	Cmd_SetCommandCompletionFunc("condump", Cmd_CompleteTxtName);
}

/**
 * @brief Free client console commands before shutdown
 */
void Con_Shutdown(void)
{
	Cmd_RemoveCommand("toggleconsole");
	Cmd_RemoveCommand("clear");
	Cmd_RemoveCommand("condump");
}

/**
 * @brief Line feed
 */
void Con_Linefeed(void)
{
	int i;
	con.x = 0;

	if (con.scrollline >= con.current)
	{
		con.scrollline++;
	}

	con.current++;

	if (con.totallines < con.maxtotallines)
	{
		con.totallines++;
	}

	for (i = 0; i < con.linewidth; i++)
	{
		con.text[(con.current % con.maxtotallines) * con.linewidth + i]      = ' ';
		con.textColor[(con.current % con.maxtotallines) * con.linewidth + i] = ColorIndex(CONSOLE_COLOR);
	}
}


#if defined(_WIN32) && !defined(LEGACY_DEBUG)
#pragma optimize( "g", off ) // msvc totally screws this function up with optimize on
#endif
/**
 * @brief Handles cursor positioning, line wrapping, etc
 */
void CL_ConsolePrint(char *txt)
{
	int y;
	int c, l;
	int color;

	// for some demos we don't want to ever show anything on the console
	if (cl_noprint && cl_noprint->integer)
	{
		return;
	}

	if (!con.initialized)
	{
		con.color[0]  = con.color[1] = con.color[2] = con.color[3] = 1.0f;
		con.linewidth = -1;
		Con_CheckResize();
		con.initialized = qtrue;
	}

	// work around for text that shows up in console but not in notify
	if (!Q_strncmp(txt, "[skipnotify]", 12))
	{
		txt += 12;
	}

	color = ColorIndex(CONSOLE_COLOR);

	while (*txt != 0)
	{
		if (Q_IsColorString(txt))
		{
			if (*(txt + 1) == COLOR_NULL)
			{
				color = ColorIndex(CONSOLE_COLOR);
			}
			else
			{
				color = ColorIndex(*(txt + 1));
			}
			txt += 2;
			continue;
		}

		// count word length
		for (l = 0 ; l < con.linewidth ; l++)
		{
			if (txt[l] <= ' ')
			{
				break;
			}
		}

		// word wrap
		if (l != con.linewidth && (con.x + l >= con.linewidth))
		{
			Con_Linefeed();
		}

		c = Q_UTF8_CodePoint(txt);

		switch (c)
		{
		case '\n':
			Con_Linefeed();
			break;
		case '\r':
			con.x = 0;
			break;
		default:
			// display character and advance
			y = con.current % con.maxtotallines;

			// sign extension caused the character to carry over
			// into the color info for high ascii chars; casting c to unsigned
			con.text[y * con.linewidth + con.x]      = c;
			con.textColor[y * con.linewidth + con.x] = color;
			con.x++;
			if (con.x >= con.linewidth)
			{
				Con_Linefeed();
				con.x = 0;
			}
			break;
		}

		txt += Q_UTF8_Width(txt);
	}
}

#if defined(_WIN32) && !defined(LEGACY_DEBUG)
#pragma optimize( "g", on ) // re-enabled optimization
#endif

/**
 * @brief Draw version text
 */
void Con_DrawVersion(void)
{
	int  x, i;
	char version[256] = ET_VERSION;

	// draw update
	if (Cvar_VariableIntegerValue("com_updateavailable"))
	{
		Com_sprintf(version, sizeof(version), _("%s (UPDATE AVAILABLE)"), ET_VERSION);
	}

	i = strlen(version);

	for (x = 0 ; x < i ; x++)
	{
		if (x > strlen(ET_VERSION))
		{
			re.SetColor(g_color_table[ColorIndex(COLOR_GREEN)]);
		}

		SCR_DrawSmallChar(cls.glconfig.vidWidth - (i - x + 1) * SMALLCHAR_WIDTH,
		                  con.scanlines - 1.25f * SMALLCHAR_HEIGHT, version[x]);
	}

}

void Con_DrawDebug(void)
{
	// debug
	int  x, i;
	char buffer[256];

	re.SetColor(g_color_table[ColorIndex(COLOR_YELLOW)]);

	Com_sprintf(buffer, sizeof(buffer), "Total %d/%d Current %d Scroll %d Display %d to %d (%d) (%d lines/%ipx)",
		con.totallines, con.maxtotallines, con.current, con.scrollline,
		con.display-con.vislines, con.display, con.scrollline-con.display, con.vislines, SMALLCHAR_HEIGHT);

	i = strlen(buffer);

	for (x = 0 ; x < i ; x++)
	{
		SCR_DrawSmallChar(cls.glconfig.vidWidth - (i - x + 1) * SMALLCHAR_WIDTH,
		                  con.scanlines - 1.25f * SMALLCHAR_HEIGHT, buffer[x]);
	}
}

/**
 * @brief Draw the editline after a ] prompt
 */
void Con_DrawInput(void)
{
	int y;

	if (cls.state != CA_DISCONNECTED && !(cls.keyCatchers & KEYCATCH_CONSOLE))
	{
		return;
	}

	y = con.scanlines - 1.25f * SMALLCHAR_HEIGHT;

	// hightlight the current autocompleted part
	if (con.highlightOffset)
	{
		if (strlen(g_consoleField.buffer) > 0)
		{
			re.SetColor(console_highlightcolor);
			re.DrawStretchPic(con.xadjust + (2 + con.highlightOffset) * SMALLCHAR_WIDTH,
			                  y + 2,
			                  (strlen(g_consoleField.buffer) - con.highlightOffset) * SMALLCHAR_WIDTH,
			                  SMALLCHAR_HEIGHT - 2, 0, 0, 0, 0, cls.whiteShader);
		}
	}

	re.SetColor(con.color);

	SCR_DrawSmallChar(con.xadjust + 1 * SMALLCHAR_WIDTH, y, ']');

	Field_Draw(&g_consoleField, con.xadjust + 2 * SMALLCHAR_WIDTH, y,
	           SCREEN_WIDTH - 3 * SMALLCHAR_WIDTH, qtrue, qtrue);
}

/**
 * @brief Draw scrollbar
 */
void Con_DrawScrollbar(int length, float x, float y)
{
	vec4_t      color          = { 0.2f, 0.2f, 0.2f, 0.75f };
	const float width          = 1.0f;
	const float handleLength   = con.totallines ? length *MIN(1.0f, (float) con.vislines / con.totallines) : 0;
	const float lengthPerLine  = (length - handleLength) / (con.totallines - con.vislines);
	const float relativeScroll = con.current - con.totallines + MIN(con.vislines, con.totallines);
	const float handlePosition = lengthPerLine * (con.display - relativeScroll);

	SCR_FillRect(x, y, width, length, color);

	color[0] = 0.5f;
	color[1] = 0.5f;
	color[2] = 0.5f;
	color[3] = 1.0f;

	// draw the handle
	if (handlePosition >= 0 && handleLength > 0)
	{
		SCR_FillRect(x, y + handlePosition, width, handleLength, color);
	}
	// this happens when line appending gets us over the top position in a roll-lock situation (scrolling itself won't do that)
	else if (con.totallines)
	{
		SCR_FillRect(x, y, width, handleLength, color);
	}
}

/**
 * @brief Draws the console with the solid background
 */
void Con_DrawSolidConsole(float frac)
{
	int          i, x, y;
	unsigned int *text;
	byte         *textColor;
	int          row;
	int          currentColor;
	vec4_t       color;

	if (frac <= 0)
	{
		return;
	}

	if (frac > 1)
	{
		frac = 1;
	}

	con.scanlines = frac * cls.glconfig.vidHeight;

	// on wide screens, we will center the text
	con.xadjust = 0;
	SCR_AdjustFrom640(&con.xadjust, NULL, NULL, NULL);

	// draw the background
	y = frac * SCREEN_HEIGHT;

	if (y < 1)
	{
		y = 0;
	}
	else
	{
		SCR_DrawPic(0, 0, SCREEN_WIDTH, y, cls.consoleShader);

		/*
		// draw the logo
		if (frac >= 0.5f)
		{
		    color[0] = color[1] = color[2] = frac * 2.0f;
		    color[3] = 1.0f;
		    re.SetColor(color);

		    SCR_DrawPic(192, 70, 256, 128, cls.consoleShader2);
		    re.SetColor(NULL);
		}
		*/
	}

	// matching light text
	color[0] = 0.75f;
	color[1] = 0.75f;
	color[2] = 0.75f;
	color[3] = 1.0f;

	if (frac < 1)
	{
		SCR_FillRect(0, y, SCREEN_WIDTH, 1.25f, color);
	}

	re.SetColor(g_color_table[ColorIndex(CONSOLE_COLOR)]);

	// draw the input prompt, user text, and cursor
	Con_DrawInput();
	// draw scrollbar
	Con_DrawScrollbar(y - 1.5f * SMALLCHAR_HEIGHT, SCREEN_WIDTH - 5, 3);
	// draw the version number
	//Con_DrawVersion();
	Con_DrawDebug();

	// draw text
	con.vislines = (con.scanlines - SMALLCHAR_HEIGHT) / SMALLCHAR_HEIGHT - 1;  // rows of text to draw

	y = con.scanlines - 3 * SMALLCHAR_HEIGHT;

	// draw from the bottom up
	if (con.scrollline < con.current - 1)
	{
		// draw arrows to show the buffer is backscrolled
		re.SetColor(g_color_table[ColorIndex(COLOR_WHITE)]);

		for (x = 0; x < con.linewidth; x += 4)
		{
			SCR_DrawSmallChar(con.xadjust + (x + 1) * SMALLCHAR_WIDTH, y + 0.75f * SMALLCHAR_HEIGHT, '^');
		}
	}

	row = con.display;

	if (con.x == 0)
	{
		row--;
	}

	currentColor = 7;
	re.SetColor(g_color_table[currentColor]);

	for (i = 0 ; i < con.vislines ; i++, y -= SMALLCHAR_HEIGHT, row--)
	{
		if (row < 0)
		{
			break;
		}

		if (con.current - row >= con.maxtotallines)
		{
			// past scrollback wrap point
			continue;
		}

		text      = con.text + (row % con.maxtotallines) * con.linewidth;
		textColor = con.textColor + (row % con.maxtotallines) * con.linewidth;

		for (x = 0 ; x < con.linewidth ; x++)
		{
			if (text[x] == ' ')
			{
				continue;
			}

			if (textColor[x] != currentColor)
			{
				currentColor = textColor[x];
				re.SetColor(g_color_table[currentColor]);
			}

			SCR_DrawSmallChar(con.xadjust + (x + 1) * SMALLCHAR_WIDTH, y, text[x]);
		}
	}

	re.SetColor(NULL);
}

/**
 * @brief Draw console
 */
void Con_DrawConsole(void)
{
	// render console only if opened but also if disconnected
	if (!con.displayFrac && !(cls.state == CA_DISCONNECTED &&
	                          !(cls.keyCatchers & (KEYCATCH_UI | KEYCATCH_CGAME))))
	{
		return;
	}

	// check for console width changes from a vid mode change
	Con_CheckResize();

	Con_DrawSolidConsole(con.displayFrac);
}

/**
 * @brief Scroll console up or down
 */
void Con_RunConsole(void)
{
	// decide on the destination height of the console
	// short console support via shift+~
	if (cls.keyCatchers & KEYCATCH_CONSOLE)
	{
		con.finalFrac = con.desiredFrac;
	}
	else
	{
		con.finalFrac = 0.0f;  // none visible
	}

	// scroll towards the destination height
	if (con.finalFrac < con.displayFrac)
	{
		con.displayFrac -= con_conspeed->value * cls.realFrametime * 0.001;

		if (con.finalFrac > con.displayFrac)
		{
			con.displayFrac = con.finalFrac;
		}
	}
	else if (con.finalFrac > con.displayFrac)
	{
		con.displayFrac += con_conspeed->value * cls.realFrametime * 0.001;

		if (con.finalFrac < con.displayFrac)
		{
			con.displayFrac = con.finalFrac;
		}
	}

	// animated scroll
	if (con.displayFrac > 0)
	{
		const int scrolldiff = abs(con.scrollline - con.display);

		if (con.display < con.scrollline)
		{
			con.display += con_conspeed->value * cls.realFrametime * 0.005 * scrolldiff;

			if (con.display > con.scrollline)
			{
				con.display = con.scrollline;
			}
		}
		else if (con.display > con.scrollline)
		{
			con.display -= con_conspeed->value * cls.realFrametime * 0.005 * scrolldiff;

			if (con.display < con.scrollline)
			{
				con.display = con.scrollline;
			}
		}
	}
	else
	{
		//con.display = con.scrollline;
	}
}


/**
 * @brief Page up
 */
void Con_PageUp(void)
{
	con.scrollline -= con.vislines / 2;

	// if (con.scrollline < con.current - con.totallines + con.vislines)
	// {
	// 	con.scrollline = con.current - con.totallines + con.vislines;
	// }
}

/**
 * @brief Page down
 */
void Con_PageDown(void)
{
	con.scrollline += con.vislines / 2;

	// if (con.scrollline > con.current)
	// {
	// 	con.scrollline = con.current;
	// }
}

/**
 * @brief Scroll to top
 */
void Con_Top(void)
{
	con.scrollline = con.current - con.totallines + con.vislines;

	// if (con.scrollline < con.current - con.totallines + con.vislines)
	// {
	// 	con.scrollline = con.current - con.totallines + con.vislines;
	// }
}

/**
 * @brief Scroll to bottom
 */
void Con_Bottom(void)
{
	con.scrollline = con.current;
}

/**
 * @brief Close console
 */
void Con_Close(void)
{
	if (!com_cl_running->integer)
	{
		return;
	}

	if (com_developer->integer > 1)
	{
		return;
	}

	Field_Clear(&g_consoleField);
	cls.keyCatchers &= ~KEYCATCH_CONSOLE;
	con.finalFrac    = 0;
	con.displayFrac  = 0;
}
