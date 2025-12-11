// Compatibility.h
// Self-contained compatibility helpers for TMM (no ATL required)
#pragma once

#include <string>
#include <vector>
#include <windows.h>
#include <algorithm>
#include <cwctype>
#include <cctype>
#include <cstring>

// -----------------------------
// ANSI -> WIDE conversion
// -----------------------------
inline std::wstring A2W(const char* src) {
    if (!src) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, src, -1, nullptr, 0);
    if (len <= 0) return L"";
    std::wstring out(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, src, -1, &out[0], len);
    if (!out.empty() && out.back() == L'\0') out.pop_back();
    return out;
}

// -----------------------------
// WIDE -> ANSI conversion
// -----------------------------
inline std::string W2A(const wchar_t* src) {
    if (!src) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, src, -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0) return "";
    std::string out(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, src, -1, &out[0], len, nullptr, nullptr);
    if (!out.empty() && out.back() == '\0') out.pop_back();
    return out;
}

#define USES_CONVERSION

// -----------------------------
// IsAnsi helpers
// -----------------------------
inline bool IsAnsi(const std::string& s) {
    for (unsigned char c : s)
        if (c > 0x7F) return false;
    return true;
}
inline bool IsAnsi(const std::wstring& s) {
    for (wchar_t c : s)
        if (c > 0x7F) return false;
    return true;
}

// -----------------------------
// ToUpper helpers
// -----------------------------
inline std::string ToUpper(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(),
        [](unsigned char c) { return (char)std::toupper(c); });
    return out;
}
inline std::wstring ToUpper(const std::wstring& s) {
    std::wstring out = s;
    std::transform(out.begin(), out.end(), out.begin(),
        [](wchar_t c) { return std::towupper(c); });
    return out;
}

// -----------------------------
// Version constants (missing from original code)
// -----------------------------
#ifndef VER_TERA_CLASSIC
#define VER_TERA_CLASSIC 0x01
#endif
#ifndef VER_TERA_MODERN
#define VER_TERA_MODERN 0x02
#endif
#ifndef VER_TERA_FILEMOD
#define VER_TERA_FILEMOD 0x04
#endif

// Expose memcpy
using ::memcpy;
