/*
 * example_peripheral.cpp
 * CraftOS-Tweaked
 *
 * This file is an example of a peripheral plugin with a few simple methods.
 *
 * This code is licensed under the GNU AGPL v3.0 or later (AGPL-3.0-or-later).
 * Copyright (c) 2019-2024 JackMacWindows.
 * Originally released under the MIT License; see the LICENSE file.
 */

extern "C" {
#include <lua.h>
#include <lauxlib.h>
}
#include <CraftOS-PC.hpp>

class example_peripheral: public peripheral {
    int add(lua_State *L) {
        lua_pushnumber(L, luaL_checknumber(L, 1) + luaL_checknumber(L, 2));
        return 1;
    }
    int subtract(lua_State *L) {
        lua_pushnumber(L, luaL_checknumber(L, 1) - luaL_checknumber(L, 2));
        return 1;
    }
    int ping(lua_State *L) {
        lua_pushstring(L, "pong");
        return 1;
    }
public:
    static library_t methods;
    example_peripheral(lua_State *L, const char * side) {}
    ~example_peripheral(){}
    int call(lua_State *L, const char * method) {
        std::string m(method);
        if (m == "add") return add(L);
        else if (m == "subtract") return subtract(L);
        else if (m == "ping") return ping(L);
        else return luaL_error(L, "No such method");
    }
    static peripheral * init(lua_State *L, const char * side) {return new example_peripheral(L, side);}
    static void deinit(peripheral * p) {delete (example_peripheral*)p;}
    virtual destructor getDestructor() const {return deinit;}
    void update(){}
    virtual library_t getMethods() const {return methods;}
};

static luaL_Reg methods_reg[] = {
    {"add", NULL},
    {"subtract", NULL},
    {"ping", NULL},
};
static PluginInfo info;
library_t example_peripheral::methods = {"example_peripheral", methods_reg, nullptr, nullptr};

extern "C" {
DLLEXPORT PluginInfo * plugin_init(PluginFunctions * func, const path_t& path) {
    func->registerPeripheralFn("example_peripheral", &example_peripheral::init);
    return &info;
}
}
