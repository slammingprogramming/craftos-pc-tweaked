/*
 * peripheral/scripted.hpp
 * CraftOS-Tweaked
 *
 * This file defines the class for scripted peripherals, whose methods are Lua functions.
 *
 * This file is part of CraftOS-Tweaked, a fork of CraftOS-PC 2 by JackMacWindows.
 * Copyright (c) 2026 slammingprogramming. Licensed under the GNU Affero General Public License, version 3 or later.
 */

#ifndef PERIPHERAL_SCRIPTED_HPP
#define PERIPHERAL_SCRIPTED_HPP
#include "../util.hpp"

// periphemu.create(side, "scripted", {types = {"type", "other_type"}, methods = {name = function(...) ... end}}) attaches
// a peripheral that is made of Lua functions. It is what the test suite uses to stand in for peripherals that only exist
// inside Minecraft (CC: Tweaked's own tests have a fake peripheral hub), and it works for any program that needs one.
class scripted: public peripheral {
    int ref = LUA_NOREF;
    std::string typeName;
    std::vector<std::string> types, methodNames;
    std::vector<luaL_Reg> functions;
    library_t library;
public:
    static peripheral * init(lua_State *L, const char * side) {return new scripted(L, side);}
    static void deinit(peripheral * p) {delete (scripted*)p;}
    destructor getDestructor() const override {return deinit;}
    library_t getMethods() const override {return library;}
    std::vector<std::string> getTypes() const override {return types;}
    void reinitialize(lua_State *L) override {ref = LUA_NOREF;} // the functions lived in the state that was closed
    scripted(lua_State *L, const char * side);
    ~scripted();
    int call(lua_State *L, const char * method) override;
};

#endif
