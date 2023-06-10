// Copyright (c) 2023, Charles Mason <chuck+github@borboggle.com>
// All rights reserved.
// 
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. 
#pragma once

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <variant>

typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short int u16;
typedef short int s16;
typedef unsigned int u32;
typedef int s32;
typedef unsigned long long u64;
typedef long long s64;

inline bool StringEndsWith(std::string const& value, std::string const& ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

template <class T>
inline void WriteVarInt(std::ostream& os, T const& v)
{
    assert(v >= 0); // TODO support negative, u64
    if(v < 254) {
        u8 s = (u8)v;
        os.write((char*)&s, 1);
    } else if(v < 0x10000) {
        u8 c = 254;
        u16 s = (u16)v;
        os.write((char*)&c, 1);
        os.write((char*)&s, 2);
    } else {
        assert(v < 0x100000000LL);
        u8 c = 255;
        u32 s = (u32)v;
        os.write((char*)&c, 1);
        os.write((char*)&s, 4);
    }
}

template <class T>
inline T ReadVarInt(std::istream& is)
{
    u8 v;
    is.read((char*)&v, 1);
    if(v < 254) {
        return (T)v;
    } else if(v == 254) {
        u16 v;
        is.read((char*)&v, 2);
        return (T)v;
    } else {
        u32 v;
        is.read((char*)&v, 4);
        return (T)v;
    }

    return 0;
}

inline void WriteString(std::ostream& os, std::string const& s)
{
    WriteVarInt(os, s.size());
    os.write(s.c_str(), s.size());
}

inline void ReadString(std::istream& is, std::string& s)
{
    auto len = ReadVarInt<u32>(is);
    s.resize(len);
    is.read(&s[0], len);
}

template<class T>
inline void WriteEnum(std::ostream& os, T const& enum_value)
{
    WriteVarInt(os, static_cast<int>(enum_value));
}

template<class T>
inline T ReadEnum(std::istream& is)
{
    int tmp = ReadVarInt<int>(is);
    return static_cast<T>(tmp);
}

template<class T>
inline void zero(T* mem) {
    memset(mem, 0, sizeof(T));
}


// https://stackoverflow.com/questions/3418231/replace-part-of-a-string-with-another-string
inline void strreplace(std::string& str, const std::string& from, const std::string& to) {
    if(from.empty()) return;

    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

inline std::string strlower(std::string const& s)
{
    std::string ret = s;
    std::transform(ret.begin(), ret.end(), ret.begin(), [](unsigned char c){ return std::tolower(c); });
    return ret;
}

inline bool file_exists(std::string const& name)
{
    std::ifstream f(name);
    return f.good();
}

// from https://stackoverflow.com/questions/66588729/is-there-an-alternative-to-stdbind-that-doesnt-require-placeholders-if-functi
// TODO convert the whole of the code base away from std::bind
template<typename Func, typename Obj>
inline auto quick_bind(Func f, Obj* obj) 
{
    return [=](auto&&... args) {
        return (obj->*f)(std::forward<decltype(args)>(args)...);
    };
}

// from https://stackoverflow.com/questions/47203255/convert-stdvariant-to-another-stdvariant-with-super-set-of-types
template <class... Args>
struct variant_cast_proxy {
    std::variant<Args...> v;

    template <class... ToArgs>
    operator std::variant<ToArgs...>() const {
        return std::visit([](auto&& arg) -> std::variant<ToArgs...> { return arg ; }, v);
    }
};

template <class... Args>
auto variant_cast(const std::variant<Args...>& v) -> variant_cast_proxy<Args...>
{
    return {v};
}
