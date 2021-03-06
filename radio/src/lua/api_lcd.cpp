/*
 * Authors (alphabetical order)
 * - Andre Bernet <bernet.andre@gmail.com>
 * - Andreas Weitl
 * - Bertrand Songis <bsongis@gmail.com>
 * - Bryan J. Rentoul (Gruvin) <gruvin@gmail.com>
 * - Cameron Weeks <th9xer@gmail.com>
 * - Erez Raviv
 * - Gabriel Birkus
 * - Jean-Pierre Parisy
 * - Karl Szmutny
 * - Michael Blandford
 * - Michal Hlavinka
 * - Pat Mackenzie
 * - Philip Moss
 * - Rob Thomson
 * - Romolo Manfredini <romolo.manfredini@gmail.com>
 * - Thomas Husterer
 *
 * opentx is based on code named
 * gruvin9x by Bryan J. Rentoul: http://code.google.com/p/gruvin9x/,
 * er9x by Erez Raviv: http://code.google.com/p/er9x/,
 * and the original (and ongoing) project by
 * Thomas Husterer, th9x: http://code.google.com/p/th9x/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <ctype.h>
#include <stdio.h>
#include "opentx.h"
#include "lua/lua_api.h"

/*luadoc
@function lcd.lock()

@notice This function has no effect in OpenTX 2.1
*/
static int luaLcdLock(lua_State *L)
{
  // disabled in opentx 2.1
  // TODO: remove this function completely in opentx 2.2
  return 0;
}

/*luadoc
@function lcd.clear()

Erases all contents of LCD.

@notice This function only works in stand-alone and telemetry scripts.
*/
static int luaLcdClear(lua_State *L)
{
  if (luaLcdAllowed) lcd_clear();
  return 0;
}

/*luadoc
@function lcd.drawPoint(x, y)

Draws a single pixel on LCD

@param x (positive number) x position, starts from 0 in top left corner. 

@param y (positive number) y position, starts from 0 in top left corner and goes down.

@notice Drawing on an existing black pixel produces white pixel (TODO check this!)
*/
static int luaLcdDrawPoint(lua_State *L)
{
  if (!luaLcdAllowed) return 0;
  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  lcd_plot(x, y);
  return 0;
}

/*luadoc
@function lcd.drawLine(x1, y1, x2, y2, pattern, flags)

Draws a straight line on LCD

@param x1,y1 (positive numbers) starting coordinate

@param x2,y2 (positive numbers) end coordinate

@param pattern TODO

@param flags TODO

@notice If the start or the end of the line is outside the LCD dimensions, then the
whole line will not be drawn (starting from OpenTX 2.1.5)
*/
static int luaLcdDrawLine(lua_State *L)
{
  if (!luaLcdAllowed) return 0;
  int x1 = luaL_checkinteger(L, 1);
  int y1 = luaL_checkinteger(L, 2);
  int x2 = luaL_checkinteger(L, 3);
  int y2 = luaL_checkinteger(L, 4);
  int pat = luaL_checkinteger(L, 5);
  int flags = luaL_checkinteger(L, 6);
  lcd_line(x1, y1, x2, y2, pat, flags);
  return 0;
}

static int luaLcdGetLastPos(lua_State *L)
{
  lua_pushinteger(L, lcdLastPos);
  return 1;
}

static int luaLcdDrawText(lua_State *L)
{
  if (!luaLcdAllowed) return 0;
  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  const char * s = luaL_checkstring(L, 3);
  unsigned int att = luaL_optunsigned(L, 4, 0);
  lcd_putsAtt(x, y, s, att);
  return 0;
}

static int luaLcdDrawTimer(lua_State *L)
{
  if (!luaLcdAllowed) return 0;
  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  int seconds = luaL_checkinteger(L, 3);
  unsigned int att = luaL_optunsigned(L, 4, 0);
  putsTimer(x, y, seconds, att|LEFT, att);
  return 0;
}

static int luaLcdDrawNumber(lua_State *L)
{
  if (!luaLcdAllowed) return 0;
  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  float val = luaL_checknumber(L, 3);
  unsigned int att = luaL_optunsigned(L, 4, 0);
  int n;
  if ((att & PREC2) == PREC2)
    n = val * 100;
  else if ((att & PREC1) == PREC1)
    n = val * 10;
  else
    n = val;
  lcd_outdezAtt(x, y, n, att);
  return 0;
}

static int luaLcdDrawChannel(lua_State *L)
{
  if (!luaLcdAllowed) return 0;
  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  int channel = -1;
  if (lua_isnumber(L, 3)) {
    channel = luaL_checkinteger(L, 3);
  }
  else {
    const char * what = luaL_checkstring(L, 3);
    LuaField field;
    bool found = luaFindFieldByName(what, field);
    if (found) {
      channel = field.id;
    }
  }
  unsigned int att = luaL_optunsigned(L, 4, 0);
  getvalue_t value = getValue(channel);
  putsTelemetryChannelValue(x, y, (channel-MIXSRC_FIRST_TELEM)/3, value, att);
  return 0;
}

static int luaLcdDrawSwitch(lua_State *L)
{
  if (!luaLcdAllowed) return 0;
  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  int s = luaL_checkinteger(L, 3);
  unsigned int att = luaL_optunsigned(L, 4, 0);
  putsSwitches(x, y, s, att);
  return 0;
}

static int luaLcdDrawSource(lua_State *L)
{
  if (!luaLcdAllowed) return 0;
  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  int s = luaL_checkinteger(L, 3);
  unsigned int att = luaL_optunsigned(L, 4, 0);
  putsMixerSource(x, y, s, att);
  return 0;
}

static int luaLcdDrawPixmap(lua_State *L)
{
  if (!luaLcdAllowed) return 0;
  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  const char * filename = luaL_checkstring(L, 3);
  uint8_t bitmap[BITMAP_BUFFER_SIZE(LCD_W/2, LCD_H)]; // width max is LCD_W/2 pixels for saving stack and avoid a malloc here
  const pm_char * error = bmpLoad(bitmap, filename, LCD_W/2, LCD_H);
  if (!error) {
    lcd_bmp(x, y, bitmap);
  }
  return 0;
}

static int luaLcdDrawRectangle(lua_State *L)
{
  if (!luaLcdAllowed) return 0;
  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  int w = luaL_checkinteger(L, 3);
  int h = luaL_checkinteger(L, 4);
  unsigned int flags = luaL_optunsigned(L, 5, 0);
  lcd_rect(x, y, w, h, 0xff, flags);
  return 0;
}

static int luaLcdDrawFilledRectangle(lua_State *L)
{
  if (!luaLcdAllowed) return 0;
  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  int w = luaL_checkinteger(L, 3);
  int h = luaL_checkinteger(L, 4);
  unsigned int flags = luaL_optunsigned(L, 5, 0);
  drawFilledRect(x, y, w, h, SOLID, flags);
  return 0;
}

static int luaLcdDrawGauge(lua_State *L)
{
  if (!luaLcdAllowed) return 0;
  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  int w = luaL_checkinteger(L, 3);
  int h = luaL_checkinteger(L, 4);
  int num = luaL_checkinteger(L, 5);
  int den = luaL_checkinteger(L, 6);
  // int flags = luaL_checkinteger(L, 7);
  lcd_rect(x, y, w, h);
  uint8_t len = limit((uint8_t)1, uint8_t(w*num/den), uint8_t(w));
  for (int i=1; i<h-1; i++) {
    lcd_hline(x+1, y+i, len);
  }
  return 0;
}

static int luaLcdDrawScreenTitle(lua_State *L)
{
  if (!luaLcdAllowed) return 0;
  const char * str = luaL_checkstring(L, 1);
  int idx = luaL_checkinteger(L, 2);
  int cnt = luaL_checkinteger(L, 3);

  if (cnt) displayScreenIndex(idx-1, cnt, 0);
  drawFilledRect(0, 0, LCD_W, FH, SOLID, FILL_WHITE|GREY_DEFAULT);
  title(str);

  return 0;
}

static int luaLcdDrawCombobox(lua_State *L)
{
  if (!luaLcdAllowed) return 0;
  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  int w = luaL_checkinteger(L, 3);
  luaL_checktype(L, 4, LUA_TTABLE);
  int count = luaL_len(L, 4);  /* get size of table */
  int idx = luaL_checkinteger(L, 5);
  unsigned int flags = luaL_optunsigned(L, 6, 0);
  if (idx >= count) {
    // TODO error
  }
  if (flags & BLINK) {
    drawFilledRect(x, y, w-9, count*9+2, SOLID, ERASE);
    lcd_rect(x, y, w-9, count*9+2);
    for (int i=0; i<count; i++) {
      lua_rawgeti(L, 4, i+1);
      const char * item = luaL_checkstring(L, -1);
      lcd_putsAtt(x+2, y+2+9*i, item, 0);
    }
    drawFilledRect(x+1, y+1+9*idx, w-11, 9);
    drawFilledRect(x+w-10, y, 10, 11, SOLID, ERASE);
    lcd_rect(x+w-10, y, 10, 11);
  }
  else if (flags & INVERS) {
    drawFilledRect(x, y, w, 11);
    drawFilledRect(x+w-9, y+1, 8, 9, SOLID, ERASE);
    lua_rawgeti(L, 4, idx+1);
    const char * item = luaL_checkstring(L, -1);
    lcd_putsAtt(x+2, y+2, item, INVERS);
  }
  else {
    drawFilledRect(x, y, w, 11, SOLID, ERASE);
    lcd_rect(x, y, w, 11);
    drawFilledRect(x+w-10, y+1, 9, 9, SOLID);
    lua_rawgeti(L, 4, idx+1);
    const char * item = luaL_checkstring(L, -1);
    lcd_putsAtt(x+2, y+2, item, 0);
  }

  lcd_hline(x+w-8, y+3, 6);
  lcd_hline(x+w-8, y+5, 6);
  lcd_hline(x+w-8, y+7, 6);

  return 0;
}

const luaL_Reg lcdLib[] = {
  { "lock", luaLcdLock },
  { "clear", luaLcdClear },
  { "getLastPos", luaLcdGetLastPos },
  { "drawPoint", luaLcdDrawPoint },
  { "drawLine", luaLcdDrawLine },
  { "drawRectangle", luaLcdDrawRectangle },
  { "drawFilledRectangle", luaLcdDrawFilledRectangle },
  { "drawGauge", luaLcdDrawGauge },
  { "drawText", luaLcdDrawText },
  { "drawTimer", luaLcdDrawTimer },
  { "drawNumber", luaLcdDrawNumber },
  { "drawChannel", luaLcdDrawChannel },
  { "drawSwitch", luaLcdDrawSwitch },
  { "drawSource", luaLcdDrawSource },
  { "drawPixmap", luaLcdDrawPixmap },
  { "drawScreenTitle", luaLcdDrawScreenTitle },
  { "drawCombobox", luaLcdDrawCombobox },
  { NULL, NULL }  /* sentinel */
};
