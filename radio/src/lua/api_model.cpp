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
#include "timers.h"

static int luaModelGetInfo(lua_State *L)
{
  lua_newtable(L);
  lua_pushtablezstring(L, "name", g_model.header.name);
  lua_pushtablenzstring(L, "bitmap", g_model.header.bitmap);
  return 1;
}

static int luaModelSetInfo(lua_State *L)
{
  luaL_checktype(L, -1, LUA_TTABLE);
  for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
    luaL_checktype(L, -2, LUA_TSTRING); // key is string
    const char * key = luaL_checkstring(L, -2);
    if (!strcmp(key, "name")) {
      const char * name = luaL_checkstring(L, -1);
      str2zchar(g_model.header.name, name, sizeof(g_model.header.name));
      memcpy(modelHeaders[g_eeGeneral.currModel].name, g_model.header.name, sizeof(g_model.header.name));
    }
    else if (!strcmp(key, "bitmap")) {
      const char * name = luaL_checkstring(L, -1);
      strncpy(g_model.header.bitmap, name, sizeof(g_model.header.bitmap));
    }
  }
  eeDirty(EE_MODEL);
  return 0;
}

static int luaModelGetModule(lua_State *L)
{
  unsigned int idx = luaL_checkunsigned(L, 1);
  if (idx < NUM_MODULES) {
    ModuleData & module = g_model.moduleData[idx];
    lua_newtable(L);
    lua_pushtableinteger(L, "rfProtocol", module.rfProtocol);
    lua_pushtableinteger(L, "modelId", g_model.header.modelId[idx]);
    lua_pushtableinteger(L, "firstChannel", module.channelsStart);
    lua_pushtableinteger(L, "channelsCount", module.channelsCount + 8);
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}

static int luaModelSetModule(lua_State *L)
{
  unsigned int idx = luaL_checkunsigned(L, 1);

  if (idx < NUM_MODULES) {
    ModuleData & module = g_model.moduleData[idx];
    luaL_checktype(L, -1, LUA_TTABLE);
    for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
      luaL_checktype(L, -2, LUA_TSTRING); // key is string
      const char * key = luaL_checkstring(L, -2);
      if (!strcmp(key, "rfProtocol")) {
        module.rfProtocol = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "modelId")) {
        g_model.header.modelId[idx] = modelHeaders[g_eeGeneral.currModel].modelId[idx] = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "firstChannel")) {
        module.channelsStart = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "channelsCount")) {
        module.channelsCount = luaL_checkinteger(L, -1) - 8;
      }
    }
    eeDirty(EE_MODEL);
  }
  return 0;
}

static int luaModelGetTimer(lua_State *L)
{
  unsigned int idx = luaL_checkunsigned(L, 1);
  if (idx < MAX_TIMERS) {
    TimerData & timer = g_model.timers[idx];
    lua_newtable(L);
    lua_pushtableinteger(L, "mode", timer.mode);
    lua_pushtableinteger(L, "start", timer.start);
    lua_pushtableinteger(L, "value", timersStates[idx].val);
    lua_pushtableinteger(L, "countdownBeep", timer.countdownBeep);
    lua_pushtableboolean(L, "minuteBeep", timer.minuteBeep);
    lua_pushtableinteger(L, "persistent", timer.persistent);
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}

static int luaModelSetTimer(lua_State *L)
{
  unsigned int idx = luaL_checkunsigned(L, 1);

  if (idx < MAX_TIMERS) {
    TimerData & timer = g_model.timers[idx];
    luaL_checktype(L, -1, LUA_TTABLE);
    for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
      luaL_checktype(L, -2, LUA_TSTRING); // key is string
      const char * key = luaL_checkstring(L, -2);
      if (!strcmp(key, "mode")) {
        timer.mode = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "start")) {
        timer.start = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "value")) {
        timersStates[idx].val = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "countdownBeep")) {
        timer.countdownBeep = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "minuteBeep")) {
        timer.minuteBeep = lua_toboolean(L, -1);
      }
      else if (!strcmp(key, "persistent")) {
        timer.persistent = luaL_checkinteger(L, -1);
      }
    }
    eeDirty(EE_MODEL);
  }
  return 0;
}

static int luaModelResetTimer(lua_State *L)
{
  unsigned int idx = luaL_checkunsigned(L, 1);
  if (idx < MAX_TIMERS) {
    timerReset(idx);
  }
  return 0;
}

static unsigned int getFirstInput(unsigned int chn)
{
  for (unsigned int i=0; i<MAX_INPUTS; i++) {
    ExpoData * expo = expoAddress(i);
    if (!expo->srcRaw || expo->chn >= chn) {
      return i;
    }
  }
  return 0;
}

static unsigned int getInputsCountFromFirst(unsigned int chn, unsigned int first)
{
  unsigned int count = 0;
  for (unsigned int i=first; i<MAX_INPUTS; i++) {
    ExpoData * expo = expoAddress(i);
    if (!expo->srcRaw || expo->chn!=chn) break;
    count++;
  }
  return count;
}

static unsigned int getInputsCount(unsigned int chn)
{
  return getInputsCountFromFirst(chn, getFirstInput(chn));
}

/*luadoc
@function model.getInputsCount(input)

Returns number of lines for given input 

@param input (unsigned number) input number (use 0 for Input1)

@retval number number of configured lines for given input

@status current Introduced in 2.0.0
*/
static int luaModelGetInputsCount(lua_State *L)
{
  unsigned int chn = luaL_checkunsigned(L, 1);
  int count = getInputsCount(chn);
  lua_pushinteger(L, count);
  return 1;
}

/*luadoc
@function model.getInput(input, line)

Returns input data for given input and line number 

@param input (unsigned number) input number (use 0 for Input1)

@param line  (unsigned number) input line (use 0 for first line)

@retval nil requested input or line does not exist

@retval table input data:
 * `name` (string) input line name
 * `source` (number) input source index
 * `weight` (number) input weight 
 * `offset` (number) input offset 
 * `switch` (number) input switch index

@status current Introduced in 2.0.0, `switch` added in TODO
*/
static int luaModelGetInput(lua_State *L)
{
  unsigned int chn = luaL_checkunsigned(L, 1);
  unsigned int idx = luaL_checkunsigned(L, 2);
  unsigned int first = getFirstInput(chn);
  unsigned int count = getInputsCountFromFirst(chn, first);
  if (idx < count) {
    ExpoData * expo = expoAddress(first+idx);
    lua_newtable(L);
    lua_pushtablezstring(L, "name", expo->name);
    lua_pushtableinteger(L, "source", expo->srcRaw);
    lua_pushtableinteger(L, "weight", expo->weight);
    lua_pushtableinteger(L, "offset", expo->offset);
    lua_pushtableinteger(L, "switch", expo->swtch);
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}

/*luadoc
@function model.insertInput(input, line, value)

Inserts an Input at specified line

@param input (unsigned number) input number (use 0 for Input1)

@param line  (unsigned number) input line (use 0 for first line)

@param value (table) input data, see model.getInput()

@status current Introduced in 2.0.0, `switch` added in TODO
*/
static int luaModelInsertInput(lua_State *L)
{
  unsigned int chn = luaL_checkunsigned(L, 1);
  unsigned int idx = luaL_checkunsigned(L, 2);

  unsigned int first = getFirstInput(chn);
  unsigned int count = getInputsCountFromFirst(chn, first);

  if (chn<MAX_INPUTS && getExpoMixCount(1)<MAX_EXPOS && idx<=count) {
    idx = first + idx;
    s_currCh = chn + 1;
    insertExpoMix(1, idx);
    ExpoData * expo = expoAddress(idx);
    luaL_checktype(L, -1, LUA_TTABLE);
    for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
      luaL_checktype(L, -2, LUA_TSTRING); // key is string
      const char * key = luaL_checkstring(L, -2);
      if (!strcmp(key, "name")) {
        const char * name = luaL_checkstring(L, -1);
        str2zchar(expo->name, name, sizeof(expo->name));
      }
      else if (!strcmp(key, "source")) {
        expo->srcRaw = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "weight")) {
        expo->weight = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "offset")) {
        expo->offset = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "switch")) {
        expo->swtch = luaL_checkinteger(L, -1);
      }
    }
  }

  return 0;
}

/*luadoc
@function model.deleteInput(input, line)

Delete line from specified input

@param input (unsigned number) input number (use 0 for Input1)

@param line  (unsigned number) input line (use 0 for first line)

@status current Introduced in 2.0.0
*/
static int luaModelDeleteInput(lua_State *L)
{
  unsigned int chn = luaL_checkunsigned(L, 1);
  unsigned int idx = luaL_checkunsigned(L, 2);

  int first = getFirstInput(chn);
  unsigned int count = getInputsCountFromFirst(chn, first);

  if (idx < count) {
    deleteExpoMix(1, first+idx);
  }

  return 0;
}

/*luadoc
@function model.deleteInputs()

Delete all Inputs 

@status current Introduced in 2.0.0
*/
static int luaModelDeleteInputs(lua_State *L)
{
  clearInputs();
  return 0;
}

/*luadoc
@function model.defaultInputs()

Set all inputs to defaults

@status current Introduced in 2.0.0
*/
static int luaModelDefaultInputs(lua_State *L)
{
  defaultInputs();
  return 0;
}

static unsigned int getFirstMix(unsigned int chn)
{
  for (unsigned int i=0; i<MAX_MIXERS; i++) {
    MixData * mix = mixAddress(i);
    if (!mix->srcRaw || mix->destCh>=chn) {
      return i;
    }
  }
  return 0;
}

static unsigned int getMixesCountFromFirst(unsigned int chn, unsigned int first)
{
  unsigned int count = 0;
  for (unsigned int i=first; i<MAX_MIXERS; i++) {
    MixData * mix = mixAddress(i);
    if (!mix->srcRaw || mix->destCh!=chn) break;
    count++;
  }
  return count;
}

static unsigned int getMixesCount(unsigned int chn)
{
  return getMixesCountFromFirst(chn, getFirstMix(chn));
}

static int luaModelGetMixesCount(lua_State *L)
{
  unsigned int chn = luaL_checkunsigned(L, 1);
  unsigned int count = getMixesCount(chn);
  lua_pushinteger(L, count);
  return 1;
}

static int luaModelGetMix(lua_State *L)
{
  unsigned int chn = luaL_checkunsigned(L, 1);
  unsigned int idx = luaL_checkunsigned(L, 2);
  unsigned int first = getFirstMix(chn);
  unsigned int count = getMixesCountFromFirst(chn, first);
  if (idx < count) {
    MixData * mix = mixAddress(first+idx);
    lua_newtable(L);
    lua_pushtablezstring(L, "name", mix->name);
    lua_pushtableinteger(L, "source", mix->srcRaw);
    lua_pushtableinteger(L, "weight", mix->weight);
    lua_pushtableinteger(L, "offset", mix->offset);
    lua_pushtableinteger(L, "switch", mix->swtch);
    lua_pushtableinteger(L, "curveType", mix->curve.type);
    lua_pushtableinteger(L, "curveValue", mix->curve.value);
    lua_pushtableinteger(L, "multiplex", mix->mltpx);
    lua_pushtableinteger(L, "flightModes", mix->flightModes);
    lua_pushtableboolean(L, "carryTrim", mix->carryTrim);
    lua_pushtableinteger(L, "mixWarn", mix->mixWarn);
    lua_pushtableinteger(L, "delayUp", mix->delayUp);
    lua_pushtableinteger(L, "delayDown", mix->delayDown);
    lua_pushtableinteger(L, "speedUp", mix->speedUp);
    lua_pushtableinteger(L, "speedDown", mix->speedDown);
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}

static int luaModelInsertMix(lua_State *L)
{
  unsigned int chn = luaL_checkunsigned(L, 1);
  unsigned int idx = luaL_checkunsigned(L, 2);

  unsigned int first = getFirstMix(chn);
  unsigned int count = getMixesCountFromFirst(chn, first);

  if (chn<NUM_CHNOUT && getExpoMixCount(0)<MAX_MIXERS && idx<=count) {
    idx += first;
    s_currCh = chn+1;
    insertExpoMix(0, idx);
    MixData *mix = mixAddress(idx);
    luaL_checktype(L, -1, LUA_TTABLE);
    for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
      luaL_checktype(L, -2, LUA_TSTRING); // key is string
      const char * key = luaL_checkstring(L, -2);
      if (!strcmp(key, "name")) {
        const char * name = luaL_checkstring(L, -1);
        str2zchar(mix->name, name, sizeof(mix->name));
      }
      else if (!strcmp(key, "source")) {
        mix->srcRaw = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "weight")) {
        mix->weight = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "offset")) {
        mix->offset = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "switch")) {
        mix->swtch = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "curveType")) {
        mix->curve.type = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "curveValue")) {
        mix->curve.value = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "multiplex")) {
        mix->mltpx = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "flightModes")) {
        mix->flightModes = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "carryTrim")) {
        mix->carryTrim = lua_toboolean(L, -1);
      }
      else if (!strcmp(key, "mixWarn")) {
        mix->mixWarn = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "delayUp")) {
        mix->delayUp = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "delayDown")) {
        mix->delayDown = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "speedUp")) {
        mix->speedUp = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "speedDown")) {
        mix->speedDown = luaL_checkinteger(L, -1);
      }
    }
  }

  return 0;
}

static int luaModelDeleteMix(lua_State *L)
{
  unsigned int chn = luaL_checkunsigned(L, 1);
  unsigned int idx = luaL_checkunsigned(L, 2);

  unsigned int first = getFirstMix(chn);
  unsigned int count = getMixesCountFromFirst(chn, first);

  if (idx < count) {
    deleteExpoMix(0, first+idx);
  }

  return 0;
}

static int luaModelDeleteMixes(lua_State *L)
{
  memset(g_model.mixData, 0, sizeof(g_model.mixData));
  return 0;
}

static int luaModelGetLogicalSwitch(lua_State *L)
{
  unsigned int idx = luaL_checkunsigned(L, 1);
  if (idx < NUM_LOGICAL_SWITCH) {
    LogicalSwitchData * sw = lswAddress(idx);
    lua_newtable(L);
    lua_pushtableinteger(L, "func", sw->func);
    lua_pushtableinteger(L, "v1", sw->v1);
    lua_pushtableinteger(L, "v2", sw->v2);
    lua_pushtableinteger(L, "v3", sw->v3);
    lua_pushtableinteger(L, "and", sw->andsw);
    lua_pushtableinteger(L, "delay", sw->delay);
    lua_pushtableinteger(L, "duration", sw->duration);
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}

static int luaModelSetLogicalSwitch(lua_State *L)
{
  unsigned int idx = luaL_checkunsigned(L, 1);
  if (idx < NUM_LOGICAL_SWITCH) {
    LogicalSwitchData * sw = lswAddress(idx);
    memclear(sw, sizeof(LogicalSwitchData));
    luaL_checktype(L, -1, LUA_TTABLE);
    for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
      luaL_checktype(L, -2, LUA_TSTRING); // key is string
      const char * key = luaL_checkstring(L, -2);
      if (!strcmp(key, "func")) {
        sw->func = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "v1")) {
        sw->v1 = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "v2")) {
        sw->v2 = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "v3")) {
        sw->v3 = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "and")) {
        sw->andsw = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "delay")) {
        sw->delay = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "duration")) {
        sw->duration = luaL_checkinteger(L, -1);
      }
    }
    eeDirty(EE_MODEL);
  }

  return 0;
}

static int luaModelGetCurve(lua_State *L)
{
  unsigned int idx = luaL_checkunsigned(L, 1);
  if (idx < MAX_CURVES) {
    CurveInfo & curveInfo = g_model.curves[idx];
    lua_newtable(L);
    lua_pushtablezstring(L, "name", g_model.curveNames[idx]);
    lua_pushtableinteger(L, "type", curveInfo.type);
    lua_pushtableboolean(L, "smooth", curveInfo.smooth);
    lua_pushtableinteger(L, "points", curveInfo.points+5);
    lua_pushstring(L, "y");
    lua_newtable(L);
    int8_t * point = curveAddress(idx);
    for (int i=0; i<curveInfo.points+5; i++) {
      lua_pushinteger(L, i);
      lua_pushinteger(L, *point++);
      lua_settable(L, -3);
    }
    lua_settable(L, -3);
    if (curveInfo.type == CURVE_TYPE_CUSTOM) {
      lua_pushstring(L, "x");
      lua_newtable(L);
      lua_pushinteger(L, 0);
      lua_pushinteger(L, 0);
      lua_settable(L, -3);
      for (int i=0; i<curveInfo.points+3; i++) {
        lua_pushinteger(L, i+1);
        lua_pushinteger(L, *point++);
        lua_settable(L, -3);
      }
      lua_pushinteger(L, curveInfo.points+4);
      lua_pushinteger(L, 100);
      lua_settable(L, -3);
      lua_settable(L, -3);
    }
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}

static int luaModelGetCustomFunction(lua_State *L)
{
  unsigned int idx = luaL_checkunsigned(L, 1);
  if (idx < NUM_CFN) {
    CustomFunctionData * cfn = &g_model.customFn[idx];
    lua_newtable(L);
    lua_pushtableinteger(L, "switch", CFN_SWITCH(cfn));
    lua_pushtableinteger(L, "func", CFN_FUNC(cfn));
    if (CFN_FUNC(cfn) == FUNC_PLAY_TRACK || CFN_FUNC(cfn) == FUNC_BACKGND_MUSIC || CFN_FUNC(cfn) == FUNC_PLAY_SCRIPT) {
      lua_pushtablenzstring(L, "name", cfn->play.name);
    }
    else {
      lua_pushtableinteger(L, "value", cfn->all.val);
      lua_pushtableinteger(L, "mode", cfn->all.mode);
      lua_pushtableinteger(L, "param", cfn->all.param);
    }
    lua_pushtableinteger(L, "active", CFN_ACTIVE(cfn));
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}

static int luaModelSetCustomFunction(lua_State *L)
{
  unsigned int idx = luaL_checkunsigned(L, 1);
  if (idx < NUM_CFN) {
    CustomFunctionData * cfn = &g_model.customFn[idx];
    memclear(cfn, sizeof(CustomFunctionData));
    luaL_checktype(L, -1, LUA_TTABLE);
    for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
      luaL_checktype(L, -2, LUA_TSTRING); // key is string
      const char * key = luaL_checkstring(L, -2);
      if (!strcmp(key, "switch")) {
        CFN_SWITCH(cfn) = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "func")) {
        CFN_FUNC(cfn) = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "name")) {
        const char * name = luaL_checkstring(L, -1);
        strncpy(cfn->play.name, name, sizeof(cfn->play.name));
      }
      else if (!strcmp(key, "value")) {
        cfn->all.val = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "mode")) {
        cfn->all.mode = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "param")) {
        cfn->all.param = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "active")) {
        CFN_ACTIVE(cfn) = luaL_checkinteger(L, -1);
      }
    }
    eeDirty(EE_MODEL);
  }

  return 0;
}

static int luaModelGetOutput(lua_State *L)
{
  unsigned int idx = luaL_checkunsigned(L, 1);
  if (idx < NUM_CHNOUT) {
    LimitData * limit = limitAddress(idx);
    lua_newtable(L);
    lua_pushtablezstring(L, "name", limit->name);
    lua_pushtableinteger(L, "min", limit->min-1000);
    lua_pushtableinteger(L, "max", limit->max+1000);
    lua_pushtableinteger(L, "offset", limit->offset);
    lua_pushtableinteger(L, "ppmCenter", limit->ppmCenter);
    lua_pushtableinteger(L, "symetrical", limit->symetrical);
    lua_pushtableinteger(L, "revert", limit->revert);
    if (limit->curve)
      lua_pushtableinteger(L, "curve", limit->curve-1);
    else
      lua_pushtablenil(L, "curve");
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}

/*luadoc
@function model.setOutput(index, value)

Sets current global variable value. See also model.getGlobalVariable()

@param index  zero based output index, use 0 for CH1, 31 for CH32

@param value  new value for output. The `value` is a table with following items:
  * `name` name of the output (channel)
  * `min` negative limit 
  * `max` positive limit
  * `offset` subtrim value
  * `ppmCenter` ppm center value
  * `symetrical` 1 for symmetric limits, 0 for normal
  * `revert` 1 for inverted output, 0 for normal
  * `curve` curve reference (zero based index, 0 means Curve 1)
*/
static int luaModelSetOutput(lua_State *L)
{
  unsigned int idx = luaL_checkunsigned(L, 1);
  if (idx < NUM_CHNOUT) {
    LimitData * limit = limitAddress(idx);
    luaL_checktype(L, -1, LUA_TTABLE);
    for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
      luaL_checktype(L, -2, LUA_TSTRING); // key is string
      const char * key = luaL_checkstring(L, -2);
      if (!strcmp(key, "name")) {
        const char * name = luaL_checkstring(L, -1);
        str2zchar(limit->name, name, sizeof(limit->name));
      }
      else if (!strcmp(key, "min")) {
        limit->min = luaL_checkinteger(L, -1)+1000;
      }
      else if (!strcmp(key, "max")) {
        limit->max = luaL_checkinteger(L, -1)-1000;
      }
      else if (!strcmp(key, "offset")) {
        limit->offset = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "ppmCenter")) {
        limit->ppmCenter = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "symetrical")) {
        limit->symetrical = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "revert")) {
        limit->revert = luaL_checkinteger(L, -1);
      }
      else if (!strcmp(key, "curve")) {
        if (lua_isnil(L, -1))
          limit->curve = 0;
        else
          limit->curve = luaL_checkinteger(L, -1) + 1;
      }
    }
    eeDirty(EE_MODEL);
  }

  return 0;
}

/*luadoc
@function model.getGlobalVariable(index [, phase])

Returns current global variable value. 
See also model.setGlobalVariable()

@notice a simple warning or notice

@param index  zero based global variable index, use 0 for GV1, 8 for GV9

@param phase  zero based phase index, use 0 for Phase 1, 5 for Phase 6

@retval nil   requested global variable does not exist

@retval number current value of global variable

Example:

```lua
  -- get GV3 (index = 2) from flight phase 1 (phase = 0)
  val = model.getGlobalVariable(2, 0)
```
*/
static int luaModelGetGlobalVariable(lua_State *L)
{
  unsigned int idx = luaL_checkunsigned(L, 1);
  unsigned int phase = luaL_checkunsigned(L, 2);
  if (phase < MAX_FLIGHT_MODES && idx < MAX_GVARS)
    lua_pushinteger(L, g_model.flightModeData[phase].gvars[idx]);
  else
    lua_pushnil(L);
  return 1;
}

/*luadoc
@function model.setGlobalVariable(index, phase, value)

Sets current global variable value. See also model.getGlobalVariable()

@param index  zero based global variable index, use 0 for GV1, 8 for GV9

@param phase  zero based phase index, use 0 for Phase 1, 5 for Phase 6
   
@param value  new value for global variable. Permitted range is
from -1024 to 1024. 
    

@notice Global variable can only store integer values, 
any floating point value is converted (todo check how) into integer value.
*/
static int luaModelSetGlobalVariable(lua_State *L)
{
  unsigned int idx = luaL_checkunsigned(L, 1);
  unsigned int phase = luaL_checkunsigned(L, 2);
  int value = luaL_checkinteger(L, 3);
  if (phase < MAX_FLIGHT_MODES && idx < MAX_GVARS && value >= -GVAR_MAX && value <= GVAR_MAX) {
    g_model.flightModeData[phase].gvars[idx] = value;
    eeDirty(EE_MODEL);
  }
  return 0;
}

const luaL_Reg modelLib[] = {
  { "getInfo", luaModelGetInfo },
  { "setInfo", luaModelSetInfo },
  { "getModule", luaModelGetModule },
  { "setModule", luaModelSetModule },
  { "getTimer", luaModelGetTimer },
  { "setTimer", luaModelSetTimer },
  { "resetTimer", luaModelResetTimer },
  { "getInputsCount", luaModelGetInputsCount },
  { "getInput", luaModelGetInput },
  { "insertInput", luaModelInsertInput },
  { "deleteInput", luaModelDeleteInput },
  { "deleteInputs", luaModelDeleteInputs },
  { "defaultInputs", luaModelDefaultInputs },
  { "getMixesCount", luaModelGetMixesCount },
  { "getMix", luaModelGetMix },
  { "insertMix", luaModelInsertMix },
  { "deleteMix", luaModelDeleteMix },
  { "deleteMixes", luaModelDeleteMixes },
  { "getLogicalSwitch", luaModelGetLogicalSwitch },
  { "setLogicalSwitch", luaModelSetLogicalSwitch },
  { "getCustomFunction", luaModelGetCustomFunction },
  { "setCustomFunction", luaModelSetCustomFunction },
  { "getCurve", luaModelGetCurve },
  { "getOutput", luaModelGetOutput },
  { "setOutput", luaModelSetOutput },
  { "getGlobalVariable", luaModelGetGlobalVariable },
  { "setGlobalVariable", luaModelSetGlobalVariable },
  { NULL, NULL }  /* sentinel */
};
