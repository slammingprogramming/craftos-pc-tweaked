/*
 * platform/win.cpp
 * CraftOS-Tweaked
 * 
 * This file implements functions specific to Windows.
 * 
 * This code is licensed under the GNU AGPL v3.0 or later (AGPL-3.0-or-later).
 * Copyright (c) 2019-2024 JackMacWindows.
 * Copyright (c) 2026 slammingprogramming.
 * Originally released under the MIT License; see the LICENSE file.
 */

#ifdef _WIN32
#include <Windows.h>
#include <vector>
#include <string>
#include <fstream>
#include <cstring>
#include <codecvt>
#include <Poco/SHA2Engine.h>
#include <Poco/URI.h>
#include <Poco/Version.h>
#include <Poco/Crypto/X509Certificate.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/SSLException.h>
#include <processenv.h>
#include <Shlwapi.h>
#include <wincrypt.h>
#include <aclapi.h>
#include <sddl.h>
#include <commctrl.h>
#include <dirent.h>
#include <SDL2/SDL_syswm.h>
#include <sys/stat.h>
#include "../platform.hpp"
#include "../util.hpp"

// The per-user data folder. src/location.cpp decides whether it is used (or a folder next to the program, or a -d folder);
// a CraftOS-PC 2 folder under the old name is found and offered there.
const wchar_t * base_path = L"%appdata%\\CraftOS-Tweaked";
path_t base_path_expanded;
path_t rom_path_expanded;
wchar_t expand_tmp[32768];

void setBasePath(path_t path) {
    base_path_expanded = path;
}

void setROMPath(path_t path) {
    rom_path_expanded = path;
}

path_t getBasePath() {
    if (!base_path_expanded.empty()) return base_path_expanded;
    ExpandEnvironmentStringsW(base_path, expand_tmp, 32768);
    base_path_expanded = path_t(expand_tmp, expand_tmp + wcsnlen(expand_tmp, 32768));
    return base_path_expanded;
}

path_t getROMPath() {
    if (!rom_path_expanded.empty()) return rom_path_expanded;
    GetModuleFileNameW(NULL, expand_tmp, 32768);
    rom_path_expanded = path_t(expand_tmp, expand_tmp + wcsnlen(expand_tmp, 32768)).parent_path();
    return rom_path_expanded;
}

path_t getPlugInPath() { return getROMPath() / "plugins"; }

path_t getMCSavePath() {
    ExpandEnvironmentStringsW(L"%appdata%\\.minecraft\\saves\\", expand_tmp, 32768);
    return path_t(expand_tmp);
}

void* kernel32handle = NULL;
HRESULT(*_SetThreadDescription)(HANDLE, PCWSTR) = NULL;

void setThreadName(std::thread &t, const std::string& name) {
    if (kernel32handle == NULL) {
        kernel32handle = SDL_LoadObject("kernel32");
        _SetThreadDescription = (HRESULT(*)(HANDLE, PCWSTR))SDL_LoadFunction(kernel32handle, "SetThreadDescription");
    }
    if (_SetThreadDescription != NULL) _SetThreadDescription((HANDLE)t.native_handle(), std::wstring(name.begin(), name.end()).c_str());
}

static std::string makeSize(double n) {
    if (n >= 100) return std::to_string((long)floor(n));
    else return std::to_string(n).substr(0, 4);
}

void updateNow(const std::string& tagname, const Poco::JSON::Object::Ptr root) {
    // If a delta update in the form "CraftOS-Tweaked-Setup_Delta-v2.x.y.exe" is available, use that instead of the full installer
    // "v2.x.y" indicates the oldest version that can update from this installer
    // NOTE: only used when built with CRAFTOSTWEAKED_AUTOUPDATE (see main.cpp); releases don't ship installers yet.
    const std::string deltaPrefix = "CraftOS-Tweaked-Setup_Delta-v";
    std::string assetName = "CraftOS-Tweaked-Setup.exe";
    Poco::JSON::Array::Ptr assets = root->getArray("assets");
    for (auto it = assets->begin(); it != assets->end(); it++) {
        Poco::JSON::Object::Ptr obj = it->extract<Poco::JSON::Object::Ptr>();
        std::string name = obj->getValue<std::string>("name");
        if (name.compare(0, deltaPrefix.size(), deltaPrefix) == 0) {
            std::string tag = name.substr(deltaPrefix.size() - 1, name.size() - deltaPrefix.size() - 3); // "v2.x.y", without ".exe"
            if (strcmp(tag.c_str(), CRAFTOSPC_VERSION) <= 0) assetName = name;
            break;
        }
    }
    HTTPDownload("https://github.com/" CRAFTOSTWEAKED_REPO "/releases/download/" + tagname + "/sha256-hashes.txt", [tagname, &assetName](std::istream * shain, Poco::Exception * e, Poco::Net::HTTPResponse * res){
        if (e != NULL) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Update Error", std::string("An error occurred while downloading the update: " + e->displayText()).c_str(), NULL);
            return;
        }
        std::string line;
        bool found = false;
        while (!shain->eof()) {
            std::getline(*shain, line);
            if (line.find(assetName) != std::string::npos) {found = true; break;}
        }
        if (!found) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Update Error", "A file required for verification could not be downloaded sucessfully. Please download the installer manually.", NULL);
            return;
        }
        std::string hash = line.substr(0, 64);
        HTTPDownload("https://github.com/" CRAFTOSTWEAKED_REPO "/releases/download/" + tagname + "/" + assetName, [&hash](std::istream * in, Poco::Exception * e, Poco::Net::HTTPResponse * res) {
            if (e != NULL) {
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Update Error", std::string("An error occurred while downloading the update: " + e->displayText()).c_str(), NULL);
                return;
            }

            size_t totalSize = res->getContentLength64();
            SDL_Window * win = SDL_CreateWindow("Downloading...", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 300, 50, SDL_WINDOW_UTILITY);
            SDL_FillRect(SDL_GetWindowSurface(win), NULL, 0xeeeeee);
            SDL_UpdateWindowSurface(win);
            SDL_SysWMinfo info;
            SDL_VERSION(&info.version);
            SDL_GetWindowWMInfo(win, &info);
            InitCommonControls();
            HWND hwndPB = CreateWindowEx(0, PROGRESS_CLASS, (LPTSTR) NULL, 
                                    WS_CHILD | WS_VISIBLE,
                                    5, 25, 290, 20,
                                    info.info.win.window, (HMENU) 0, info.info.win.hinstance, NULL);
            SendMessage(hwndPB, PBM_SETRANGE, 0, MAKELPARAM(0, 10000));
            SendMessage(hwndPB, PBM_SETSTEP, (WPARAM) 1, 0); 
            HWND hwndLabel = CreateWindow("static", "ST_U",
                              WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                              5, 3, 290, 20,
                              info.info.win.window, (HMENU)(501),
                              info.info.win.hinstance, NULL);
            std::string label = "0.0 / " + makeSize(totalSize / 1048576.0) + " MB, 0 B/s";
            SetWindowText(hwndLabel, label.c_str());
            HFONT hFont = CreateFont(
		            18, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE, ANSI_CHARSET, 
		            OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
		            DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            SendMessage(hwndLabel, WM_SETFONT, (WPARAM)hFont, TRUE);

            std::string data;
            data.reserve(totalSize);
            char buf[2048];
            size_t total = 0;
            size_t bps = 0;
            size_t lastSecondSize = 0;
            std::chrono::system_clock::time_point lastSecond = std::chrono::system_clock::now();
            while (in->good() && !in->eof()) {
                in->read(buf, 2048);
                size_t sz = in->gcount();
                data += std::string(buf, sz);
                total += sz;
                if (std::chrono::system_clock::now() - lastSecond >= std::chrono::milliseconds(50) || in->eof()) {
                    bps = (total - lastSecondSize) * 20;
                    lastSecondSize = total;
                    lastSecond = std::chrono::system_clock::now();
                    label = makeSize(total / 1048576.0) + " / " + makeSize(totalSize / 1048576.0) + " MB, ";
                    if (bps >= 1048576) label += makeSize(bps / 1048576.0) + " MB/s";
                    else if (bps >= 1024) label += makeSize(bps / 1024.0) + " kB/s";
                    else label += std::to_string(bps) + " B/s";
                    SetWindowText(hwndLabel, label.c_str());
                    SendMessage(hwndPB, PBM_SETPOS, (WPARAM)((double)total / (double)totalSize * 10000) + 1, 0);
                    SendMessage(hwndPB, PBM_SETPOS, (WPARAM)((double)total / (double)totalSize * 10000), 0);
                    SDL_PumpEvents();
                }
            }
            SendMessage(hwndPB, PBM_SETPOS, (WPARAM)((double)total / (double)totalSize * 10000) - 1, 0);
            SendMessage(hwndPB, PBM_SETPOS, (WPARAM)((double)total / (double)totalSize * 10000), 0);
            SDL_PumpEvents();

            Poco::SHA2Engine engine;
            engine.update(data);
            std::string myhash = Poco::SHA2Engine::digestToHex(engine.digest());
            if (hash != myhash) {
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Update Error", "The installer file could not be verified. Please try again later. If this issue persists, please download the installer manually.", NULL);
                return;
            }
            char str[261];
            GetTempPathA(261, str);
            const std::string path = std::string(str) + "\\setup.exe";
            std::ofstream out(path, std::ios::binary);
            out << data;
            out.close();
            DestroyWindow(hwndPB);
            DestroyWindow(hwndLabel);
            SDL_DestroyWindow(win);

            STARTUPINFOA sinfo;
            memset(&sinfo, 0, sizeof(sinfo));
            sinfo.cb = sizeof(sinfo);
            PROCESS_INFORMATION process;
            CreateProcessA(path.c_str(), (char*)(path + " /SILENT").c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &sinfo, &process);
            CloseHandle(process.hProcess);
            CloseHandle(process.hThread);
            exit(0);
        });
    });
}

std::vector<std::wstring> failedCopy;

static int recursiveMove(const std::wstring& path, const std::wstring& toPath) {
    const DWORD attr = GetFileAttributesW(path.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES) return GetLastError();
    if (attr & FILE_ATTRIBUTE_DIRECTORY) {
        if (CreateDirectoryExW(toPath.substr(0, toPath.find_last_of('\\', toPath.size() - 2)).c_str(), toPath.c_str(), NULL) == 0) return GetLastError();
        WIN32_FIND_DATAW find;
        std::wstring s = path;
        if (path[path.size() - 1] != '\\') s += L"\\";
        s += L"*";
        const HANDLE h = FindFirstFileW(s.c_str(), &find);
        if (h != INVALID_HANDLE_VALUE) {
            do {
                if (!(find.cFileName[0] == '.' && (wcslen(find.cFileName) == 1 || (find.cFileName[1] == '.' && wcslen(find.cFileName) == 2)))) {
                    std::wstring newpath = path;
                    if (path[path.size() - 1] != '\\') newpath += L"\\";
                    newpath += find.cFileName;
                    const int res = recursiveMove(newpath, toPath + L"\\" + std::wstring(find.cFileName));
                    if (res) failedCopy.push_back(toPath + L"\\" + std::wstring(find.cFileName));
                }
            } while (FindNextFileW(h, &find));
            FindClose(h);
        }
        return RemoveDirectoryW(path.c_str()) ? 0 : (int)GetLastError();
    } else return MoveFileW(path.c_str(), toPath.c_str()) ? 0 : (int)GetLastError();
}

void migrateOldData() {
    ExpandEnvironmentStringsW(L"%USERPROFILE%\\.craftos", expand_tmp, 32767);
    const std::wstring oldpath = expand_tmp;
    struct _stat st;
    if (_wstat(oldpath.c_str(), &st) == 0 && S_ISDIR(st.st_mode) && _wstat(getBasePath().c_str(), &st) != 0)
        recursiveMove(oldpath, getBasePath());
    if (!failedCopy.empty())
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Migration Failure", "Some files were unable to be moved while migrating the user data directory. These files have been left in place, and they will not appear inside the computer. You can copy them over from the old directory manually.", NULL);
}

void copyImage(SDL_Surface* surf, SDL_Window* win) {
    char * bmp = new char[surf->w*surf->h*surf->format->BytesPerPixel + 128];
    SDL_RWops * rw = SDL_RWFromMem(bmp, surf->w*surf->h*surf->format->BytesPerPixel + 128);
    SDL_SaveBMP_RW(surf, rw, false);
    const HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, rw->seek(rw, 0, RW_SEEK_CUR) - sizeof(BITMAPFILEHEADER));
    if (hMem == NULL) { delete[] bmp; return; }
    memcpy(GlobalLock(hMem), bmp + sizeof(BITMAPFILEHEADER), rw->seek(rw, 0, RW_SEEK_CUR) - sizeof(BITMAPFILEHEADER));
    GlobalUnlock(hMem);
    OpenClipboard(0);
    EmptyClipboard();
    SetClipboardData(CF_DIB, hMem);
    CloseClipboard();
    delete[] bmp;
}

static const char * exceptionName(DWORD code) {
    switch (code) {
    case EXCEPTION_ACCESS_VIOLATION: return "Access violation";
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return "Array bounds exceeded";
    case EXCEPTION_DATATYPE_MISALIGNMENT: return "Datatype misalignment";
    case EXCEPTION_FLT_DIVIDE_BY_ZERO: return "Floating-point divide by zero";
    case EXCEPTION_ILLEGAL_INSTRUCTION: return "Illegal instruction";
    case EXCEPTION_IN_PAGE_ERROR: return "In-page error";
    case EXCEPTION_INT_DIVIDE_BY_ZERO: return "Integer divide by zero";
    case EXCEPTION_PRIV_INSTRUCTION: return "Privileged instruction";
    case EXCEPTION_STACK_OVERFLOW: return "Stack overflow";
    default: return "Unhandled exception";
    }
}

#if defined(_M_ARM64)
#define CRASH_PLATFORM "Windows (ARM64)"
#else
#define CRASH_PLATFORM "Windows (x64)"
#endif

// Saves a crash log and tells the user where it is. Nothing is uploaded; on the next start CraftOS-Tweaked offers to
// open a pre-filled GitHub issue (see offerPendingCrashReport in main.cpp).
LONG WINAPI exceptionHandler(PEXCEPTION_POINTERS pExceptionInfo) {
    static volatile LONG handling = 0;
    if (InterlockedCompareExchange(&handling, 1, 0) != 0) return EXCEPTION_CONTINUE_SEARCH; // already crashing
    static char report[16384];
    char reason[160];
    const DWORD code = pExceptionInfo->ExceptionRecord->ExceptionCode;
    snprintf(reason, sizeof(reason), "%s (0x%08lX) at address %p", exceptionName(code), (unsigned long)code, pExceptionInfo->ExceptionRecord->ExceptionAddress);
    size_t n = beginCrashReport(report, sizeof(report), CRASH_PLATFORM, reason);
    n = crashReportf(report, sizeof(report), n, "Details:\nLast C function: %s\n", lastCFunction);
    if (!loadingPlugin.empty()) n = crashReportf(report, sizeof(report), n, "Plugin being loaded: %s\n", loadingPlugin.c_str());
    n = crashReportf(report, sizeof(report), n, "Backtrace (module+offset; resolve with the matching .pdb symbols):\n");
    void * frames[48];
    const USHORT count = CaptureStackBackTrace(0, 48, frames, NULL);
    for (USHORT i = 0; i < count + 1; i++) {
        void * addr = i == 0 ? pExceptionInfo->ExceptionRecord->ExceptionAddress : frames[i - 1];
        HMODULE mod = NULL;
        char modName[MAX_PATH] = "?";
        if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)addr, &mod) && GetModuleFileNameA(mod, modName, MAX_PATH) > 0) {
            const char * base = strrchr(modName, '\\');
            n = crashReportf(report, sizeof(report), n, "[bt]: (%u) %s+0x%llX\n", (unsigned)i, base ? base + 1 : modName, (unsigned long long)((char*)addr - (char*)mod));
        } else n = crashReportf(report, sizeof(report), n, "[bt]: (%u) %p\n", (unsigned)i, addr);
    }
    bool saved = false;
    const wchar_t * logPath = newCrashLogPath();
    if (logPath != NULL) {
        const HANDLE h = CreateFileW(logPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h != INVALID_HANDLE_VALUE) {
            DWORD written = 0;
            WriteFile(h, report, (DWORD)n, &written, NULL);
            CloseHandle(h);
            saved = true;
        }
    }
    std::wstring msg = L"Uh oh, CraftOS-Tweaked has crashed!";
    if (!loadingPlugin.empty()) msg += L" It appears the plugin \"" + std::wstring(loadingPlugin.begin(), loadingPlugin.end()) + L"\" may have been responsible for this. Please remove it and try again.";
    if (saved) msg += std::wstring(L"\n\nA crash log was saved to:\n") + logPath + L"\n\nNothing has been uploaded. The next time you start CraftOS-Tweaked you can choose to open a pre-filled bug report on GitHub. You can also report it at " L"" CRAFTOSTWEAKED_BUGREPORT_URL L" and attach the log.";
    else msg += L"\n\nThe crash log could not be saved. Please report this at " L"" CRAFTOSTWEAKED_BUGREPORT_URL L".";
    MessageBoxW(NULL, msg.c_str(), L"Application Error", MB_OK | MB_ICONERROR);
    return EXCEPTION_CONTINUE_SEARCH;
}

// Do nothing. We definitely don't want to crash when there's only an invalid parameter, and I assume functions affected will return some value that won't cause problems. (I know strftime, used in os.date, will be fine.)
void invalidParameterHandler(const wchar_t * expression, const wchar_t * function, const wchar_t * file, unsigned int line, uintptr_t pReserved) {}

void setupCrashHandler() {
    initCrashLog();
    SetUnhandledExceptionFilter(exceptionHandler);
    _set_invalid_parameter_handler(invalidParameterHandler);
}

void setFloating(SDL_Window* win, bool state) {
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(win, &info);
    if (info.subsystem != SDL_SYSWM_WINDOWS) return; // should always be true
    SetWindowPos(info.info.win.window, state ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void platformExit() {
    if (kernel32handle != NULL) SDL_UnloadObject(kernel32handle);
}

void addSystemCertificates(Poco::Net::Context::Ptr context) {
    HCERTSTORE store = CertOpenSystemStore(NULL, "ROOT");
    if (store == NULL) return;
    for (PCCERT_CONTEXT c = CertEnumCertificatesInStore(store, NULL); c != NULL; c = CertEnumCertificatesInStore(store, c)) {
        X509 * cert = d2i_X509(NULL, (const unsigned char**)&c->pbCertEncoded, c->cbCertEncoded);
        context->addCertificateAuthority(Poco::Crypto::X509Certificate(cert));
    }
    CertCloseStore(store, 0);
}

void unblockInput() {
    DWORD tmp;
    INPUT_RECORD ir[2];
    ir[0].EventType = KEY_EVENT;
    ir[0].Event.KeyEvent.bKeyDown = TRUE;
    ir[0].Event.KeyEvent.dwControlKeyState = 0;
    ir[0].Event.KeyEvent.uChar.UnicodeChar = VK_RETURN;
    ir[0].Event.KeyEvent.wRepeatCount = 1;
    ir[0].Event.KeyEvent.wVirtualKeyCode = VK_RETURN;
    ir[0].Event.KeyEvent.wVirtualScanCode = MapVirtualKey(VK_RETURN, MAPVK_VK_TO_VSC);
    ir[1] = ir[0];
    ir[1].Event.KeyEvent.bKeyDown = FALSE;
    WriteConsoleInput(GetStdHandle(STD_INPUT_HANDLE), ir, 2, &tmp);
}

// This function was partially generated by ChatGPT. It has been edited and
// verified for correctness and security.
bool winFolderIsReadOnly(path_t path) {
    DWORD desiredAccess = FILE_GENERIC_WRITE;
    DWORD grantedAccess = 0;
    BOOL accessStatus = FALSE;
    HANDLE tokenHandle = nullptr;

    // Get current process token
    if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &tokenHandle)) {
        if (GetLastError() == ERROR_NO_TOKEN) {
            if (!ImpersonateSelf(SecurityImpersonation))
                return true;
            if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &tokenHandle))
                return true;
        } else return true;
    }

    PSECURITY_DESCRIPTOR securityDesc = nullptr;
    PACL dacl = nullptr;
    PSID owner = nullptr, group = nullptr;

    DWORD result = GetNamedSecurityInfoW(path.wstring().c_str(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION, &owner, &group, &dacl, nullptr, &securityDesc);
    if (result != ERROR_SUCCESS) {
        CloseHandle(tokenHandle);
        return true;
    }

    PRIVILEGE_SET privileges = {};
    DWORD privilegeSetLength = sizeof(privileges);
    GENERIC_MAPPING mapping = { FILE_GENERIC_READ, FILE_GENERIC_WRITE, FILE_GENERIC_EXECUTE, FILE_ALL_ACCESS };

    MapGenericMask(&desiredAccess, &mapping);

    if (!AccessCheck(securityDesc, tokenHandle, desiredAccess, &mapping, &privileges, &privilegeSetLength, &grantedAccess, &accessStatus)) {
        accessStatus = FALSE;
    }

    if (securityDesc) LocalFree(securityDesc);
    CloseHandle(tokenHandle);

    return accessStatus == FALSE;
}

#endif