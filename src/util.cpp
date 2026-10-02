/*
 * util.cpp
 * CraftOS-Tweaked
 * 
 * This file implements some commonly-used functions.
 * 
 * This code is licensed under the GNU AGPL v3.0 or later (AGPL-3.0-or-later).
 * Copyright (c) 2019-2024 JackMacWindows.
 * Originally released under the MIT License; see the LICENSE file.
 */

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <atomic>
#include <cstring>
#include <sstream>
#include <Computer.hpp>
#include <dirent.h>
#include <Poco/Net/IPAddress.h>
#include <Poco/Base64Decoder.h>
#include <Poco/Base64Encoder.h>
#include <sys/stat.h>
#include <FileEntry.hpp>
#include "platform.hpp"
#include "runtime.hpp"
#include "terminal/SDLTerminal.hpp"
#include "util.hpp"
#ifndef WIN32
#include <libgen.h>
#include <fcntl.h>
#include <unistd.h>
#endif
#include <cstdarg>
#include <ctime>
#include <fstream>

#ifdef STANDALONE_ROM
extern FileEntry standaloneROM;
extern FileEntry standaloneDebug;
extern std::string standaloneBIOS;
#endif

const char * lastCFunction = "(none!)";
char computer_key = 'C';
static ProtectedObject<std::unordered_map<void*, Computer*> > getCompCache;

const wchar_t charsetConversion[256] = {
    // lower CP437 characters
    0x0000, 0x263A, 0x263B, 0x2665, 0x2666, 0x2663, 0x2660, 0x25CF, 0x25CB, 0x0009, 0x000A, 0x2642, 0x2640, 0x000D, 0x266A, 0x266C,
    0x25B6, 0x25C0, 0x2195, 0x203C, 0x00B6, 0x00A7, 0x25AC, 0x21A8, 0x2B06, 0x2B07, 0x27A1, 0x2B05, 0x221F, 0x29FA, 0x25B2, 0x25BC,
    // ASCII characters
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
    0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
    0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x1FB99,
    // drawing characters
    0x0020, 0x1FB00, 0x1FB01, 0x1FB02, 0x1FB03, 0x1FB04, 0x1FB05, 0x1FB06, 0x1FB07, 0x1FB08, 0x1FB09, 0x1FB0A, 0x1FB0B, 0x1FB0C, 0x1FB0D, 0x1FB0E,
    0x1FB0F, 0x1FB10, 0x1FB11, 0x1FB12, 0x1FB13, 0x258C, 0x1FB14, 0x1FB15, 0x1FB16, 0x1FB17, 0x1FB18, 0x1FB19, 0x1FB1A, 0x1FB1B, 0x1FB1C, 0x1FB1D,
    // ISO-8859-1 characters
    0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
    0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
    0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7, 0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
    0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
    0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7, 0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF
};

Computer * get_comp(lua_State *L) {
    try {
        return getCompCache->at(L->l_G);
    } catch (std::out_of_range &e) {
        LockGuard lock(getCompCache);
        lua_rawgeti(L, LUA_REGISTRYINDEX, 1);
        Computer * retval = (Computer*)lua_touserdata(L, -1);
        lua_pop(L, 1);
        getCompCache->insert(std::make_pair(L->l_G, retval));
        return retval;
    }
}

void uncache_state(lua_State *L) {
    LockGuard lock(getCompCache);
    getCompCache->erase(L->l_G);
}

void load_library(Computer *comp, lua_State *L, const library_t& lib) {
    lua_newtable(L);
    luaL_setfuncs(L, lib.functions, 0);
    lua_setglobal(L, lib.name);
    if (lib.init != NULL) lib.init(comp);
}

std::string b64encode(const std::string& orig) {
    std::stringstream ss;
    Poco::Base64Encoder enc(ss);
    enc.write(orig.c_str(), orig.size());
    enc.close();
    return ss.str();
}

std::string b64decode(const std::string& orig) {
    std::stringstream ss;
    std::stringstream out(orig);
    Poco::Base64Decoder dec(out);
    std::copy(std::istreambuf_iterator<char>(dec), std::istreambuf_iterator<char>(), std::ostreambuf_iterator<char>(ss));
    return ss.str();
}

std::vector<std::string> split(const std::string& strToSplit, const char * delims) {
    std::vector<std::string> retval;
    size_t pos = strToSplit.find_first_not_of(delims);
    while (pos != std::string::npos) {
        const size_t end = strToSplit.find_first_of(delims, pos);
        retval.push_back(strToSplit.substr(pos, end - pos));
        pos = strToSplit.find_first_not_of(delims, end);
    }
    return retval;
}

std::vector<std::wstring> split(const std::wstring& strToSplit, const wchar_t * delims) {
    std::vector<std::wstring> retval;
    size_t pos = strToSplit.find_first_not_of(delims);
    while (pos != std::string::npos) {
        const size_t end = strToSplit.find_first_of(delims, pos);
        retval.push_back(strToSplit.substr(pos, end - pos));
        pos = strToSplit.find_first_not_of(delims, end);
    }
    return retval;
}

std::vector<path_t> split(const path_t& strToSplit, const path_t::value_type * delims) {
    std::vector<path_t> retval;
    path_t::string_type str = strToSplit.native();
    size_t pos = str.find_first_not_of(delims);
    while (pos != std::string::npos) {
        const size_t end = str.find_first_of(delims, pos);
        retval.push_back(str.substr(pos, end - pos));
        pos = str.find_first_not_of(delims, end);
    }
    return retval;
}

static std::string concat(const std::list<std::string> &c, char sep) {
    std::stringstream ss;
    bool started = false;
    for (const std::string& s : c) {
        if (started) ss << sep;
        ss << s;
        started = true;
    }
    return ss.str();
}

static std::list<std::string> split_list(const std::string& strToSplit, const char * delims) {
    std::list<std::string> retval;
    size_t pos = strToSplit.find_first_not_of(delims);
    while (pos != std::string::npos) {
        size_t end = strToSplit.find_first_of(delims, pos);
        retval.push_back(strToSplit.substr(pos, end - pos));
        pos = strToSplit.find_first_not_of(delims, end);
    }
    return retval;
}

path_t fixpath_mkdir(Computer * comp, const std::string& path, bool md, std::string * mountPath) {
    if (md && fixpath_ro(comp, path)) return path_t();
    path_t firstTest = fixpath(comp, path, true, true, mountPath);
    if (!firstTest.empty()) return firstTest;
    std::list<std::string> components = split_list(path, "/\\");
    while (!components.empty() && components.front().empty()) components.pop_front();
    if (components.empty()) return fixpath(comp, "", true);
    components.pop_back();
    std::list<std::string> append;
    path_t maxPath = fixpath(comp, concat(components, '/'), false, true, mountPath);
    while (maxPath.empty()) {
        append.push_front(components.back());
        components.pop_back();
        if (components.empty()) return path_t();
        maxPath = fixpath(comp, concat(components, '/'), false, true, mountPath);
    }
    if (!md) return maxPath;
    for (const std::string& s : append) maxPath /= s;
    std::error_code e;
    fs::create_directories(maxPath, e);
    if (e) return path_t();
    return fixpath(comp, path, false, true, mountPath);
}

static bool _nothrow(std::function<void()> f) { try { f(); return true; } catch (...) { return false; } }
#define nothrow(expr) _nothrow([&](){ expr ;})

inline bool isVFSPath(path_t path) {
    if (!std::isdigit(path.native()[0])) return false;
    for (const path_t::value_type& c : path.native()) {
        if (c == ':') return true;
        else if (!std::isdigit(c)) return false;
    }
    return false;
}

path_t fixpath(Computer *comp, std::string path, bool exists, bool addExt, std::string * mountPath, bool * isRoot) {
    path.erase(std::remove_if(path.begin(), path.end(), [](char c)->bool {return c == '"' || c == '*' || c == ':' || c == '<' || c == '>' || c == '?' || c == '|' || c < 32; }), path.end());
    std::vector<std::string> elems = split(path, "/\\");
    std::list<std::string> pathc;
    for (std::string s : elems) {
        if (s == "..") {
            if (pathc.empty() && addExt) return path_t();
            else if (pathc.empty()) pathc.push_back("..");
            else pathc.pop_back();
        } else if (!s.empty() && s.find_first_not_of(' ') != std::string::npos && !std::all_of(s.begin(), s.end(), [](const char c)->bool{return c == '.';})) {
            s = s.substr(s.find_first_not_of(' '), s.find_last_not_of(' ') - s.find_first_not_of(' ') + 1);
            pathc.push_back(s);
        }
    }
    while (!pathc.empty() && pathc.front().empty()) pathc.pop_front();
    if (!pathc.empty() && pathc.back().size() > 255) {
        std::string s = pathc.back().substr(0, 255);
        pathc.pop_back();
        s = s.substr(0, s.find_last_not_of(' '));
        pathc.push_back(s);
    }
    if (comp->isDebugger && addExt && pathc.size() == 1 && pathc.front() == "bios.lua")
#ifdef STANDALONE_ROM
        return path_t(":bios.lua", path_t::format::generic_format);
#else
        return getROMPath()/"bios.lua";
#endif
    path_t ss;
    std::error_code e;
    if (addExt) {
        std::pair<size_t, std::vector<_path_t> > max_path = std::make_pair(0, std::vector<_path_t>(1, comp->dataDir));
        std::list<std::string> * mount_list = NULL;
        for (auto& m : comp->mounts) {
            std::list<std::string> &pathlist = std::get<0>(m);
            if (pathc.size() >= pathlist.size() && std::equal(pathlist.begin(), pathlist.end(), pathc.begin())) {
                if (pathlist.size() > max_path.first) {
                    max_path = std::make_pair(pathlist.size(), std::vector<_path_t>(1, std::get<1>(m)));
                    mount_list = &pathlist;
                } else if (pathlist.size() == max_path.first) {
                    max_path.second.push_back(std::get<1>(m));
                }
            }
        }
        for (size_t i = 0; i < max_path.first; i++) pathc.pop_front();
        if (isRoot != NULL) *isRoot = pathc.empty();
        if (exists) {
            bool found = false;
            for (const _path_t& p : max_path.second) {
                path_t sstmp = p;
                for (const std::string& s : pathc) sstmp /= s;
                e.clear();
                if ((isVFSPath(p) && nothrow(comp->virtualMounts[(unsigned)std::stoul(p.substr(0, p.size()-1))]->path(sstmp))) || (fs::exists(sstmp, e))) {
                    ss /= sstmp;
                    found = true;
                    break;
                }
            }
            if (!found) return path_t();
        } else if (pathc.size() > 1) {
            bool found = false;
            std::stack<std::string> oldback;
            while (!found && !pathc.empty()) {
                found = false;
                std::string back = pathc.back();
                pathc.pop_back();
                for (const _path_t& p : max_path.second) {
                    path_t sstmp = p;
                    for (const std::string& s : pathc) sstmp /= s;
                    e.clear();
                    if (
                        (isVFSPath(p) && (nothrow(comp->virtualMounts[(unsigned)std::stoul(p.substr(0, p.size()-1))]->path(ss/back)) ||
                        (nothrow(comp->virtualMounts[(unsigned)std::stoul(p.substr(0, p.size()-1))]->path(sstmp)) && comp->virtualMounts[(unsigned)std::stoul(p.substr(0, p.size()-1))]->path(sstmp).isDir))) ||
                        (fs::exists(sstmp/back, e)) || (fs::is_directory(sstmp, e))) {
                        ss /= sstmp/back;
                        while (!oldback.empty()) {
                            ss /= oldback.top();
                            oldback.pop();
                        }
                        found = true;
                        break;
                    }
                }
                if (!found) oldback.push(back);
            }
            if (!found) return path_t();
        } else {
            ss /= max_path.second.front();
            for (const std::string& s : pathc) ss /= s;
        }
        if (mountPath != NULL) {
            if (mount_list == NULL) *mountPath = "hdd";
            else {
                std::stringstream ss2;
                for (auto it = mount_list->begin(); it != mount_list->end(); ++it) {
                    if (it != mount_list->begin()) ss2 << "/";
                    ss2 << *it;
                }
                *mountPath = ss2.str();
            }
        }
    } else for (const std::string& s : pathc) ss /= s;
    if (path_t::preferred_separator != (path_t::value_type)'/' && (!addExt || isVFSPath(ss))) {
        path_t::string_type str = ss.native();
        std::replace(str.begin(), str.end(), path_t::preferred_separator, (path_t::value_type)'/');
        ss = path_t(str);
    }
    return ss;
}

bool fixpath_ro(Computer *comp, std::string path) {
    path.erase(std::remove_if(path.begin(), path.end(), [](char c)->bool {return c == '"' || c == '*' || c == ':' || c == '<' || c == '>' || c == '?' || c == '|' || c < 32; }), path.end());
    std::vector<std::string> elems = split(path, "/\\");
    std::list<std::string> pathc;
    for (std::string s : elems) {
        if (s == "..") { if (pathc.empty()) return false; else pathc.pop_back(); }
        else if (!s.empty() && !std::all_of(s.begin(), s.end(), [](const char c)->bool{return c == '.';})) {
            s = s.substr(s.find_first_not_of(' '), s.find_last_not_of(' ') - s.find_first_not_of(' ') + 1);
            pathc.push_back(s);
        }
    }
    while (!pathc.empty() && pathc.front().empty()) pathc.pop_front();
    if (!pathc.empty() && pathc.back().size() > 255) {
        std::string s = pathc.back().substr(0, 255);
        pathc.pop_back();
        s = s.substr(0, s.find_last_not_of(' '));
        pathc.push_back(s);
    }
    std::pair<size_t, bool> max_path = std::make_pair(0, false);
    for (const auto& m : comp->mounts)
        if (pathc.size() >= std::get<0>(m).size() && std::get<0>(m).size() > max_path.first && std::equal(std::get<0>(m).begin(), std::get<0>(m).end(), pathc.begin()))
            max_path = std::make_pair(std::get<0>(m).size(), std::get<2>(m));
    return max_path.second;
}

std::set<std::string> getMounts(Computer * computer, std::string comp_path) {
    comp_path.erase(std::remove_if(comp_path.begin(), comp_path.end(), [](char c)->bool {return c == '"' || c == '*' || c == ':' || c == '<' || c == '>' || c == '?' || c == '|' || c < 32; }), comp_path.end());
    std::vector<std::string> elems = split(comp_path, "/\\");
    std::list<std::string> pathc;
    std::set<std::string> retval;
    for (std::string s : elems) {
        if (s == "..") { if (pathc.empty()) return retval; else pathc.pop_back(); }
        else if (!s.empty() && !std::all_of(s.begin(), s.end(), [](const char c)->bool{return c == '.';})) {
            pathc.push_back(s);
        }
    }
    for (const auto& m : computer->mounts)
        if (pathc.size() + 1 == std::get<0>(m).size() && std::equal(pathc.begin(), pathc.end(), std::get<0>(m).begin()))
            retval.insert(std::get<0>(m).back());
    return retval;
}

static void xcopy_internal(lua_State *from, lua_State *to, int n, int copies_slot) {
    for (int i = n - 1; i >= 0; i--) {
        size_t sz = 0;
        switch (lua_type(from, -1-i)) {
            case LUA_TNIL: case LUA_TNONE: lua_pushnil(to); break;
            case LUA_TBOOLEAN: lua_pushboolean(to, lua_toboolean(from, -1-i)); break;
            case LUA_TNUMBER: lua_pushnumber(to, lua_tonumber(from, -1-i)); break;
            case LUA_TSTRING: {
                const char * str = lua_tolstring(from, -1-i, &sz);
                lua_pushlstring(to, str, sz); break;
            } case LUA_TTABLE: {
                const void* ptr = lua_topointer(from, -1-i);
                lua_rawgeti(to, copies_slot, (ptrdiff_t)ptr);
                if (!lua_isnil(to, -1)) continue;
                lua_pop(to, 1);
                lua_newtable(to);
                lua_pushvalue(to, -1);
                lua_rawseti(to, copies_slot, (ptrdiff_t)ptr);
                lua_pushnil(from);
                while (lua_next(from, -2-i) != 0) {
                    xcopy_internal(from, to, 2, copies_slot);
                    lua_settable(to, -3);
                    lua_pop(from, 1);
                }
                break;
            }
            default: {
                if (luaL_callmeta(from, -1-i, "__tostring")) {
                    lua_pushlstring(to, lua_tostring(from, -1), lua_rawlen(from, -1));
                    lua_pop(from, 1);
                } else lua_pushfstring(to, "<%s: %p>", lua_typename(from, lua_type(from, -1-i)), lua_topointer(from, -1-i));
                break;
            }
        }
    }
}

void xcopy(lua_State *from, lua_State *to, int n) {
    lua_newtable(to);
    int cslot = lua_gettop(to);
    xcopy_internal(from, to, n, cslot);
    lua_remove(to, cslot);
}

// Deprecated as of CCPC v2.8; text mode no longer exists
std::string makeASCIISafe(const char * retval, size_t len) {
    return std::string(retval, len);
}

// Addresses are handled as 16 bytes (IPv4 addresses as IPv4-mapped IPv6 ones), so that one prefix match covers both.
using IPBytes = std::array<uint8_t, 16>;

static IPBytes ipv4Bytes(int a, int b, int c, int d) {
    return IPBytes{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, (uint8_t)a, (uint8_t)b, (uint8_t)c, (uint8_t)d};
}

// Parses an IPv4 or IPv6 literal (IPv6 may be in brackets). Returns false for anything else, such as host names.
static bool parseIPLiteral(std::string text, IPBytes& out) {
    if (text.size() > 1 && text.front() == '[' && text.back() == ']') text = text.substr(1, text.size() - 2);
    if (text.find('%') != std::string::npos) return false; // scoped addresses are refused elsewhere
    Poco::Net::IPAddress ip;
    if (!Poco::Net::IPAddress::tryParse(text, ip)) return false;
    if (ip.family() == Poco::Net::IPAddress::IPv4) {
        const uint8_t * b = (const uint8_t*)ip.addr();
        out = ipv4Bytes(b[0], b[1], b[2], b[3]);
    } else {
        std::memcpy(out.data(), ip.addr(), 16);
    }
    return true;
}

static bool inNetwork(const IPBytes& address, const IPBytes& network, int bits) {
    for (int i = 0; i < 16 && bits > 0; i++, bits -= 8) {
        const uint8_t mask = bits >= 8 ? 0xff : (uint8_t)(0xff << (8 - bits));
        if ((address[i] & mask) != (network[i] & mask)) return false;
    }
    return true;
}

// The addresses behind "$private", the same ones CC: Tweaked's AddressPredicate.PrivatePattern refuses: the
// unspecified, loopback, link-local, site-local and multicast addresses, plus the extra ranges IANA reserves.
// IPv4 ranges are written as IPv6 prefixes (96 + the IPv4 prefix length).
static const std::vector<std::pair<IPBytes, int> >& privateRanges() {
    static const std::vector<std::pair<IPBytes, int> > ranges = [] {
        std::vector<std::pair<IPBytes, int> > r;
        const auto v4 = [&r](int a, int b, int c, int d, int bits) {r.push_back({ipv4Bytes(a, b, c, d), 96 + bits});};
        v4(0, 0, 0, 0, 32);        // 0.0.0.0
        v4(10, 0, 0, 0, 8);        // site-local
        v4(100, 64, 0, 0, 10);     // shared address space (carrier-grade NAT)
        v4(127, 0, 0, 0, 8);       // loopback
        v4(169, 254, 0, 0, 16);    // link-local
        v4(172, 16, 0, 0, 12);     // site-local
        v4(192, 0, 0, 0, 24);      // IETF protocol assignments
        v4(192, 0, 2, 0, 24);      // TEST-NET-1
        v4(192, 88, 99, 0, 24);    // 6to4 relay anycast
        v4(192, 168, 0, 0, 16);    // site-local
        v4(198, 18, 0, 0, 15);     // benchmark testing
        v4(198, 51, 100, 0, 24);   // TEST-NET-2
        v4(203, 0, 113, 0, 24);    // TEST-NET-3
        v4(224, 0, 0, 0, 4);       // multicast
        const auto v6 = [&r](std::initializer_list<uint16_t> groups, int bits) {
            IPBytes b{};
            int i = 0;
            for (uint16_t g : groups) {b[i++] = g >> 8; b[i++] = g & 0xff;}
            r.push_back({b, bits});
        };
        v6({0, 0, 0, 0, 0, 0, 0, 0}, 128);      // ::
        v6({0, 0, 0, 0, 0, 0, 0, 1}, 128);      // ::1, loopback
        v6({0xfe80}, 10);                        // link-local
        v6({0xfec0}, 10);                        // site-local
        v6({0xff00}, 8);                         // multicast
        v6({0x64, 0xff9b}, 96);                  // IPv4/IPv6 translation
        v6({0x64, 0xff9b, 1}, 48);               // local-use IPv4/IPv6 translation
        v6({0x2001}, 23);                        // IETF protocol assignments (Teredo, ORCHID, ...)
        v6({0xfc00}, 7);                         // unique local addresses
        return r;
    }();
    return ranges;
}

// Java's Double.toString for the numbers CC: Tweaked prints: the shortest digits that read back as the same number, as
// 123.456 between 1e-3 and 1e7, as 1.23456E10 outside of that
static std::string javaDoubleToString(double d) {
    if (d != d) return "NaN";
    if (d == HUGE_VAL) return "Infinity";
    if (d == -HUGE_VAL) return "-Infinity";
    if (d == 0) return std::signbit(d) ? "-0.0" : "0.0";
    char buf[40];
    int precision = 0;
    for (; precision < 17; precision++) { // digits after the first one in scientific form
        snprintf(buf, sizeof(buf), "%.*e", precision, d);
        if (strtod(buf, NULL) == d) break;
    }
    std::string text = buf; // d.ddddde+XX
    const bool negative = text[0] == '-';
    if (negative) text.erase(0, 1);
    const size_t e = text.find('e');
    int exponent = atoi(text.c_str() + e + 1);
    std::string digits = text.substr(0, e);
    digits.erase(std::remove(digits.begin(), digits.end(), '.'), digits.end());
    while (digits.size() > 1 && digits.back() == '0') digits.pop_back();
    std::string result;
    if (std::fabs(d) >= 1e-3 && std::fabs(d) < 1e7) {
        if (exponent >= 0) {
            while ((int)digits.size() < exponent + 1) digits += '0';
            result = digits.substr(0, exponent + 1) + "." + (digits.size() > (size_t)exponent + 1 ? digits.substr(exponent + 1) : "0");
        } else result = "0." + std::string(-exponent - 1, '0') + digits;
    } else {
        result = digits.substr(0, 1) + "." + (digits.size() > 1 ? digits.substr(1) : "0") + "E" + std::to_string(exponent);
    }
    return negative ? "-" + result : result;
}

std::string coerceToString(lua_State *L, int idx) {
    switch (lua_type(L, idx)) {
        case LUA_TNONE: case LUA_TNIL: return "nil";
        case LUA_TBOOLEAN: return lua_toboolean(L, idx) ? "true" : "false";
        case LUA_TSTRING: {size_t len; const char * s = lua_tolstring(L, idx, &len); return std::string(s, len);}
        case LUA_TNUMBER: {
            const double d = lua_tonumber(L, idx);
            const int i = d >= -2147483648.0 && d <= 2147483647.0 ? (int)d : (d > 0 ? 2147483647 : (-2147483647 - 1)); // a Java (int) cast saturates
            return (double)i == d ? std::to_string(i) : javaDoubleToString(d);
        }
        default: {
            char buf[40];
            snprintf(buf, sizeof(buf), "%s: %08x", luaL_typename(L, idx), (unsigned)(uintptr_t)lua_topointer(L, idx));
            return buf;
        }
    }
}

std::string normaliseLabel(const std::string& label) {
    std::string result = label.substr(0, 32);
    for (char& c : result) {
        const unsigned char u = (unsigned char)c;
        if (!((u >= ' ' && u <= '~') || (u >= 161 && u != 167))) c = '?';
    }
    return result;
}

bool matchIPClass(const std::string& address, const std::string& pattern) {
    static const std::regex regex_escape("[\\^\\$\\\\\\.\\+\\?\\(\\)\\[\\]\\{\\}\\|]");
    static const std::regex regex_wildcard("\\*");
    const std::regex patreg(std::regex_replace(std::regex_replace(pattern, regex_escape, "\\$&"), regex_wildcard, ".*"), std::regex::icase);
    if ((pattern == "$private" && address == "localhost") || std::regex_match(address, patreg)) return true;
    IPBytes ip;
    if (!parseIPLiteral(address, ip)) return false; // a host name: only the wildcard match above applies
    if (pattern == "$private") {
        for (const auto& range : privateRanges()) if (inNetwork(ip, range.first, range.second)) return true;
        return false;
    }
    // "<address>/<bits>": a network of IPv4 or IPv6 addresses
    const size_t slash = pattern.find('/');
    if (slash == std::string::npos) return false;
    IPBytes network;
    if (!parseIPLiteral(pattern.substr(0, slash), network)) return false;
    char * end = NULL;
    const long bits = strtol(pattern.c_str() + slash + 1, &end, 10);
    if (end == pattern.c_str() + slash + 1 || *end != 0 || bits < 0) return false;
    const bool networkIsV4 = inNetwork(network, ipv4Bytes(0, 0, 0, 0), 96);
    if (bits > (networkIsV4 ? 32 : 128)) return false;
    return inNetwork(ip, network, (int)bits + (networkIsV4 ? 96 : 0));
}


/* ---- Crash logs ---- */

static path_t::value_type crashLogDirNative[2048] = {0};
static path_t::value_type crashLogFileNative[2200] = {0};

void initCrashLog() {
    try {
        const path_t dir = getBasePath() / "crash-logs";
        std::error_code ec;
        fs::create_directories(dir, ec);
        const path_t::string_type& native = dir.native();
        if (native.size() < sizeof(crashLogDirNative) / sizeof(crashLogDirNative[0]) - 1) {
            std::copy(native.begin(), native.end(), crashLogDirNative);
            crashLogDirNative[native.size()] = 0;
        }
    } catch (...) {}
}

size_t beginCrashReport(char * buf, size_t size, const char * platform, const char * reason) {
    char when[32] = "unknown";
    const time_t now = time(NULL);
    struct tm tmv;
#ifdef _WIN32
    if (gmtime_s(&tmv, &now) == 0)
#else
    if (gmtime_r(&now, &tmv) != NULL)
#endif
        strftime(when, sizeof(when), "%Y-%m-%d %H:%M:%S", &tmv);
    int n = snprintf(buf, size,
        "CraftOS-Tweaked crash log\n"
        "=========================\n"
        "This file was saved on your computer when CraftOS-Tweaked crashed. Nothing has been sent anywhere.\n"
        "It may contain file paths (which can include your user name), so look it over before you share it.\n"
        "Report problems at " CRAFTOSTWEAKED_BUGREPORT_URL "\n\n"
        "Version: " CRAFTOSPC_VERSION
#if defined(CRAFTOSPC_COMMIT)
        " (commit " CRAFTOSPC_COMMIT ")"
#endif
        "\nCC: Tweaked version: %s (%s)\n"
        "Platform: %s\n"
        "Time (UTC): %s\n"
        "Reason: %s\n",
        ccVersionString().c_str(), activeROMVersion().id.empty() ? "custom ROM" : activeROMVersion().id.c_str(), platform, when, reason);
    if (n < 0) return 0;
    if ((size_t)n >= size) return size - 1;
    return (size_t)n;
}

const path_t::value_type * newCrashLogPath() {
    if (crashLogDirNative[0] == 0) return NULL;
    char name[48];
    const time_t now = time(NULL);
    struct tm tmv;
#ifdef _WIN32
    if (gmtime_s(&tmv, &now) != 0) return NULL;
#else
    if (gmtime_r(&now, &tmv) == NULL) return NULL;
#endif
    strftime(name, sizeof(name), "crash-%Y%m%d-%H%M%S.log", &tmv);
    size_t i = 0;
    while (crashLogDirNative[i] != 0 && i < 2047) { crashLogFileNative[i] = crashLogDirNative[i]; i++; }
    crashLogFileNative[i++] = (path_t::value_type)fs::path::preferred_separator;
    for (const char * c = name; *c != 0; c++) crashLogFileNative[i++] = (path_t::value_type)*c;
    crashLogFileNative[i] = 0;
    return crashLogFileNative;
}

const path_t::value_type * lastCrashLogPath() {
    return crashLogFileNative[0] != 0 ? crashLogFileNative : NULL;
}

size_t crashReportf(char * buf, size_t size, size_t used, const char * fmt, ...) {
    if (used >= size) return used;
    va_list args;
    va_start(args, fmt);
    const int n = vsnprintf(buf + used, size - used, fmt, args);
    va_end(args);
    if (n < 0) return used;
    return (size_t)n >= size - used ? size - 1 : used + (size_t)n;
}

#ifndef _WIN32
int openCrashLog() {
    const path_t::value_type * path = newCrashLogPath();
    if (path == NULL) return -1;
    const int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) crashLogFileNative[0] = 0; // nothing was saved
    return fd;
}
#endif

static bool isCrashLog(const path_t& p) {
    const std::string name = p.filename().string();
    return name.size() > 10 && name.compare(0, 6, "crash-") == 0 && name.compare(name.size() - 4, 4, ".log") == 0 &&
        (name.size() < 9 || name.compare(name.size() - 9, 9, ".seen.log") != 0);
}

std::vector<path_t> pendingCrashLogs() {
    std::vector<path_t> retval;
    try {
        std::error_code ec;
        const path_t dir = getBasePath() / "crash-logs";
        for (fs::directory_iterator it(dir, ec), end; !ec && it != end; it.increment(ec))
            if (it->is_regular_file(ec) && isCrashLog(it->path())) retval.push_back(it->path());
        std::sort(retval.begin(), retval.end());
    } catch (...) {}
    return retval;
}

void markCrashLogsSeen(const std::vector<path_t>& logs) {
    for (const path_t& log : logs) {
        std::error_code ec;
        path_t seen = log;
        seen.replace_extension(".seen.log");
        fs::rename(log, seen, ec);
    }
}

static std::string urlEncode(const std::string& str) {
    static const char hex[] = "0123456789ABCDEF";
    std::string out;
    for (unsigned char c : str) {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '.' || c == '_' || c == '~') out += (char)c;
        else {out += '%'; out += hex[c >> 4]; out += hex[c & 15];}
    }
    return out;
}

std::string crashReportIssueURL(const path_t& log) {
    std::string version, platform, reason, details;
    {
        std::ifstream in(log, std::ios::binary);
        std::string line;
        bool inDetails = false;
        while (std::getline(in, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (inDetails) {
                if (details.size() < 1800) details += line + "\n";
            } else if (line == "Details:") inDetails = true;
            else if (line.compare(0, 9, "Version: ") == 0) version = line.substr(9);
            else if (line.compare(0, 10, "Platform: ") == 0) platform = line.substr(10);
            else if (line.compare(0, 8, "Reason: ") == 0) reason = line.substr(8);
        }
    }
    // GitHub limits URL length, so only the start of the details are included. The full log is attached by hand.
    return std::string(CRAFTOSTWEAKED_BUGREPORT_URL "/new?template=crash_report.yml") +
        "&title=" + urlEncode("Crash: " + reason) +
        "&version=" + urlEncode(version) +
        "&platform=" + urlEncode(platform) +
        "&crash_details=" + urlEncode("Reason: " + reason + "\n" + details);
}

std::string crashLogFolderURL() {
    std::string path = (getBasePath() / "crash-logs").generic_u8string();
    std::string out = "file://";
    if (!path.empty() && path[0] != '/') out += '/'; // Windows drive letters
    for (unsigned char c : path) {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '.' || c == '_' || c == '~' || c == '/' || c == ':') out += (char)c;
        else {static const char hex[] = "0123456789ABCDEF"; out += '%'; out += hex[c >> 4]; out += hex[c & 15];}
    }
    return out;
}


/* ---- CC: Tweaked version selection ---- */

static ROMVersion currentROMVersion;

std::vector<path_t> romSearchDirectories() {
    std::vector<path_t> dirs;
    if (const char * env = getenv("CRAFTOS_TWEAKED_ROMS")) if (*env) dirs.push_back(env);
#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__) && !defined(__IPHONEOS__)
    if (char * base = SDL_GetBasePath()) { // the folder that holds the executable
        dirs.push_back(path_t(base) / "roms");
        SDL_free(base);
    }
#endif
    dirs.push_back(getBasePath() / "roms");
    dirs.push_back(getROMPath() / "roms");
    dirs.push_back(getROMPath()); // --rom may name a folder of ROMs
#ifdef __linux__
    dirs.push_back("/usr/local/share/craftos-tweaked/roms");
    dirs.push_back("/usr/share/craftos-tweaked/roms");
#endif
    std::vector<path_t> unique;
    for (const path_t& d : dirs) if (std::find(unique.begin(), unique.end(), d) == unique.end()) unique.push_back(d);
    return unique;
}

static void readROMInfo(std::istream& in, ROMVersion& v) {
    try {
        Poco::JSON::Parser parser;
        Poco::JSON::Object::Ptr info = parser.parse(in).extract<Poco::JSON::Object::Ptr>();
        if (info->has("computercraft_version")) v.ccVersion = info->getValue<std::string>("computercraft_version");
        if (info->has("minecraft_version")) v.minecraftVersion = info->getValue<std::string>("minecraft_version");
        if (info->has("upstream_branch")) v.upstreamBranch = info->getValue<std::string>("upstream_branch");
        if (info->has("upstream_commit")) v.upstreamCommit = info->getValue<std::string>("upstream_commit");
        // A ROM that came from CC: Tweaked uses GLFW key codes; "key_codes" can say otherwise ("lwjgl" or "glfw")
        v.ccTweaked = info->has("computercraft_version");
        v.glfwKeys = v.ccTweaked;
        if (info->has("key_codes")) v.glfwKeys = info->getValue<std::string>("key_codes") == "glfw";
    } catch (...) {}
}

static ROMVersion readROMVersion(const path_t& dir) {
    ROMVersion v;
    v.id = dir.filename().string();
    v.path = dir;
    std::ifstream in(dir / "rom-info.json");
    if (in.is_open()) readROMInfo(in, v);
    return v;
}

void useEmbeddedROMInfo(const std::string& json) {
    ROMVersion v;
    v.id = "embedded";
    std::istringstream in(json);
    if (!json.empty()) readROMInfo(in, v);
    currentROMVersion = v;
}

std::vector<ROMVersion> findROMVersions() {
    std::vector<ROMVersion> versions;
    for (const path_t& root : romSearchDirectories()) {
        std::error_code ec;
        for (fs::directory_iterator it(root, ec), end; !ec && it != end; it.increment(ec)) {
            if (!it->is_directory(ec) || !fs::exists(it->path() / "bios.lua", ec)) continue;
            const std::string id = it->path().filename().string();
            // the first folder found wins when the same version is installed in several places
            if (std::none_of(versions.begin(), versions.end(), [&id](const ROMVersion& v) {return v.id == id;}))
                versions.push_back(readROMVersion(it->path()));
        }
    }
    std::sort(versions.begin(), versions.end(), [](const ROMVersion& a, const ROMVersion& b) {return a.id < b.id;});
    return versions;
}

bool selectROMVersion(const std::string& id, std::string& error) {
    // the search directories were not checked in order by findROMVersions, so look for the requested id in order
    for (const path_t& root : romSearchDirectories()) {
        std::error_code ec;
        if (fs::exists(root / id / "bios.lua", ec)) {
            currentROMVersion = readROMVersion(root / id);
            setROMPath(root / id);
            return true;
        }
    }
    error = "Could not find the CC: Tweaked ROM \"" + id + "\". Looked in:";
    for (const path_t& root : romSearchDirectories()) error += "\n  " + root.string();
    const std::vector<ROMVersion> available = findROMVersions();
    if (available.empty()) error += "\nNo ROMs are installed. Put the \"roms\" folder from a CraftOS-Tweaked release next to the executable.";
    else {
        error += "\nInstalled versions:";
        for (const ROMVersion& v : available) error += " " + v.id;
    }
    return false;
}

void useROMFolder(const path_t& dir) {
    currentROMVersion = readROMVersion(dir);
    setROMPath(dir);
}

const ROMVersion& activeROMVersion() {
    return currentROMVersion;
}

const std::string& ccVersionString() {
    static const std::string fallback = CRAFTOSPC_CC_VERSION;
    return currentROMVersion.ccVersion.empty() ? fallback : currentROMVersion.ccVersion;
}


/* ---- Passing values between computers' Lua states, like CC: Tweaked does ---- */

static void ccCloneValue(lua_State *from, int idx, lua_State *to, int seen, int depth) {
    idx = lua_absindex(from, idx);
    if (depth > 200 || !lua_checkstack(to, 6) || !lua_checkstack(from, 6)) {lua_pushnil(to); return;}
    switch (lua_type(from, idx)) {
        case LUA_TBOOLEAN: lua_pushboolean(to, lua_toboolean(from, idx)); break;
        case LUA_TNUMBER: lua_pushnumber(to, lua_tonumber(from, idx)); break;
        case LUA_TSTRING: {
            size_t len = 0;
            const char * str = lua_tolstring(from, idx, &len);
            lua_pushlstring(to, str, len);
            break;
        } case LUA_TTABLE: {
            const void * ptr = lua_topointer(from, idx);
            lua_rawgetp(to, seen, ptr);
            if (!lua_isnil(to, -1)) break; // already copied: keep the reference
            lua_pop(to, 1);
            lua_newtable(to);
            lua_pushvalue(to, -1);
            lua_rawsetp(to, seen, ptr);
            lua_pushnil(from);
            while (lua_next(from, idx) != 0) {
                ccCloneValue(from, -2, to, seen, depth + 1);
                ccCloneValue(from, -1, to, seen, depth + 1);
                if (lua_isnil(to, -2) || lua_isnil(to, -1)) lua_pop(to, 2); // entries that are not representable are dropped
                else lua_rawset(to, -3);
                lua_pop(from, 1);
            }
            break;
        } default: lua_pushnil(to);
    }
}

void ccCloneValues(lua_State *from, lua_State *to, int n) {
    const int base = lua_gettop(from) - n + 1;
    for (int i = 0; i < n; i++) {
        lua_newtable(to); // identity map: each argument gets its own
        const int seen = lua_gettop(to);
        ccCloneValue(from, base + i, to, seen, 0);
        lua_remove(to, seen);
    }
}
