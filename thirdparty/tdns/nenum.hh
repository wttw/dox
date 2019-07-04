#pragma once
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <array>
#include <string.h>

#define SMARTENUMSTART(x) static constexpr std::pair<x, const char*> enumtypemap##x[]= {
#if 0
// Clever. Doesn't work on MSVC, and I'm not sure MSVC is wrong.
#define SENUM(x,a1) { x::a1, #a1},
#define SENUM2(x, a1, ...) SENUM(x,a1) SENUM(x, __VA_ARGS__)
#define SENUM3(x, a1, ...)  SENUM(x,a1) SENUM2(x, __VA_ARGS__)
#define SENUM4(x, a1, ...)  SENUM(x,a1) SENUM3(x, __VA_ARGS__)
#define SENUM5(x, a1, ...)  SENUM(x,a1) SENUM4(x, __VA_ARGS__)
#define SENUM6(x, a1, ...)  SENUM(x,a1) SENUM5(x, __VA_ARGS__)
#define SENUM7(x, a1, ...)  SENUM(x,a1) SENUM6(x, __VA_ARGS__)
#define SENUM8(x, a1, ...)  SENUM(x,a1) SENUM7(x, __VA_ARGS__)
#define SENUM9(x, a1, ...)  SENUM(x,a1) SENUM8(x, __VA_ARGS__)
#define SENUM10(x, a1, ...) SENUM(x,a1) SENUM9(x, __VA_ARGS__)
#define SENUM11(x, a1, ...) SENUM(x,a1) SENUM10(x, __VA_ARGS__)
#define SENUM12(x, a1, ...) SENUM(x,a1) SENUM11(x, __VA_ARGS__)
#define SENUM13(x, a1, ...) SENUM(x,a1) SENUM12(x, __VA_ARGS__)

#else
#define SENUM(x,a1) { x::a1, #a1},
#define SENUM2(x, a1, a2) SENUM(x, a1) SENUM(x, a2)
#define SENUM3(x, a1, a2, a3) SENUM2(x, a1, a2) SENUM(x, a3)
#define SENUM4(x, a1, a2, a3, a4) SENUM2(x, a1, a2) SENUM2(x, a3, a4)
#define SENUM5(x, a1, a2, a3, a4, a5) SENUM4(x, a1, a2, a3, a4) SENUM(x, a5)
#define SENUM6(x, a1, a2, a3, a4, a5, a6) SENUM4(x, a1, a2, a3, a4) SENUM2(x, a5, a6)
#define SENUM7(x, a1, a2, a3, a4, a5, a6, a7) SENUM4(x, a1, a2, a3, a4) SENUM3(x, a5, a6, a7)
#define SENUM8(x, a1, a2, a3, a4, a5, a6, a7, a8) SENUM4(x, a1, a2, a3, a4) SENUM4(x, a5, a6, a7, a8)
#define SENUM9(x, a1, a2, a3, a4, a5, a6, a7, a8, a9) SENUM8(x,a1,a2,a3,a4,a5,a6,a7,a8) SENUM(x, a9)
#define SENUM10(x, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) SENUM8(x,a1,a2,a3,a4,a5,a6,a7,a8) SENUM2(x, a9, a10)
#define SENUM11(x, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) SENUM8(x,a1,a2,a3,a4,a5,a6,a7,a8) SENUM3(x, a9, a10, a11)
#define SENUM12(x, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) SENUM8(x,a1,a2,a3,a4,a5,a6,a7,a8) SENUM4(x, a9, a10, a11, a12)
#define SENUM13(x, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13) SENUM8(x,a1,a2,a3,a4,a5,a6,a7,a8) SENUM5(x, a9, a10, a11, a12, a13)
#endif

#define SMARTENUMEND(x) };                                             \
inline const char* toString(const x& t)                                \
{                                                                      \
 for(const auto &a : enumtypemap##x)                                   \
   if(a.first == t)                                                    \
       return a.second;                                                \
  return "?";                                                          \
}                                                                      \
inline x make##x(const char* from) {                                   \
for(const auto& a : enumtypemap##x)                                    \
  if(!strcmp(a.second, from))                                          \
    return a.first;                                                    \
  throw std::runtime_error("Unknown value '" + std::string(from) + "' for enum "#x); \
 }                                                                     \
inline std::ostream& operator<<(std::ostream &os, const x& s) {        \
  os << toString(s); return os; }                                      \
                                                                      
#define COMBOENUM4(x, a1,b1,a2,b2,a3,b3,a4,b4) enum class x : uint16_t {     \
    a1=b1, a2=b2, a3=b3, a4=b4 }; SMARTENUMSTART(x) SENUM4(x, a1, a2, a3,a4) \
  SMARTENUMEND(x)  
