#include <lua.h>
#include <lauxlib.h>

#if LUA_VERSION_NUM <= 501
#error Lua <= 5.1 not supported
#endif

static int regkey_upval;  /* registry key for the upvalues table */

static int getupval(lua_State *L) {
  luaL_checkstack(L, 2, NULL); /* we need 2 slots */
  lua_pushvalue(L, -1);
  lua_pushlightuserdata(L, &regkey_upval);  /* get upvalues table */
  lua_rawget(L, LUA_REGISTRYINDEX);
  if (lua_type(L, -1) != LUA_TTABLE) {
    return luaL_error(L, "Couldn't find upvalue table");
  }
  lua_insert(L, -2);
  lua_rawget(L, -2);
  if (lua_type(L, -1) != LUA_TFUNCTION) {  /* get upvalue */
    lua_pop(L, 2);  /* remove object and the upvalues table */
    return 0;  /* no upvalue */
  } else {
    lua_remove(L, -2);  /* remove upvalues table */
    lua_remove(L, -2);  /* remove lightuserdata */
    return 1;  /* got upvalue */
  }
}

static int load_aux (lua_State *L, int status, int envidx, int upvalidx, int upvalcnt) {
  int i;
  if (status == LUA_OK) {
    if (envidx != 0) {  /* 'env' parameter? */
      lua_pushvalue(L, envidx);  /* environment for loaded function */
      if (!getupval(L)) {  /* try to get shared upvalue */
        if (!lua_setupvalue(L, -2, 1))  /* set it as 1st upvalue */
          lua_pop(L, 1);  /* remove 'env' if not used by previous call */
      } else {
        lua_upvaluejoin(L, -2, 1, -1, 1);
        lua_pop(L, 1);
      }
    }
    if (upvalidx != 0) {  /* upvalues? */
      for (i = 0; i < upvalcnt; ++i) {
        lua_pushvalue(L, upvalidx + i);  /* upvalue for function */
        if (!getupval(L)) {  /* try to get shared upvalue */
          if (!lua_setupvalue(L, -2, (2 + i))) {  /* set it */
            lua_pop(L, 1);  /* remove it if not used by previous call */
            break;  /* do not waste CPU time trying to set upvalues */
          }
        } else {
          lua_upvaluejoin(L, -2, (2 + i), -1, 1);
          lua_pop(L, 1);
        }
      }
    }
    return 1;
  }
  else {  /* error (message is on top of the stack) */
    lua_pushnil(L);
    lua_insert(L, -2);  /* put before error message */
    return 2;  /* return nil plus error message */
  }
}


/*
** reserved slot, above all arguments, to hold a copy of the returned
** string to avoid it being collected while parsed. 'load' has four
** optional arguments (chunk, source name, mode, and environment).
*/
#define RESERVEDSLOT	5


/*
** Reader for generic 'load' function: 'lua_load' uses the
** stack for internal stuff, so the reader cannot change the
** stack top. Instead, it keeps its resulting string in a
** reserved slot inside the stack.
*/
static const char *generic_reader (lua_State *L, void *ud, size_t *size) {
  (void)(ud);  /* not used */
  luaL_checkstack(L, 2, "too many nested functions");
  lua_pushvalue(L, 1);  /* get function */
  lua_call(L, 0, 1);  /* call it */
  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);  /* pop result */
    *size = 0;
    return NULL;
  }
  else if (!lua_isstring(L, -1))
    luaL_error(L, "reader function must return a string");
  lua_replace(L, RESERVEDSLOT);  /* save string in reserved slot */
  return lua_tolstring(L, RESERVEDSLOT, size);
}

static int loadx (lua_State *L) {
  int status;
  size_t l;
  const char *s = lua_tolstring(L, 1, &l);
  const char *mode = luaL_optstring(L, 3, "bt");
  int env = (!lua_isnone(L, 4) ? 4 : 0);  /* 'env' index or 0 if no 'env' */
  int upidx = (!lua_isnone(L, 5) ? 5 : 0);
  int upcnt = (upidx ? lua_gettop(L) - upidx + 1 : 0);
  if (s != NULL) {  /* loading a string? */
    const char *chunkname = luaL_optstring(L, 2, s);
    status = luaL_loadbufferx(L, s, l, chunkname, mode);
  }
  else {  /* loading from a reader function */
    const char *chunkname = luaL_optstring(L, 2, "=(load)");
    luaL_checktype(L, 1, LUA_TFUNCTION);
    lua_settop(L, RESERVEDSLOT + upcnt);
    lua_insert(L, RESERVEDSLOT);  /* create reserved slot */
    status = lua_load(L, generic_reader, NULL, chunkname, mode);
    lua_remove(L, RESERVEDSLOT);  /* remove reserved slot */
  }
  return load_aux(L, status, env, upidx, upcnt);
}

static int newupval(lua_State *L) {
  if (luaL_loadstring(L, "local up return function() return up end") == 0) {  /* create upvalue */
    lua_call(L, 0, 1);  /* get function */
    lua_pushlightuserdata(L, &regkey_upval);  /* get upvalues table */
    lua_rawget(L, LUA_REGISTRYINDEX);
    lua_pushlightuserdata(L, lua_upvalueid(L, 1, 1));  /* push upvalue ID */
    lua_pushvalue(L, -1);
    lua_pushvalue(L, 1);  /* set upvalue ID -> upvalue */
    lua_rawset(L, 2);
    return 1;  /* return upvalue */
  } else {
    return lua_error(L);
  }
}

static const struct luaL_Reg loadxlib[] = {
  {"loadx", loadx},
  {"newupval", newupval},
  {NULL, NULL}
};

int luaopen_loadx (lua_State *L) {
  lua_pushlightuserdata(L, &regkey_upval);  /* registry key */
  lua_newtable(L);  /* upvalues */
  lua_newtable(L);  /* metatable */
  lua_pushliteral(L, "k");  /* weak keys */
  lua_setfield(L, -2, "__mode");
  lua_setmetatable(L, -2);
  lua_rawset(L, LUA_REGISTRYINDEX);  /* setup registry. can't use rawsetp because Lua 5.1 */
  luaL_newlib(L, loadxlib);
  return 1;
}

