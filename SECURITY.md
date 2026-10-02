<!--
Copyright (c) 2019-2024 JackMacWindows (original CraftOS-PC 2 security policy, MIT License).
Copyright (c) 2026 slammingprogramming. Licensed under CC-BY-SA-4.0 (see LICENSE).
-->
# Security Policy

## Supported Versions

Only the latest release of CraftOS-Tweaked receives security fixes.

## Reporting a Vulnerability

If a bug is related to any of the following, it is a security vulnerability and should be reported **privately**, not as a public issue:
- Filesystem sandbox escape (outside of mounts)
- Process/library loading
- Arbitrary code execution
- Network rule bypass
- Any other form of reading host information (excluding LuaJIT)

**How to report it:** contact the maintainer privately on [SimpleX Chat](https://simplex.chat/) using this contact address:

<https://smp14.simplex.im/a#3gZ-zeHs4QrFZKLAN0o3SC_XQJXhj1eYBVTO_c0FAtg>

(Open it with the SimpleX app, or scan it as a QR code from the app.) Please include the CraftOS-Tweaked version, your operating system, and steps or a small Lua program that reproduces the problem. Do not post details in a public issue, discussion or pull request until a fix has been released.

The report will be reviewed, and if it is valid a patch will be made available within a week (depending on the severity).

## Other private contact

The same SimpleX address can be used for anything that should not be public. For ordinary bugs and feature requests please use the [issue forms](https://github.com/slammingprogramming/craftos-pc-tweaked/issues/new/choose).
