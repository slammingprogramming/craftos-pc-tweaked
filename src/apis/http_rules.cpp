/*
 * apis/http_rules.cpp
 * CraftOS-Tweaked
 *
 * This file implements CC: Tweaked's HTTP address rules, following dan200.computercraft.core.apis.http.options.
 *
 * This file is part of CraftOS-Tweaked, a fork of CraftOS-PC 2 by JackMacWindows.
 * Copyright (c) 2026 slammingprogramming. Licensed under the GNU Affero General Public License, version 3 or later.
 */

#include "http_rules.hpp"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <Poco/ByteOrder.h>
#include <Poco/Exception.h>
#include <Poco/Net/DNS.h>
#include <Poco/Net/IPAddress.h>
#include <configuration.hpp>
#include "../util.hpp"

bool httpRulesExplicit = false;

static const long long DEFAULT_MAX_UPLOAD = 4LL * 1024 * 1024;
static const long long DEFAULT_MAX_DOWNLOAD = 16LL * 1024 * 1024;
static const int DEFAULT_WEBSOCKET_MESSAGE = 128 * 1024;

static std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {return (char)tolower(c);});
    return s;
}

bool parseHTTPRule(const std::string& text, HTTPRule& rule) {
    std::istringstream in(text);
    std::string token;
    rule = HTTPRule();
    bool first = true;
    while (in >> token) {
        const std::string lowered = lower(token);
        if (first && (lowered == "allow" || lowered == "deny")) rule.allow = lowered == "allow";
        else if (rule.host.empty() && token.find('=') == std::string::npos) rule.host = token;
        else {
            const size_t eq = token.find('=');
            if (eq == std::string::npos) return false;
            const std::string key = lower(token.substr(0, eq)), value = token.substr(eq + 1);
            char * end = NULL;
            if (key == "use_proxy") {
                if (lower(value) == "true") rule.useProxy = 1;
                else if (lower(value) == "false") rule.useProxy = 0;
                else return false;
            } else {
                const long long n = strtoll(value.c_str(), &end, 10);
                if (value.empty() || *end != 0 || n < 0) return false;
                if (key == "port") rule.port = (int)n;
                else if (key == "max_upload") rule.maxUpload = n;
                else if (key == "max_download") rule.maxDownload = n;
                else if (key == "websocket_message") rule.websocketMessage = (int)n;
                else return false;
            }
        }
        first = false;
    }
    return !rule.host.empty();
}

std::vector<std::string> defaultHTTPRules(bool ccTweaked) {
    // CC: Tweaked's own defaults (private addresses are blocked); other ROMs keep CraftOS-PC's open defaults
    if (ccTweaked) return {"deny $private", "allow *"};
    return {"allow *"};
}

std::vector<std::string> effectiveHTTPRuleTexts() {
    std::vector<std::string> texts;
    const bool legacyLists = config.http_whitelist != std::vector<std::string>{"*"} || !config.http_blacklist.empty();
    if (httpRulesExplicit) texts = config.http_rules;
    else if (legacyLists) {
        // the older http_blacklist and http_whitelist settings: blocked addresses first, then the allowed ones
        for (const std::string& b : config.http_blacklist) texts.push_back("deny " + b);
        for (const std::string& w : config.http_whitelist) texts.push_back("allow " + w);
    } else texts = defaultHTTPRules(activeROMVersion().ccTweaked);
    return texts;
}

std::vector<HTTPRule> effectiveHTTPRules() {
    const std::vector<std::string> texts = effectiveHTTPRuleTexts();
    std::vector<HTTPRule> rules;
    for (const std::string& text : texts) {
        HTTPRule rule;
        if (parseHTTPRule(text, rule)) rules.push_back(rule);
        else fprintf(stderr, "Ignoring invalid HTTP rule \"%s\"\n", text.c_str());
    }
    return rules;
}

// The address as Java's InetAddress.getHostAddress() writes it
static std::string javaAddressString(const Poco::Net::IPAddress& address) {
    if (address.family() != Poco::Net::IPAddress::IPv6) return address.toString();
    const Poco::UInt16 * groups = (const Poco::UInt16 *)address.addr();
    std::string result;
    char buf[8];
    for (int i = 0; i < 8; i++) {
        snprintf(buf, sizeof(buf), "%x", (unsigned)Poco::ByteOrder::fromNetwork(groups[i]));
        if (i) result += ':';
        result += buf;
    }
    return result;
}

bool applyHTTPRules(const std::vector<HTTPRule>& rules, const std::string& host, int port, const Poco::Net::IPAddress& rawAddress, HTTPOptions& options) {
    Poco::Net::IPAddress address = rawAddress;
    if (address.family() == Poco::Net::IPAddress::IPv6 && address.isIPv4Mapped()) { // Java turns these into IPv4 addresses
        const unsigned char * b = (const unsigned char *)address.addr();
        address = Poco::Net::IPAddress(b + 12, 4);
    }
    const std::string addressText = javaAddressString(address);
    std::string embedded; // the IPv4 address inside a 6to4 address (2002::/16)
    if (address.family() == Poco::Net::IPAddress::IPv6) {
        const unsigned char * b = (const unsigned char *)address.addr();
        if (b[0] == 0x20 && b[1] == 0x02) embedded = Poco::Net::IPAddress(b + 2, 4).toString();
    }
    int allow = -1, useProxy = -1, websocketMessage = -1;
    long long maxUpload = -1, maxDownload = -1;
    for (const HTTPRule& rule : rules) {
        if (rule.port >= 0 && rule.port != port) continue;
        if (!matchIPClass(host, rule.host) && !matchIPClass(addressText, rule.host) && (embedded.empty() || !matchIPClass(embedded, rule.host))) continue;
        if (allow < 0) allow = rule.allow;
        if (maxUpload < 0) maxUpload = rule.maxUpload;
        if (maxDownload < 0) maxDownload = rule.maxDownload;
        if (websocketMessage < 0) websocketMessage = rule.websocketMessage;
        if (useProxy < 0) useProxy = rule.useProxy;
    }
    // whatever a rule does not say comes from the general settings, which default to CC: Tweaked's values
    options.maxUpload = maxUpload >= 0 ? maxUpload : (config.http_max_upload > 0 ? config.http_max_upload : DEFAULT_MAX_UPLOAD);
    options.maxDownload = maxDownload >= 0 ? maxDownload : (config.http_max_download > 0 ? config.http_max_download : DEFAULT_MAX_DOWNLOAD);
    options.websocketMessage = websocketMessage >= 0 ? websocketMessage : (config.http_max_websocket_message > 0 ? config.http_max_websocket_message : DEFAULT_WEBSOCKET_MESSAGE);
    // CC: Tweaked only uses its proxy when a rule asks for it; the older behavior was to use it for everything
    options.useProxy = useProxy >= 0 ? useProxy == 1 : (!activeROMVersion().ccTweaked && !config.http_proxy_server.empty());
    return allow == 1;
}

std::string resolveHTTPTarget(const std::string& hostIn, int port, bool secure, Poco::Net::SocketAddress& result, HTTPOptions& options) {
    std::string host = hostIn;
    if (host.size() >= 2 && host.front() == '[' && host.back() == ']') host = host.substr(1, host.size() - 2);
    if (port <= 0) port = secure ? 443 : 80;
    if (host.find('%') != std::string::npos) return "Scoped address not permitted";
    Poco::Net::IPAddress address;
    if (!Poco::Net::IPAddress::tryParse(host, address)) {
        try {
            const Poco::Net::HostEntry entry = Poco::Net::DNS::hostByName(host);
            const auto& addresses = entry.addresses();
            if (addresses.empty()) return "Unknown host";
            // Java prefers IPv4 addresses
            address = addresses.front();
            for (const auto& a : addresses) if (a.family() == Poco::Net::IPAddress::IPv4) {address = a; break;}
        } catch (Poco::Exception &) {
            return "Unknown host";
        }
    }
    if (address.family() == Poco::Net::IPAddress::IPv6 && address.scope() != 0) return "Scoped address not permitted";
    if (!applyHTTPRules(effectiveHTTPRules(), host, port, address, options)) return "Domain not permitted";
    result = Poco::Net::SocketAddress(address, (Poco::UInt16)port);
    return "";
}
