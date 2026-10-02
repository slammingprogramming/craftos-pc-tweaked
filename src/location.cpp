/*
 * location.cpp
 * CraftOS-Tweaked
 *
 * This file implements the choice of the data folder (see location.hpp).
 *
 * This file is part of CraftOS-Tweaked, a fork of CraftOS-PC 2 by JackMacWindows.
 * Copyright (c) 2026 slammingprogramming. Licensed under the GNU Affero General Public License, version 3 or later.
 */

#include "location.hpp"

#if defined(__ANDROID__) || defined(__IPHONEOS__) || defined(__EMSCRIPTEN__)
// the mobile and web builds keep their sandbox locations
void resolveDataLocations(bool dataDirExplicit, bool romDirExplicit) {}
#else
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <SDL2/SDL.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Stringifier.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include "platform.hpp"

extern int selectedRenderer;

static path_t executableDir() {
    char * base = SDL_GetBasePath();
    if (base == NULL) return path_t();
    path_t dir(base);
    SDL_free(base);
    return dir.has_filename() ? dir : dir.parent_path(); // SDL ends the path with a separator
}

static bool isWritableDir(const path_t& dir) {
    if (dir.empty()) return false;
    std::error_code ec;
    if (!fs::is_directory(dir, ec)) return false;
    const path_t probe = dir / ".craftos-tweaked-write-test";
    {
        std::ofstream out(probe);
        if (!out.is_open()) return false;
    }
    fs::remove(probe, ec);
    return true;
}

// The folder of a CraftOS-PC 2 installation, if there is one that holds computers or settings
static path_t legacyDataDir() {
    path_t dir;
#if defined(_WIN32)
    if (const char * appdata = getenv("APPDATA")) dir = path_t(appdata) / "CraftOS-PC";
#elif defined(__APPLE__)
    if (const char * home = getenv("HOME")) dir = path_t(home) / "Library" / "Application Support" / "CraftOS-PC";
#else
    if (const char * xdg = getenv("XDG_DATA_HOME"); xdg && *xdg) dir = path_t(xdg) / "craftos-pc";
    else if (const char * home = getenv("HOME")) dir = path_t(home) / ".local" / "share" / "craftos-pc";
#endif
    std::error_code ec;
    if (!dir.empty() && (fs::is_directory(dir / "computer", ec) || fs::is_directory(dir / "config", ec))) return dir;
    return path_t();
}

static bool insideAppBundle(const path_t& dir) {
#ifdef __APPLE__
    return dir.string().find(".app/Contents/") != std::string::npos;
#else
    return false;
#endif
}

static path_t resolveRelative(const std::string& value, const path_t& folder) {
    path_t p(value);
    return p.is_absolute() ? p : (folder / p).lexically_normal();
}

static bool readSettings(const path_t& file, std::string& dataDirectory, std::string& romDirectory) {
    std::ifstream in(file);
    if (!in.is_open()) return false;
    try {
        Poco::JSON::Parser parser;
        Poco::JSON::Object::Ptr root = parser.parse(in).extract<Poco::JSON::Object::Ptr>();
        if (root->has("dataDirectory")) dataDirectory = root->getValue<std::string>("dataDirectory");
        if (root->has("romDirectory")) romDirectory = root->getValue<std::string>("romDirectory");
        return true;
    } catch (...) {
        fprintf(stderr, "Could not read %s; ignoring it\n", file.string().c_str());
        return false;
    }
}

static void saveSettings(const path_t& file, const path_t& dataDirectory) {
    std::error_code ec;
    fs::create_directories(file.parent_path(), ec);
    std::ofstream out(file);
    if (!out.is_open()) return;
    Poco::JSON::Object root(true);
    root.set("dataDirectory", dataDirectory.string());
    Poco::JSON::Stringifier::stringify(root, out, 2);
    out << "\n";
}

// Asks where to keep the data. Returns the index of the choice, or -1 if nobody could be asked.
static int askForLocation(const std::vector<std::string>& labels, const std::string& message) {
    if (selectedRenderer == -1 || selectedRenderer == 0 || selectedRenderer == 5) {
        std::vector<SDL_MessageBoxButtonData> buttons;
        for (size_t i = 0; i < labels.size(); i++) buttons.push_back({i == 0 ? (Uint32)SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT : 0u, (int)i, labels[i].c_str()});
        SDL_MessageBoxData data = {SDL_MESSAGEBOX_INFORMATION, NULL, "Where should CraftOS-Tweaked keep its data?", message.c_str(), (int)buttons.size(), buttons.data(), NULL};
        int choice = -1;
        if (SDL_ShowMessageBox(&data, &choice) == 0) return choice;
        return -1;
    }
#ifndef _WIN32
    if (selectedRenderer == 2 && isatty(0) && isatty(1)) {
        std::cout << "\nWhere should CraftOS-Tweaked keep its data?\n" << message << "\n\n";
        for (size_t i = 0; i < labels.size(); i++) std::cout << "  " << i + 1 << ") " << labels[i] << (i == 0 ? " (default)" : "") << "\n";
        std::cout << "Choice [1]: " << std::flush;
        std::string line;
        std::getline(std::cin, line);
        const int n = atoi(line.c_str());
        return n >= 1 && (size_t)n <= labels.size() ? n - 1 : 0;
    }
#endif
    return -1;
}

void resolveDataLocations(bool dataDirExplicit, bool romDirExplicit) {
    const path_t exeDir = executableDir();
    const path_t userDefault = getBasePath(); // what the platform uses (named CraftOS-Tweaked)
    const path_t exeSettings = exeDir / "craftos-tweaked.json", userSettings = userDefault / "location.json";

    std::string dataDirectory, romDirectory;
    path_t settingsFolder;
    bool found = false;
    if (!exeDir.empty() && readSettings(exeSettings, dataDirectory, romDirectory)) {found = true; settingsFolder = exeDir;}
    else if (readSettings(userSettings, dataDirectory, romDirectory)) {found = true; settingsFolder = userDefault;}

    if (found) {
        if (!dataDirExplicit && !dataDirectory.empty()) setBasePath(resolveRelative(dataDirectory, settingsFolder));
        if (!romDirExplicit && !romDirectory.empty()) setROMPath(resolveRelative(romDirectory, settingsFolder));
        return;
    }
    if (dataDirExplicit) return;

    // first run
    const path_t legacy = legacyDataDir();
    const bool canBePortable = !exeDir.empty() && !insideAppBundle(exeDir) && isWritableDir(exeDir);
    const path_t portable = exeDir / "computercraft";
    std::vector<std::string> labels;
    std::vector<path_t> targets;
    if (canBePortable) {labels.push_back("Next to the program"); targets.push_back(portable);}
    labels.push_back("In my user folder"); targets.push_back(userDefault);
    if (!legacy.empty()) {labels.push_back("Keep my CraftOS-PC data"); targets.push_back(legacy);}

    std::string message = "CraftOS-Tweaked keeps its computers, settings and crash logs in one folder.\n\n";
    if (canBePortable) message += "Next to the program: " + portable.string() + "\n(a portable install; CC: Tweaked also keeps computers in a \"computercraft\" folder)\n\n";
    message += "In my user folder: " + userDefault.string() + "\n";
    if (!legacy.empty()) message += "\nA CraftOS-PC folder with computers in it was found: " + legacy.string() + "\n";
    message += "\nThe choice is saved in craftos-tweaked.json, which you can edit later, and -d <folder> overrides it for one run.";

    int choice = askForLocation(labels, message);
    const bool asked = choice >= 0;
    if (!asked) choice = !legacy.empty() ? (int)targets.size() - 1 : 0; // nobody to ask: keep existing data, else the first option
    setBasePath(targets[choice]);
    if (asked) {
        // remember the choice next to the program if that is possible, else in the user folder
        saveSettings(canBePortable ? exeSettings : userSettings, targets[choice]);
    }
}
#endif
