/*
 * apis/http_rules.hpp
 * CraftOS-Tweaked
 *
 * This file declares CC: Tweaked's HTTP address rules (the "http.rules" of its config file).
 *
 * This file is part of CraftOS-Tweaked, a fork of CraftOS-PC 2 by JackMacWindows.
 * Copyright (c) 2026 slammingprogramming. Licensed under the GNU Affero General Public License, version 3 or later.
 */

#ifndef APIS_HTTP_RULES_HPP
#define APIS_HTTP_RULES_HPP
#include <string>
#include <vector>
#include <Poco/Net/SocketAddress.h>

// What the rules allow for one request or websocket
struct HTTPOptions {
    long long maxUpload;
    long long maxDownload;
    int websocketMessage;
    bool useProxy;
};

// One address rule. Rules are written as strings in the "http_rules" setting:
//   [allow|deny] <host> [port=<n>] [max_upload=<n>] [max_download=<n>] [websocket_message=<n>] [use_proxy=<true|false>]
// where <host> is a domain pattern with * wildcards, an address with a CIDR size (10.0.0.0/8) or $private.
struct HTTPRule {
    std::string host;
    int allow = -1;                 // 1 = allow, 0 = deny, -1 = this rule does not decide
    int port = -1;
    long long maxUpload = -1, maxDownload = -1;
    int websocketMessage = -1;
    int useProxy = -1;
};

extern bool httpRulesExplicit;      // http_rules was set by the user, so it is used instead of the defaults

extern bool parseHTTPRule(const std::string& text, HTTPRule& rule);
extern std::vector<std::string> defaultHTTPRules(bool ccTweaked);
extern std::vector<HTTPRule> effectiveHTTPRules();
extern std::vector<std::string> effectiveHTTPRuleTexts();   // the same, as the strings of the http_rules setting

// Merges the rules that match like CC: Tweaked does (the first rule that has a value for an option decides it). The
// address is where the host name resolves to.
extern bool applyHTTPRules(const std::vector<HTTPRule>& rules, const std::string& host, int port, const Poco::Net::IPAddress& address, HTTPOptions& options);

// Resolves a host, then checks the result against the rules. Returns an empty string on success or the reason a
// request must fail: "Unknown host", "Scoped address not permitted" or "Domain not permitted". The same checks
// happen for http.get, http.websocket and http.checkURL; the resolved address is returned so that the connection can
// be made to exactly the address that was checked.
extern std::string resolveHTTPTarget(const std::string& host, int port, bool secure, Poco::Net::SocketAddress& address, HTTPOptions& options);
#endif
