/*
 * location.hpp
 * CraftOS-Tweaked
 *
 * This file declares where the data folder is chosen.
 *
 * This file is part of CraftOS-Tweaked, a fork of CraftOS-PC 2 by JackMacWindows.
 * Copyright (c) 2026 slammingprogramming. Licensed under the GNU Affero General Public License, version 3 or later.
 */

#ifndef LOCATION_HPP
#define LOCATION_HPP
#include "util.hpp"

// Decides where the data folder (computers, settings, crash logs) and the ROM folder are. Order of precedence:
//   1. -d / --rom on the command line (the arguments say dataDirExplicit / romDirExplicit);
//   2. craftos-tweaked.json next to the executable (or location.json in the per-user folder if the program's own folder
//      cannot be written): {"dataDirectory": "...", "romDirectory": "..."}, relative paths start in the file's folder;
//   3. the first-run choice: the "computercraft" folder next to the executable (CC: Tweaked keeps computers in
//      computercraft/computer/<id> too, and this keeps the install portable), or the user's own data folder. A window or
//      terminal asks once and remembers; headless and raw runs just use the default without asking.
// An existing CraftOS-PC data folder is offered (and, when nobody can be asked, kept in use) so no computers are lost.
extern void resolveDataLocations(bool dataDirExplicit, bool romDirExplicit);

#endif
