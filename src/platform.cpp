/*
 * platform.cpp
 * CraftOS-PC 2
 * 
 * This file controls which platform implementation will be compiled.
 * 
 * This code is licensed under the GNU AGPL v3.0 or later (AGPL-3.0-or-later).
 * Copyright (c) 2019-2024 JackMacWindows.
 * Originally released under the MIT License; see the LICENSE file.
 */

#include "platform.hpp"
#ifdef WIN32
#include "platform/win.cpp"
#else
#ifdef __APPLE__
#include "platform/darwin.cpp"
#else
#ifdef __ANDROID__
#include "platform/android.cpp"
#else
#ifdef __linux__
#include "platform/linux.cpp"
#else
#ifdef __EMSCRIPTEN__
#include "platform/emscripten.cpp"
#else
#error Unknown platform
#endif
#endif
#endif
#endif
#endif