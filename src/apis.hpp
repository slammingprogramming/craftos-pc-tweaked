/*
 * apis.hpp
 * CraftOS-PC 2
 *
 * This file defines the library structures for all CraftOS APIs.
 *
 * This code is licensed under the GNU AGPL v3.0 or later (AGPL-3.0-or-later).
 * Copyright (c) 2019-2024 JackMacWindows.
 * Originally released under the MIT License; see the LICENSE file.
 */

#ifndef APIS_HPP
#define APIS_HPP
#include "util.hpp"
extern library_t config_lib;
extern library_t fs_lib;
extern library_t http_lib;
#ifndef NO_MOUNTER
extern library_t mounter_lib;
#endif
extern library_t os_lib;
extern library_t periphemu_lib;
extern library_t peripheral_lib;
extern library_t rs_lib;
extern library_t term_lib;
#endif
