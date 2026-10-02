/*
 * peripheral/computer.hpp
 * CraftOS-Tweaked
 * 
 * This file defines the class for the computer peripheral.
 * 
 * This code is licensed under the GNU AGPL v3.0 or later (AGPL-3.0-or-later).
 * Copyright (c) 2019-2024 JackMacWindows. 
 * Originally released under the MIT License; see the LICENSE file.
 */

#ifndef PERIPHERAL_COMPUTER_HPP
#define PERIPHERAL_COMPUTER_HPP
#include <peripheral.hpp>

class computer: public peripheral {
    friend struct Computer;
    Computer * comp;
    Computer * thiscomp;
    int turnOn(lua_State *L);
    int shutdown(lua_State *L);
    int reboot(lua_State *L);
    int getID(lua_State *L);
    int isOn(lua_State *L);
    int getLabel(lua_State *L);
public:
    static library_t methods;
    static peripheral * init(lua_State *L, const char * side) {return new computer(L, side);}
    static void deinit(peripheral * p) {delete (computer*)p;}
    destructor getDestructor() const override {return deinit;}
    library_t getMethods() const override {return methods;}
    computer(lua_State *L, const char * side);
    ~computer();
    int call(lua_State *L, const char * method) override;
};

#endif