#include <lua.h>
#include <lauxlib.h>

static int load_aux (lua_State *L, int status, int envidx, int upvalidx, int upvalcnt) {
  int i;
#if LUA_VERSION_NUM == 501
  if (status == 0) {
#else
  if (status == LUA_OK) {
#endif
    if (envidx != 0) {  /* 'env' parameter? */
      lua_pushvalue(L, envidx);  /* environment for loaded function */
      if (!lua_setupvalue(L, -2, 1))  /* set it as 1st upvalue */
        lua_pop(L, 1);  /* remove 'env' if not used by previous call */
    }
    if (upvalidx != 0) { /* upvalcnt = 0 means 1 upvalue */
      for (i = 0; i < upvalcnt; ++i) {
        lua_pushvalue(L, upvalidx + i);
        if (!lua_setupvalue(L, -2, (2 + i))) {
          lua_pop(L, 1);
          break; /* do not waste CPU time trying to set upvalues */
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
#if LUA_VERSION_NUM == 501
    status = luaL_loadbuffer(L, s, l, chunkname);
#else
    status = luaL_loadbufferx(L, s, l, chunkname, mode);
#endif
  }
  else {  /* loading from a reader function */
    const char *chunkname = luaL_optstring(L, 2, "=(load)");
    luaL_checktype(L, 1, LUA_TFUNCTION);
    lua_settop(L, RESERVEDSLOT + upcnt);
    /* lua_pushnil(L); / * probably not needed */
    lua_insert(L, RESERVEDSLOT);  /* create reserved slot */
#if LUA_VERSION_NUM == 501
    status = lua_load(L, generic_reader, NULL, chunkname);
#else
    status = lua_load(L, generic_reader, NULL, chunkname, mode);
#endif
    lua_remove(L, RESERVEDSLOT);
  }
  return load_aux(L, status, env, upidx, upcnt);
}

static const struct luaL_Reg loadxlib[] = {
  {"loadx", loadx},
  {NULL, NULL}
};

int luaopen_loadx (lua_State *L) {
#if LUA_VERSION_NUM == 501
  luaL_register(L, "loadx", loadxlib);
#else
  luaL_newlib(L, loadxlib);
#endif
  return 1;
}

