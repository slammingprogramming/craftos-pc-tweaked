/*
 * peripheral/scripted.cpp
 * CraftOS-Tweaked
 *
 * This file implements scripted peripherals, whose methods are Lua functions.
 *
 * This file is part of CraftOS-Tweaked, a fork of CraftOS-PC 2 by JackMacWindows.
 * Copyright (c) 2026 slammingprogramming. Licensed under the GNU Affero General Public License, version 3 or later.
 */

#include <algorithm>
#include <stdexcept>
#include "scripted.hpp"

scripted::scripted(lua_State *L, const char * side) {
    if (!lua_istable(L, 3)) throw std::invalid_argument("bad argument #3 (table expected, got " + std::string(argTypeName(L, 3)) + ")");
    lua_getfield(L, 3, "types");
    if (lua_istable(L, -1)) {
        for (int i = 1; ; i++) {
            lua_rawgeti(L, -1, i);
            if (!lua_isstring(L, -1)) {lua_pop(L, 1); break;}
            types.push_back(lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    } else if (lua_isstring(L, -1)) types.push_back(lua_tostring(L, -1));
    lua_pop(L, 1);
    lua_getfield(L, 3, "type");
    if (types.empty() && lua_isstring(L, -1)) types.push_back(lua_tostring(L, -1));
    lua_pop(L, 1);
    if (types.empty()) throw std::invalid_argument("the peripheral needs a type (\"type\" or \"types\")");
    lua_getfield(L, 3, "methods");
    if (!lua_istable(L, -1)) {lua_pop(L, 1); throw std::invalid_argument("the peripheral needs a \"methods\" table");}
    lua_pushnil(L);
    while (lua_next(L, -2)) {
        if (lua_type(L, -2) == LUA_TSTRING && lua_isfunction(L, -1)) methodNames.push_back(lua_tostring(L, -2));
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    std::sort(methodNames.begin(), methodNames.end());
    lua_pushvalue(L, 3);
    ref = luaL_ref(L, LUA_REGISTRYINDEX);
    typeName = types.size() == 1 ? types[0] : "!!MULTITYPE";
    for (const std::string& name : methodNames) functions.push_back({name.c_str(), NULL});
    functions.push_back({NULL, NULL});
    library = {typeName.c_str(), functions.data(), nullptr, nullptr};
}

scripted::~scripted() {
    // the reference belongs to a Lua state that may be gone; leaving it in the registry is harmless
}

int scripted::call(lua_State *L, const char * method) {
    lastCFunction = __func__;
    if (ref == LUA_NOREF) return luaL_error(L, "The scripted peripheral does not exist any more");
    const int nargs = lua_gettop(L);
    lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
    lua_getfield(L, -1, "methods");
    lua_getfield(L, -1, method);
    if (!lua_isfunction(L, -1)) return luaL_error(L, "No such method %s", method);
    lua_insert(L, 1);   // the function goes below its arguments: function, arguments..., spec, methods
    lua_pop(L, 2);
    lua_call(L, nargs, LUA_MULTRET);
    return lua_gettop(L);
}
