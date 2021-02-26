#ifndef _H_common
#define _H_common

#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <cstring>
#include <string>
#include <memory>
#include <algorithm>
#include <stdexcept>

#define ZERO(s) memset(&s, 0, sizeof(s))

// Convert all std::strings to const char* using constexpr if (C++17)
template<typename T>
auto convert(T&& t) {
  if constexpr (std::is_same<std::remove_cv_t<std::remove_reference_t<T>>, std::string>::value) {
    return std::forward<T>(t).c_str();
  }
  else {
    return std::forward<T>(t);
  }
}

// printf like formatting for C++ with std::string
// https://gist.github.com/Zitrax/a2e0040d301bf4b8ef8101c0b1e3f1d5
template<typename ... Args>
std::string fmtInternal(const std::string& format, Args&& ... args)
{
  size_t size = std::snprintf(nullptr, 0, format.c_str(), std::forward<Args>(args) ..., NULL) + 1;
  if( size <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
  std::unique_ptr<char[]> buf(new char[size]);
  std::snprintf(buf.get(), size, format.c_str(), args ..., NULL);
  return std::string(buf.get(), buf.get() + size - 1);
}

template<typename ... Args>
std::string fmt(std::string fmt, Args&& ... args) {
  return fmtInternal(fmt, convert(std::forward<Args>(args))...);
}

#define notef(...) { fprintf(stderr, "%s", fmt(__VA_ARGS__).c_str()); fputc('\n', stderr); }
#define fatalf(...) { notef(__VA_ARGS__); exit(EXIT_FAILURE); }

#define ensure(cond,...) if (!(cond)) { throw; }
#define ensuref(cond,...) if (!(cond)) { notef(__VA_ARGS__); throw; }

#define fmtc(...) fmt(__VA_ARGS__).c_str()

template <typename T, typename B>
void if_is(B* value, std::function<void(T*)> action) {
  auto cast_value = dynamic_cast<T*>(value);
  if (cast_value != nullptr) {
    action(cast_value);
  }
}

#define MaxEntity 1000000

template <typename C, typename V>
bool contains(const C& c, const V& v) {
  return std::find(c.begin(), c.end(), v) != c.end();
}

template <typename C>
void deduplicate(C& c) {
  std::sort(c.begin(), c.end());
  c.erase(std::unique(c.begin(), c.end()), c.end());
}

template <typename C, typename F>
void discard_if(C& c, F fn) {
  c.erase(std::remove_if(c.begin(), c.end(), fn), c.end());
}

#endif
