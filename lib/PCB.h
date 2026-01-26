/**
 * Copyright (c) 2025 Piotr Mikolajewski
 * This is PCB, a header-only general purpose C library
 * with build capabilities.
 *
 * PCB is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PCB is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with PCB. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef PCB_H
#define PCB_H

#ifndef PCB_VERSION_MAJOR
#define PCB_VERSION_MAJOR 0
#endif //PCB_VERSION_MAJOR

#ifndef PCB_VERSION_MINOR
#define PCB_VERSION_MINOR 6
#endif //PCB_VERSION_MINOR

#ifndef PCB_VERSION_PATCH
#define PCB_VERSION_PATCH 2
#endif //PCB_VERSION_PATCH

#ifndef PCB_VERSION
#define PCB_VERSION (PCB_VERSION_MAJOR * 1000000 + PCB_VERSION_MINOR * 1000 + PCB_VERSION_PATCH)
#endif //PCB_VERSION

#if !defined(__STDC_VERSION__) && !defined(__cplusplus)
#error "PCB Error: C89 is not supported"
#endif //C89/90?

#ifdef __cplusplus
extern "C" {
#endif //C++

//Section 1: The preprocessor shenanigans
//Section 1.1: Identify the target platform

//https://sourceforge.net/p/predef/wiki/OperatingSystems/
#ifndef PCB_PLATFORM
#if defined(_WIN32) || defined(WIN32) || defined(__WIN32__) || defined(__NT__)
#define PCB_PLATFORM_WINDOWS 1
#define PCB_PLATFORM_LINUX 0
#define PCB_PLATFORM_BSD 0
#define PCB_PLATFORM_MACOS 0
#define PCB_PLATFORM_IOS 0
#define PCB_PLATFORM_WASM 0
#define PCB_PLATFORM "Windows"
#elif defined(__linux__)
#define PCB_PLATFORM_WINDOWS 0
#define PCB_PLATFORM_LINUX 1
#define PCB_PLATFORM_BSD 0
#define PCB_PLATFORM_MACOS 0
#define PCB_PLATFORM_IOS 0
#define PCB_PLATFORM_WASM 0
#define PCB_PLATFORM "Linux"
#elif defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_IOS
#define PCB_PLATFORM_WINDOWS 0
#define PCB_PLATFORM_LINUX 0
#define PCB_PLATFORM_BSD 0
#define PCB_PLATFORM_IOS 1
#define PCB_PLATFORM_MACOS 0
#define PCB_PLATFORM_WASM 0
#define PCB_PLATFORM "iOS"
#elif TARGET_OS_MAC
#define PCB_PLATFORM_WINDOWS 0
#define PCB_PLATFORM_LINUX 0
#define PCB_PLATFORM_BSD 0
#define PCB_PLATFORM_IOS 0
#define PCB_PLATFORM_MACOS 1
#define PCB_PLATFORM_WASM 0
#define PCB_PLATFORM "Mac OS"
#else
#error PCB Error: Unsupported Apple platform
#endif //Apple platforms
#elif defined(__wasm__)
#define PCB_PLATFORM_WINDOWS 0
#define PCB_PLATFORM_LINUX 0
#define PCB_PLATFORM_BSD 0
#define PCB_PLATFORM_IOS 0
#define PCB_PLATFORM_MACOS 0
#define PCB_PLATFORM_WASM 1
#define PCB_PLATFORM "WebAssembly"
#error PCB Error: WebAssembly target is currently not supported
#else
#error PCB Error: Unsupported platform
#define PCB_PLATFORM "Unknown"
#endif //platform
#endif //PCB_PLATFORM

//This macro is used for certain #include's of system headers
//and some function implementations.
//POSIX-compliant platforms can safely share implementations.
//Other platforms require dedicated implementations (*ekhem* Windows...).
#ifndef PCB_PLATFORM_POSIX
#if PCB_PLATFORM_WINDOWS
#define PCB_PLATFORM_POSIX 0
#elif PCB_PLATFORM_LINUX || PCB_PLATFORM_BSD || PCB_PLATFORM_MACOS || PCB_PLATFORM_IOS
#define PCB_PLATFORM_POSIX 1
#else
#define PCB_PLATFORM_POSIX 0
#endif //platform
#endif //PCB_PLATFORM_POSIX

#if PCB_PLATFORM_POSIX
#if PCB_PLATFORM_LINUX
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif //_GNU_SOURCE
#else
#if !defined(_XOPEN_SOURCE) && !defined(_POSIX_C_SOURCE)
#if defined(__GLIBC__) && __GLIBC__+0 >= 2 && __GLIBC_MINOR__+0 < 10
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 700
#endif //glibc 2.10
#endif //only #define if no feature test macro is #defined
#endif //Use _GNU_SOURCE on Linux
#endif //POSIX sources used locally



//Section 1.2: Identify the compiler used to compile this code

#ifndef PCB_COMPILER
#if defined(__GNUC__) && !defined(__clang__) && !defined(__INTEL_COMPILER)
//https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
#define PCB_COMPILER_GCC (__GNUC__*100*100 + __GNUC_MINOR__*100 + __GNUC_PATCHLEVEL__)
#define PCB_COMPILER_CLANG 0
#define PCB_COMPILER_MSVC 0
#if PCB_PLATFORM_WINDOWS
#define PCB_COMPILER "MinGW"
#ifdef __cplusplus
#define PCB_COMPILER_PATH "g++"
#define PCB_COMPILER_PATH_ALT "gcc"
#else
#define PCB_COMPILER_PATH "gcc"
#define PCB_COMPILER_PATH_ALT "g++"
#endif //C++?
#else
#ifdef __MINGW32__
#define PCB_COMPILER "MinGW"
#ifdef __cplusplus
#ifdef PCB_CROSS_COMPILE
#define PCB_COMPILER_PATH "x86_64-w64-mingw32-g++"
#define PCB_COMPILER_PATH_ALT "x86_64-w64-mingw32-gcc"
#else
#define PCB_COMPILER_PATH "g++" //MinGW is named "gcc" on Windows
#define PCB_COMPILER_PATH_ALT "gcc"
#endif //MinGW is named "gcc" on Windows, but "x86_64-w64-mingw32-gcc" on Linux
#else
#ifdef PCB_CROSS_COMPILE
#define PCB_COMPILER_PATH "x86_64-w64-mingw32-gcc"
#define PCB_COMPILER_PATH_ALT "x86_64-w64-mingw32-g++"
#else
#define PCB_COMPILER_PATH "g++"
#define PCB_COMPILER_PATH_ALT "gcc"
#endif //MinGW is named "gcc" on Windows, but "x86_64-w64-mingw32-gcc" on Linux
#endif //C++?
#else
#define PCB_COMPILER "GCC"
#ifdef __cplusplus
#define PCB_COMPILER_PATH "g++"
#define PCB_COMPILER_PATH_ALT "gcc"
#else
#define PCB_COMPILER_PATH "gcc"
#define PCB_COMPILER_PATH_ALT "g++"
#endif //C++?
#endif //MinGW check
#endif //platform
#elif defined(__clang__)
#define PCB_COMPILER_GCC 0
//same schema as GCC
#define PCB_COMPILER_CLANG (__clang_major__*100*100 + __clang_minor__*100 + __clang_patchlevel__)
#define PCB_COMPILER_MSVC 0
#define PCB_COMPILER "Clang"
#ifdef __cplusplus
#define PCB_COMPILER_PATH "clang++"
#define PCB_COMPILER_PATH_ALT "clang"
#else
#define PCB_COMPILER_PATH "clang"
#define PCB_COMPILER_PATH_ALT "clang++"
#endif //C++?
#elif defined(_MSC_VER) && !defined(__clang__)
#define PCB_COMPILER_GCC 0
#define PCB_COMPILER_CLANG 0
//MSVC uses a different versioning scheme, https://learn.microsoft.com/en-us/cpp/overview/compiler-versions
#define PCB_COMPILER_MSVC (_MSC_VER)
#define PCB_COMPILER "MSVC"
//Note: To actually use MSVC like this, one has to set PATH appropiately. See 
#define PCB_COMPILER_PATH "cl"
#define PCB_COMPILER_PATH_ALT PCB_COMPILER_PATH
#else
#error PCB Error: Unsupported compiler
#endif //compiler
#endif //PCB_COMPILER

#ifndef PCB_COMPILER_VERSION
#if PCB_COMPILER_GCC
#define PCB_COMPILER_VERSION PCB_COMPILER_GCC
#elif PCB_COMPILER_CLANG
#define PCB_COMPILER_VERSION PCB_COMPILER_CLANG
#elif PCB_COMPILER_MSVC
#define PCB_COMPILER_VERSION PCB_COMPILER_MSVC
#else
#define PCB_COMPILER_VERSION 0
#endif //compiler
#endif //PCB_COMPILER_VERSION

#ifndef PCB_STRICT_ISO
#if (PCB_COMPILER_GCC || PCB_COMPILER_CLANG) && defined(__STRICT_ANSI__)
#define PCB_STRICT_ISO 1
#else
#define PCB_STRICT_ISO 0
#endif //ISO C/C++ compliance warnings for GCC & Clang (MSVC doesn't have something like this)
#endif //PCB_STRICT_ISO

//Section 1.3: Identify the target architecture
#ifndef PCB_ARCH
//https://stackoverflow.com/questions/152016/detecting-cpu-architecture-compile-time
//https://sourceforge.net/p/predef/wiki/Architectures/
#if PCB_COMPILER_MSVC
//https://learn.microsoft.com/en-us/cpp/preprocessor/predefined-macros
#if defined(_M_X64) || defined(_M_AMD64)
#define PCB_ARCH_x86_64 1
#define PCB_ARCH_x64 1
#define PCB_ARCH "x64"
#elif defined(_M_IX86)
#define PCB_ARCH_i386 1
#define PCB_ARCH_x86 1
#define PCB_ARCH "x86"
#elif defined(_M_ARM64)
#define PCB_ARCH_AArch64 1
#define PCB_ARCH_ARM64 1
#define PCB_ARCH "ARM64"
#elif defined(_M_ARM)
#define PCB_ARCH_ARM _M_ARM
#define PCB_ARCH "ARMv?"
#elif defined(_M_ALPHA)
#define PCB_ARCH_Alpha 1
#define PCB_ARCH "Alpha"
#elif defined(_M_PPC)
#define PCB_ARCH_PowerPC 1
#define PCB_ARCH "PowerPC"
#endif //architectures
#elif PCB_COMPILER_GCC || PCB_COMPILER_CLANG
#if defined(__x86_64__) || defined(__x86_64) || \
    defined(__amd64__)  || defined(__amd64)
#define PCB_ARCH_x86_64 1
#define PCB_ARCH_x64 1
#define PCB_ARCH "x86_64"
#elif defined(i386) || defined(__i386) || defined(__i386__)
#define PCB_ARCH_i386 1
#define PCB_ARCH_x86 1
#define PCB_ARCH "i386"
#elif defined(__aarch64__)
#define PCB_ARCH_AArch64 1
#define PCB_ARCH_ARM64 1
#define PCB_ARCH "AArch64"
#elif defined(__arm__)
#if defined(__ARM_ARCH_2__)
#define PCB_ARCH_ARM 2
#define PCB_ARCH_ARMv2 1
#define PCB_ARCH "ARMv2"
#elif defined(__ARM_ARCH_3__) || defined(__ARM_ARCH_3M__)
#define PCB_ARCH_ARM 3
#define PCB_ARCH_ARMv3 1
#define PCB_ARCH "ARMv3"
#elif defined(__ARM_ARCH_4T__) || defined(__TARGET_ARM_4T)
#define PCB_ARCH_ARM 4
#define PCB_ARCH_ARMv4T 1
#define PCB_ARCH "ARMv4T"
#elif defined(__ARM_ARCH_5_) || defined(__ARM_ARCH_5E_)
#define PCB_ARCH_ARM 5
#define PCB_ARCH_ARMv5 1
#define PCB_ARCH "ARMv5"
#elif defined(__ARM_ARCH_5T__) || defined(__ARM_ARCH_5TE__) ||
      defined(__ARM_ARCH_5TEJ__)
#define PCB_ARCH_ARM 5
#define PCB_ARCH_ARMv5T 1
#define PCB_ARCH "ARMv5T"
#elif defined(__ARM_ARCH_6T2__)
#define PCB_ARCH_ARM 6
#define PCB_ARCH_ARMv6T2 1
#define PCB_ARCH "ARMv6T2"
#elif defined(__ARM_ARCH_6__)  || defined(__ARM_ARCH_6J__) || \
      defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) || \
      defined(__ARM_ARCH_6ZK__)
#define PCB_ARCH_ARM 6
#define PCB_ARCH_ARMv6 1
#define PCB_ARCH "ARMv6"
#elif defined(__ARM_ARCH_7__)  || defined(__ARM_ARCH_7A__) || \
      defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || \
      defined(__ARM_ARCH_7S__)
#define PCB_ARCH_ARM 7
#if defined(__ARM_ARCH_7A__)
#define PCB_ARCH_ARMv7A 1
#define PCB_ARCH "ARMv7A"
#elif defined(__ARM_ARCH_7R__)
#define PCB_ARCH_ARMv7R 1
#define PCB_ARCH "ARMv7R"
#elif defined(__ARM_ARCH_7M__)
#define PCB_ARCH_ARMv7M 1
#define PCB_ARCH "ARMv7M"
#elif defined(__ARM_ARCH_7S__)
#define PCB_ARCH_ARMv7S 1
#define PCB_ARCH "ARMv7S"
#endif //ARMv7
#endif //different ARM ISAs
#elif defined(__alpha__)
#define PCB_ARCH_Alpha 1
#define PCB_ARCH "Alpha"
#elif defined(__m68k__)
#define PCB_ARCH_M68k 1
#define PCB_ARCH "Motorola 68k"
#elif defined(__mips__) || defined(mips)
#define PCB_ARCH_MIPS 1
#define PCB_ARCH "MIPS"
//What in bloody hell is this?! Are you OK, GNU people?
#elif defined(__powerpc)   || defined(__powerpc__) || defined(__powerpc64__) || \
      defined(__POWERPC__) || defined(__ppc__)     || defined(__ppc64__) || \
      defined(__PPC__)     || defined(__PPC64__)   || \
      defined(_ARCH_PPC)   || defined(_ARCH_PPC64)
#define PCB_ARCH_PowerPC 1
#define PCB_ARCH "PowerPC"
#elif defined(__sparc__)
#define PCB_ARCH_SPARC 1
#define PCB_ARCH "SPARC"
#endif //architectures
#else
#endif //compiler

#ifndef PCB_ARCH_x86_64
#define PCB_ARCH_x86_64 0
#endif //PCB_ARCH_x86_64
#ifndef PCB_ARCH_x64
#define PCB_ARCH_x64
#endif //PCB_ARCH_x64
#ifndef PCB_ARCH_i386
#define PCB_ARCH_i386 0
#endif //PCB_ARCH_i386
#ifndef PCB_ARCH_x86
#define PCB_ARCH_x86 0
#endif //PCB_ARCH_x86
#ifndef PCB_ARCH_ARM64
#define PCB_ARCH_ARM64 0
#endif //PCB_ARCH_ARM64
#ifndef PCB_ARCH_AArch64
#define PCB_ARCH_AArch64 0
#endif //PCB_ARCH_AArch64
#ifndef PCB_ARCH_ARM
#define PCB_ARCH_ARM 0
#endif //PCB_ARCH_ARM
#ifndef PCB_ARCH_ARMv2
#define PCB_ARCH_ARMv2 0
#endif //PCB_ARCH_ARMv2
#ifndef PCB_ARCH_ARMv3
#define PCB_ARCH_ARMv3 0
#endif //PCB_ARCH_ARMv3
#ifndef PCB_ARCH_ARMv4T
#define PCB_ARCH_ARMv4T 0
#endif //PCB_ARCH_ARMv4T
#ifndef PCB_ARCH_ARMv5
#define PCB_ARCH_ARMv5 0
#endif //PCB_ARCH_ARMv5
#ifndef PCB_ARCH_ARMv5T
#define PCB_ARCH_ARMv5T 0
#endif //PCB_ARCH_ARMv5T
#ifndef PCB_ARCH_ARMv6T2
#define PCB_ARCH_ARMv6T2 0
#endif //PCB_ARCH_ARMv6T2
#ifndef PCB_ARCH_ARMv6
#define PCB_ARCH_ARMv6 0
#endif //PCB_ARCH_ARMv6
#ifndef PCB_ARCH_ARMv7A
#define PCB_ARCH_ARMv7A 0
#endif //PCB_ARCH_ARMv7A
#ifndef PCB_ARCH_ARMv7R
#define PCB_ARCH_ARMv7R 0
#endif //PCB_ARCH_ARMv7R
#ifndef PCB_ARCH_ARMv7M
#define PCB_ARCH_ARMv7M 0
#endif //PCB_ARCH_ARMv7M
#ifndef PCB_ARCH_ARMv7S
#define PCB_ARCH_ARMv7S 0
#endif //PCB_ARCH_ARMv7S
#ifndef PCB_ARCH_Alpha
#define PCB_ARCH_Alpha 0
#endif //PCB_ARCH_Alpha
#ifndef PCB_ARCH_M68k
#define PCB_ARCH_M68k 0
#endif //PCB_ARCH_M68k
#ifndef PCB_ARCH_MIPS
#define PCB_ARCH_MIPS 0
#endif //PCB_ARCH_MIPS
#ifndef PCB_ARCH_PowerPC
#define PCB_ARCH_PowerPC 0
#endif //PCB_ARCH_PowerPC
#ifndef PCB_ARCH_SPARC
#define PCB_ARCH_SPARC 0
#endif //PCB_ARCH_SPARC

#ifndef PCB_ARCH
#define PCB_ARCH "Unknown"
#endif //PCB_ARCH

#endif //PCB_ARCH



//Section 1.4: Define useful, but often compiler-specific macros

#ifndef PCB_DO_PRAGMA
#if PCB_COMPILER_MSVC
//https://learn.microsoft.com/en-us/cpp/preprocessor/pragma-directives-and-the-pragma-keyword
//MSVC, could you stop being an annoying b*tch FOR FIVE MINUTES!?
#define PCB_DO_PRAGMA(x) __pragma(x)
#else
#define PCB_DO_PRAGMA(x) _Pragma(#x)
#endif //MSVC...
#endif //PCB_DO_PRAGMA
#ifndef PCB_EmitWarning
//https://stackoverflow.com/questions/471935/user-warnings-on-msvc-and-gcc
//https://releases.llvm.org/3.3/tools/clang/docs/UsersManual.html
#if PCB_COMPILER_GCC >= 40800 || PCB_COMPILER_CLANG >= 50000
#define PCB_EmitWarning(w) PCB_DO_PRAGMA(GCC warning w)
#elif PCB_COMPILER_GCC >= 40407 || PCB_COMPILER_MSVC >= 1500
#define PCB_EmitWarning(w) PCB_DO_PRAGMA(message w)
#else
#define PCB_EmitWarning(w)
#endif //compilers
#endif //PCB_EmitWarning

//Save current warning state for every warning in the compiler's stack.
#ifndef PCB_PushWarnings
#if PCB_COMPILER_GCC >= 40600 || PCB_COMPILER_CLANG >= 30100
#define PCB_PushWarnings() PCB_DO_PRAGMA(GCC diagnostic push)
#elif PCB_COMPILER_MSVC
#define PCB_PushWarnings() PCB_DO_PRAGMA(warning(push))
#else
#define PCB_PushWarnings()
#endif //compilers
#endif //PCB_PushWarnings

//Disable the specified warning.
#ifndef PCB_IgnoreWarning
#if PCB_COMPILER_GCC >= 40600 || PCB_COMPILER_CLANG >= 30100
#define PCB_IgnoreWarning(w) PCB_DO_PRAGMA(GCC diagnostic ignored w)
#elif PCB_COMPILER_MSVC
#define PCB_IgnoreWarning(w) PCB_DO_PRAGMA(warning(disable: w))
#else
#define PCB_IgnoreWarning(w)
#endif //compilers
#endif //PCB_IgnoreWarning

//Elevate the specified warning to an error.
#ifndef PCB_ElevateWarning
#if PCB_COMPILER_GCC >= 40600 || PCB_COMPILER_CLANG >= 30100
#define PCB_ElevateWarning(w) PCB_DO_PRAGMA(GCC diagnostic error w)
#elif PCB_COMPILER_MSVC
#define PCB_ElevateWarning(w) PCB_DO_PRAGMA(warning(error: w))
#else
#define PCB_ElevateWarning(w)
#endif //compilers
#endif //PCB_ElevateWarning

//Here would be `GCC diagnostic warning ...`, but MSVC doesn't have an equivalent.

//Restore warning state for every warning from the compiler's stack.
#ifndef PCB_PopWarnings
#if PCB_COMPILER_GCC >= 40600 || PCB_COMPILER_CLANG >= 30100
#define PCB_PopWarnings() PCB_DO_PRAGMA(GCC diagnostic pop)
#elif PCB_COMPILER_MSVC
#define PCB_PopWarnings() PCB_DO_PRAGMA(warning(pop))
#else
#define PCB_PopWarnings()
#endif //compilers
#endif //PCB_PopWarnings


#ifndef PCB_NoDiscard
#ifdef __cplusplus
#if __cplusplus >= 201703L
#define PCB_NoDiscard [[nodiscard]]
#else
#if PCB_COMPILER_GCC || PCB_COMPILER_CLANG
#define PCB_NoDiscard __attribute__((warn_unused_result))
#elif PCB_COMPILER_MSVC
#define PCB_NoDiscard
#else
#define PCB_NoDiscard
#endif //Compilers
#endif //C++17
#else //C
#if __STDC_VERSION__ >= 202311L
#define PCB_NoDiscard [[nodiscard]]
#else
#if PCB_COMPILER_GCC || PCB_COMPILER_CLANG
#define PCB_NoDiscard __attribute__((warn_unused_result))
#elif PCB_COMPILER_MSVC
#define PCB_NoDiscard
#else
#define PCB_NoDiscard
#endif //Compilers
#endif //C23
#endif //C++?
#endif //PCB_NoDiscard

#ifndef PCB_NoDiscardReason
#ifdef __cplusplus
#if __cplusplus >= 202002L
#define PCB_NoDiscardReason(reason) [[nodiscard(reason)]]
#else
#if PCB_COMPILER_GCC || PCB_COMPILER_CLANG
#define PCB_NoDiscardReason(reason) __attribute__((warn_unused_result))
#elif PCB_COMPILER_MSVC
#define PCB_NoDiscardReason(reason)
#else
#define PCB_NoDiscardReason(reason)
#endif //Compilers
#endif //C++17
#else //C
#if __STDC_VERSION__ >= 202311L
#define PCB_NoDiscardReason(reason) [[nodiscard(reason)]]
#else
#if PCB_COMPILER_GCC || PCB_COMPILER_CLANG
#define PCB_NoDiscardReason(reason) __attribute__((warn_unused_result))
#elif PCB_COMPILER_MSVC
#define PCB_NoDiscardReason(reason)
#else
#define PCB_NoDiscardReason(reason)
#endif //Compilers
#endif //C23
#endif //C++?
#endif //PCB_NoDiscardReason

#ifndef PCB_Deprecated
#ifdef __cplusplus
#if __cplusplus >= 201402L
#define PCB_Deprecated [[deprecated]]
#else
#if PCB_COMPILER_GCC || PCB_COMPILER_CLANG
#define PCB_Deprecated __attribute__((deprecated))
#elif PCB_COMPILER_MSVC
#define PCB_Deprecated __declspec(deprecated)
#else
#define PCB_Deprecated
#endif //Compilers
#endif //C++14
#else //C
#if __STDC_VERSION__ >= 202311L
#define PCB_Deprecated [[deprecated]]
#else
#if PCB_COMPILER_GCC || PCB_COMPILER_CLANG
#define PCB_Deprecated __attribute__((deprecated))
#elif PCB_COMPILER_MSVC
#define PCB_Deprecated __declspec(deprecated)
#else
#define PCB_Deprecated
#endif //Compilers
#endif //C23
#endif //C++?
#endif //PCB_Deprecated

#ifndef PCB_DeprecatedReason
#ifdef __cplusplus
#if __cplusplus >= 201402L
#define PCB_DeprecatedReason(reason) [[deprecated(reason)]]
#else
#if PCB_COMPILER_GCC || PCB_COMPILER_CLANG
#define PCB_DeprecatedReason(reason) __attribute__((deprecated(reason)))
#elif PCB_COMPILER_MSVC
#define PCB_Deprecated(reason) __declspec(deprecated(reason))
#else
#define PCB_Deprecated(reason)
#endif //Compilers
#endif //C++14
#else //C
#if __STDC_VERSION__ >= 202311L
#define PCB_DeprecatedReason(reason) [[deprecated(reason)]]
#else
#if PCB_COMPILER_GCC || PCB_COMPILER_CLANG
#define PCB_DeprecatedReason(reason) __attribute__((deprecated(reason)))
#elif PCB_COMPILER_MSVC
#define PCB_DeprecatedReason(reason) __declspec(deprecated(reason))
#else
#define PCB_DeprecatedReason(reason)
#endif //Compilers
#endif //C23
#endif //C++?
#endif //PCB_Deprecated

#ifndef PCB_NoReturn
#ifdef __cplusplus
#if __cplusplus >= 201103L
#define PCB_NoReturn [[noreturn]]
#else
#if PCB_COMPILER_GCC
#define PCB_NoReturn __attribute__((noreturn))
#elif PCB_COMPILER_MSVC
#define PCB_NoReturn __declspec(noreturn)
#else
#define PCB_NoReturn
#endif //Compilers
#endif //C++11
#else //C
#if __STDC_VERSION__ >= 202311L
#define PCB_NoReturn [[noreturn]]
#else
#if PCB_COMPILER_GCC
#define PCB_NoReturn __attribute__((noreturn))
#elif PCB_COMPILER_CLANG
#define PCB_NoReturn _Noreturn
#elif PCB_COMPILER_MSVC
#define PCB_NoReturn __declspec(noreturn)
#else
#pragma "PCB Warning: PCB_NoReturn does not mark function as one that doesn't return"
#define PCB_NoReturn
#endif //Compilers
#endif //C23
#endif //C++?
#endif //PCB_NoReturn

#ifndef PCB_Unused
#if (defined(__cplusplus) && __cplusplus+0 >= 201703L) || \
    (defined(__STDC_VERSION__) && __STDC_VERSION__+0 >= 202311L)
#define PCB_Unused [[maybe_unused]]
#else
#if PCB_COMPILER_GCC >= 29503 || PCB_COMPILER_CLANG >= 40000
#define PCB_Unused __attribute__((unused))
#else
#define PCB_Unused
#endif //GCC 2.95.3 || Clang 4.0.0 docs are the oldest versions mentioning this attribute
#endif //C++17+ || C23+ || compilers
#endif //PCB_Unused

#ifndef PCB_ForceInline
#if (PCB_COMPILER_GCC >= 30101) || PCB_COMPILER_CLANG
#define PCB_ForceInline inline __attribute__((always_inline))
#elif PCB_COMPILER_MSVC
#define PCB_ForceInline __forceinline
#else
#define PCB_ForceInline inline
#endif //Compilers
#endif //PCB_ForceInline

#ifndef PCB_restrict
#if defined(__STDC_VERSION__) && __STDC_VERSION__+0 >= 199901L
#define PCB_restrict restrict
#elif PCB_COMPILER_GCC || PCB_COMPILER_CLANG || PCB_COMPILER_MSVC
#define PCB_restrict __restrict
#else
#define PCB_restrict
#endif //why tf is there no "restrict" keyword in C++?!
#endif //PCB_restrict

#ifndef PCB_Nullable
#if PCB_COMPILER_CLANG >= 40000
/*
 * NOTE: *ONLY* use this if you're ready to mark LITERALLY ALL pointers
 * with this, `PCB_Nonnull` or `PCB_Null_Unspecified` *if* you're using Clang.
 * Otherwise it's HIGHLY ADVISED to NOT use it.
*/
#define PCB_Nullable _Nullable
#else
#define PCB_Nullable
#endif //compilers
#endif //PCB_Nullable

#ifndef PCB_Nonnull
#if PCB_COMPILER_CLANG >= 40000
/*
 * NOTE: *ONLY* use this if you're ready to mark LITERALLY ALL pointers
 * with this, `PCB_Nullable` or `PCB_Null_Unspecified` *if* you're using Clang.
 * Otherwise it's HIGHLY ADVISED to NOT use it.
*/
#define PCB_Nonnull _Nonnull
#else
#define PCB_Nonnull
#endif //compilers
#endif //PCB_Nonnull

#ifndef PCB_Null_Unspecified
#if PCB_COMPILER_CLANG >= 40000
/*
 * NOTE: *ONLY* use this if you're ready to mark LITERALLY ALL pointers
 * with this, `PCB_Nullable` or `PCB_Nonnull` *if* you're using Clang.
 * Otherwise it's HIGHLY ADVISED to NOT use it.
*/
#define PCB_Null_Unspecified _Null_unspecified
#else
#define PCB_Null_Unspecified
#endif //compilers
#endif //PCB_Null_Unspecified

/*
 * GNU people of course had to make their own version of nonnull...
 * Used on function declarations with indices of parameters regarded as
 * "not allowed to be NULL".
 * `PCB_Nonnull` would be better for this, but GCC doesn't support it, while
 * Clang mandates to mark LITERALLY EVERY F***ING POINTER WITH IT after marking
 * just ONE.
 * No Clang, I WILL NOT bend to your stupid demands, SHUT UP!
 */
#ifndef PCB_Nonnull_Arg
#if PCB_COMPILER_GCC >= 30306 || PCB_COMPILER_CLANG >= 40000
#define PCB_Nonnull_Arg(...) __attribute__((nonnull(__VA_ARGS__)))
#ifndef PCB_HAS_NONNULL_ARG
#define PCB_HAS_NONNULL_ARG
#endif //PCB_HAS_NONNULL_ARG
#else
#define PCB_Nonnull_Arg(...)
#endif //compilers
#endif //PCB_Nonnull_Arg

#ifndef PCB_Nonnull_Return
#if PCB_COMPILER_GCC >= 40904 || PCB_COMPILER_CLANG >= 40000
#define PCB_Nonnull_Return __attribute__((returns_nonnull))
#else
#define PCB_Nonnull_Return
#endif //compilers
#endif //PCB_Nonnull_Return

#ifndef PCB_BeforeMain
#ifdef __cplusplus
//Using C++'s constructor trickery we can construct
//an empty object with a static lifetime, which means
//running a function at startup.
#define PCB_BeforeMain(f) \
static void f(void); \
struct f##_i { f##_i() { f(); } }; static f##_i f##_i_; \
static void f(void)
#else //C
#if PCB_COMPILER_MSVC
//https://stackoverflow.com/questions/1113409/attribute-constructor-equivalent-in-vc
//https://github.com/nodejs/node/issues/41852
#define PCB_INITIALIZER_(f,p) PCB_DO_PRAGMA(section(".CRT$XCU",read)) \
static void f(void); \
__declspec(allocate(".CRT$XCU")) void (*f##_)(void) = f; \
__pragma(comment(linker,"/include:" p #f "_")) \
static void f(void)
#ifdef _WIN64
#define PCB_BeforeMain(f) PCB_INITIALIZER_(f,"")
#else
#define PCB_BeforeMain(f) PCB_INITIALIZER_(f,"_")
#endif
#elif PCB_COMPILER_GCC || PCB_COMPILER_CLANG
#define PCB_BeforeMain(f) static __attribute__((constructor)) void f(void)
#else
#define PCB_BeforeMain(f) \
_Pragma("PCB Warning: function '" #f "' will not run before main because the compiler used does not support it") \
static void f(void)
#endif //Compilers
#endif //C++?
#endif //PCB_BeforeMain

#ifndef PCB_AfterMain
#ifdef __cplusplus
#define PCB_AfterMain(f) static void f(void); \
struct f##_o { ~f##_o() { f(); } }; static f##_o f##_o_; \
static void f(void)
#else
#if PCB_COMPILER_GCC || PCB_COMPILER_CLANG
#define PCB_AfterMain(f) static __attribute__((destructor)) void f(void)
#elif PCB_COMPILER_MSVC
#define PCB_AfterMain(f)                    \
static void f(void);                        \
PCB_BeforeMain(f##_register) { atexit(f); } \
static void f(void)
#else
#define PCB_AfterMain(f) \
_Pragma("PCB Warning: function '" #f "' will not run before main because the compiler used does not support it") \
static void f(void)
#endif //compilers
#endif //C++?
#endif //PCB_AfterMain

#ifndef PCB_InitFn
#define PCB_InitFn(f) PCB_BeforeMain(f)
#endif //PCB_InitFn
#ifndef PCB_DeinitFn
#define PCB_DeinitFn(f) PCB_AfterMain(f)
#endif //PCB_DeinitFn

#ifndef PCB_Unreachable
#if PCB_COMPILER_GCC >= 40500 || PCB_COMPILER_CLANG >= 30400
#define PCB_Unreachable __builtin_unreachable()
#elif PCB_COMPILER_MSVC
#define PCB_Unreachable (__assume(false))
#else
#if PCB_COMPILER_GCC || PCB_COMPILER_CLANG
#warning "PCB Warning: PCB_Unreachable does not mark unreachability"
#elif PCB_COMPILER_MSVC
#pragma message "PCB Warning: PCB_Unreachable does not mark unreachability"
#endif //compilers
#define PCB_Unreachable
#endif //Compilers
#endif //PCB_Unreachable

#ifndef PCB_Printf_Format
#if PCB_COMPILER_GCC || PCB_COMPILER_CLANG
#define PCB_Printf_Format(fmtIndex, rest) __attribute__((format(printf, fmtIndex, rest)))
#else
#define PCB_Printf_Format(fmtIndex, rest)
#endif //compilers
#endif //PCB_Printf_Format

#ifndef PCB_Cleanup
#ifdef PCB_WANT_CLEANUP
#if PCB_COMPILER_GCC || PCB_COMPILER_CLANG
#define PCB_Cleanup(f) __attribute__((cleanup(f)))
#else
#error "Cleanup attribute is unavailable with the current compiler or the compiler is not supported."
#define PCB_Cleanup(f)
#endif //Compilers
#endif //PCB_WANT_CLEANUP
#endif //PCB_Cleanup

//"thread" storage class specifier.
//Portable applications MUST check whether `PCB_thread_local` is #defined before use.
#ifndef PCB_thread_local
#ifndef PCB_RTLOAD
#if (defined(__cplusplus ) && __cplusplus+0 >= 201103L) || \
    (defined(__STDC_VERSION__) && __STDC_VERSION__+0 >= 202311L)
#define PCB_thread_local thread_local
#elif defined(__STDC_VERSION__) && __STDC_VERSION__+0 >= 201112L
#define PCB_thread_local _Thread_local
#elif PCB_COMPILER_GCC || PCB_COMPILER_CLANG
#define PCB_thread_local __thread
#elif PCB_COMPILER_MSVC
#define PCB_thread_local __declspec(thread)
#endif //different "thread-local"-ness sources
#endif //"thread-local"-ness for "plugins" is highly platform-dependent, better to disallow it completely
#endif //PCB_thread_local

//Get type of expression.
//Portable applications MUST check whether `PCB_Typeof` is #defined before use.
//In C++11+, `PCB_Typeof` expands to `decltype` if it is available.
//NOTE: Be *very* careful when using in external-facing declarations. This WILL cause problems!
#ifndef PCB_Typeof
#if defined(__cplusplus) && defined(__cpp_decltype) && __cpp_decltype+0 >= 200707L
#define PCB_Typeof(expr) decltype(expr)
#elif defined(__STDC_VERSION__)
#if PCB_COMPILER_GCC
#if PCB_COMPILER_GCC >= 130000 && __STDC_VERSION__ >= 202311L
#define PCB_Typeof(expr) typeof(expr)
#else
#define PCB_Typeof(expr) __typeof__(expr)
#endif //GCC 13+ && C23
#elif PCB_COMPILER_CLANG
#if PCB_COMPILER_CLANG >= 160000 && __STDC_VERSION__ >= 202311L
#define PCB_Typeof(expr) typeof(expr)
#else
#define PCB_Typeof(expr) __typeof__(expr)
#endif //Clang 16+ && C23
#elif PCB_COMPILER_MSVC
#if PCB_COMPILER_MSVC >= 1939 && __STDC_VERSION__ >= 202311L
#define PCB_Typeof(expr) typeof(expr)
#endif //VS 2022 17.9 && C23
#endif //compilers
#endif //C++ && __cpp_decltype || C
#endif //PCB_Typeof

//NOTE: For compatibility with older language versions, you need to *always*
//pass `msg`, but it may be an empty token, i.e. passing ',' after `expr`
//is enough to satisfy this macro.
#ifndef PCB_static_assert
#if defined(__cplusplus) && defined(__cpp_static_assert) && __cpp_static_assert+0 >= 200410L
#define PCB_static_assert(expr, msg) static_assert(expr, msg "")
#elif defined(__STDC_VERSION__)
#if __STDC_VERSION__+0 >= 202311L
#if PCB_COMPILER_GCC >= 90000 || PCB_COMPILER_CLANG >= 90000 || PCB_COMPILER_MSVC
#define PCB_static_assert(expr, msg) static_assert(expr, msg "")
#endif //GCC 9 || Clang 9 || surprisingly MSVC?
#elif __STDC_VERSION__ >= 201112L
//https://releases.llvm.org/3.0/docs/ClangReleaseNotes.html
#if PCB_COMPILER_GCC >= 40600 || PCB_COMPILER_CLANG >= 30000 || PCB_COMPILER_MSVC
#define PCB_static_assert(expr, msg) _Static_assert(expr, msg "")
#endif //GCC 4.6 || Clang 3 || surprisingly MSVC?
#endif //C23 || C11
#endif //C++ && __cpp_static_assert FTM || C

#ifndef PCB_static_assert
//Sorry, MSVC99 users, you'll have to live with "unused variable" warnings.
//It's only like this for MSVC99 because otherwise it'd make using this macro annoying for everyone.
//The workaround is as follows:
//PCB_PushWarnings()
//PCB_IgnoreWarning(4189) //or another warning
//PCB_static_assert(...);
//PCB_PopWarnings()
#define PCB_static_assert(expr, msg) \
    PCB_Unused static char PCB_MANGLE(static_assert_at_line)[(expr) ? 1 : -1]
#endif //hackish fallbacks
#endif //PCB_static_assert

#ifndef PCB_HAS_INCLUDE
#if (PCB_COMPILER_GCC >= 50000) || \
    (PCB_COMPILER_CLANG >= 30100) || \
    (defined(__STDC_VERSION__) && __STDC_VERSION__+0 >= 202311L) || \
    (defined(__cplusplus) && __cplusplus+0 >= 201703L)
#define PCB_HAS_INCLUDE __has_include
#else
#define PCB_HAS_INCLUDE 1 //assume that #include is available
#endif //whether __has_include is available
#endif //PCB_HAS_INCLUDE



//Section 1.5: Import libc, unless this macro is defined as 0
#ifndef PCB_USE_LIBC
#define PCB_USE_LIBC 1
#endif //PCB_USE_LIBC

#if defined(PCB_USE_LIBC) && PCB_USE_LIBC+0
//for "_s" functions
#ifndef __STDC_WANT_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 1
#endif //__STDC_WANT_LIB_EXT1__

#ifndef PCB_HAS_STDIO_H
#if PCB_HAS_INCLUDE(<stdio.h>)
#include <stdio.h>
#define PCB_HAS_STDIO_H
#endif //has stdio.h
#endif //PCB_HAS_STDIO_H

#ifndef PCB_HAS_STDLIB_H
#if PCB_HAS_INCLUDE(<stdlib.h>)
#include <stdlib.h>
#define PCB_HAS_STDLIB_H
#endif //has stdlib.h
#endif //PCB_HAS_STDLIB_H

#ifndef PCB_HAS_ASSERT_H
#if PCB_HAS_INCLUDE(<assert.h>)
#include <assert.h>
#define PCB_HAS_ASSERT_H
#endif //has assert.h
#endif //PCB_HAS_ASSERT_H

#ifndef PCB_HAS_STRING_H
#if PCB_HAS_INCLUDE(<string.h>)
#include <string.h>
#define PCB__HAS_STRING_H
#endif //has string.h
#if PCB_HAS_INCLUDE(<strings.h>)
#define PCB__HAS_STRINGS_H
#include <strings.h>
#endif //has strings.h
#if PCB_HAS_INCLUDE(<wchar.h>)
#include <wchar.h>
#define PCB__HAS_WCHAR_H
#endif //has wchar.h

#if defined(PCB__HAS_STRING_H)  && \
    defined(PCB__HAS_STRINGS_H) && \
    defined(PCB__HAS_STRING_H)
#define PCB_HAS_STRING_H
#endif

#endif //PCB_HAS_STRING_H

//These 4 should be universally available on every single platform in
//a conforming C99+ implementation.
//If they're somehow not, then WTF?! Are we programming an ENIAC?!
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#if defined(__STDC_VERSION__) && __STDC_VERSION__+0 >= 201112L && PCB_HAS_INCLUDE(<uchar.h>)
#include <uchar.h>
#define PCB_HAS_UCHAR_H
#endif //C11's uchar
//A useful command to list errno info: errno -l | sort -k2 -n
#if PCB_HAS_INCLUDE(<errno.h>)
#include <errno.h>
#define PCB_HAS_ERRNO_H
#else
#ifndef errno
PCB_DeprecatedReason("errno is unavailable, this is a stub.") extern int errno_stub;
#define errno errno_stub
#endif //errno stub
#endif //has errno.h
#if PCB_HAS_INCLUDE(<inttypes.h>)
#include <inttypes.h>
#define PCB_HAS_INTTYPES_H
#endif //has inttypes.h
#if PCB_HAS_INCLUDE(<ctype.h>)
#include <ctype.h>
#define PCB_HAS_CTYPE_H
#endif //has ctype.h
#if PCB_HAS_INCLUDE(<time.h>)
#include <time.h>
#define PCB_HAS_TIME_H
#endif //has time.h
#else
//fallback for no booleans
#if !defined(__cplusplus) && defined(__STDC_VERSION__) && \
    __STDC_VERSION__+0 < 202311L && !defined(bool)
#ifndef PCB_BOOL_LOCALLY_DEFINED
#define PCB_BOOL_LOCALLY_DEFINED
#define bool _Bool
#ifndef true
#define true 1
#endif //true
#ifndef false
#define false 0
#endif //false
#endif //PCB_BOOL_LOCALLY_DEFINED
#endif //bool

#endif //PCB_USE_LIBC?

//Section 1.6: Define functions/macros that the library uses from libc.
#ifndef PCB_realloc
#ifdef PCB_HAS_STDLIB_H
#define PCB_realloc(oldPtr, newSize) (errno = 0, realloc(oldPtr, newSize))
#define PCB_realloc_failed (errno != 0)
#else
#error "PCB Error: PCB requires PCB_realloc defined, but none is available. Perhaps you can't use libc, in which case you need to #define it manually."
#define PCB_realloc(oldPtr, newSize) NULL //stub
#define PCB_realloc_failed 1 //stub
#endif //PCB_HAS_STDLIB_H
#endif //PCB_realloc

#ifndef PCB_free
#ifdef PCB_HAS_STDLIB_H
#define PCB_free(ptr) free(ptr)
#else
#error "PCB Error: PCB requires PCB_free defined, but none is available. Perhaps you can't use libc, in which case you need to #define it manually."
#define PCB_free(ptr) //stub
#endif //PCB_HAS_STDLIB_H
#endif //PCB_free

#ifndef PCB_memcpy
#ifdef PCB_HAS_STRING_H
#define PCB_memcpy memcpy
#endif //PCB_HAS_STRING_H
#endif //PCB_memcpy

#ifndef PCB_memmove
#ifdef PCB_HAS_STRING_H
#define PCB_memmove memmove
#endif //PCB_HAS_STRING_H
#endif //PCB_memmove

#ifndef PCB_memset
#ifdef PCB_HAS_STRING_H
#define PCB_memset memset
#endif //PCB_HAS_STRING_H
#endif //PCB_memset

#ifndef PCB_memcmp
#ifdef PCB_HAS_STRING_H
#define PCB_memcmp memcmp
#endif //PCB_HAS_STRING_H
#endif //PCB_memcmp

#ifndef PCB_strcmp
#ifdef PCB_HAS_STRING_H
#define PCB_strcmp strcmp
#endif //PCB_HAS_STRING_H
#endif //PCB_strcmp

#ifndef PCB_strncmp
#ifdef PCB_HAS_STRING_H
#define PCB_strncmp strncmp
#endif //PCB_HAS_STRING_H
#endif //PCB_strncmp

#ifndef PCB_strncasecmp
#ifdef PCB__HAS_STRINGS_H
#define PCB_strncasecmp strncasecmp
#endif //PCB__HAS_STRINGS_H
#endif //PCB_strncasecmp

#ifndef PCB_strlen
#ifdef PCB_HAS_STRING_H
#define PCB_strlen strlen
#endif //PCB_HAS_STRING_H
#endif //PCB_strlen

#ifndef PCB_strnlen
#if defined(PCB_HAS_STRING_H)
#ifdef __GLIBC__
#if __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 10
#if defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE+0 >= 200809L
#define PCB_strnlen strnlen
#endif //POSIX source of strnlen
#else
#if defined(_GNU_SOURCE)
#define PCB_strnlen strnlen
#endif //GNU source of strnlen
#endif //glibc 2.10 checks
#endif //glibc
#endif //PCB_HAS_STRING_H
#endif //PCB_strnlen

#ifndef PCB_isspace
#ifdef PCB_HAS_CTYPE_H
#define PCB_isspace isspace
#endif //PCB_HAS_CTYPE_H
#endif //PCB_isspace

#ifdef PCB_HAS_STDIO_H
//MSVC, again being an outlier, doesn't natively support positional argument
//syntax. Therefore we need to #define macros specifically for it...smh.
//https://learn.microsoft.com/en-us/cpp/c-runtime-library/printf-p-positional-parameters
//PCB_printf is not used by the library, but is provided anyway since
//`printf` is the most widely used function from the printf family.
#ifndef PCB_printf
#if PCB_COMPILER_MSVC
#define PCB_printf _printf_p
#else
#define PCB_printf printf
#endif //MSVC scheiße
#endif //PCB_printf

#ifndef PCB_fprintf
#if PCB_COMPILER_MSVC
#define PCB_fprintf _fprintf_p
#else
#define PCB_fprintf fprintf
#endif //MSVC scheiße
#endif //PCB_fprintf

#ifndef PCB_vfprintf
#if PCB_COMPILER_MSVC
#define PCB_vfprintf _vfprintf_p
#else
#define PCB_vfprintf vfprintf
#endif //MSVC scheiße
#endif //PCB_vfprintf

#ifndef PCB_snprintf
#if PCB_COMPILER_MSVC
//This one is weird. It's named as if it was sprintf, but takes in buffer size...
#define PCB_snprintf _sprintf_p
#else
#define PCB_snprintf snprintf
#endif //MSVC scheiße
#endif //PCB_snprintf

#ifndef PCB_vsnprintf
#if PCB_COMPILER_MSVC
//Another weird one.
#define PCB_vsnprintf _vsprintf_p
#else
#define PCB_vsnprintf vsnprintf
#endif //MSVC scheiße
#endif //PCB_vsnprintf

//wchar_t variants
#ifndef PCB_wprintf
#if PCB_COMPILER_MSVC
#define PCB_wprintf _wprintf_p
#else
#define PCB_wprintf wprintf
#endif //MSVC scheiße
#endif //PCB_wprintf

#ifndef PCB_fwprintf
#if PCB_COMPILER_MSVC
#define PCB_fwprintf _fwprintf_p
#else
#define PCB_fwprintf fwprintf
#endif //MSVC scheiße
#endif //PCB_fwprintf

#ifndef PCB_vfwprintf
#if PCB_COMPILER_MSVC
#define PCB_vfwprintf _vfwprintf_p
#else
#define PCB_vfwprintf vfwprintf
#endif //MSVC scheiße
#endif //PCB_vfwprintf

//This one is even weirder than `snprintf`. There is no `snwprintf`/`swprintf`
//pair, only `swprintf` that functions like `snprintf`.
#ifndef PCB_swprintf
#if PCB_COMPILER_MSVC
#define PCB_swprintf _swprintf_p
#else
#define PCB_swprintf swprintf
#endif //MSVC scheiße
#endif //PCB_swprintf

#ifndef PCB_vswprintf
#if PCB_COMPILER_MSVC
//Another weird one.
#define PCB_vswprintf _vswprintf_p
#else
#define PCB_vswprintf vswprintf
#endif //MSVC scheiße
#endif //PCB_vswprintf

#ifndef PCB_fflush
#define PCB_fflush fflush
#endif //PCB_fflush

#ifndef PCB_stdout
#define PCB_stdout stdout
#endif //PCB_stdout

#ifndef PCB_stderr
#define PCB_stderr stderr
#endif //PCB_stderr


#endif //PCB_HAS_STDIO_H

#ifndef PCB_assert
#ifdef PCB_HAS_ASSERT_H
#define PCB_assert(expr) assert(expr)
#define PCB__ASSERT_HANDLED
#else
#define PCB_assert(expr) ((expr) ? (void)0 : PCB__assert_fail(#expr, __FILE__, __LINE__, __func__))
#endif //sources of assert
#else
#define PCB__ASSERT_HANDLED //assume available
#endif //PCB_assert

//Section 1.7: Define other useful macros
//Section 1.7.1: General purpose macros
#ifndef PCB_TODO
#define PCB_TODO(msg) PCB_assert(0 && msg " not yet implemented")
#endif //PCB_TODO

#ifndef PCB_ARRAY_LEN
//Get `arr`'s length. `arr` must be an array, otherwise the result is incorrect.
#define PCB_ARRAY_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif //PCB_ARRAY_LEN

#ifndef PCB_SHIFT_UNCHECKED
//Bash-like shifting of `args` counted by `count`. If `count == 0`,
//the behavior is undefined.
#define PCB_SHIFT_UNCHECKED(count, args) (--(count), *(args)++)
#endif //PCB_SHIFT_UNCHECKED

#ifndef PCB_SHIFT
#ifdef PCB_DISABLE_ASSERT
#define PCB_SHIFT(count, args) PCB_SHIFT_UNCHECKED(count, args)
#else
//Bash-like shifting of `args` counted by `count`. Asserts that `count > 0`.
#define PCB_SHIFT(count, args) (PCB_assert((count) > 0), --(count), *(args)++)
#endif //PCB_DISABLE_ASSERT
#endif //PCB_SHIFT

#ifndef PCB__STRINGIFY
//Turn `x` preprocessor token into a string literal (helper).
#define PCB__STRINGIFY(x) #x
#endif //PCB__STRINGIFY

#ifndef PCB_STRINGIFY
//Turn `x` preprocessor token into a string literal.
#define PCB_STRINGIFY(x) PCB__STRINGIFY(x)
#endif //PCB_STRINGIFY

#ifndef PCB__CONCAT
//Concatenate `x` & `y` preprocessor tokens (helper).
#define PCB__CONCAT(x, y) x##y
#endif //PCB__CONCAT

#ifndef PCB_CONCAT
//Concatenate `x` & `y` preprocessor tokens.
#define PCB_CONCAT(x, y) PCB__CONCAT(x, y)
#endif //PCB_CONCAT

#ifndef PCB_LOC
#define PCB_LOC __FILE__ ":" PCB_STRINGIFY(__LINE__)
#endif //PCB_LOC

//Macro used for "mangling" names by adding the line number to the name.
//Portable and sufficient for most use cases.
//You may find this useful in your code, hence it's public.
#ifndef PCB_MANGLE
#define PCB_MANGLE(x) PCB_CONCAT(PCB_CONCAT(x, _), __LINE__)
#endif //PCB_MANGLE

//Macro used for actually mangling names by adding a unique value to the name.
//May not work with all compilers (GCC, Clang and MSVC all support it though).
//For traceability the line number is also present.
//You may find this useful in your code, hence it's public.
#ifndef PCB_MANGLE_STRONG
#define PCB_MANGLE_STRONG(x) PCB_CONCAT(PCB_CONCAT(PCB_MANGLE(x), _), __COUNTER__)
#endif //PCB_MANGLE_STRONG

//Macro controlling certain safety checks within the library.
#ifndef PCB_SAFETY_CHECKS
#define PCB_SAFETY_CHECKS 1
#endif //PCB_SAFETY_CHECKS

//Macro used for state validation within the library.
//You may find this useful in your code, hence it's public.
#ifndef PCB_CHECK
#if PCB_SAFETY_CHECKS < 0
//If a safety check fails, abort.
#define PCB_CHECK(cond, retValOnTrue) PCB_assert(!(cond))
#elif PCB_SAFETY_CHECKS <= 2
//Fail gracefully (default).
#define PCB_CHECK(cond, retValOnTrue) if(cond) return retValOnTrue
#elif PCB_SAFETY_CHECKS > 2
//Disable safety checks. Improves performance, but risks undefined behavior.
#define PCB_CHECK(cond, retValOnTrue)
#endif //different levels of `PCB_SAFETY_CHECKS`
#endif //PCB_CHECK

//Macro used in "methods" (in a C++ sense) to validate that the instance passed
//is non-NULL. Disabled by default.
#ifndef PCB_CHECK_SELF
#if PCB_SAFETY_CHECKS <= 0
#ifndef PCB_HAS_NONNULL_ARG
//Verify the instance argument. Action taken depends on `PCB_SAFETY_CHECKS`.
#define PCB_CHECK_SELF(self, retValOnTrue) PCB_CHECK((self) == NULL, retValOnTrue)
#else
//Compiler will hopefully diagnose it.
#define PCB_CHECK_SELF(self, retValOnTrue)
#endif //has nonnull compiler attribute
#else
//Do not verify the instance argument (default).
#define PCB_CHECK_SELF(self, retValOnTrue)
#endif //different levels of `PCB_SAFETY_CHECKS`
#endif //PCB_CHECK_SELF

#ifndef PCB_CHECK_NULL
#ifndef PCB_HAS_NONNULL_ARG
#if PCB_SAFETY_CHECKS < 0
//If a NULL check fails, abort.
#define PCB_CHECK_NULL(arg, retValOnTrue) PCB_assert((arg) != NULL)
#elif PCB_SAFETY_CHECKS <= 2
//Fail gracefully (default).
#define PCB_CHECK_NULL(arg, retValOnTrue) if((arg) == NULL) return retValOnTrue
#elif PCB_SAFETY_CHECKS > 2
//Disable NULL checks. Improves performance, but risks undefined behavior.
#define PCB_CHECK_NULL(arg, retValOnTrue)
#endif //different levels of `PCB_SAFETY_CHECKS`
#else
//Compiler hopefully will diagnose it at compile time.
#define PCB_CHECK_NULL(arg, retValOnTrue)
#endif //has nonnull compiler attribute
#endif //PCB_CHECK_NULL

//Macro used for pointer variables that should not point to the same memory,
//but it might not be the case for safety reasons.
#ifndef PCB_maybe_restrict
#if PCB_SAFETY_CHECKS > 1
//Assume the underlying thing is not aliased by any other pointer.
//Improves performance, but risks undefined behavior.
#define PCB_maybe_restrict PCB_restrict
#else
//Assume the underlying thing may be aliased by another pointer (default).
#define PCB_maybe_restrict
#endif //PCB_SAFETY_CHECKS > 1
#endif //PCB_maybe_restrict

#ifndef PCB_maybe_inline
#ifdef PCB_NO_INLINE_EXPORTS
#define PCB_maybe_inline PCBAPI
#else
#define PCB_maybe_inline static PCB_ForceInline
#endif //PCB_NO_INLINE_EXPORTS
#endif //PCB_maybe_inline

#ifndef PCB_swap
/**
 * @brief Swaps `a` and `b`. Both must be XORable lvalues.
 */
#define PCB_swap(a, b) do { (a) ^= (b); (b) ^= (a); (a) ^= (b); } while(0)
#endif //PCB_swap

#ifndef PCB_swap_ptr
/**
 * @brief Swaps pointers `a` and `b`. Both must be lvalues.
 */
#define PCB_swap_ptr(a, b) do {                           \
    *(uintptr_t*)&(a) = (uintptr_t)(a) ^ (uintptr_t)(b);  \
    *(uintptr_t*)&(b) = (uintptr_t)(b) ^ (uintptr_t)(a);  \
    *(uintptr_t*)&(a) = (uintptr_t)(a) ^ (uintptr_t)(b);  \
} while(0)
#endif //PCB_swap_ptr

#ifndef PCB_X1_5
#define PCB_X1_5(x) ((x)+(x)/2)
#endif //PCB_X1_5

#ifndef PCB_NOOP
//Macro used for replacing functions/macros that do something in a macro argument.
#define PCB_NOOP(...) ((void)0)
#endif //PCB_NOOP


#ifndef PCB_defer_varl
/**
 * @brief Set the `var` variable to `val` and jump to `label`.
 */
#define PCB_defer_varl(var, val, label) do { var = val; goto label; } while(0)
#endif //PCB_defer_varl

#ifndef PCB_defer_l
/**
 * @brief Set the `result` variable to `val` and jump to `label`.
 * Parameterized version of `PCB_defer_varl` for `label`.
 */
#define PCB_defer_l(val, label) PCB_defer_varl(result, val, label)
#endif //PCB_defer_l

#ifndef PCB_return_defer_var
/**
 * @brief Set the `var` variable to `val` and jump to `defer`.
 * Parameterized version of `PCB_return_defer` for `var`.
 * See `PCB_defer_varl` for a more generic version.
 */
#define PCB_return_defer_var(var, val) PCB_defer_varl(var, val, defer)
#endif //PCB_return_defer_var

#ifndef PCB_return_defer
/**
 * @brief Set the `result` variable to `val` and jump to `defer`.
 * Used for resource cleanup at function exit.
 *
 * This macro uses `result` as the return variable and `defer` as the final
 * cleanup label by convention.
 * See `PCB_return_defer_var` for a more generic version.
 *
 * You can safely #define this macro in your code to whatever is best suited
 * for you prior to #include'ing the library. The library uses a private
 * version of this macro.
 */
#define PCB_return_defer(val) PCB_return_defer_var(result, val)
#endif //PCB_return_defer

//Macro used for ensuring a particular storage type for enums.
#ifndef PCB_Enum
#if defined(__cpluslus) || (defined(__STDC_VERSION__) && __STDC_VERSION__+0 >= 202311L)
#define PCB_Enum(name, type) \
    enum name : type; typedef enum name name; enum name : type
#else
#define PCB_Enum(name, type) \
    typedef type name; enum
#endif
#endif //PCB_Enum


//Section 1.7.2: template<*> struct vector in C let's goooo
#ifndef PCB_VEC_INITIAL_CAPACITY
#define PCB_VEC_INITIAL_CAPACITY 64
#endif //PCB_VEC_INITIAL_CAPACITY

#ifndef PCB_Vec_realloc_fail_action
#ifdef PCB_VEC_REALLOC_SAFE
#define PCB_Vec_realloc_fail_action break
#else
#define PCB_Vec_realloc_fail_action \
    PCB_assert(0 && "Realloc failed (download more RAM lmao)")
#endif //PCB_VEC_REALLOC_SAFE
#endif //PCB_Vec_realloc_fail_action

#ifndef PCB_Vec_integer_overflow_action
#ifdef PCB_VEC_OVERFLOW_SAFE
#define PCB_Vec_integer_overflow_action break
#else
#define PCB_Vec_integer_overflow_action \
    PCB_assert(0 && "Integer overflow (download more address space lmao)")
#endif //PCB_VEC_REALLOC_SAFE
#endif //PCB_Vec_integer_overflow_action

#ifndef PCB_Vec_reserve
#ifdef __cplusplus
#define PCB_Vec_reserve(vec, howMany) do {                          \
    if((howMany) == 0) { break; }                                   \
    size_t PCB_MANGLE(c) = (vec)->capacity;                         \
    if(PCB_MANGLE(c) == 0) PCB_MANGLE(c) = PCB_VEC_INITIAL_CAPACITY;\
    while((vec)->length + (howMany) > PCB_MANGLE(c)) {              \
        PCB_MANGLE(c) *= 2;                                         \
    }                                                               \
    if(PCB_MANGLE(c) <= (vec)->capacity) { break; }                 \
    if(PCB_MANGLE(c) > (SIZE_MAX/2) / sizeof(*(vec)->data)) {       \
        PCB_Vec_integer_overflow_action;                            \
    } void* PCB_MANGLE(d) = (void*)PCB_realloc(                     \
        (vec)->data, PCB_MANGLE(c) * sizeof(*(vec)->data)           \
    ); if(PCB_MANGLE(d) == NULL) { PCB_Vec_realloc_fail_action; }   \
    PCB_memcpy(&(vec)->data, &PCB_MANGLE(d), sizeof(PCB_MANGLE(d)));\
    (vec)->capacity = PCB_MANGLE(c);                                \
} while(0)
#else
/**
 * @brief Reserves `howMany` additional slots in `vec`.
 *
 * If reallocation fails, `vec`'s capacity remains unchanged.
 */
#define PCB_Vec_reserve(vec, howMany) do {                          \
    if((howMany) == 0) { break; }                                   \
    size_t PCB_MANGLE(c) = (vec)->capacity;                         \
    if(PCB_MANGLE(c) == 0) PCB_MANGLE(c) = PCB_VEC_INITIAL_CAPACITY;\
    while((vec)->length + (howMany) > PCB_MANGLE(c)) {              \
        PCB_MANGLE(c) *= 2;                                         \
    }                                                               \
    if(PCB_MANGLE(c) <= (vec)->capacity) { break; }                 \
    if(PCB_MANGLE(c) > (SIZE_MAX/2) / sizeof(*(vec)->data)) {       \
        PCB_Vec_integer_overflow_action;                            \
    } void* PCB_MANGLE(d) = (void*)PCB_realloc(                     \
        (vec)->data, PCB_MANGLE(c) * sizeof(*(vec)->data)           \
    ); if(PCB_MANGLE(d) == NULL) { PCB_Vec_realloc_fail_action; }   \
    (vec)->data = PCB_MANGLE(d); (vec)->capacity = PCB_MANGLE(c);   \
} while(0)
#endif //C++
#endif //PCB_Vec_reserve

#ifndef PCB_Vec_reserve_check
#ifdef PCB_VEC_REALLOC_SAFE
#define PCB_Vec_reserve_check(vec, howMany) \
    { PCB_Vec_reserve(vec, howMany);  if(PCB_realloc_failed) break; }
#else
#define PCB_Vec_reserve_check(vec, howMany) PCB_Vec_reserve(vec, howMany)
#endif //PCB_VEC_REALLOC_SAFE
#endif //PCB_Vec_reserve_check

#ifndef PCB_Vec_free
#define PCB_Vec_free(vec) PCB_free((vec)->data)
#endif //PCB_Vec_free

#ifndef PCB_Vec_destroy
/**
 * @brief Destroys `vec`, i.e. frees its buffer and resets fields
 * to their default values (0 and NULL).
 */
#define PCB_Vec_destroy(vec) do {           \
    PCB_Vec_free(vec); (vec)->data = NULL;  \
    (vec)->length = (vec)->capacity = 0;    \
} while(0)
#endif //PCB_Vec_destroy

#ifndef PCB_Vec_append
/**
 * @brief Appends `item` to `vec`.
 *
 * If reallocation fails, `vec`'s capacity remains unchanged.
 */
#define PCB_Vec_append(vec, item) do {      \
    PCB_Vec_reserve_check(vec, 1);          \
    (vec)->data[(vec)->length++] = (item);  \
} while(0)
#endif //PCB_Vec_append

#ifndef PCB_Vec_append_multiple
/**
 * @brief Appends `howMany` `items` to `vec`.
 *
 * There is also `PCB_Vec_append_variadic`, which is more convenient in use,
 * but requires specifying the type of arguments provided since C doesn't have
 * type inference before C23 and `typeof` is a GNU extension, so it's not portable.
 */
#define PCB_Vec_append_multiple(vec, items, howMany) do {                       \
    PCB_Vec_reserve_check(vec, howMany);                                        \
    for(size_t PCB_MANGLE(j) = 0; PCB_MANGLE(j) < (howMany); PCB_MANGLE(j)++) { \
        (vec)->data[PCB_MANGLE(j) + (vec)->length] = (items)[PCB_MANGLE(j)];    \
    } (vec)->length += (howMany);                                               \
} while(0)
#endif //PCB_Vec_append_multiple

#ifndef PCB_Vec_append_variadic
/**
 * @brief Appends a variadic number of elements to `vec`.
 *
 * The `type` of a variadic argument is needed since C doesn't have
 * type inference before C23 and `typeof` is a GNU
 * extension that is not portable.
 */
#define PCB_Vec_append_variadic(vec, type, ...) do {                    \
    type PCB_MANGLE(items)[] = { __VA_ARGS__ };                         \
    size_t PCB_MANGLE(count) = PCB_ARRAY_LEN(PCB_MANGLE(items));        \
    PCB_Vec_append_multiple(vec, PCB_MANGLE(items), PCB_MANGLE(count)); \
} while(0)
#endif //PCB_Vec_append_variadic

#ifndef PCB_Vec_pop_unchecked
/**
 * @brief Pops the last element from `vec`.
 * If `vec->length == 0`, the behavior is undefined.
 */
#define PCB_Vec_pop_unchecked(vec) ((vec)->data[--(vec)->length])
#endif //PCB_Vec_pop_unchecked

#ifndef PCB_Vec_pop
#ifdef PCB_DISABLE_ASSERT
#define PCB_Vec_pop(vec) PCB_Vec_pop_unchecked(vec)
#else
/**
 * @brief Pops the last element from `vec`.
 */
#define PCB_Vec_pop(vec) \
    (PCB_assert((vec)->length > 0), (vec)->data[--(vec)->length])
#endif //PCB_DISABLE_ASSERT
#endif //PCB_Vec_pop

#ifndef PCB_Vec_last_unchecked
/**
 * @brief Returns a pointer to the last element of `vec`.
 * If `vec->length == 0`, the behavior is undefined.
 */
#define PCB_Vec_last_unchecked(vec) (&(vec)->data[(vec)->length - 1])
#endif //PCB_Vec_last_unchecked

#ifndef PCB_Vec_last
#ifdef PCB_DISABLE_ASSERT
#define PCB_Vec_last(vec) PCB_Vec_last_unchecked(vec)
#else
/**
 * @brief Returns a pointer to the last element of `vec`.
 */
#define PCB_Vec_last(vec) \
    (PCB_assert((vec)->length > 0), &(vec)->data[(vec)->length - 1])
#endif //PCB_DISABLE_ASSERT
#endif //PCB_Vec_last

#ifndef PCB_Vec_clear
/**
 * @brief Clears `vec`...which literally only resets its length.
 * Use `PCB_Vec_clear_d` if elements hold data that needs to be destroyed.
 */
#define PCB_Vec_clear(vec) ((vec)->length = 0)
#endif //PCB_Vec_clear

#ifndef PCB_Vec_clear_d
/**
 * @brief Executes `destructor(ptr-to-element)` on each element and sets
 * the number of elements to 0.
 */
#define PCB_Vec_clear_d(vec, destructor) do {                                   \
    for(size_t i = 0; i < (vec)->length; i++) { destructor(&(vec)->data[i]); }  \
    (vec)->length = 0;                                                          \
} while(0)
#endif //PCB_Vec_clear_d

//Aliases
#ifndef PCB_Vec_reset
#define PCB_Vec_reset(vec) PCB_Vec_clear(vec)
#endif //PCB_Vec_reset

#ifndef PCB_Vec_reset_d
#define PCB_Vec_reset_d(vec, destructor) PCB_Vec_clear_d(vec, destructor)
#endif //PCB_Vec_reset_d

//This (op) is kind of a stupid way to not make a 2nd macro, but if it works...
#ifndef PCB__Vec_check_index
#ifdef PCB_VEC_OOB_SAFE
#define PCB__Vec_check_index(vec, index, op) if((index) op (vec)->length) break
#else
#ifdef PCB_DISABLE_ASSERT
#define PCB__Vec_check_index(vec, index, op)
#else
#define PCB__Vec_check_index(vec, index, op) PCB_assert(!((index) op (vec)->length))
#endif //PCB_DISABLE_ASSERT
#endif //PCB_VEC_OOB_SAFE
#endif //PCB__Vec_check_index

#ifndef PCB_Vec_insert
/**
 * @brief Inserts `item` into `vec` at position `index`.
 *
 * If `index` > current length of `vec`, nothing happens.
 */
#define PCB_Vec_insert(vec, item, index) do {   \
    const size_t PCB_MANGLE(i) = (index);       \
    PCB__Vec_check_index(vec, PCB_MANGLE(i), >);\
    PCB_Vec_reserve_check(vec, 1);              \
    PCB_memmove(                                \
        (vec)->data + PCB_MANGLE(i) + 1,        \
        (vec)->data + PCB_MANGLE(i),            \
        ((vec)->length - PCB_MANGLE(i)) *       \
            sizeof(*(vec)->data)                \
    );                                          \
    (vec)->data[PCB_MANGLE(i)] = (item);        \
    ++(vec)->length;                            \
} while(0)
#endif //PCB_Vec_insert

#ifndef PCB_Vec_erase
/**
 * @brief Erases the element at index `index` from `vec`.
 *
 * If the element holds any important value, it needs to be copied beforehand.
 * If `index` >= current length of `vec`, nothing happens.
 */
#define PCB_Vec_erase(vec, index) do {              \
    const size_t PCB_MANGLE(i) = (index);           \
    PCB__Vec_check_index(vec, PCB_MANGLE(i), >=);   \
    PCB_memmove(                                    \
        (vec)->data + PCB_MANGLE(i),                \
        (vec)->data + PCB_MANGLE(i) + 1,            \
        ((vec)->length - PCB_MANGLE(i) - 1) *       \
            sizeof(*(vec)->data)                    \
    ); --(vec)->length;                             \
}
#endif //PCB_Vec_erase

#ifndef PCB_Vec_swap
/**
 * @brief Swap 2 `vec`s `v1` and `v2`. Both must be lvalues.
 */
#define PCB_Vec_swap(v1, v2) do {               \
    PCB_swap_ptr((v1)->data, (v2)->data);       \
    PCB_swap((v1)->length,   (v2)->length);     \
    PCB_swap((v1)->capacity, (v2)->capacity);   \
} while(0)
#endif //PCB_Vec_swap

// #ifndef PCB_Vec_move
// #define PCB_Vec_move(vecFrom, vecTo) do {
//     PCB_assert((vecTo)->data == NULL && "Cannot move into non-empty vector.");
//     *(vecTo) = *(vecFrom); *(vecFrom) = PCB_ZEROED;
// } while(0)
// #endif //PCB_Vec_move

#ifndef PCB_Vec_isEmpty
#define PCB_Vec_isEmpty(vec) ((vec)->data == NULL || (vec)->length == 0)
#endif //PCB_Vec_isEmpty

#ifndef PCB_Vec_isFull
#define PCB_Vec_isFull(vec) ((vec)->length == (vec)->capacity)
#endif //PCB_Vec_isFull

#ifndef PCB_Vec_bytelen
#define PCB_Vec_bytelen(vec) (sizeof(*(vec)->data) * (vec)->length)
#endif //PCB_Vec_bytelen

#ifndef PCB_Vec_bytecap
#define PCB_Vec_bytecap(vec) (sizeof(*(vec)->data) * (vec)->capacity)
#endif //PCB_Vec_bytecap

#ifndef PCB_Vec_forEach
/**
 * @brief Executes `expr` on every element of `vec`.
 *
 * `expr` can be a function receiving a pointer to the element
 * or a macro for inline expressions.
 * An example usage with a macro is as follows:
 * ```c
 * //struct with `data`, `length` and `capacity` fields
 * typedef struct {
 *   int* data;
 *   size_t length;
 *   size_t capacity;
 * } Ints v = {0};
 * ...
 * #define EXPR(ptr) printf("%d ", *ptr)
 * PCB_Vec_forEach(&v, EXPR);
 * #undef EXPR
 * ...
 * ```
 */
#define PCB_Vec_forEach(vec, expr)      \
for(                                    \
    size_t PCB_MANGLE(i) = 0;           \
    PCB_MANGLE(i) < (vec)->length;      \
    PCB_MANGLE(i)++                     \
) { expr(&(vec)->data[PCB_MANGLE(i)]); }
#endif //PCB_Vec_forEach

#ifndef PCB_Vec_forEach_it
#ifdef PCB_Typeof
#define PCB_Vec_forEach_it(vec, itName, ...)        \
for(                                                \
    PCB_Typeof((vec)->data) itName = (vec)->data;   \
    itName != (vec)->data + (vec)->length; itName++ \
)
#else
/**
 * @brief Traditional for-each with an iterator.
 * Adding elements is not allowed as it may invalidate the iterator.
 *
 * An example usage is as follows:
 * ```c
 * Vec_int v_new = {0};
 * ...
 * PCB_Vec_forEach_it(&v, it, int) {
 *  (*it) += 69;
 * }
 * ...
 * ```
 */
#define PCB_Vec_forEach_it(vec, itName, underlyingType) \
for(                                                    \
    underlyingType *itName = (vec)->data;               \
    itName != (vec)->data + (vec)->length; itName++     \
)
#endif //PCB_Typeof?
#endif //PCB_Vec_forEach_it

#ifndef PCB_Vec_enumerate
#ifdef PCB_Typeof
#define PCB_Vec_enumerate(vec, i, it, enumPair, ...)                                \
for(                                                                                \
    struct { size_t i; PCB_Typeof((vec)->data) it; } enumPair = { 0, (vec)->data }; \
    enumPair.i < (vec)->length; enumPair.i++, enumPair.it++                         \
)
#else
/**
 * @brief Enumerate `vec` with index `i` and pointer-to-element `it`.
 * Due to limitations of C, `i` and `it` have to be wrapped inside a struct
 * named `enumPair`.
 * Adding elements is not allowed as it may invalidate the iterator.
 *
 * An example usage is as follows
 * ```c
 * typedef struct {
 *     const char* const* data;
 *     size_t length;
 *     size_t capacity;
 * } CStrings;
 * ...
 * CStrings cstrs = {0};
 * ...
 * PCB_Vec_enumerate(&cstrs, i, it, iter, const char*) {
 *     printf("%4lu | %s\n", iter.i, iter.it);
 * }
 * ```
 */
#define PCB_Vec_enumerate(vec, i, it, enumPair, type)               \
for(                                                                \
    struct { size_t i; type *it; } enumPair = { 0, (vec)->data };   \
    enumPair.i < (vec)->length; enumPair.i++, enumPair.it++         \
)
#endif //PCB_Typeof?
#endif //PCB_Vec_enumerate

#ifndef PCB__Vec_erase_if_rd
#define PCB__Vec_erase_if_rd(vec, rStart, rEnd, predicate, destructor, maybeAssert)   \
do {                                                                            \
    maybeAssert((ssize_t)(rStart) <= (ssize_t)(rEnd));                          \
    maybeAssert((ssize_t)(rEnd)   <= (ssize_t)(vec)->length);                   \
    if((vec)->length == 0) break;                                               \
    size_t PCB_MANGLE(cur), PCB_MANGLE(start), PCB_MANGLE(gapStart);            \
    PCB_MANGLE(cur) = PCB_MANGLE(start) = PCB_MANGLE(gapStart) = (rStart);      \
    const size_t PCB_MANGLE(L) = (rEnd);                                        \
    int PCB_MANGLE(v) = (predicate((vec)->data + PCB_MANGLE(cur))+0);           \
    while(PCB_MANGLE(cur) < PCB_MANGLE(L)) {                                    \
        PCB_MANGLE(start) = PCB_MANGLE(cur);                                    \
        while(PCB_MANGLE(v)) {                                                  \
            if(++PCB_MANGLE(cur) >= PCB_MANGLE(L)) goto PCB_MANGLE(e1);         \
            PCB_MANGLE(v) = (predicate((vec)->data + PCB_MANGLE(cur))+0);       \
        }                                                                       \
        const size_t PCB_MANGLE(end) = PCB_MANGLE(cur);                         \
        while(!PCB_MANGLE(v) && ++PCB_MANGLE(cur) < PCB_MANGLE(L))              \
            PCB_MANGLE(v) = (predicate((vec)->data + PCB_MANGLE(cur))+0);       \
        for(                                                                    \
            size_t PCB_MANGLE(i) = PCB_MANGLE(start);                           \
            PCB_MANGLE(i) < PCB_MANGLE(end); PCB_MANGLE(i)++                    \
        ) { destructor((vec)->data + PCB_MANGLE(i)); }                          \
        PCB_memmove(                                                            \
            (vec)->data + PCB_MANGLE(gapStart), (vec)->data + PCB_MANGLE(end),  \
            sizeof(*(vec)->data) * (PCB_MANGLE(cur) - PCB_MANGLE(end))          \
        ); PCB_MANGLE(gapStart) += PCB_MANGLE(cur) - PCB_MANGLE(end);           \
    } goto PCB_MANGLE(e);                                                       \
PCB_MANGLE(e1):                                                                 \
    for(                                                                        \
        size_t PCB_MANGLE(i) = PCB_MANGLE(start);                               \
        PCB_MANGLE(i) < PCB_MANGLE(cur); PCB_MANGLE(i)++                        \
    ) { destructor((vec)->data + PCB_MANGLE(i)); }                              \
PCB_MANGLE(e): (void)0;                                                         \
    size_t PCB_MANGLE(etm) = ((vec)->length - PCB_MANGLE(cur));                 \
    if(PCB_MANGLE(etm) > 0) {                                                   \
        PCB_memmove(                                                            \
            (vec)->data + PCB_MANGLE(gapStart), (vec)->data + PCB_MANGLE(cur),  \
            PCB_MANGLE(etm) * sizeof(*(vec)->data)                              \
        ); PCB_MANGLE(gapStart) += PCB_MANGLE(etm);                             \
    } (vec)->length = PCB_MANGLE(gapStart);                                     \
} while(0)
#endif //PCB__Vec_erase_if_rd

#ifndef PCB_Vec_erase_if_rd_unchecked
/**
 * Same as `PCB_Vec_erase_if_rd`, except bounds assertions are not performed.
 */
#define PCB_Vec_erase_if_rd_unchecked(vec, rangeStart, rangeEnd, predicate, destructor) \
    PCB__Vec_erase_if_rd(vec, rangeStart, rangeEnd, predicate, destructor, PCB_NOOP)
#endif //PCB_Vec_erase_if_rd_unchecked

#ifndef PCB_Vec_erase_if_rd
/**
 * @brief Removes elements from `vec`'s subrange [`rangeStart`, `rangeEnd`)
 * for which `predicate(ptr-to-element)` is true.
 * `predicate` must be an expression.
 * `destructor(ptr-to-element)` is executed on elements to be removed.
 * Use `PCB_Vec_erase_if_r` if a destructor is not needed.
 *
 * See `PCB_Vec_erase_if_d` for example usage.
 */
#define PCB_Vec_erase_if_rd(vec, rangeStart, rangeEnd, predicate, destructor) \
    PCB__Vec_erase_if_rd(vec, rangeStart, rangeEnd, predicate, destructor, PCB_assert)
#endif //PCB_Vec_erase_if_rd

#ifndef PCB_Vec_erase_if_r_unchecked
/**
 * Same as `PCB_Vec_erase_if_r`, except bounds assertions are not performed.
 */
#define PCB_Vec_erase_if_r_unchecked(vec, rangeStart, rangeEnd, predicate) \
    PCB_Vec_erase_if_rd_unchecked(vec, rangeStart, rangeEnd, predicate, PCB_NOOP)
#endif //PCB_Vec_erase_if_r_unchecked

#ifndef PCB_Vec_erase_if_r
/**
 * @brief Removes elements from `vec`'s subrange [`rangeStart`, `rangeEnd`)
 * for which `predicate(ptr-to-element)` is true.
 * `predicate` must be an expression.
 * Use `PCB_Vec_erase_if_rd` instead of this if `vec`'s elements require
 * explicit cleanup.
 *
 * See `PCB_Vec_erase_if_d` for example usage.
 */
#define PCB_Vec_erase_if_r(vec, rangeStart, rangeEnd, predicate) \
    PCB_Vec_erase_if_rd(vec, rangeStart, rangeEnd, predicate, PCB_NOOP)
#endif //PCB_Vec_erase_if_r

#ifndef PCB_Vec_erase_if_d_unchecked
/**
 * Identical to `PCB_Vec_erase_if_d`, provided for consistency with range variants.
 */
#define PCB_Vec_erase_if_d_unchecked(vec, predicate, destructor) \
    PCB_Vec_erase_if_rd_unchecked(vec, 0, (vec)->length, predicate, destructor)
#endif //PCB_Vec_erase_if_d_unchecked

#ifndef PCB_Vec_erase_if_d
/**
 * @brief Removes elements from `vec` for which `predicate(ptr-to-element)` is true.
 * `predicate` must be an expression.
 * `destructor(ptr-to-element)` is executed on elements to be removed.
 * Use `PCB_Vec_erase_if` if a destructor is not needed.
 *
 * An example usage is as follows:
 * ```c
 * typedef struct { char* data; size_t length; size_t capacity; } String;
 * typedef struct { String* data; size_t length; size_t capacity; } Strings;
 * ...
 * void String_destroy(String* str) { ... }
 * bool String_startsWith_cstr(String* str, const char* cstr) { ... }
 * ...
 *     Strings strs = {0};
 *     const char* prefix = "nob";
 *     ...
 * #define starts_with_prefix(pstr) String_startsWith_cstr((pstr), prefix)
 *     PCB_Vec_erase_if_rd(&strs, starts_with_nob, String_destroy);
 * #undef starts_with_prefix
 * ```
 */
#define PCB_Vec_erase_if_d(vec, predicate, destructor) \
    PCB_Vec_erase_if_rd(vec, 0, (vec)->length, predicate, destructor)
#endif //PCB_Vec_erase_if_d

#ifndef PCB_Vec_erase_if_unchecked
/**
 * Identical to `PCB_Vec_erase_if`, provided for consistency with range variants.
 */
#define PCB_Vec_erase_if_unchecked(vec, predicate) \
    PCB_Vec_erase_if_rd_unchecked(vec, 0, (vec)->length, predicate, PCB_NOOP)
#endif //PCB_Vec_erase_if_unchecked

#ifndef PCB_Vec_erase_if
/**
 * @brief Removes elements from `vec` for which `predicate(ptr-to-element)` is true.
 * `predicate` must be an expression.
 * Use `PCB_Vec_erase_if_d` instead of this if `vec`'s elements require
 * explicit cleanup.
 *
 * See `PCB_Vec_erase_if_d` for example usage.
 */
#define PCB_Vec_erase_if(vec, predicate) \
    PCB_Vec_erase_if_rd(vec, 0, (vec)->length, predicate, PCB_NOOP)
#endif //PCB_Vec_erase_if


//Section 1.7.3: Macros for C++ compatibility
#ifndef PCB_ZEROED
#if defined(__cplusplus) || (defined(__STDC_VERSION__) && __STDC_VERSION__+0 >= 202311L)
#define PCB_ZEROED {}
#else
#define PCB_ZEROED {0}
#endif //C++ || >=C23
#endif //PCB_ZEROED

#ifndef PCB_ZEROED_T
#ifdef __cplusplus
#define PCB_ZEROED_T(T) T{}
#else
#define PCB_ZEROED_T(T) (T)PCB_ZEROED
#endif //C++?
#endif //PCB_ZEROED_T

#ifndef PCB_CLITERAL
#ifdef __cplusplus
#define PCB_CLITERAL(Type) Type
#else
#define PCB_CLITERAL(Type) (Type)
#endif //C++
#endif //PCB_CLITERAL

#ifndef PCB_extern_C
#ifdef __cplusplus
#define PCB_extern_C extern "C"
#else
#define PCB_extern_C
#endif //C++
#endif //PCB_extern_C


//Section 1.7.4: Macros for views, slices
#ifndef PCB_View_Vec_unchecked
/**
 * @brief Constructs a view on vector `vec` in the range of
 * [`start`, `end`). If `vec->data == NULL` or `start > end` or
 * `end > vec->length`, the behavior is undefined.
 */
#define PCB_View_Vec_unchecked(vec, start, end) \
    { (vec)->data + (start), (end) - (start) }
#endif //PCB_View_Vec_unchecked

#ifndef PCB_View_Arr_unchecked
/**
 * @brief Constructs a view on array (!) `arr` in the range of [`start`, `end`).
 * Used similarly to `PCB_View_Vec_unchecked`, except it's only valid for arrays,
 * like `int arr[16];`.
 * If `start > end`, the behavior is undefined.
 */
#define PCB_View_Arr_unchecked(arr, start, end) \
    { &(arr)[start], (end) - (start) }
#endif //PCB_View_Arr_unchecked

#ifndef PCB_View_Ptr_unchecked
/**
 * @brief Constructs a view on a pointer `ptr` in the range of [`start`, `end`).
 * If `ptr == NULL || start > end` or `end` goes out of bounds,
 * the behavior is undefined.
 */
#define PCB_View_Ptr_unchecked(ptr, start, end) \
    { (ptr) + (start), (end) - (start) }
#endif //PCB_View_Ptr_unchecked

#ifndef PCB_View_Vec
#ifdef PCB_DISABLE_ASSERT
#define PCB_View_Vec(vec, start, end) PCB_View_Vec_unchecked(vec, start, end)
#else
/**
 * @brief Constructs a view on vector `vec` in the range of
 * [`start`, `end`).
 *
 * Example:
 * ```c
 * typedef struct { int* data; size_t length; size_t capacity; } Ints;
 * typedef struct { const int* data; size_t length; } Ints_view;
 * ...
 * Ints my_cool_integers = {0};
 * ...
 * Ints_view look_at_my_integers = PCB_View_Vec(&my_cool_integers, 6, 9);
 * ```
 */
#define PCB_View_Vec(vec, start, end)  {                                \
    ((end) - (start) != 0 ? PCB_assert((vec)->data != NULL) : (void)0,  \
     (vec)->data + (start)),                                            \
    (PCB_assert((ssize_t)(start) <= (ssize_t)(end)),                    \
     PCB_assert((ssize_t)(end)   <= (ssize_t)(vec)->length),            \
     (end) - (start))                                                   \
}
//For those wondering about casts to ssize_t: it's to suppress warnings about
//"unsigned comparison with 0 is always <true/false>" when passing literals
//to `start`/`end`. There is no other way around it.
#endif //PCB_DISABLE_ASSERT
#endif //PCB_View_Vec

#ifndef PCB_View_Arr
#ifdef PCB_DISABLE_ASSERT
#define PCB_View_Arr(arr, start, end) PCB_View_Arr_unchecked(arr, start, end)
#else
/**
 * @brief Constructs a view on array (!) `arr` in the range of [`start`, `end`).
 * Used similarly to `PCB_View_Vec`, except it's only valid for arrays,
 * like `int arr[16];`.
 */
#define PCB_View_Arr(arr, start, end) {                     \
     &(arr)[start],                                         \
    (PCB_assert((ssize_t)(start) <= (ssize_t)(end)),        \
     PCB_assert((ssize_t)(end)   <= PCB_ARRAY_LEN(arr)),    \
     (end) - (start))                                       \
}
#endif //PCB_DISABLE_ASSERT
#endif //PCB_View_Arr

#ifndef PCB_View_Ptr
#ifdef PCB_DISABLE_ASSERT
#define PCB_View_Ptr(ptr, start, end) PCB_View_Ptr_unchecked(ptr, start, end)
#else
/**
 * @brief Constructs a view on pointer `ptr` in the range of [`start`, `end`).
 * If `end` goes out of bounds, the behavior is undefined.
 */
#define PCB_View_Ptr(ptr, start, end) {                         \
    ((end) - (start) != 0 ? PCB_assert((ptr) != NULL) : (void)0,\
     (ptr) + (start)),                                          \
    (PCB_assert((ssize_t)(start) <= (ssize_t)(end)),            \
     (end) - (start))                                           \
}
#endif //PCB_DISABLE_ASSERT
#endif //PCB_View_Ptr


#ifndef PCB_View_Vec_T
/**
 * @brief Constructs a view of type `viewType` on vector `vec` in the range of
 * [`start`, `end`).
 * Primarily meant to be used when passing views by value to functions.
 * Example:
 * ```c
 * typedef struct { int* data; size_t length; size_t capacity; } Ints;
 * typedef struct { const int* data; size_t length; } Ints_view;
 * void print_ints(Ints_view ints) {
 *     for(size_t i = 0; i < ints.length; i++) {
 *         printf("%d ", ints.data[i]);
 *     }
 *     printf("\n");
 * }
 * ...
 * Ints my_cool_integers = {0};
 * ...
 * print_ints(PCB_View_Vec_T(&my_cool_integers, 6, 9, Ints_view));
 * ```
 */
#define PCB_View_Vec_T(vec, start, end, viewType) \
    (PCB_CLITERAL(viewType) PCB_View_Vec(vec, start, end))
#endif //PCB_View_Vec_T

#ifndef PCB_View_Arr_T
/**
 * @brief Constructs a view of type `viewType` on array (!) `arr` in the range of
 * [`start`, `end`).
 * Used similarly to `PCB_View_Vec_T`, except it's only valid for arrays,
 * like `int arr[16];`.
 */
#define PCB_View_Arr_T(arr, start, end, viewType) \
    (PCB_CLITERAL(viewType) PCB_View_Arr(arr, start, end))
#endif //PCB_View_Arr_T

#ifndef PCB_View_Ptr_T
/**
 * @brief Constructs a view of type `viewType` on pointer `ptr` in the range of
 * [`start`, `end`).
 * If `end` goes out of bounds, the behavior is undefined.
 */
#define PCB_View_Ptr_T(ptr, start, end, viewType) \
    (PCB_CLITERAL(viewType) PCB_View_Ptr(ptr, start, end))
#endif //PCB_View_Ptr_T



#ifndef PCB_View_Vec_A
/**
 * @brief Constructs a view on the entire vector `vec`.
 */
#define PCB_View_Vec_A(vec) PCB_View_Vec(vec, 0, (vec)->length)
#endif //PCB_View_Vec_A

#ifndef PCB_View_Arr_A
/**
 * @brief Constructs a view on the entire array (!) `arr`.
 * Used similarly to `PCB_View_Vec_A`, except it's only valid for arrays,
 * like `int arr[16];`.
 */
#define PCB_View_Arr_A(arr) PCB_View_Arr(arr, 0, PCB_ARRAY_LEN(arr))
#endif //PCB_View_Arr_A

#ifndef PCB_View_Ptr_A
/**
 * @brief Constructs a view on pointer `ptr` in the range of [0, `end`).
 * This differs from both `PCB_View_Vec_A` and `PCB_View_Arr_A` where `end` is
 * known. In the case of pointers, you are the one that provides the upper bound.
 *
 * If `end` goes out of bounds, the behavior is undefined.
 */
#define PCB_View_Ptr_A(ptr, end) PCB_View_Ptr(ptr, 0, end)
#endif //PCB_View_Ptr_A


#ifndef PCB_View_Vec_A_T
/**
 * @brief Constructs a view of type `viewType` on the entire vector `vec`.
 */
#define PCB_View_Vec_A_T(vec, viewType) \
    PCB_View_Vec_T(vec, 0, (vec)->length, viewType)
#endif //PCB_View_Vec_A_T

#ifndef PCB_View_Arr_A_T
/**
 * @brief Constructs a view of type `viewType` on the entire array (!) `arr`.
 * Used similarly to `PCB_View_Vec_A_T`, except it's only valid for arrays,
 * like `int arr[16];`.
 */
#define PCB_View_Arr_A_T(arr, viewType) \
    PCB_View_Arr_T(arr, 0, PCB_ARRAY_LEN(arr), viewType)
#endif //PCB_View_Arr_A_T

#ifndef PCB_View_Ptr_A_T
/**
 * @brief Constructs a view of type `viewType` on a pointer `ptr`
 * in the range of [0, `end`).
 * This differs from both `PCB_View_Vec_A_T` and `PCB_View_Arr_A_T` where `end` is
 * known. In the case of pointers, you are the one that provides the upper bound.
 *
 * If `end` goes out of bounds, the behavior is undefined.
 */
#define PCB_View_Ptr_A_T(ptr, viewType, end) PCB_View_Ptr_T(ptr, 0, end, viewType)
#endif //PCB_View_Ptr_A_T



#ifndef PCB_Slice_Vec
/**
 * @brief Constructs a slice of vector `vec` in the range of
 * [`start`, `end`).
 * Functionally identical to `PCB_View_Vec`. Use with slice types for clarity.
 */
#define PCB_Slice_Vec(vec, start, end) PCB_View_Vec(vec, start, end)
#endif //PCB_Slice_Vec

#ifndef PCB_Slice_Arr
/**
 * @brief Constructs a slice of array (!) `arr` in the range of
 * [`start`, `end`).
 * Functionally identical to `PCB_View_Arr`. Use with slice types for clarity.
 */
#define PCB_Slice_Arr(arr, start, end) PCB_View_Arr(arr, start, end)
#endif //PCB_Slice_Arr

#ifndef PCB_Slice_Ptr
/**
 * @brief Constructs a slice of pointer `ptr` in the range of
 * [`start`, `end`).
 * Functionally identical to `PCB_View_Ptr`. Use with slice types for clarity.
 */
#define PCB_Slice_Ptr(ptr, start, end) PCB_View_Ptr(ptr, start, end)
#endif //PCB_Slice_Ptr


#ifndef PCB_Slice_Vec_T
/**
 * @brief Constructs a slice of type `sliceType` on vector `vec`
 * in the range of [`start`, `end`).
 * Functionally identical to `PCB_View_Vec_T`. Use with slice types for clarity.
 */
#define PCB_Slice_Vec_T(vec, start, end, sliceType) \
    PCB_View_Vec_T(vec, start, end, sliceType)
#endif //PCB_Slice_Vec_T

#ifndef PCB_Slice_Arr_T
/**
 * @brief Constructs a slice of type `sliceType` on array (!) `arr`
 * in the range of [`start`, `end`).
 * Functionally identical to `PCB_View_Arr_T`. Use with slice types for clarity.
 */
#define PCB_Slice_Arr_T(arr, start, end, sliceType) \
    PCB_View_Arr_T(arr, start, end, sliceType)
#endif //PCB_Slice_Arr_T

#ifndef PCB_Slice_Ptr_T
/**
 * @brief Constructs a slice of type `sliceType` on pointer `ptr`
 * in the range of [`start`, `end`).
 * Functionally identical to `PCB_View_Ptr_T`. Use with slice types for clarity.
 */
#define PCB_Slice_Ptr_T(ptr, start, end, sliceType) \
    PCB_View_Ptr_T(ptr, start, end, sliceType)
#endif //PCB_Slice_Ptr_T


#ifndef PCB_Slice_Vec_A
/**
 * @brief Constructs a slice on the entire vector `vec`.
 * Functionally identical to `PCB_View_Vec_A`. Use with slice types for clarity.
 */
#define PCB_Slice_Vec_A(vec) PCB_Slice_Vec(vec, 0, (vec)->length)
#endif //PCB_Slice_Vec_A

#ifndef PCB_Slice_Arr_A
/**
 * @brief Constructs a slice on the entire array (!) `arr`.
 * Functionally identical to `PCB_View_Arr_A`. Use with slice types for clarity.
 */
#define PCB_Slice_Arr_A(arr) PCB_Slice_Arr(arr, 0, PCB_ARRAY_LEN(arr))
#endif //PCB_Slice_Arr_A

#ifndef PCB_Slice_Ptr_A
/**
 * @brief Constructs a slice on pointer `ptr` in the range of [0, `end`).
 * This differs from both `PCB_Slice_Vec_A` and `PCB_Slice_Arr_A` where `end` is
 * known. In the case of pointers, you are the one that provides the upper bound.
 *
 * Functionally identical to `PCB_View_Ptr_A`. Use with slice types for clarity.
 */
#define PCB_Slice_Ptr_A(ptr, end) PCB_Slice_Ptr(ptr, 0, end)
#endif //PCB_Slice_Ptr_A


#ifndef PCB_Slice_Vec_A_T
/**
 * @brief Constructs a slice of type `sliceType` on the entire vector `vec`.
 * Functionally identical to `PCB_View_Vec_A_T`. Use with slice types for clarity.
 */
#define PCB_Slice_Vec_A_T(vec, sliceType) \
    PCB_Slice_Vec_T(vec, 0, (vec)->length, sliceType)
#endif //PCB_Slice_Vec_A_T

#ifndef PCB_Slice_Arr_A_T
/**
 * @brief Constructs a slice of type `sliceType` on the entire array (!) `arr`.
 * Functionally identical to `PCB_View_Arr_A_T`. Use with slice types for clarity.
 */
#define PCB_Slice_Arr_A_T(arr, sliceType) \
    PCB_Slice_Arr_T(arr, 0, PCB_ARRAY_LEN(arr), sliceType)
#endif //PCB_Slice_Arr_A_T

#ifndef PCB_Slice_Ptr_A_T
/**
 * @brief Constructs a slice of type `viewType` on a pointer `ptr`
 * in the range of [0, `end`).
 * This differs from both `PCB_Slice_Vec_A_T` and `PCB_Slice_Arr_A_T` where `end` is
 * known. In the case of pointers, you are the one that provides the upper bound.
 *
 * Functionally identical to `PCB_View_Ptr_A_T`. Use with slice types for clarity.
 */
#define PCB_Slice_Ptr_A_T(ptr, sliceType, end) \
    PCB_Slice_Ptr_T(ptr, 0, end, sliceType)
#endif //PCB_Slice_Ptr_A_T



//Section 1.7.5: Other macros
#ifndef PCB_VA_forEach_until
#define PCB_VA_forEach_until(args, argType, end, name)      \
for(                                                        \
    argType name = va_arg((args), argType);                 \
    name != (end);                                          \
    name = va_arg((args), argType)                          \
)
#endif //PCB_VA_forEach_until

#ifndef PCB_Arr_forEach_it
#ifdef PCB_Typeof
#define PCB_Arr_forEach_it(arr, itName, ...)            \
for(                                                    \
    PCB_Typeof(&(arr)[0]) itName = &(arr)[0];           \
    itName != &(arr)[PCB_ARRAY_LEN(arr)]; itName++      \
)
#else
#define PCB_Arr_forEach_it(arr, itName, underlyingType) \
for(                                                    \
    underlyingType *itName = &(arr)[0];                 \
    itName != &(arr)[PCB_ARRAY_LEN(arr)]; itName++      \
)
#endif //PCB_Typeof?
#endif //PCB_Arr_forEach_it

#ifndef PCB_Arr_enumerate
#ifdef PCB_Typeof
#define PCB_Arr_enumerate(arr, i, it, enumPair, type)                           \
for(                                                                            \
    struct { size_t i; PCB_Typeof(&(arr)[0]) it; } enumPair = { 0, &(arr)[0] }; \
    enumPair.i < PCB_ARRAY_LEN(arr); enumPair.i++, enumPair.it++                \
)
#else
#define PCB_Arr_enumerate(arr, i, it, enumPair, type)               \
for(                                                                \
    struct { size_t i; type *it; } enumPair = { 0, &(arr)[0] };     \
    enumPair.i < PCB_ARRAY_LEN(arr); enumPair.i++, enumPair.it++    \
)
#endif //PCB_Typeof?
#endif //PCB_Arr_enumerate

#ifndef PCB_Ptr_forEach_it
#ifdef PCB_Typeof
#define PCB_Ptr_forEach_it(ptr, n, itName, ptrType)                 \
for(                                                                \
    PCB_Typeof(ptr) itName = (ptr), PCB_MANGLE(end) = (ptr) + (n);  \
    itName != PCB_MANGLE(end); itName++                             \
)
#else
#define PCB_Ptr_forEach_it(ptr, n, itName, ptrType)                 \
for(                                                                \
    ptrType itName = (ptr), PCB_MANGLE(end) = (ptr) + (n);          \
    itName != PCB_MANGLE(end); itName++                             \
)
#endif //PCB_Typeof?
#endif //PCB_Ptr_forEach_it

#ifndef PCB_Ptr_enumerate
#ifdef PCB_Typeof
#define PCB_Ptr_enumerate(ptr, n, i, it, enumPair, type)        \
for(                                                            \
    struct { size_t i; PCB_Typeof(ptr) it, PCB_MANGLE(end); }   \
    enumPair = { 0, (ptr), (ptr) + (n) };                       \
    enumPair.it != enumPair.PCB_MANGLE(end);                    \
    enumPair.i++, enumPair.it++                                 \
)
#else
#define PCB_Ptr_enumerate(ptr, n, i, it, enumPair, type)    \
for(                                                        \
    struct { size_t i; type *it, *PCB_MANGLE(end); }        \
    enumPair = { 0, (ptr), (ptr) + (n) };                   \
    enumPair.it != enumPair.PCB_MANGLE(end);                \
    enumPair.i++, enumPair.it++                             \
)
#endif //PCB_Typeof?
#endif //PCB_Ptr_enumerate

#ifndef PCB_pause
#if PCB_ARCH_x86_64 || PCB_ARCH_i386
#if PCB_COMPILER_GCC || PCB_COMPILER_CLANG
#define PCB_pause __asm__ __volatile__ ("rep nop" ::: "memory")
#elif PCB_COMPILER_MSVC
#define PCB_pause __mm_pause()
#else
#define PCB_pause
#endif //compilers
#else
#define PCB_pause
#endif //architectures
#endif //PCB_pause


//Section 1.8: Import platform-specific header files
#if PCB_PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windef.h>
#include <winbase.h>
#include <wingdi.h>
#include <wincon.h>
#undef WIN32_LEAN_AND_MEAN
#elif PCB_PLATFORM_POSIX
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
// #include <sys/shm.h>
#include <sys/wait.h>
#include <dirent.h>
// #include <spawn.h>
#include <signal.h>
#else
#error PCB Error: platforms outside of Windows and POSIX ones are not supported right now.
#endif //platform-specific APIs



//Section 1.9: Import architecture-specific header files
#if PCB_ARCH_x86_64
#if PCB_COMPILER_MSVC
#include <intrin.h>
#elif PCB_COMPILER_GCC || PCB_COMPILER_CLANG
#endif //compilers
#endif //architectures



//Section 2: declarations of library structures, enums, functions, etc.
#ifdef PCB_BUILD_DYN
#ifndef PCB_DYN
#define PCB_DYN
#endif //PCB_DYN
#endif //PCB_BUILD_DYN implies PCB_DYN

#ifndef PCBAPI
#if PCB_PLATFORM_WINDOWS
#ifdef PCB_BUILD_DYN //we are building a DLL
#define PCBAPI __declspec(dllexport)
#elif defined(PCB_DYN) //we are *using* a DLL
#define PCBAPI __declspec(dllimport)
#endif //DLL-related options
#else
#ifdef PCB_BUILD_DYN
#define PCBAPI __attribute__((visibility("default")))
#endif //override visibility on non-Windows platforms
#endif //platform
#endif //PCBAPI

#ifndef PCBAPI
#define PCBAPI
#endif //PCBAPI

#ifndef PCBCALL
#if PCB_PLATFORM_WINDOWS
#define PCBCALL __cdecl
#else
#define PCBCALL
#endif //use C calling convention on Windows
#endif //PCBCALL, by default the C calling convention


//The user may not need declarations, for example if only macros are used.
//This macro allows for a slight preprocessor optimization by omitting declarations.
#ifndef PCB_NO_DECLARATIONS

typedef enum {
    PCB_LOGLEVEL_NONE,  PCB_LOGLEVEL_NONE_NL,  //without the prefix
    PCB_LOGLEVEL_TRACE, PCB_LOGLEVEL_TRACE_NL,
    PCB_LOGLEVEL_DEBUG, PCB_LOGLEVEL_DEBUG_NL,
    PCB_LOGLEVEL_INFO,  PCB_LOGLEVEL_INFO_NL,
    PCB_LOGLEVEL_WARN,  PCB_LOGLEVEL_WARN_NL,
    PCB_LOGLEVEL_ERROR, PCB_LOGLEVEL_ERROR_NL,
    PCB_LOGLEVEL_FATAL, PCB_LOGLEVEL_FATAL_NL
    //With '\n'         Without '\n'
} PCB_LogLevel;

//AESEQ stands for "ANSI escape sequence"
//Reference: https://stackoverflow.com/questions/4842424/list-of-ansi-color-escape-sequences
#ifndef PCB_AESEQ_RESET
//Remove all current effects.
#define PCB_AESEQ_RESET "\033[0m"
#endif //PCB_AESEQ_RESET

#ifndef PCB_AESEQ_BOLD
//Make text bold.
#define PCB_AESEQ_BOLD "\033[1m"
#endif //PCB_AESEQ_BOLD

#ifndef PCB_AESEQ_FG_RGB
//Set foreground (text) color to an RGB triplet.
#define PCB_AESEQ_FG_RGB(r, g, b) "\033[38;2;" PCB__STRINGIFY(r) ";" PCB__STRINGIFY(g) ";" PCB__STRINGIFY(b) "m"
#endif //PCB_AESEQ_FG_RGB

#ifndef PCB_AESEQ_BG_RGB
//Set background color to an RGB triplet.
#define PCB_AESEQ_BG_RGB(r, g, b) "\033[48;2;" PCB__STRINGIFY(r) ";" PCB__STRINGIFY(g) ";" PCB__STRINGIFY(b) "m"
#endif //PCB_AESEQ_BG_RGB

#ifndef PCB_AESEQ_UNDERLINE
//Make text underlined.
#define PCB_AESEQ_UNDERLINE "\033[4m"
#endif //PCB_AESEQ_UNDERLINE



typedef enum {
    //Unknown/unsupported file type.
    PCB_FILETYPE_UNKNOWN = 0x1,
    //Regular file.
    PCB_FILETYPE_REG = 0x2,
    //Directory.
    PCB_FILETYPE_DIR = 0x3,
    //Pipe, socket, etc. Further checks are platform-specific unfortunately.
    PCB_FILETYPE_STREAM = 0x4,
    //Character device, for example a console or some USB device.
    PCB_FILETYPE_CHAR = 0x5,
    //Block device, for example a hard drive.
    PCB_FILETYPE_BLK = 0x6,
    //Non-existent filesystem entry.
    PCB_FILETYPE_NONE = 0x10,
    //Symbolic link, always returned alongside another filetype.
    PCB_FILETYPE_SYMLINK = 0x20,

    //Unknown/unsupported file type that is pointed to via a symlink.
    PCB_FILETYPE_UNKNOWN_SYM = PCB_FILETYPE_UNKNOWN | PCB_FILETYPE_SYMLINK,
    //Regular file that is pointed to via a symlink.
    PCB_FILETYPE_REG_SYM = PCB_FILETYPE_REG | PCB_FILETYPE_SYMLINK,
    //Directory that is pointed to via a symlink.
    PCB_FILETYPE_DIR_SYM = PCB_FILETYPE_DIR | PCB_FILETYPE_SYMLINK,
    //Pipe, socket, etc. that is pointed to via a symlink.
    PCB_FILETYPE_STREAM_SYM = PCB_FILETYPE_STREAM | PCB_FILETYPE_SYMLINK,
    //Character device that is pointed to via a symlink.
    PCB_FILETYPE_CHAR_SYM = PCB_FILETYPE_CHAR | PCB_FILETYPE_SYMLINK,
    //Block device that is pointed to via a symlink.
    PCB_FILETYPE_BLK_SYM = PCB_FILETYPE_BLK | PCB_FILETYPE_SYMLINK,
    //Symlink that points to a non-existent filesystem entry.
    PCB_FILETYPE_NONE_SYM = PCB_FILETYPE_NONE | PCB_FILETYPE_SYMLINK,

    //An error occured while checking the type; to get the error code
    //call `PCB_GetError()`.
    PCB_FILETYPE_ERROR = 0,
    //A convenience value to strip away the symlink bit if one doesn't care.
    //Applied with bitwise AND to the target.
    PCB_FILETYPE_SYMLINK_IGN = ~PCB_FILETYPE_SYMLINK
} PCB_FileType;

#ifndef PCB_FS_DIR_DELIM
#if PCB_PLATFORM_WINDOWS
#define PCB_FS_DIR_DELIM '\\'
#elif PCB_PLATFORM_POSIX
#define PCB_FS_DIR_DELIM '/'
#else
#define PCB_FS_DIR_DELIM 0x15 //stub
#endif //platforms
#endif //PCB_FS_DIR_DELIM



/* A dynamic array of characters with a trailing zero at the end - a string.
 * Has a concrete implementation unlike other dynamic arrays.
 * The trailing zero is not included in its length.
 *
 * You can safely pass a `PCB_String*` to functions that expect a
 * `PCB_StringView*` since they share the prefix.
 *
 * You can, in most situations, safely pass a `const PCB_StringView*`
 * to functions that expect a `const PCB_String*`.
 *
 * If a function relates to memory management, it likely reads the `capacity`
 * field, which isn't present in a `PCB_StringView`, so you can't
 * pass it there (I mean, duh).
 *
 * For example, you can safely pass a `const PCB_StringView*` instead of
 * `const PCB_String*` into `PCB_String_append`, but you can't do that
 * with `PCB_String_clone`.
 */
typedef struct {
    char* data;
    size_t length; //in bytes, ignores variable length encoding of UTF-8
    size_t capacity;
} PCB_String;

/* A non-owning view at a portion of some string.
 * Likely does not end with a zero, keep that in mind when passing
 * `data` to a function expecting a C string.
 *
 * You can safely pass a `PCB_String*` to functions that expect a
 * `PCB_StringView*` since they share the prefix.
 *
 * You can, in most situations, safely pass a `const PCB_StringView*`
 * to functions that expect a `const PCB_String*`.
 *
 * If a function relates to memory management, it likely reads the `capacity`
 * field, which isn't present in a `PCB_StringView`, so you can't
 * pass it there (I mean, duh).
 *
 * For example, you can safely pass a `const PCB_StringView*` instead of
 * `const PCB_String*` into `PCB_String_append`, but you can't do that
 * with `PCB_String_clone`.
 */
typedef struct {
    const char* data;
    size_t length;
} PCB_StringView;

typedef struct {
    char* data;
    size_t length;
} PCB_StringSlice;

typedef struct {
    PCB_String* data;
    size_t length;
    size_t capacity;
} PCB_Strings;

typedef struct {
    PCB_StringView* data;
    size_t length;
    size_t capacity;
} PCB_StringViews;

typedef struct {
    const char** data;
    size_t length;
    size_t capacity;
} PCB_CStrings;

typedef struct {
    const char* const* data;
    size_t length;
} PCB_CStringsView;

typedef struct {
    const char* key;
    const char* value;
} PCB_CStringPair;

typedef struct {
    PCB_CStringPair* data;
    size_t length;
    size_t capacity;
} PCB_CStringPairs;

typedef struct {
    const PCB_CStringPair* data;
    size_t length;
} PCB_CStringPairsView;

#ifndef PCB_SV_Fmt
#define PCB_SV_Fmt "%.*s"
#endif //PCB_SV_Fmt
#ifndef PCB_SV_Arg
#define PCB_SV_Arg(sv) (int)(sv).length, (sv).data
#endif //PCB_SV_Arg

typedef PCB_CStrings PCB_ShellCommand;

#ifndef PCB_CStrings_append
#define PCB_CStrings_append(cstrs, str) PCB_Vec_append(cstrs, str)
#endif //PCB_CStrings_append
#ifndef PCB_ShellCommand_append_arg
#define PCB_ShellCommand_append_arg(cmd, arg) PCB_CStrings_append(cmd, arg)
#endif //PCB_ShellCommand_append_arg

#ifndef PCB_CStrings_append_many
#define PCB_CStrings_append_many(cstrs, ...) \
    PCB_Vec_append_variadic(cstrs, const char*, __VA_ARGS__)
#endif //PCB_CStrings_append_many
#ifndef PCB_ShellCommand_append_args
#define PCB_ShellCommand_append_args(cmd, ...) PCB_CStrings_append_many(cmd, __VA_ARGS__)
#endif //PCB_ShellCommand_append_args
#ifndef PCB_ShellCommand_append_n_args
#define PCB_ShellCommand_append_n_args(cmd, args, n) PCB_Vec_append_multiple(cmd, args, n)
#endif //PCB_ShellCommand_append_n_args



#ifndef PCB_char8
#if defined(__cplusplus)
#if defined(__cpp_char8_t) && __cpp_char8_t+0 >= 201811L
#define PCB_char8 char8_t
#else
#define PCB_char8 uint8_t
#endif //__cpp_char8_t FTM
#elif defined(__STDC_VERSION__)
#if __STDC_VERSION__+0 >= 202311L && defined(PCB_HAS_UCHAR_H)
#define PCB_char8 char8_t
#else
#define PCB_char8 uint8_t
#endif //C23 && <uchar.h>
#else
#define PCB_char8 unsigned char
#endif //language
#endif //PCB_char8

#ifndef PCB_char16
#if defined(__cplusplus)
#if defined(__cpp_unicode_characters) && __cpp_unicode_characters+0 >= 200704L
#define PCB_char16 char16_t
#else
#define PCB_char16 uint16_t
#endif //__cpp_unicode_characters FTM
#elif defined(__STDC_VERSION__)
#if __STDC_VERSION__+0 >= 201112L && defined(PCB_HAS_UCHAR_H)
#define PCB_char16 char16_t
#else
#define PCB_char16 uint16_t
#endif //C11?
#else
#define PCB_char16 unsigned short
#endif //language
#endif //PCB_char16

#ifndef PCB_char32
#if defined(__cplusplus)
#if defined(__cpp_unicode_characters) && __cpp_unicode_characters+0 >= 200704L
#define PCB_char32 char32_t
#else
#define PCB_char32 uint32_t
#endif //__cpp_unicode_characters FTM
#elif defined(__STDC_VERSION__)
#if __STDC_VERSION__+0 >= 201112L && defined(PCB_HAS_UCHAR_H)
#define PCB_char32 char32_t
#else
#define PCB_char32 uint32_t
#endif //C11?
#else
#define PCB_char32 unsigned long
#endif //language
#endif //PCB_char32

typedef struct { wchar_t*    data; size_t length; size_t capacity; } PCB_WString;
typedef struct { PCB_char8*  data; size_t length; size_t capacity; } PCB_U8String;
typedef struct { PCB_char16* data; size_t length; size_t capacity; } PCB_U16String;
typedef struct { PCB_char32* data; size_t length; size_t capacity; } PCB_U32String;

typedef struct { const wchar_t*    data; size_t length; } PCB_WStringView;
typedef struct { const PCB_char8*  data; size_t length; } PCB_U8StringView;
typedef struct { const PCB_char16* data; size_t length; } PCB_U16StringView;
typedef struct { const PCB_char32* data; size_t length; } PCB_U32StringView;

typedef struct { PCB_WString*   data; size_t length; size_t capacity; } PCB_WStrings;
typedef struct { PCB_U8String*  data; size_t length; size_t capacity; } PCB_U8Strings;
typedef struct { PCB_U16String* data; size_t length; size_t capacity; } PCB_U16Strings;
typedef struct { PCB_U32String* data; size_t length; size_t capacity; } PCB_U32Strings;

typedef struct { PCB_WStringView*   data; size_t length; size_t capacity; } PCB_WStringViews;
typedef struct { PCB_U8StringView*  data; size_t length; size_t capacity; } PCB_U8StringViews;
typedef struct { PCB_U16StringView* data; size_t length; size_t capacity; } PCB_U16StringViews;
typedef struct { PCB_U32StringView* data; size_t length; size_t capacity; } PCB_U32StringViews;

typedef struct { const wchar_t**    data; size_t length; size_t capacity; } PCB_WCStrings;
typedef struct { const PCB_char8**  data; size_t length; size_t capacity; } PCB_U8CStrings;
typedef struct { const PCB_char16** data; size_t length; size_t capacity; } PCB_U16CStrings;
typedef struct { const PCB_char32** data; size_t length; size_t capacity; } PCB_U32CStrings;

typedef struct { const wchar_t*    const* data; size_t length; } PCB_WCStringsView;
typedef struct { const PCB_char8*  const* data; size_t length; } PCB_U8CStringsView;
typedef struct { const PCB_char16* const* data; size_t length; } PCB_U16CStringsView;
typedef struct { const PCB_char32* const* data; size_t length; } PCB_U32CStringsView;

/**
 * Unicode codepoint.
 * `code` stores the actual scalar value, while `length` is the number of code
 * units to skip to get the next scalar value (using terminology from
 * Unicode ch03§3.9 Unicode Encoding Forms), depending on the encoding used.
 * For example, when decoding the UTF-8 string "ඞ" at byte 0, the structure
 * contains `code = 0xD9E, length = 3`.
 * Functions that return this structure define meaning of its fields on error.
 */
typedef struct {
    int32_t code;
    uint32_t length;
} PCB_Codepoint;



typedef struct {
#if PCB_PLATFORM_WINDOWS
    HANDLE handle;
#elif PCB_PLATFORM_POSIX
    pid_t handle;
    int status; //required due to wait(2) semantics
#else
    int handle; //stub
#endif //platform-dependent handles to processes
} PCB_Process;

#ifndef PCB_PROCESS_INVALID_HANDLE
#if PCB_PLATFORM_WINDOWS
#define PCB_PROCESS_INVALID_HANDLE INVALID_HANDLE_VALUE
#elif PCB_PLATFORM_POSIX
/* 0 is the kernel (or part of it), i.e. no userspace process can be 0.
 * Any negative PID is also invalid as defined by POSIX.
 * See https://pubs.opengroup.org/onlinepubs/9699919799/.
 */
#define PCB_PROCESS_INVALID_HANDLE -1
#else
#define PCB_PROCESS_INVALID_HANDLE 0 //stub
#endif //platform
#endif //PCB_PROCESS_INVALID_HANDLE

typedef struct {
    PCB_Process* data;
    size_t length;
    size_t capacity;
} PCB_Processes;



typedef struct PCB_Arena PCB_Arena;
typedef struct PCB_ArenaMark PCB_ArenaMark;
/**
 * @brief A prefix of `PCB_Arena` for metadata.
 *
 * You MAY cast `PCB_Arena*` to `PCB_Arena_Prefix*` and access the internal
 * structure directly. However:
 *
 * The library reserves the ability to make breaking changes
 * to this structure without them treated as a breaking change.
 * Previous incompatible versions of this structure shall be available
 * by requesting a specific version of the library.
 */
typedef struct {
    size_t length;
    size_t capacity;
    PCB_Arena* next;
} PCB_Arena_Prefix;


typedef enum {
    /* Unknown argv syntax, causes `PCB_BuildContext`'s options
     * to be passed without processing.
     */
    PCB_ARGVSYNTAX_UNKNOWN,
    //POSIX argv syntax ("-...(=...)", for example "-DLEVEL=2").
    PCB_ARGVSYNTAX_POSIX,
    //Microsoft argv syntax ("/...(:...)", for example "/DLEVEL:2").
    PCB_ARGVSYNTAX_MS,
    PCB_ARGVSYNTAX_COUNT
} PCB_ArgvSyntax;

/* Enum for compiler identification at *runtime* (hence the _RT suffix)
 * rather than compile time (without the _RT suffix).
 */
typedef enum {
    PCB_COMPILER_RT_UNKNOWN,
    PCB_COMPILER_RT_GCC,
    PCB_COMPILER_RT_CLANG,
    PCB_COMPILER_RT_MSVC,
    /*
     * Compiler identified when #including the library.
     * Somewhat confusingly, it is done at compile time, but labeled as runtime.
     * This is because the value is *used* at runtime,
     * rather than, unlike a macro, at compile time.
     */
    PCB_COMPILER_RT_CURRENT =
#if   PCB_COMPILER_GCC
        PCB_COMPILER_RT_GCC,
#elif PCB_COMPILER_CLANG
        PCB_COMPILER_RT_CLANG,
#elif PCB_COMPILER_MSVC
        PCB_COMPILER_RT_MSVC,
#else
        PCB_COMPILER_RT_UNKNOWN,
#endif //compilers
} PCB_Compiler_RT;

PCB_Enum(PCB_BuildType, uint8_t) {
    PCB_BUILDTYPE_EXEC,
    PCB_BUILDTYPE_STATICLIB,
    PCB_BUILDTYPE_DYNAMICLIB,
    /**
     * "plugin" in this context means "library loaded at runtime".
     * Compiler toolchains don't have a notion of "plugins", so the specific
     * meaning of this enum value is up to you, the user.
     * Otherwise equivalent to `PCB_BUILDTYPE_DYNAMICLIB`.
     * PCB distinguishes "plugin" and "dynamic library" via the `PCB_RTLOAD` macro.
     */
    PCB_BUILDTYPE_PLUGIN = PCB_BUILDTYPE_DYNAMICLIB,
};

typedef struct {
    /*
     * Path to the compiler executable to use for C and C++ respectively.
     * Defaults to the compiler's name used to build this file.
     */
    const char* compilerPath;
    //Path to the build directory for caching object files. Defaults to "build/".
    const char* buildPath;
    //Name of the final executable/shared object.
    const char* outputPath;
    /*
     * Vector of paths to source directories/individual files.
     * Currently only 1 source directory is supported,
     * while individual files are not implemented yet.
     */
    PCB_CStrings sources;
    //Vector of paths to source directories/individual files to be ignored. Not yet implemented.
    PCB_CStrings sourcesBlacklist;
    //Vector of paths to include directories.
    PCB_CStrings includes;
    //Vector of names of libraries to link dynamically.
    PCB_CStrings libs;
    //Vector of names of libraries to link *statically*.
    PCB_CStrings staticLibs;
    /*
     * Vector of additional paths to pass to the compiler
     * to search for specified libraries.
     */
    PCB_CStrings librarySearchPaths;
    /*
     * Vector of compiler optimization flags.
     * Not a singular `const char*` since, for example in GCC,
     * you can pass "-f*" optimization flags on top of "-O*" flags.
     */
    PCB_CStrings optimizationFlags;
    //Vector of debug flags. Put your own sanitizer flags here.
    PCB_CStrings debugFlags;
    /*
     * Vector of warning flags, as well as warning-as-error flags.
     * Interpreted directly, a.k.a. if you add "1234",
     * the compiler will receive "1234" as a flag.
     */
    PCB_CStrings warningFlags;
    //Vector of flags for the preprocessor (defs and undefs).
    struct {
        PCB_CStringPairs defines;
        PCB_CStrings undefines;
    } preprocessorFlags;
    //Vector of other compiler flags not covered by the rest of this struct.
    PCB_CStrings otherCompilerFlags;
    //Vector of other linker flags not covered by the rest of this struct.
    PCB_CStrings otherLinkerFlags;
    //Internal buffer used for enumerating source paths.
    PCB_String currentSourcePath;
    //Internal buffer used for enumerating build paths w.r.t. the source path.
    PCB_String currentBuildPath;
    //Internal buffer used for commands when building.
    PCB_ShellCommand commandBuffer;
    //Internal vector of child process handles.
    PCB_Processes processes;
    //Internal arena allocator. May be safely reset after building and used for your own things.
    //DO NOT ALLOCATE ANYTHING PRIOR TO BUILDING!!! THE ALLOCATION WILL BE OVERRIDDEN!
    PCB_Arena* arena;
    //Internal buffer used for accumulating source file paths for compilation.
    PCB_CStrings sourceFiles;
    /*
     * Mapping of `sourceFiles` to corresponding build paths.
     * You can add your own object files here, as long as you do so
     * before calling `PCB_build_fromContext`.
     * If you manage the build process manually, you must ensure that the mapping
     * remains correct during the entire compilation step.
     * This means that if you add a source path to `sourceFiles`, you MUST either
     * add a corresponding build path here or
     */
    PCB_CStrings objectFiles;
    /*
     * The language standard used to compile source files.
     * Defaults to the standard used to build this file.
     * Setting it to 0 stops the standard flag from being added.
     */
    long standard;

    //Flags for the build context.
    union {
        unsigned int all;
//temporary macro for choosing between an unnamed struct if in C11+ or a named one
#ifndef PCB_TEMP
#if defined(__cplusplus) || (defined(__STDC_VERSION__) && __STDC_VERSION__+0 < 201112L)
#define PCB_TEMP fields
#else
#define PCB_TEMP
#endif //C++ || <C11
#endif //PCB_TEMP
        struct {
            /*
             * Specifies how many commands should be run in parallel.
             *
             * A value of 0, 1, >1 means
             * no parallelism (*),
             * running "number of cores in the system" commands in parallel (*),
             * running this exact amount of commands in parallel respectively.
             *
             * (*) - this logic is flipped if the library is built with
             * `PCB_BUILD_0_AS_PARALLEL` macro; see
             * "list of macros with special meaning" for details.
             */
            uint8_t parallel;
            //Whether to force recompilation of all detected source files.
            unsigned char alwaysBuild : 1;
            /* Specifies the command-line argument parsing syntax.
             * See the `PCB_ArgvSyntax` enum for details.
             */
            PCB_ArgvSyntax argvSyntax : 2;
            /*
             * Whether to use GNU extensions.
             * Only relevant with compilers that support it.
             * Otherwise it should be set to false.
             */
            uint8_t gnu : 1;
            PCB_Compiler_RT compilerUsed : 3;
            /*
             * Whether to use a C compiler for C files in a C++ build.
             * Setting this flag in C will cause C++ files to be compiled
             * with a C++ compiler instead of being skipped.
             */
            uint8_t ccInCpp : 1;
            //Whether anything was rebuilt. Only use when the entire build succeeded.
            uint8_t rebuiltAnything : 1;
            PCB_BuildType buildType : 2;
            //Suppresses the "up-to-date" log if set to `true`.
            uint8_t noutd : 1;
            /*
             * Defer checks for source file modifications until all source
             * files are collected.
             * If set, `sourceFiles` contains all paths to source files
             * discovered, regardless if they need to be rebuilt or not.
             * Otherwise, modification checks are performed as soon as the file
             * is discovered, while the content of `sourceFiles` depends on
             * the `buildImmediately` flag.
             */
            uint8_t deferModChecks : 1;
            /*
             * Build source files as soon as they are discovered.
             * If set, `sourceFiles` will be empty.
             * Otherwise, defer rebuilding until all source files that need
             * to be rebuilt are discovered.
             * `sourceFiles` will contain all paths to source files
             * discovered that need to be rebuilt.
             */
            uint8_t buildImmediately: 1;
            unsigned int _unused : 10;
        } PCB_TEMP;
#undef PCB_TEMP
    } flags;
    union {
        struct {
            struct {
                uint16_t major, minor, patch;
            } version;
        } lib;
    } btso; //build type specific options
} PCB_BuildContext;

#ifndef PCB_BuildContext_flags
#if defined(__cplusplus) || (defined(__STDC_VERSION__) && __STDC_VERSION__+0 < 201112L)
#define PCB_BuildContext_flags(ctx) (ctx)->flags.fields
#else
#define PCB_BuildContext_flags(ctx) (ctx)->flags
#endif //C++ || <C11
#endif //PCB_BuildContext_flags

PCB_Enum(PCB_BuildOption, uint64_t) {
    PCB_BUILDOPTION_NONE = 0,
    //Sets build path to "build/", adds "src/" to sources and "include/" to includes.
    PCB_BUILDOPTION_DEFAULT_PATHS = 1 << 1,
    //Sets compiler path, language standard to the compiler name, standard
    //used to compile this file and the compiler's argv syntax respectively.
    PCB_BUILDOPTION_DEFAULT_COMPILER = 1 << 2,
    //Adds a list of warnings from `PCB__BuildContext_addDefaultWarnings()`.
    PCB_BUILDOPTION_DEFAULT_WARNINGS = 1 << 3,
    //Sets some fields to commonly used defaults.
    PCB_BUILDOPTION_DEFAULTS =
        PCB_BUILDOPTION_DEFAULT_PATHS    |
        PCB_BUILDOPTION_DEFAULT_COMPILER |
        PCB_BUILDOPTION_DEFAULT_WARNINGS,
    //Disables debug information.
    PCB_BUILDOPTION_NODEBUG = 1 << 4,
    //Equivalent to -O2 or /O2. For more granularity modify the build context manually.
    PCB_BUILDOPTION_OPTIMIZE = 1 << 5,
    //Turns the thread sanitizer (TSan) on if available. Incompatible with ASan and LSan.
    PCB_BUILDOPTION_TSAN = 1 << 6,
    //Turns the address sanitizer (ASan) on if available. Incompatible with TSan.
    PCB_BUILDOPTION_ASAN = 1 << 7,
    //Turns the leak sanitizer (LSan) on if available. Incompatible with TSan.
    PCB_BUILDOPTION_LSAN = 1 << 8,
    //Turns the undefined behavior sanitizer (UBSan) on if available.
    PCB_BUILDOPTION_UBSAN = 1 << 9,
    /*
     * There are more sanitizers available, but they are compiler-exclusive:
     * Memory sanitizer (clang only):
     * https://clang.llvm.org/docs/MemorySanitizer.html
     * Fuzzing (MSVC only):
     * https://learn.microsoft.com/en-us/cpp/build/reference/fsanitize?view=msvc-170
     */
    //Hint that the target will only be used on the system of whoever's building it.
    //Adds "-march=native" in GCC/Clang, "/arch:<depends on local system>" in MSVC
    //when used with `PCB_BUILDOPTION_OPTIMIZE`.
    PCB_BUILDOPTION_LOCAL_SYSTEM = 1 << 10,
};



/**
 * @brief Holds the value of `PCB_SAFETY_CHECKS` macro for runtime
 * inspection by application code.
 */
PCBAPI extern const int8_t PCB_SAFETY_CHECKS_LEVEL;
/**
 * @brief Holds the value of `PCB_UNICODE_CONFORMANT` macro for runtime
 * inspection by application code.
 */
PCBAPI extern const bool PCB_STRICT_UNICODE_CONFORMANCE;
/**
 * @brief Holds the value of `PCB_VERSION` macro for runtime
 * inspection by application code.
 */
PCBAPI extern const uint32_t PCB_LIB_VERSION;
/**
 * @brief Holds the value of `PCB_BUILD_0_AS_PARALLEL` macro for runtime
 * inspection by application code.
 */
PCBAPI extern const bool PCB_BUILD_0_MEANS_PARALLEL;

/**
 * @brief Logs a `printf`-like string to either stdout or stderr
 * based on `level`.
 *
 * @param level log level
 *
 * If `level == PCB_LOGLEVEL_ERROR(_NL) || level == PCB_LOGLEVEL_FATAL(_NL)`,
 * logs to stderr. Otherwise logs to stdout.
 */
PCBAPI void PCBCALL PCB_log(
    PCB_LogLevel level,
    const char* fmt,
    ...
) PCB_Printf_Format(2, 3) PCB_Nonnull_Arg(2);

#ifndef PCB_logTrace
#ifdef PCB_DEBUG
#if PCB_DEBUG+0 > 2
#define PCB_logTrace(...) do { \
    PCB_log(PCB_LOGLEVEL_TRACE_NL, "[" __FILE__ "/%s:" PCB_STRINGIFY(__LINE__) "] ", __func__); \
    PCB_log(PCB_LOGLEVEL_NONE, __VA_ARGS__); \
} while(0)
#elif PCB_DEBUG+0 == 2
#define PCB_logTrace(...) PCB_log(PCB_LOGLEVEL_TRACE, __VA_ARGS__)
#else
#define PCB_logTrace(...)
#endif //PCB_DEBUG > 1
#else
#define PCB_logTrace(...)
#endif //PCB_DEBUG
#endif //PCB_logTrace

#ifndef PCB_logDebug
#ifdef PCB_DEBUG
#define PCB_logDebug(...) PCB_log(PCB_LOGLEVEL_DEBUG, __VA_ARGS__)
#else
#define PCB_logDebug(...)
#endif //PCB_DEBUG
#endif //PCB_logDebug



/**
 * @brief Get the error code of the last error that has occured.
 *
 * WARNING: for the sake of attempting to be cross-platform, the return value
 * is *only* relevant in the context of this library. Do NOT use for error
 * handling outside. This is because Windows has its own error storage entirely
 * separate to libc, which conflict, making it very difficult to write a centralized
 * error handler.
 *
 * @return 0 if no error has occured,
 *
 * On Windows: a negative value if the error comes from libc (`errno`),
 * a positive value otherwise (WinAPI).
 *
 * On POSIX systems: `-errno`.
 *
 * Generally, if the return value is negative, the error comes from libc.
 * Otherwise it's platform-dependent.
 *
 * UPDATE: This WILL change in the future to an actually unified error handling
 * based on `PCB_Status` structure.
 */
PCBAPI int PCBCALL PCB_GetError(void);
/**
 * @brief Clear the error obtainable with `PCB_GetError`.
 */
PCBAPI void PCBCALL PCB_ClearError(void);
/**
 * @brief Get the error string corresponding to `errnum` into `buf` of
 * `bufSize` size.
 *
 * WARNING: `errnum` ****MUST**** be obtained by calling `PCB_GetError`,
 * otherwise you lose **ALL** portability with regards to error handling
 * AND are **GUARANTEED** to get incorrect results.
 *
 * DO NOT BLINDLY PASS `errno` OR `GetLastError()` WITHOUT READING THE DOCS ABOVE!!
 *
 * @return 0 on success,
 * otherwise an error code according to the schema above is returned and
 * the previous error code is preserved.
 */
PCBAPI int PCBCALL PCB_GetErrorString(
    int errnum,
    char* buf,
    size_t bufSize
) PCB_Nonnull_Arg(2);
/**
 * @brief Log the latest error obtained from `PCB_GetError()` to stderr.
 * Otherwise functions similarly to `printf`.
 *
 * @param fmt `printf`-like format string
 */
PCBAPI void PCBCALL PCB_logLatestError(
    const char* fmt,
    ...
) PCB_Printf_Format(1, 2) PCB_Nonnull_Arg(1);



#ifndef PCB_FS_PATH_DELIM
#if PCB_PLATFORM_WINDOWS
#define PCB_FS_PATH_DELIM ';'
#elif PCB_PLATFORM_POSIX
#define PCB_FS_PATH_DELIM ':'
#else
#define PCB_FS_PATH_DELIM 0x15 //stub
#endif //platforms
#endif //PCB_FS_PATH_DELIM



/**
 * @brief Creates a directory in the given `path`.
 * Returns whether the operation succeeded.
 *
 * Failure by "it already exists" is treated as success.
 *
 * On Linux, permission field of the created directory is `rwxrwxr-x`.
 * @param path path/to/directory/to/create, not transitive
 */
PCBAPI bool PCBCALL PCB_mkdir(const char* path) PCB_Nonnull_Arg(1);
/**
 * @brief Checks if a filesystem entry exists.
 *
 * @param path path/to/thing/in/filesystem
 * @return `true` if exists, `false` if doesn't, a negative value
 * otherwise. To get the error code call `PCB_GetError`.
 */
PCBAPI int PCBCALL PCB_FS_Exists(const char* path) PCB_Nonnull_Arg(1);
/**
 * @brief Checks if `target` exists in PATH (environment variable).
 * `PATH` (argument) is used to allow the caller to decide what to do on error
 * as well as checking for alternative entries.
 *
 * `PATH` shall be initialized to an empty `PCB_StringView` when calling
 * this function for the 1st time, after which it'll be filled with the PATH
 * environment variable.
 * The function will then check each component delimited by `PCB_FS_PATH_DELIM`
 * until it either finds an existing entry, consumes all components or
 * an error occurs. To continue searching after the erroneous component, skip
 * bytes in `PATH` until `PCB_FS_PATH_DELIM` (this can be done via
 * `PCB_StringView_findCharFrom` + shifting 1 byte) and call this function again.
 *
 * This function requires temporary memory allocations, which will be done in
 * `arena`. If `arena == NULL`, the function will use `PCB_realloc`.
 *
 * @return 0 if `target` was not found in any component of `PATH`, a positive
 * value containing the number of bytes to copy from `PATH` to get `target`'s
 * location in the filesystem (for example "gcc" found in "/usr/bin" -> 8)
 * or a negative value corresponding to the underlying system API on error
 * (WinAPI on Windows, probably libc otherwise).
 *
 * NOTE: Error handling will be changed in future versions of the library,
 * including this function!
 */
PCBAPI int PCBCALL PCB_FS_ExistsPath(
    const char*      target,
    PCB_StringView*  PATH,
    PCB_Arena*      arena
) PCB_Nonnull_Arg(1, 2);
/**
 * @brief Get the file type of `path`.
 * Note that if `path` refers to a symbolic link, the value returned
 * contains a bitwise OR of the underlying file type and `PCB_FILETYPE_SYMLINK`.
 * Therefore you shouldn't use `==` directly with the return value.
 * To ignore the symlink bit, apply bitwise AND with `PCB_FILETYPE_SYMLINK_IGN`
 * to remove it.
 * @param path path/to/thing/in/filesystem
 */
PCBAPI PCB_FileType PCBCALL PCB_FS_GetType(const char* path) PCB_Nonnull_Arg(1);
/**
 * @brief Checks if a filesystem entry is a directory.
 * @param path path/to/thing/in/filesystem
 * @return `true` if it is a directory, `false` if doesn't, a negative value
 * otherwise. To get the error code call `PCB_GetError`.
 */
PCBAPI int PCBCALL PCB_FS_IsDirectory(const char* path) PCB_Nonnull_Arg(1);
/**
 * @brief Get the modification time of a filesystem entry.
 *
 * @return 0 on error, 1 if the entry doesn't exist, some other integer otherwise.
 */
PCBAPI uint64_t PCBCALL PCB_FS_GetModificationTime(const char* path) PCB_Nonnull_Arg(1);
/**
* @brief Loads entire file from `path` into a dynamically allocated buffer.
* @return `true` on success, `false` on error; to get the error
* code call `PCB_GetError()`.
*/
PCBAPI bool PCBCALL PCB_FS_ReadEntireFile(
    const char* path,
    PCB_String* buf
) PCB_Nonnull_Arg(1, 2);
/**
 * @brief Get the basename (filename) of `path`.
 *
 * If `path` is an empty `PCB_StringView`,
 * the returned `PCB_StringView` points to statically allocated ".".
 * This behavior is equivalent to basename(3), except the buffer is not modified.
 */
PCBAPI PCB_StringView PCBCALL PCB_FS_Basename(PCB_StringView path);
/**
 * @brief Get the dirname (everything except filename and directory separator) of `path`.
 *
 * If `path` is an empty `PCB_StringView` or there is no separator,
 * the returned `PCB_StringView` points to statically allocated ".".
 * This behavior is equivalent to dirname(3), except the buffer is not modified.
 */
PCBAPI PCB_StringView PCBCALL PCB_FS_Dirname(PCB_StringView path);
/**
 * @brief Get the extension of `path`, without ".".
 * If there is none or it's empty ("./foo."), an empty `PCB_StringView` is returned.
 *
 * This function operates purely on strings. For example, if `path` refers to
 * a directory and there is a ".", for example "/etc/grub.d", this function
 * will pick up "d" as the extension. It is the caller's responsibility to
 * first check that `path` refers to a regular file!
 */
PCBAPI PCB_StringView PCBCALL PCB_FS_Extension(PCB_StringView path);
/**
 * @brief Get the extension of `path`, without "."
 * This function differs from `PCB_FS_Extension` in that it assumes `path` is
 * already a basename and is therefore faster. If it's not, the returned
 * `PCB_StringView` may be incorrect. Otherwise the behavior is identical.
 *
 * If you called `PCB_FS_Basename` already, it is better to use this function
 * for performance. If in doubt, use `PCB_FS_Extension`.
 */
PCBAPI PCB_StringView PCBCALL PCB_FS_Extension_base(PCB_StringView path);



#ifndef PCB_strlen_char
#define PCB_strlen_char(str) PCB_strlen(str)
#endif //PCB_strlen_char

#ifndef PCB_strlen_wchar_t
#ifdef PCB__HAS_WCHAR_H
#define PCB_strlen_wchar_t(str) wcslen(str)
#else
PCBAPI size_t PCBCALL PCB_strlen_wchar_t(const wchar_t* str) PCB_Nonnull_Arg(1);
#endif //PCB__HAS_WCHAR_H
#endif //PCB_strlen_wchar_t

PCBAPI size_t PCBCALL PCB_strlen_char8 (const PCB_char8*  str) PCB_Nonnull_Arg(1);
PCBAPI size_t PCBCALL PCB_strlen_char16(const PCB_char16* str) PCB_Nonnull_Arg(1);
PCBAPI size_t PCBCALL PCB_strlen_char32(const PCB_char32* str) PCB_Nonnull_Arg(1);

/**
 * NOTE: Functions returning `false` may also fail when provided invalid
 * arguments like `str` being NULL. This behavior depends on the value of
 * `PCB_SAFETY_CHECKS_LEVEL`. Additionally, if `PCB_SAFETY_CHECKS > 1`,
 * `PCB_maybe_restrict` expands into `PCB_restrict`, which in C expands to
 * `restrict`.
 * See `PCB_SAFETY_CHECKS` macro and `PCB_SAFETY_CHECKS_LEVEL` for details.
 */

/**
 * @brief Frees `str`'s buffer and resets fields to 0.
 */
PCBAPI void PCBCALL PCB_String_destroy(PCB_String* PCB_restrict str) PCB_Nonnull_Arg(1);
#ifndef PCB_String_reset
/**
 * @brief Resets `str` to hold no string. Does nothing if `str->data == NULL`.
 */
#define PCB_String_reset(str) while((str)->data != NULL) { \
    (str)->data[(str)->length = 0] = '\0'; break; \
}
#endif //PCB_String_reset
/**
 * @brief Reserves `howMany` *additional* bytes in `str`.
 * @return whether the operation succeeded: fails on realloc failure.
 */
PCBAPI bool PCBCALL PCB_String_reserve(
    PCB_String* PCB_restrict str,
    const size_t howMany
) PCB_Nonnull_Arg(1);
/**
 * @brief Reserves bytes in `str` so that its capacity is at least `cap`.
 * @return whether the operation succeeded: fails on realloc failure.
 */
PCBAPI bool PCBCALL PCB_String_reserve_to(
    PCB_String* PCB_restrict str,
    const size_t cap
) PCB_Nonnull_Arg(1);
/**
 * @brief Resizes `str` to fit a string of `targetLength` length.
 * Truncates the string to `targetLength` if `targetLength < str->length`.
 * Does nothing if `targetLength == str->length`.
 * Behaves identically to `PCB_String_reserve` otherwise.
 * @return whether the operation succeeded: fails on realloc failure.
 */
PCBAPI bool PCBCALL PCB_String_resize(
    PCB_String* PCB_restrict str,
    const size_t targetLength
) PCB_Nonnull_Arg(1);
/**
 * @brief Appends `other` to `str`.
 *
 * Note: it is permitted that `other == str`.
 * @return whether the operation succeeded:
 * fails on realloc failure or if `other == NULL`.
 */
PCBAPI bool PCBCALL PCB_String_append(
    PCB_String* str,
    const PCB_String* other
) PCB_Nonnull_Arg(1, 2);
/**
 * @brief Appends `sv` to `str`. `sv` may be empty.
 *
 * Note: it is NOT permitted that `sv` is a view of `str` as it may be
 * invalidated by a realloc.
 * @return whether the operation succeeded: fails on realloc failure.
 */
PCBAPI bool PCBCALL PCB_String_append_sv(
    PCB_String* PCB_restrict str,
    PCB_StringView sv
) PCB_Nonnull_Arg(1);
/**
 * @brief Appends `cstr` to `str`.
 *
 * Note: it is NOT permitted that `cstr` overlaps with `str->data`, i.e. points to
 * a range [`str->data`, `str->data + str->length`) as it may be
 * invalidated by a realloc.
 * @return whether the operation succeeded:
 * fails on realloc failure or if `cstr == NULL` or the note above.
 */
PCBAPI bool PCBCALL PCB_String_append_cstr(
    PCB_String* PCB_restrict str,
    const char* PCB_maybe_restrict cstr
) PCB_Nonnull_Arg(1, 2);
/**
 * @brief Appends C-strings from `cstrs` to `str`.
 *
 * If all entries in `cstrs` are empty C-strings or if `cstrs` is empty,
 * nothing is appended.
 * NULL entries and entries that overlap with `str` (see above)
 * in `cstrs` are skipped.
 * @return number of entries in `cstrs` appended, -1 on error.
 */
PCBAPI ssize_t PCBCALL PCB_String_append_cstrs(
    PCB_String* PCB_restrict str,
    PCB_CStringsView cstrs
) PCB_Nonnull_Arg(1);
/**
 * @brief Appends variable number of C-strings to `str`.
 *
 * C-strings overlapping with `str->data` are skipped.
 *
 * The last argument MUST be `NULL`. Otherwise the behavior is undefined.
 * @return number of C-strings appended, 0 on error.
 */
PCBAPI ssize_t PCBCALL PCB_String_append_cstr_v(
    PCB_String* PCB_restrict str,
    ...
) PCB_Nonnull_Arg(1);
/**
 * @brief Appends `c` to `str` `howManyTimes` times.
 * `c == '\0'` is treated as a no-op.
 * @return whether the operation succeeded: can fail on realloc failure.
 */
PCBAPI bool PCBCALL PCB_String_append_chars(
    PCB_String* PCB_restrict str,
    const char c,
    const size_t howManyTimes
) PCB_Nonnull_Arg(1);
/**
 * @brief Appends a `printf`-like formatted string to `str`.
 *
 * Note: it is NOT permitted to append a formatted string to `str` based on
 * `str` itself, i.e.`fmt` cannot overlap with `[str->data, str->data + str->length`).
 *
 * This function is only available if PCB was compiled with stdio.h present.
 * Otherwise it does nothing and always returns `false`.
 *
 * @return whether the operation succeeded: can fail on realloc failure.
 */
PCBAPI bool PCBCALL PCB_String_appendf(
    PCB_String* PCB_restrict str,
    const char* PCB_maybe_restrict fmt,
    ...
) PCB_Printf_Format(2, 3) PCB_Nonnull_Arg(1, 2);
/**
 * @brief Appends a Unicode `codepoint` to `str`.
 * @return whether the operation succeeded: fails on realloc failure or
 * if `codepoint` is invalid.
 */
PCBAPI bool PCBCALL PCB_String_append_codepoint(
    PCB_String* PCB_restrict str,
    int32_t codepoint
) PCB_Nonnull_Arg(1);
/**
 * @brief Appends a UTF-32 sequence of `codepoints` to `str`.
 * @return number of codepoints appended, -1 on error
 */
PCBAPI ssize_t PCBCALL PCB_String_append_codepoints(
    PCB_String* PCB_restrict str,
    PCB_U32StringView codepoints
) PCB_Nonnull_Arg(1);
/**
 * @brief Inserts `other` into `str` at position `position`.
 * Inserting `str` into itself (`str == other`) is not allowed.
 * @return whether the operation succeeded:
 * fails on invalid arguments passed or if realloc failed.
 */
PCBAPI bool PCBCALL PCB_String_insert(
    PCB_String* PCB_maybe_restrict str,
    const PCB_String* PCB_maybe_restrict other,
    size_t position
) PCB_Nonnull_Arg(1, 2);
/**
 * @brief Inserts `sv` into `str` at position `position`
 *
 * Note: It is NOT permitted that `sv` overlaps with `str`, i.e. views a range
 * [`str->data`, `str->data + str->length`) as it may be invalidated by a realloc.
 * @return whether the operation succeeded.
 */
PCBAPI bool PCBCALL PCB_String_insert_sv(
    PCB_String* PCB_restrict str,
    PCB_StringView sv,
    size_t position
) PCB_Nonnull_Arg(1);
/**
 * @brief Inserts `cstr` into `str` at `position`.
 *
 * Note: it is NOT permitted that `cstr` overlaps with `str->data`, i.e. points to
 * a range [`str->data`, `str->data + str->length`) as it may be
 * invalidated by a realloc.
 * @return whether the operation succeeded: fails on
 * invalid arguments passed or if realloc failed.
 */
PCBAPI bool PCBCALL PCB_String_insert_cstr(
    PCB_String* PCB_restrict str,
    const char* PCB_maybe_restrict cstr,
    size_t position
) PCB_Nonnull_Arg(1, 2);
/**
 * @brief Inserts C-string from `cstrs` into `str` at position `position`.
 *
 * If all entries in `cstrs` are empty C-strings or if `cstrs` is empty,
 * nothing is appended.
 * NULL entries and entries that overlap with `str->data` (see above)
 * in `cstrs` are skipped.
 * @return number of entries in `cstrs` inserted, -1 on error.
 */
PCBAPI ssize_t PCBCALL PCB_String_insert_cstrs(
    PCB_String* PCB_restrict str,
    PCB_CStringsView cstrs,
    size_t position
) PCB_Nonnull_Arg(1);
/**
 * @brief Inserts variable number of C-strings into `str` at position `position`.
 * The last argument MUST be `NULL`.
 *
 * C-strings overlapping with `str->data` are skipped.
 *
 * @return number of entries in `cstrs` inserted, -1 on error.
 */
PCBAPI ssize_t PCBCALL PCB_String_insert_cstr_v(
    PCB_String* PCB_restrict str,
    size_t position,
    ...
) PCB_Nonnull_Arg(1);
/**
 * @brief Inserts `c` into `str` at position `position` `howManyTimes` times.
 * Inserting '\0' is not allowed.
 * @return whether the operation succeeded:
 * fails on invalid arguments passed or if realloc failed.
 */
PCBAPI bool PCBCALL PCB_String_insert_chars(
    PCB_String* PCB_restrict str,
    const char c,
    size_t howManyTimes,
    size_t position
) PCB_Nonnull_Arg(1);
/**
 * @brief Inserts a `printf`-like formatted string into `str` at position `position`.
 *
 * Note: it is NOT permitted to insert a formatted string to `str` based on
 * `str` itself, i.e.`fmt` cannot overlap with `str->data`.
 *
 * This function is only available if the library was compiled with stdio.h present.
 * Otherwise it does nothing and always returns `false`.
 *
 * @return whether the operation succeeded:
 * fails on invalid arguments passed or if realloc failed.
 */
PCBAPI bool PCBCALL PCB_String_insertf(
    PCB_String* PCB_restrict str,
    const char* PCB_maybe_restrict fmt,
    size_t position,
    ...
) PCB_Printf_Format(2, 4) PCB_Nonnull_Arg(1, 2);
/**
 * @brief Inserts a Unicode `codepoint` at position `position` into `str`.
 * Inserting 0 is not allowed.
 * @return whether the operation succeeded: fails on realloc failure or
 * if `codepoint` is invalid.
 */
PCBAPI bool PCBCALL PCB_String_insert_codepoint(
    PCB_String* PCB_restrict str,
    int32_t codepoint,
    size_t position
) PCB_Nonnull_Arg(1);
/**
 * @brief Inserts a UTF-32 sequence of `codepoints` to `str` at position `position`.
 * @return number of codepoints inserted, -1 on error
 */
PCBAPI ssize_t PCBCALL PCB_String_insert_codepoints(
    PCB_String* PCB_restrict str,
    PCB_U32StringView codepoints,
    size_t position
) PCB_Nonnull_Arg(1);
/**
 * @brief Replaces characters in the range `[start, start + length)`
 * in `str` with `other`.
 *
 * Note: It is NOT permitted that `other` overlaps with `str`, i.e. views a range
 * [`str->data`, `str->data + str->length`) as it may be invalidated by a realloc.
 * @return whether the operation succeded
 */
PCBAPI bool PCBCALL PCB_String_replace_range(
    PCB_String* PCB_restrict str,
    size_t start,
    size_t length,
    PCB_StringView other
) PCB_Nonnull_Arg(1);
/**
 * @brief Replaces characters in the range `[start, start + length)`
 * in `str` with the character `c`. A glorified memset wrapper.
 * Replacing with '\0' is not allowed.
 * @return whether the operation succeded: 
 */
PCBAPI bool PCBCALL PCB_String_replace_range_chars(
    PCB_String* PCB_restrict str,
    size_t start,
    size_t length,
    char c
) PCB_Nonnull_Arg(1);
/**
 * @brief Removes characters in the range `[start, start + length)`.
 * @return whether the operation succeded: can only fail on invalid range passed
 */
PCBAPI bool PCBCALL PCB_String_remove_range(
    PCB_String* PCB_restrict str,
    size_t start,
    size_t length
) PCB_Nonnull_Arg(1);
/**
 * @brief Makes `c` the last character in `str`.
 * If `c` is not the last character, it appends it.
 * Otherwise does nothing.
 * @return `false` if reallocation failed, `true` otherwise
 */
PCBAPI bool PCBCALL PCB_String_setSuffix_char(
    PCB_String* PCB_restrict str,
    const char c
) PCB_Nonnull_Arg(1);
/**
 * @brief Truncate `str` until `c` is found. If `c` is not found, `str` is *not*
 * truncated.
 * @return `true` if successfully truncated, `false` if `c` was not found
 */
PCBAPI bool PCBCALL PCB_String_truncate_until_char(
    PCB_String* PCB_restrict str,
    const char c
) PCB_Nonnull_Arg(1);
/**
 * @brief Clones `str`.
 * @return an initialized `PCB_String` structure or a zeroed out one on failure
 */
PCBAPI PCB_String PCBCALL PCB_String_clone(
    const PCB_String* PCB_restrict str
) PCB_Nonnull_Arg(1);
/**
 * @brief Compares `a` and `b` lexicographically.
 * @return
 * 0 on equality or if `a` and `b` are invalid,
 *
 * a negative value if `a` < `b` or `b` is invalid,
 *
 * a positive value if `a` > `b` or `a` is invalid.
 */
PCBAPI int PCBCALL PCB_String_compare(
    const PCB_String* a,
    const PCB_String* b
) PCB_Nonnull_Arg(1, 2);
/**
 * @brief Compares `a` and `b`, case insensitive version*.
 * @return same as `PCB_String_compare`.
 *
 * * - the comparison is done as if `a` and `b` are ASCII strings and ignores
 * Unicode
 */
PCBAPI int PCBCALL PCB_String_compare_ci(
    const PCB_String* a,
    const PCB_String* b
) PCB_Nonnull_Arg(1, 2);
/**
 * @brief Compares `a` and `b` lexicographically, version with a C string.
 * @return same as `PCB_String_compare`.
 */
PCBAPI int PCBCALL PCB_String_compare_cstr(
    const PCB_String* PCB_restrict a,
    const char* PCB_restrict b
) PCB_Nonnull_Arg(1, 2);
/**
 * @brief Compares `a` and `b`, case insensitive version* with a C string.
 * @return same as `PCB_String_compare`.
 *
 * * - the comparison is done as if `a` and `b` are ASCII strings and ignores
 * Unicode
 */
PCBAPI int PCBCALL PCB_String_compare_cstr_ci(
    const PCB_String* PCB_restrict a,
    const char* PCB_restrict b
) PCB_Nonnull_Arg(1, 2);
/**
 * @brief Checks whether `a` and `b` are equal.
 * Faster than `PCB_String_compare` since it can shortcut on differing lengths.
 */
PCBAPI bool PCBCALL PCB_String_eq(
    const PCB_String* a,
    const PCB_String* b
) PCB_Nonnull_Arg(1, 2);
/**
 * @brief Checks if `str` starts with `other`.
 * If any of them are empty, returns false.
 * @return whether `str` starts with `other`
 */
PCBAPI bool PCBCALL PCB_String_startsWith(
    const PCB_String* str,
    const PCB_String* other
) PCB_Nonnull_Arg(1, 2);
/**
 * @brief Checks if `str` starts with `other`.
 * If `str` is empty, returns false.
 * @return whether `str` starts with `other`
 */
PCBAPI bool PCBCALL PCB_String_startsWith_cstr(
    const PCB_String* PCB_restrict str,
    const char* PCB_restrict other
) PCB_Nonnull_Arg(1, 2);
/**
 * @brief Checks if `str` ends with `other`.
 * If any of them are empty, returns false.
 * @return whether `str` ends with `other`
 */
PCBAPI bool PCBCALL PCB_String_endsWith(
    const PCB_String* str,
    const PCB_String* other
) PCB_Nonnull_Arg(1, 2);
/**
 * @brief Checks if `str` ends with `other`.
 * If `str` is empty, returns false.
 * @return whether `str` ends with `other`
 */
PCBAPI bool PCBCALL PCB_String_endsWith_cstr(
    const PCB_String* PCB_restrict str,
    const char* PCB_restrict other
) PCB_Nonnull_Arg(1, 2);
#ifndef PCB_String_isEmpty
#define PCB_String_isEmpty(str) ((str)->data == NULL || (str)->length == 0)
#endif //PCB_String_isEmpty
/**
 * @brief Converts `str` to uppercase.
 *
 * Note: ignores Unicode.
 */
PCBAPI void PCBCALL PCB_String_toUpperCase(
    PCB_String* PCB_restrict str
) PCB_Nonnull_Arg(1);
/**
 * @brief Converts `str` to lowercase.
 *
 * Note: ignores Unicode.
 */
PCBAPI void PCBCALL PCB_String_toLowerCase(
    PCB_String* PCB_restrict str
) PCB_Nonnull_Arg(1);
/**
 * @brief Converts a clone of `str` to uppercase.
 *
 * Note: ignores Unicode.
 * @return an initialized `PCB_String` structure or a zeroed out one on failure
 */
PCBAPI PCB_String PCBCALL PCB_String_toUpperCase_copy(
    const PCB_String* PCB_restrict str
) PCB_Nonnull_Arg(1);
/**
 * @brief Converts a clone of `str` to lowercase.
 *
 * Note: ignores Unicode.
 * @return an initialized `PCB_String` structure or a zeroed out one on failure
 */
PCBAPI PCB_String PCBCALL PCB_String_toLowerCase_copy(
    const PCB_String* PCB_restrict str
) PCB_Nonnull_Arg(1);
/**
 * @brief Pops the last character in `str`.
 * If the library was compiled with `PCB_SAFETY_CHECKS = 0`, passing `NULL`
 * as `str` causes the function to return 0x15.
 * If `str` is empty, returns `\0` without modification.
 */
PCBAPI char PCBCALL PCB_String_pop(
    PCB_String* PCB_restrict str
) PCB_Nonnull_Arg(1);
/**
 * @brief Pops `howMany` characters from `str` into `out`.
 * If `howMany > str->length`, `howMany` is clamped to `str->length`.
 * If `out` is NULL, characters are discarded.
 * The caller must ensure that `out` can hold at least `howMany` bytes.
 * Note: it is NOT permitted that `out` overlaps with `str->data`, i.e. points to
 * a range [`str->data`, `str->data + str->length`) as it may trigger undefined
 * behavior.
 * @return number of characters popped
 */
PCBAPI size_t PCBCALL PCB_String_pop_many(
    PCB_String* PCB_restrict str,
    size_t howMany,
    char* PCB_restrict out
) PCB_Nonnull_Arg(1);
/**
 * @brief Removes `other->length` characters from `str` if they match.
 * @return new length, 0 on error
 */
PCBAPI size_t PCBCALL PCB_String_removeSuffix(
    PCB_String* str,
    const PCB_String* other
) PCB_Nonnull_Arg(1, 2);
/**
 * @brief Removes leading whitespace characters from `str`.
 */
PCBAPI void PCBCALL PCB_String_trim_left(
    PCB_String* PCB_restrict str
) PCB_Nonnull_Arg(1);
/**
 * @brief Removes trailing whitespace characters from `str`.
 */
PCBAPI void PCBCALL PCB_String_trim_right(
    PCB_String* PCB_restrict str
) PCB_Nonnull_Arg(1);
/**
 * @brief Makes `str` hold `n` copies of itself.
 * `n == 0` is not allowed; use `PCB_String_reset(str)` or
 * `PCB_String_resize(str, 0)` to reset `str`.
 * `n == 1` does nothing.
 * @return `true` on success or `false` on failure.
 */
PCBAPI bool PCBCALL PCB_String_repeat(
    PCB_String* PCB_restrict str,
    size_t n
) PCB_Nonnull_Arg(1);
/**
 * @brief Creates a new `PCB_String` from `sv`.
 * @return an initialized `PCB_String` structure or a zeroed out one on failure.
 */
PCBAPI PCB_String PCBCALL PCB_String_from_StringView(PCB_StringView sv);
/**
 * @brief Creates a new `PCB_String` from `cstrs` joined with `delimiter.
 * @return an initialized `PCB_String` structure or a zeroed out one on failure
 */
PCBAPI PCB_String PCBCALL PCB_String_from_CStrings(
    const PCB_CStrings* PCB_restrict cstrs,
    const char* PCB_restrict delimiter
) PCB_Nonnull_Arg(1, 2);
/**
 * @brief Removes leading and trailing whitespace characters from `str`.
 */
PCB_maybe_inline void PCBCALL PCB_String_trim(
    PCB_String* PCB_restrict str
) PCB_Nonnull_Arg(1);

/**
 * @brief Find `n`th occurence of a substring `sub` in `sv`.
 * `n` cannot be 0.
 * @return non-empty `PCB_StringView` on success or an empty one on invalid
 * arguments or if `sub` was not found.
 */
PCBAPI PCB_StringView PCBCALL PCB_StringView_substr_n(
    PCB_StringView sv,
    const PCB_StringView sub,
    size_t n
);
/**
 * @brief Find `n`th occurence of a substring `sub` in `sv`, searched from the end.
 * `n` cannot be 0.
 * @return non-empty `PCB_StringView` on success or an empty one on invalid
 * arguments or if `sub` was not found.
 */
PCBAPI PCB_StringView PCBCALL PCB_StringView_rsubstr_n(
    PCB_StringView sv,
    const PCB_StringView sub,
    size_t n
);
/**
 * @brief Splits `sv` by `delim` into string views pointing to it.
 * @return non-empty `PCB_StringViews` on success or an empty one on error.
 */
PCBAPI PCB_StringViews PCBCALL PCB_StringView_split(
    PCB_StringView sv,
    PCB_StringView delim
);
/**
 * @brief Splits `sv` by `delim` into substring copies.
 * @return non-empty `PCB_Strings` on success or an empty one on error.
 */
PCBAPI PCB_Strings PCBCALL PCB_StringView_split_copy(
    PCB_StringView sv,
    PCB_StringView delim
);
/**
 * @brief Splits `sv` by whitespace characters into string views pointing to it.
 * @return non-empty `PCB_StringViews` on success or an empty one on error.
 */
PCBAPI PCB_StringViews PCBCALL PCB_StringView_split_whitespace(
    PCB_StringView sv
);
/**
 * @brief Splits `sv` by whitespace characters into substring copies.
 * @return non-empty `PCB_Strings` on success or an empty one on error.
 */
PCBAPI PCB_Strings PCBCALL PCB_StringView_split_whitespace_copy(
    PCB_StringView sv
);
/**
 * @brief Find the `n`th occurence of any byte in `accept` inside `sv`.
 * @return a sub`PCB_StringView` with:
 *
 * - `data` pointing to the `n`th occurence of a byte in `sv`
 *   matching one of the bytes in `accept`,
 *
 * - `length` equal to `sv.length` - the amount of bytes skipped
 *
 * or an empty `PCB_StringView` if `sv` is empty or `accept` is empty or `n == 0`
 * or if no byte from `accept` was found in `sv`.
 */
PCBAPI PCB_StringView PCBCALL PCB_StringView_findCharFrom_n(
    PCB_StringView sv,
    PCB_StringView accept,
    size_t n
);
/**
 * @brief Find the `n`th occurence of any byte NOT in `accept` inside `sv`.
 * @return a sub`PCB_StringView` with:
 *
 * - `data` pointing to the `n`th occurence of a byte in `sv`
 *   matching none of the bytes in `accept`,
 *
 * - `length` equal to `sv.length` - the amount of bytes skipped
 *
 * or an empty `PCB_StringView` if `sv` is empty or `accept` is empty or `n == 0`
 * or if no byte outside of `accept` was found in `sv`.
 */
PCBAPI PCB_StringView PCBCALL PCB_StringView_findCharNotFrom_n(
    PCB_StringView sv,
    PCB_StringView accept,
    size_t n
);

/**
 * @brief Get the length of the Unicode codepoint in `sv` from byte (!)
 * at index `index`.
 *
 * ⚠️WARNING⚠️: THIS FUNCTION ONLY CHECKS THE FIRST BYTE; DO NOT USE IT TO
 * VERIFY THAT THE UNDERLYING SEQUENCE IS VALID,
 * USE `PCB_StringView_GetCodepoint` INSTEAD.
 *
 * @return value in range [1, 4*] or 0 if `sv.data[index]` is not a valid
 * first byte in UTF-8 encoding.
 * 0 is also returned if `sv` is empty or `index` goes out of bounds.
 * This is not the case with `PCB_StringView_GetCodepointLength_unchecked`.
 *
 * See RFC 2279/3629 for info about UTF-8 encoding.
 *
 * * - the full theoretical range of [1, 6] is available by #defining
 * `PCB_UTF8_FULL_RANGE`. It is disabled by default in compliance with RFC 3629
 * and is only available if `PCB_UNICODE_CONFORMANT` is not #defined.
 */
PCBAPI uint8_t PCBCALL PCB_StringView_GetCodepointLength(
    PCB_StringView sv,
    size_t index
);
/**
 * @brief Get the length of the Unicode codepoint in `sv` from byte (!)
 * at index `index`.
 *
 * This function is unsafe; only use it when certain that `sv` and `index`
 * are correct. Otherwise the behavior is undefined. You've been warned.
 *
 * ⚠️WARNING⚠️: THIS FUNCTION ONLY CHECKS THE FIRST BYTE; DO NOT USE IT TO
 * VERIFY THAT THE UNDERLYING SEQUENCE IS VALID,
 * USE `PCB_StringView_GetCodepoint_unchecked` INSTEAD.
 *
 * @return same as `PCB_StringView_GetCodepointLength`.
 */
PCBAPI uint8_t PCBCALL PCB_StringView_GetCodepointLength_unchecked(
    PCB_StringView sv,
    size_t index
);
/**
 * @brief Get the Unicode codepoint in `sv` from byte (!) at index `index`.
 * @return `PCB_Codepoint` structure with:
 * - `code` field in range [0, 0x10FFFF*] on success,
 *   -1  if `sv.data[index]` is a continuation byte,
 *   -2  if the sequence is unfinished,
 *   -3  if octets in the sequence would produce a codepoint out of range,
 *   -4  if a non-continuation byte was encountered while decoding a multi-octet codepoint,
 *   -5  if a surrogate was decoded (never returned under `PCB_UNICODE_CONFORMANT`),
 *   -6  if `sv.data[index]` is either `0xC0` or `0xC1`,
 *   -16 if `sv` is empty,
 *   -17 if `index` goes out of bounds,
 *   -18 if `sv.data + sv.length` or `sv.data + index + <decoded UTF-8 length>`
 *   would cause a pointer arithmetic overflow;
 * - `length` field describing how many bytes to skip until the next codepoint
 *   or 0 if `code <= -16` (on an error unrelated to UTF-8 decoding).
 *
 * See RFC 2279/3629 for info about UTF-8 encoding.
 *
 * * - the full theoretical range of [0, 0x7FFF FFFF] is available by #defining
 * `PCB_UTF8_FULL_RANGE`. It is disabled by default in compliance with RFC 3629
 * and is only available if `PCB_UNICODE_CONFORMANT` is not #defined.
 */
PCBAPI PCB_Codepoint PCBCALL PCB_StringView_GetCodepoint(
    PCB_StringView sv,
    size_t index
);
/**
 * @brief Get the Unicode codepoint in `sv` from byte (!) at index `index`.
 *
 * This function is unsafe; only use it when absolutely certain that `sv` and `input`
 * are correct. Otherwise the behavior is undefined. You've been warned.
 * @return same as `PCB_StringView_GetCodepoint`, except the `length` field
 * is only 0 if `sv.data + index + <decoded UTF-8 length>` would cause an overflow.
 */
PCBAPI PCB_Codepoint PCBCALL PCB_StringView_GetCodepoint_unchecked(
    PCB_StringView sv,
    size_t index
);


PCB_maybe_inline PCB_StringView PCBCALL PCB_StringView_from_String(
    const PCB_String* PCB_restrict str
) PCB_Nonnull_Arg(1);
PCB_maybe_inline PCB_StringView PCBCALL PCB_StringView_from_cstr(
    const char* PCB_restrict str
) PCB_Nonnull_Arg(1);
PCB_maybe_inline bool PCBCALL PCB_String_replace_range_cstr(
    PCB_String* PCB_restrict str,
    size_t start,
    size_t length,
    const char* PCB_restrict cstr
) PCB_Nonnull_Arg(1, 4);
/**
 * The block below declares every combination of
 * "get either the 1st or `n`th substring from either a StringView
 * or a String, where the substring is either a String(View) or a C-string,
 * getting a view to it". They all eventually call `PCB_StringView_substr_n`.
 *
 * For example, "PCB_StringView_subcstr_n" ->
 * "find the `n`th substring, which is a C-string, inside a StringView and return a view to it",
 * while "PCB_String_substr" ->
 * "find the 1st substring, which is a String, inside a String and return a view to it".
 *
 * If you want an expandable copy of the string, use `PCB_String_from_StringView`.
 *
 * There are no functions that accept arguments 1) and 2) respectively for
 * `const PCB_String*`  & `PCB_StringView` or
 * `PCB_StringView`     & `const PCB_String*` or
 * `const char*`        & `PCB_StringView` or
 * `const char*`        & `const PCB_String*` or
 * return a `const char*`
 * since that'd require 32x more functions for a total
 * of a whopping 256 combinations, which is bogus. Deal with it yourself.
 */
PCB_maybe_inline PCB_StringView PCBCALL PCB_StringView_substr   (PCB_StringView    sv,  PCB_StringView    sub);
PCB_maybe_inline PCB_StringView PCBCALL PCB_StringView_subcstr  (PCB_StringView    sv,  const char*       sub)           PCB_Nonnull_Arg(2);
PCB_maybe_inline PCB_StringView PCBCALL PCB_StringView_subcstr_n(PCB_StringView    sv,  const char*       sub, size_t n) PCB_Nonnull_Arg(2);
PCB_maybe_inline PCB_StringView PCBCALL PCB_String_substr       (const PCB_String* str, const PCB_String* sub)           PCB_Nonnull_Arg(1, 2);
PCB_maybe_inline PCB_StringView PCBCALL PCB_String_subcstr      (const PCB_String* str, const char*       sub)           PCB_Nonnull_Arg(1, 2);
PCB_maybe_inline PCB_StringView PCBCALL PCB_String_substr_n     (const PCB_String* str, const PCB_String* sub, size_t n) PCB_Nonnull_Arg(1, 2);
PCB_maybe_inline PCB_StringView PCBCALL PCB_String_subcstr_n    (const PCB_String* str, const char*       sub, size_t n) PCB_Nonnull_Arg(1, 2);
/* Similarly for PCB_StringView_rsubstr. */
PCB_maybe_inline PCB_StringView PCBCALL PCB_StringView_rsubstr   (PCB_StringView    sv,  PCB_StringView    sub);
PCB_maybe_inline PCB_StringView PCBCALL PCB_StringView_rsubcstr  (PCB_StringView    sv,  const char*       sub)           PCB_Nonnull_Arg(2);
PCB_maybe_inline PCB_StringView PCBCALL PCB_StringView_rsubcstr_n(PCB_StringView    sv,  const char*       sub, size_t n) PCB_Nonnull_Arg(2);
PCB_maybe_inline PCB_StringView PCBCALL PCB_String_rsubstr       (const PCB_String* str, const PCB_String* sub)           PCB_Nonnull_Arg(1, 2);
PCB_maybe_inline PCB_StringView PCBCALL PCB_String_rsubcstr      (const PCB_String* str, const char*       sub)           PCB_Nonnull_Arg(1, 2);
PCB_maybe_inline PCB_StringView PCBCALL PCB_String_rsubstr_n     (const PCB_String* str, const PCB_String* sub, size_t n) PCB_Nonnull_Arg(1, 2);
PCB_maybe_inline PCB_StringView PCBCALL PCB_String_rsubcstr_n    (const PCB_String* str, const char*       sub, size_t n) PCB_Nonnull_Arg(1, 2);
/**
 * The same as above is done here for `PCB_StringView_split(_copy)`.
 * "PCB_String_split_copy" ->
 * "split String into Strings separated by a String",
 * "PCB_StringView_split_cstr" ->
 * "split StringView into StringViews separated by a C-string".
 * "PCB_StringView_split_char" ->
 * "split StringView into StringViews separated by a byte".
 */
PCB_maybe_inline PCB_StringViews PCBCALL PCB_StringView_split_cstr       (PCB_StringView    sv,  const char*       delim) PCB_Nonnull_Arg(2);
PCB_maybe_inline PCB_StringViews PCBCALL PCB_StringView_split_char       (PCB_StringView    sv,  const char        delim);
PCB_maybe_inline PCB_StringViews PCBCALL PCB_String_split                (const PCB_String* str, const PCB_String* delim) PCB_Nonnull_Arg(1, 2);
PCB_maybe_inline PCB_StringViews PCBCALL PCB_String_split_cstr           (const PCB_String* str, const char*       delim) PCB_Nonnull_Arg(1, 2);
PCB_maybe_inline PCB_StringViews PCBCALL PCB_String_split_char           (const PCB_String* str, const char        delim);
PCB_maybe_inline PCB_StringViews PCBCALL PCB_String_split_whitespace     (const PCB_String* str)                          PCB_Nonnull_Arg(1);
PCB_maybe_inline PCB_Strings     PCBCALL PCB_StringView_split_cstr_copy  (PCB_StringView    sv,  const char*       delim) PCB_Nonnull_Arg(2);
PCB_maybe_inline PCB_Strings     PCBCALL PCB_StringView_split_char_copy  (PCB_StringView    sv,  const char        delim);
PCB_maybe_inline PCB_Strings     PCBCALL PCB_String_split_copy           (const PCB_String* str, const PCB_String* delim) PCB_Nonnull_Arg(1, 2);
PCB_maybe_inline PCB_Strings     PCBCALL PCB_String_split_cstr_copy      (const PCB_String* str, const char*       delim) PCB_Nonnull_Arg(1, 2);
PCB_maybe_inline PCB_Strings     PCBCALL PCB_String_split_char_copy      (const PCB_String* str, const char        delim) PCB_Nonnull_Arg(1);
PCB_maybe_inline PCB_Strings     PCBCALL PCB_String_split_whitespace_copy(const PCB_String* str)                          PCB_Nonnull_Arg(1);
/* Similarly for `PCB_StringView_findCharFrom_n`. */
PCB_maybe_inline PCB_StringView PCBCALL PCB_StringView_findCharFrom          (PCB_StringView    sv,  PCB_StringView    accept);
PCB_maybe_inline PCB_StringView PCBCALL PCB_StringView_findCharFrom_cstr     (PCB_StringView    sv,  const char*       accept)           PCB_Nonnull_Arg(2);
PCB_maybe_inline PCB_StringView PCBCALL PCB_StringView_findCharFrom_cstr_n   (PCB_StringView    sv,  const char*       accept, size_t n) PCB_Nonnull_Arg(2);
PCB_maybe_inline PCB_StringView PCBCALL PCB_String_findCharFrom              (const PCB_String* str, const PCB_String* accept)           PCB_Nonnull_Arg(1, 2);
PCB_maybe_inline PCB_StringView PCBCALL PCB_String_findCharFrom_n            (const PCB_String* str, const PCB_String* accept, size_t n) PCB_Nonnull_Arg(1, 2);
PCB_maybe_inline PCB_StringView PCBCALL PCB_String_findCharFrom_cstr         (const PCB_String* str, const char*       accept)           PCB_Nonnull_Arg(1, 2);
PCB_maybe_inline PCB_StringView PCBCALL PCB_String_findCharFrom_cstr_n       (const PCB_String* str, const char*       accept, size_t n) PCB_Nonnull_Arg(1, 2);
/* Similarly for `PCB_StringView PCBCALL_findCharNotFrom_n`. */
PCB_maybe_inline PCB_StringView PCBCALL PCB_StringView_findCharNotFrom       (PCB_StringView    sv,  PCB_StringView    accept);
PCB_maybe_inline PCB_StringView PCBCALL PCB_StringView_findCharNotFrom_cstr  (PCB_StringView    sv,  const char*       accept)           PCB_Nonnull_Arg(2);
PCB_maybe_inline PCB_StringView PCBCALL PCB_StringView_findCharNotFrom_cstr_n(PCB_StringView    sv,  const char*       accept, size_t n) PCB_Nonnull_Arg(2);
PCB_maybe_inline PCB_StringView PCBCALL PCB_String_findCharNotFrom           (const PCB_String* str, const PCB_String* accept)           PCB_Nonnull_Arg(1, 2);
PCB_maybe_inline PCB_StringView PCBCALL PCB_String_findCharNotFrom_n         (const PCB_String* str, const PCB_String* accept, size_t n) PCB_Nonnull_Arg(1, 2);
PCB_maybe_inline PCB_StringView PCBCALL PCB_String_findCharNotFrom_cstr      (const PCB_String* str, const char*       accept)           PCB_Nonnull_Arg(1, 2);
PCB_maybe_inline PCB_StringView PCBCALL PCB_String_findCharNotFrom_cstr_n    (const PCB_String* str, const char*       accept, size_t n) PCB_Nonnull_Arg(1, 2);

/**
 * `PCB_String` is only 1 of 5 string types exported. The interface for
 * the remaining 4 is almost identical.
 * Refer to `PCB_String_*` variants for behavior.
 */
PCBAPI void    PCBCALL PCB_WString_destroy(PCB_WString* PCB_restrict str) PCB_Nonnull_Arg(1);
PCBAPI bool    PCBCALL PCB_WString_reserve(PCB_WString* PCB_restrict str, const size_t howMany) PCB_Nonnull_Arg(1);
PCBAPI bool    PCBCALL PCB_WString_reserve_to(PCB_WString* PCB_restrict str, const size_t cap) PCB_Nonnull_Arg(1);
PCBAPI bool    PCBCALL PCB_WString_resize(PCB_WString* PCB_restrict str, const size_t targetLength) PCB_Nonnull_Arg(1);
PCBAPI bool    PCBCALL PCB_WString_append(PCB_WString* str, const PCB_WString* other) PCB_Nonnull_Arg(1, 2);
PCBAPI bool    PCBCALL PCB_WString_append_sv(PCB_WString* PCB_restrict str, PCB_WStringView sv) PCB_Nonnull_Arg(1);
PCBAPI bool    PCBCALL PCB_WString_append_cstr(PCB_WString* PCB_restrict str, const wchar_t* PCB_maybe_restrict cstr) PCB_Nonnull_Arg(1, 2);
PCBAPI ssize_t PCBCALL PCB_WString_append_cstrs(PCB_WString* PCB_restrict str, PCB_WCStringsView cstrs) PCB_Nonnull_Arg(1);
PCBAPI ssize_t PCBCALL PCB_WString_append_cstr_v(PCB_WString* PCB_restrict str, ...) PCB_Nonnull_Arg(1);
PCBAPI bool    PCBCALL PCB_WString_append_chars(PCB_WString* PCB_restrict str, const wchar_t c, const size_t howManyTimes) PCB_Nonnull_Arg(1);

PCBAPI bool PCBCALL PCB_WString_appendf(PCB_WString* PCB_restrict str, const wchar_t* PCB_maybe_restrict fmt, ...) PCB_Nonnull_Arg(1, 2);

PCBAPI bool PCBCALL PCB_WString_append_codepoint(PCB_WString* PCB_restrict str, int32_t codepoint) PCB_Nonnull_Arg(1);
PCBAPI ssize_t PCBCALL PCB_WString_append_codepoints(PCB_WString* PCB_restrict str, PCB_U32StringView codepoints) PCB_Nonnull_Arg(1);
PCBAPI bool PCBCALL PCB_WString_insert(PCB_WString* PCB_maybe_restrict str, const PCB_WString* PCB_maybe_restrict other, size_t position) PCB_Nonnull_Arg(1, 2);
PCBAPI bool PCBCALL PCB_WString_insert_sv(PCB_WString* PCB_restrict str, PCB_WStringView sv, size_t position) PCB_Nonnull_Arg(1);
PCBAPI bool PCBCALL PCB_WString_insert_cstr(PCB_WString* PCB_restrict str, const wchar_t* PCB_maybe_restrict cstr, size_t position) PCB_Nonnull_Arg(1, 2);
PCBAPI ssize_t PCBCALL PCB_WString_insert_cstrs(PCB_WString* PCB_restrict str, PCB_WCStringsView cstrs, size_t position) PCB_Nonnull_Arg(1);
PCBAPI ssize_t PCBCALL PCB_WString_insert_cstr_v(PCB_WString* PCB_restrict str, size_t position, ...) PCB_Nonnull_Arg(1);
PCBAPI bool PCBCALL PCB_WString_insert_chars(PCB_WString* PCB_restrict str, const wchar_t c, size_t howManyTimes, size_t position) PCB_Nonnull_Arg(1);

// PCBAPI bool PCBCALL PCB_WString_insertf(PCB_WString* PCB_restrict str, const wchar_t* PCB_maybe_restrict fmt, size_t position, ...) PCB_Printf_Format(2, 4) PCB_Nonnull_Arg(1, 2);

PCBAPI bool PCBCALL PCB_WString_insert_codepoint(PCB_WString* PCB_restrict str, int32_t codepoint, size_t position) PCB_Nonnull_Arg(1);
PCBAPI ssize_t PCBCALL PCB_WString_insert_codepoints(PCB_WString* PCB_restrict str, PCB_U32StringView codepoints, size_t position) PCB_Nonnull_Arg(1);
PCBAPI bool PCBCALL PCB_WString_replace_range(PCB_WString* PCB_restrict str, size_t start, size_t length, PCB_WStringView other) PCB_Nonnull_Arg(1);
PCBAPI bool PCBCALL PCB_WString_replace_range_chars(PCB_WString* PCB_restrict str, size_t start, size_t length, const wchar_t c) PCB_Nonnull_Arg(1);
PCBAPI bool PCBCALL PCB_WString_remove_range(PCB_WString* PCB_restrict str, size_t start, size_t length) PCB_Nonnull_Arg(1);
PCBAPI bool PCBCALL PCB_WString_setSuffix_char(PCB_WString* PCB_restrict str, const wchar_t c) PCB_Nonnull_Arg(1);
PCBAPI bool PCBCALL PCB_WString_truncate_until_char(PCB_WString* PCB_restrict str, const wchar_t c) PCB_Nonnull_Arg(1);
PCBAPI PCB_WString PCBCALL PCB_WString_clone(const PCB_WString* PCB_restrict str) PCB_Nonnull_Arg(1);
PCBAPI int PCBCALL PCB_WString_compare(const PCB_WString* a, const PCB_WString* b) PCB_Nonnull_Arg(1, 2);
PCBAPI int PCBCALL PCB_WString_compare_cstr(const PCB_WString* PCB_restrict a, const wchar_t* PCB_restrict b) PCB_Nonnull_Arg(1, 2);

PCBAPI int PCBCALL PCB_WString_compare_ci(const PCB_WString* a, const PCB_WString* b) PCB_Nonnull_Arg(1, 2);
PCBAPI int PCBCALL PCB_WString_compare_cstr_ci(const PCB_WString* PCB_restrict a, const wchar_t* PCB_restrict b) PCB_Nonnull_Arg(1, 2);

PCBAPI bool PCBCALL PCB_WString_eq(const PCB_WString* a, const PCB_WString* b) PCB_Nonnull_Arg(1, 2);
PCBAPI bool PCBCALL PCB_WString_startsWith(const PCB_WString* str, const PCB_WString* other) PCB_Nonnull_Arg(1, 2);
PCBAPI bool PCBCALL PCB_WString_startsWith_cstr(const PCB_WString* PCB_restrict str, const wchar_t* PCB_restrict other) PCB_Nonnull_Arg(1, 2);
PCBAPI bool PCBCALL PCB_WString_endsWith(const PCB_WString* str, const PCB_WString* other) PCB_Nonnull_Arg(1, 2);
PCBAPI bool PCBCALL PCB_WString_endsWith_cstr(const PCB_WString* PCB_restrict str, const wchar_t* PCB_restrict other) PCB_Nonnull_Arg(1, 2);

PCBAPI void PCBCALL PCB_WString_toUpperCase(PCB_WString* PCB_restrict str) PCB_Nonnull_Arg(1);
PCBAPI void PCBCALL PCB_WString_toLowerCase(PCB_WString* PCB_restrict str) PCB_Nonnull_Arg(1);
PCBAPI PCB_WString PCBCALL PCB_WString_toUpperCase_copy(const PCB_WString* PCB_restrict str) PCB_Nonnull_Arg(1);
PCBAPI PCB_WString PCBCALL PCB_WString_toLowerCase_copy(const PCB_WString* PCB_restrict str) PCB_Nonnull_Arg(1);

PCBAPI wchar_t PCBCALL PCB_WString_pop(PCB_WString* PCB_restrict str) PCB_Nonnull_Arg(1);
PCBAPI size_t PCBCALL PCB_WString_pop_many(PCB_WString* PCB_restrict str, size_t howMany, wchar_t* PCB_restrict out) PCB_Nonnull_Arg(1);
PCBAPI size_t PCBCALL PCB_WString_removeSuffix(PCB_WString* str, const PCB_WString* other) PCB_Nonnull_Arg(1, 2);
PCBAPI void PCBCALL PCB_WString_trim_left(PCB_WString* PCB_restrict str) PCB_Nonnull_Arg(1);
PCBAPI void PCBCALL PCB_WString_trim_right(PCB_WString* PCB_restrict str) PCB_Nonnull_Arg(1);
PCB_maybe_inline void PCBCALL PCB_WString_trim(PCB_WString* PCB_restrict str) PCB_Nonnull_Arg(1);
PCBAPI bool PCBCALL PCB_WString_repeat(PCB_WString* PCB_restrict str, size_t n) PCB_Nonnull_Arg(1);
PCBAPI PCB_WString PCBCALL PCB_WString_from_StringView(PCB_WStringView sv);
PCBAPI PCB_WString PCBCALL PCB_WString_from_CStrings(const PCB_WCStrings* PCB_restrict cstrs, const wchar_t* PCB_restrict delimiter) PCB_Nonnull_Arg(1, 2);
PCBAPI PCB_WStringView PCBCALL PCB_WStringView_substr_n(PCB_WStringView sv, const PCB_WStringView sub, size_t n, size_t start);
PCBAPI PCB_WStringViews PCBCALL PCB_WStringView_split(PCB_WStringView sv, PCB_WStringView delim);
PCBAPI PCB_WStrings PCBCALL PCB_WStringView_split_copy(PCB_WStringView sv, PCB_WStringView delim);
PCBAPI PCB_WStringViews PCBCALL PCB_WStringView_split_whitespace(PCB_WStringView sv);
PCBAPI PCB_WStrings PCBCALL PCB_WStringView_split_whitespace_copy(PCB_WStringView sv);
PCBAPI PCB_WStringView PCBCALL PCB_WStringView_findCharFrom_n(PCB_WStringView sv, PCB_WStringView accept, size_t n);
PCBAPI PCB_WStringView PCBCALL PCB_WStringView_findCharNotFrom_n(PCB_WStringView sv, PCB_WStringView accept, size_t n);
//----------------------------------------------------------------------------
PCBAPI void    PCBCALL PCB_U8String_destroy(PCB_U8String* PCB_restrict str) PCB_Nonnull_Arg(1);
PCBAPI bool    PCBCALL PCB_U8String_reserve(PCB_U8String* PCB_restrict str, const size_t howMany) PCB_Nonnull_Arg(1);
PCBAPI bool    PCBCALL PCB_U8String_reserve_to(PCB_U8String* PCB_restrict str, const size_t cap) PCB_Nonnull_Arg(1);
PCBAPI bool    PCBCALL PCB_U8String_resize(PCB_U8String* PCB_restrict str, const size_t targetLength) PCB_Nonnull_Arg(1);
PCBAPI bool    PCBCALL PCB_U8String_append(PCB_U8String* str, const PCB_U8String* other) PCB_Nonnull_Arg(1, 2);
PCBAPI bool    PCBCALL PCB_U8String_append_sv(PCB_U8String* PCB_restrict str, PCB_U8StringView sv) PCB_Nonnull_Arg(1);
PCBAPI bool    PCBCALL PCB_U8String_append_cstr(PCB_U8String* PCB_restrict str, const PCB_char8* PCB_maybe_restrict cstr) PCB_Nonnull_Arg(1, 2);
PCBAPI ssize_t PCBCALL PCB_U8String_append_cstrs(PCB_U8String* PCB_restrict str, PCB_U8CStringsView cstrs) PCB_Nonnull_Arg(1);
// PCB_maybe_inline void PCBCALL PCB_U8String_trim(PCB_U8String* PCB_restrict str) PCB_Nonnull_Arg(1);

PCBAPI void    PCBCALL PCB_U16String_destroy(PCB_U16String* PCB_restrict str) PCB_Nonnull_Arg(1);
PCBAPI bool    PCBCALL PCB_U16String_reserve(PCB_U16String* PCB_restrict str, const size_t howMany) PCB_Nonnull_Arg(1);
PCBAPI bool    PCBCALL PCB_U16String_reserve_to(PCB_U16String* PCB_restrict str, const size_t cap) PCB_Nonnull_Arg(1);
PCBAPI bool    PCBCALL PCB_U16String_resize(PCB_U16String* PCB_restrict str, const size_t targetLength) PCB_Nonnull_Arg(1);
PCBAPI bool    PCBCALL PCB_U16String_append(PCB_U16String* str, const PCB_U16String* other) PCB_Nonnull_Arg(1, 2);
PCBAPI bool    PCBCALL PCB_U16String_append_sv(PCB_U16String* PCB_restrict str, PCB_U16StringView sv) PCB_Nonnull_Arg(1);
PCBAPI bool    PCBCALL PCB_U16String_append_cstr(PCB_U16String* PCB_restrict str, const PCB_char16* PCB_maybe_restrict cstr) PCB_Nonnull_Arg(1, 2);
PCBAPI ssize_t PCBCALL PCB_U16String_append_cstrs(PCB_U16String* PCB_restrict str, PCB_U16CStringsView cstrs) PCB_Nonnull_Arg(1);
// PCB_maybe_inline void PCBCALL PCB_U16String_trim(PCB_U16String* PCB_restrict str) PCB_Nonnull_Arg(1);

PCBAPI void    PCBCALL PCB_U32String_destroy(PCB_U32String* PCB_restrict str) PCB_Nonnull_Arg(1);
PCBAPI bool    PCBCALL PCB_U32String_reserve(PCB_U32String* PCB_restrict str, const size_t howMany) PCB_Nonnull_Arg(1);
PCBAPI bool    PCBCALL PCB_U32String_reserve_to(PCB_U32String* PCB_restrict str, const size_t cap) PCB_Nonnull_Arg(1);
PCBAPI bool    PCBCALL PCB_U32String_resize(PCB_U32String* PCB_restrict str, const size_t targetLength) PCB_Nonnull_Arg(1);
PCBAPI bool    PCBCALL PCB_U32String_append(PCB_U32String* str, const PCB_U32String* other) PCB_Nonnull_Arg(1, 2);
PCBAPI bool    PCBCALL PCB_U32String_append_sv(PCB_U32String* PCB_restrict str, PCB_U32StringView sv) PCB_Nonnull_Arg(1);
PCBAPI bool    PCBCALL PCB_U32String_append_cstr(PCB_U32String* PCB_restrict str, const PCB_char32* PCB_maybe_restrict cstr) PCB_Nonnull_Arg(1, 2);
PCBAPI ssize_t PCBCALL PCB_U32String_append_cstrs(PCB_U32String* PCB_restrict str, PCB_U32CStringsView cstrs) PCB_Nonnull_Arg(1);
// PCB_maybe_inline void PCBCALL PCB_U32String_trim(PCB_U32String* PCB_restrict str) PCB_Nonnull_Arg(1);

/**
 * @brief Check if `codepoint` is a valid Unicode character.
 */
PCBAPI bool PCBCALL PCB_IsValidUnicode(int32_t codepoint);
/**
 * @brief Get the UTF-8 length of the Unicode `codepoint`.
 *
 * This function is unsafe; only use it when certain that `codepoint` is a
 * valid Unicode character. Otherwise the behavior is undefined.
 * You've been warned.
 * @return value in range [1, 4*].
 *
 * * - the full theoretical range of [1, 6] is available by #defining
 * `PCB_UTF8_FULL_RANGE`. It is disabled by default in compliance with RFC 3629.
 */
PCBAPI uint8_t PCBCALL PCB_GetUTF8Length_unchecked(int32_t codepoint);
/**
 * @brief Get the UTF-8 length of the Unicode `codepoint`.
 * @return value in range [1, 4*] or 0 if `codepoint` is invalid.
 *
 * * - the full theoretical range of [1, 6] is available by #defining
 * `PCB_UTF8_FULL_RANGE`. It is disabled by default in compliance with RFC 3629.
 */
PCBAPI uint8_t PCBCALL PCB_GetUTF8Length(int32_t codepoint);
/**
 * @brief Store UTF-8-encoded `codepoint` in `buf`.
 *
 * This function is unsafe; only use it when certain that `buf` can hold
 * `codepoint` and that `codepoint` is valid. Otherwise the behavior is undefined.
 * You've been warned.
 * @return position after the encoded codepoint
 */
PCBAPI char* PCBCALL PCB_StoreUTF8Codepoint(
    char* buf,
    int32_t codepoint
) PCB_Nonnull_Arg(1) PCB_Nonnull_Return;
PCBAPI uint16_t* PCBCALL PCB_StoreUTF16Codepoint(
    uint16_t* buf,
    int32_t codepoint
) PCB_Nonnull_Arg(1) PCB_Nonnull_Return;



/**
 * @brief Returns a `PCB_Process` structure with data
 * about itself. Not implemented.
 */
PCBAPI PCB_Process PCBCALL PCB_Process_self(void);
/**
 * @brief Create a `process` struct with invalid fields (all other functions
 * rely on this fact). This function MUST be used instead of standard
 * zero-initialization, otherwise the behavior is undefined.
*/
PCBAPI PCB_Process PCBCALL PCB_Process_init(void);
/**
 * @brief Checks whether `process` is a valid process.
 * NOTE: does NOT check whether `process` is an existing process.
 */
PCBAPI bool PCBCALL PCB_Process_isValid(const PCB_Process* process) PCB_Nonnull_Arg(1);
/**
 * @brief Waits for `process` to exit.
 * @return `true` if `process` exited, `false` on error; call `PCB_GetError()`
 * to get the error code.
 */
PCBAPI bool PCBCALL PCB_Process_waitForExit(PCB_Process* process) PCB_Nonnull_Arg(1);
/**
* @brief Checks if `process` exited.
* @return `true` if `process` exited, `false` if not, -1 on error;
* call `PCB_GetError()` to get the error code.
*/
PCBAPI int PCBCALL PCB_Process_checkExit(PCB_Process* process) PCB_Nonnull_Arg(1);
/**
* @brief Get the exit code of `process`.
* @return
* On POSIX systems:
*
* - value in the range of [0, 255] for a normal exit,
*
* - `-s-1` if terminated by a signal with a numeric value `s`,
*
* - -1 otherwise - this can mean that `process` was stopped/continued; use
*   `WIFSTOPPED`, `WSTOPSIG`, `WIFCONTINUED` macros inside
*   `#if PCB_PLATFORM_POSIX` on `process->status`
*   to handle that case since it's platform-specific.
*/
PCBAPI int PCBCALL PCB_Process_getExitCode(const PCB_Process* process) PCB_Nonnull_Arg(1);
/**
 * @brief Destroys the passed `process` structure, invalidates
 * its member fields.
 */
PCBAPI void PCBCALL PCB_Process_destroy(PCB_Process* process);

/**
 * @brief Waits for any process in `processes` to exit.
 * @return index of the exited process, -1 on error (call `PCB_GetError()`
 * to get the error code) or `-x-1` where `x` is the invalid entry index.
 */
PCBAPI int PCBCALL PCB_Processes_waitForAny(PCB_Processes* processes) PCB_Nonnull_Arg(1);
/**
 * @brief Waits for a subset of processes in `processes` in a range of
 * [`start`, `end`).
 * @return
 * - 0 on success,
 * -1 if `processes == NULL` (errno is set to EFAULT) or
 *   `end > processes->length || start >= end` (errno is set to EINVAL),
 * `-n-1` if waiting for the `n`th (at index `n-1`) process failed
 * (flip the sign and add 2 to get the index).
 *
 * In the last case, processes after the `n`th one are NOT waited on;
 * the caller must retry waiting on the rest.
 */
PCBAPI int PCBCALL PCB_Processes_waitForRange(
    PCB_Processes* PCB_restrict processes,
    size_t start,
    size_t end
) PCB_Nonnull_Arg(1);
/**
 * @brief Waits for all processes in `processes` to exit.
 * This function is equivalent to
 * `PCB_Processes_waitForRange(processes, 0, processes->length)`.
 * Read its documentation before using this function.
 * @return See `PCB_Processes_waitForRange`.
 */
PCBAPI int PCBCALL PCB_Processes_waitForAll(PCB_Processes* processes) PCB_Nonnull_Arg(1);

/**
 * @brief Spawns a child process, which runs `command` concurrently.
 * @return a valid `PCB_Process` structure with information about the
 * child process or a structure with an invalid `handle` field on error.
 * To check it, use `PCB_Process_isValid`. The error is logged automatically.
 *
 * On POSIX systems, if `command` is not null-terminated, this function will
 * append `NULL` to `command` prior to calling `exec` and remove it afterwards.
 */
PCBAPI PCB_Process PCBCALL PCB_ShellCommand_runBg(PCB_ShellCommand* command) PCB_Nonnull_Arg(1);
/**
 * @brief Runs `command` and waits for it to exit.
 * @return the exit code of `command` or -1 on error,
 * to get the error code call `PCB_GetError()`. The error is logged automatically.
 */
PCBAPI int PCBCALL PCB_ShellCommand_runAndWait(PCB_ShellCommand* command) PCB_Nonnull_Arg(1);
/**
 * @brief Constructs a flat string from `command` with escaping.
 * @return an initialized `PCB_String` structure or a zeroed out one on failure.
 */
PCBAPI PCB_String PCBCALL PCB_ShellCommand_render(const PCB_ShellCommand* command) PCB_Nonnull_Arg(1);



/**
 * @brief Creates a new arena allocator with `size` bytes as initial capacity.
 * @return a valid pointer to the arena
 * or NULL if `size == 0` or the allocation failed.
 * @sa PCB_Arena_destroy
 */
PCBAPI PCB_Arena* PCBCALL PCB_Arena_init(size_t size);
/**
 * @brief Initialize a `PCB_Arena` in a chunk of memory pointed to by `mem`
 * and size `memsize`.
 * @return a valid pointer to the arena or NULL if `memsize` is insufficient to
 * hold the arena.
 * @sa PCB_Arena_destroy
 */
PCBAPI PCB_Arena* PCBCALL PCB_Arena_init_in(void* mem, size_t memsize) PCB_Nonnull_Arg(1);
/**
 * @brief Allocates `size` bytes in `arena`.
 *
 * The actual number of bytes allocated will be rounded up to `sizeof(void*)`.
 *
 * @return a valid pointer to the allocated buffer aligned to
 * pointer size or NULL if `size == 0` or if allocation failed
 */
PCBAPI void* PCBCALL PCB_Arena_alloc(PCB_Arena* arena, size_t size) PCB_Nonnull_Arg(1);
/**
 * @brief Same as `PCB_Arena_alloc`, but zeroes the buffer before return.
 */
PCBAPI void* PCBCALL PCB_Arena_calloc(PCB_Arena* arena, size_t size) PCB_Nonnull_Arg(1);
/**
 * @brief Allocates `size` bytes in `arena` with at least `alignment` alignment.
 *
 * The actual number of bytes allocated will be rounded up to `sizeof(void*)`.
 *
 * `alignment` MUST be a power of 2 and a multiple of `sizeof(void*)` (1).
 *
 * @return a valid pointer to the allocated buffer aligned to
 * `min(sizeof(void*), alignment)` or NULL if any of the following occurs:
 *
 * - `size == 0 || alignment == 0`,
 *
 * -`size` is not a multiple of `alignment`,
 *
 * - `alignment` is invalid,
 *
 * - allocation failed.
 *
 * @sa (1) posix_memalign(3)
 */
PCBAPI void* PCBCALL PCB_Arena_aligned_alloc(
    PCB_Arena* arena,
    size_t size,
    size_t alignment
) PCB_Nonnull_Arg(1);
/**
 * @brief Same as `PCB_Arena_aligned_alloc`, but zeroes the buffer before return.
 */
PCBAPI void* PCBCALL PCB_Arena_aligned_calloc(
    PCB_Arena* arena,
    size_t size,
    size_t alignment
) PCB_Nonnull_Arg(1);
/**
 * @brief Allocates the entire contiguous memory block left unallocated in `arena`.
 * If `arena` has a next node, that node is not considered for allocation.
 *
 * A pointer to the allocated memory is stored in `*ptr`
 * and the number of bytes allocated is stored in `*size`.
 * If there is no space left, `*ptr` is set to NULL and `*size` is set to 0.
 *
 * If `ptr == NULL`, memory is not allocated and `*size` is set to the number
 * of bytes that _would_ be allocated, i.e. queries `arena` for how much space
 * is left inside it.
 *
 * @return false if `arena == NULL || size == NULL` (subject to `PCB_SAFETY_CHECKS`),
 * true otherwise.
 */
PCBAPI bool PCBCALL PCB_Arena_alloc_whole(
    PCB_Arena* arena,
    void** ptr,
    size_t* size
) PCB_Nonnull_Arg(1, 3);
/**
 * @brief Returns a pointer to the next arena in the internal linked list
 * or NULL if `arena == NULL` or `arena` doesn't have a next node.
 */
PCBAPI PCB_Arena* PCBCALL PCB_Arena_next(PCB_Arena* arena) PCB_Nonnull_Arg(1);
/**
 * @brief Get the number of bytes already allocated in `arena` or (size_t)-1
 * if `arena == NULL` (affected by `PCB_SAFETY_CHECKS`).
 */
PCBAPI size_t PCBCALL PCB_Arena_allocated(PCB_Arena* arena) PCB_Nonnull_Arg(1);
/**
 * @brief Get the number of bytes already allocated in `arena` or (size_t)-1
 * if `arena == NULL` (affected by `PCB_SAFETY_CHECKS`).
 *
 * NOTE: The entire chain is counted.
 */
PCBAPI size_t PCBCALL PCB_Arena_allocated_all(PCB_Arena* arena) PCB_Nonnull_Arg(1);
/**
 * @brief Get the number of bytes that can be allocated in `arena` or (size_t)-1
 * if `arena == NULL` (affected by `PCB_SAFETY_CHECKS`).
 *
 * NOTE: The entire chain is counted.
 */
PCBAPI size_t PCBCALL PCB_Arena_allocatable(PCB_Arena* arena) PCB_Nonnull_Arg(1);
/**
 * @brief Get `arena`'s current capacity or (size_t)-1
 * if `arena == NULL` (affected by `PCB_SAFETY_CHECKS`).
 *
 * NOTE: The entire chain is counted.
 */
PCBAPI size_t PCBCALL PCB_Arena_capacity(PCB_Arena* arena) PCB_Nonnull_Arg(1);
/**
 * @brief Create a mark for `arena`.
 *
 * A mark stores information about how much was allocated when this function
 * was called. It is used in conjunction with `PCB_Arena_restore(_to)` to implement
 * a stack allocator. Think of it as `asm("push rbp")`.
 *
 * `mark` is allocated in `arena` and cannot be used in `PCB_Arena_restore(_to)`
 * with a different arena.
 * @return pointer to the mark or NULL if allocation failed.
 * @sa PCB_Arena_alloc
 */
PCBAPI PCB_ArenaMark* PCBCALL PCB_Arena_mark(PCB_Arena* arena) PCB_Nonnull_Arg(1);
/**
 * @brief Restore the state of `arena` from `mark`.
 * `mark` becomes invalid after this function returns `true`.
 * You CANNOT use the same mark multiple times!!
 * @return `true` on success or `false` if `mark == NULL` or `arena`
 * holds less than what is recorded in `mark` (this can happen if you messed
 * up the LIFO order of `PCB_Arena_restore`) or if `mark` was not
 * allocated in `arena`.
 */
PCBAPI bool PCBCALL PCB_Arena_restore(PCB_Arena* arena, PCB_ArenaMark* mark) PCB_Nonnull_Arg(1, 2);
/**
 * @brief Restore the state of `arena` from `mark`.
 * Contrary to `PCB_Arena_restore`, `mark` is preserved and can be used later.
 * This is useful in memory-hungry loops.
 * @return same as `PCB_Arena_restore`
 */
PCBAPI bool PCBCALL PCB_Arena_restore_to(PCB_Arena* arena, PCB_ArenaMark* mark) PCB_Nonnull_Arg(1, 2);
#ifndef PCB_Arena_scope
/**
 * @brief Create an allocation scope for `arena`.
 * The lifetime of objects allocated in this scope ends when leaving it.
 * It is defined as a `for` loop that runs exactly once, so you can leave
 * the scope prematurely by `continue`ing (⚠️do NOT however `break`⚠️, otherwise
 * objects won't be deallocated!!!).
 */
#define PCB_Arena_scope(arena)                                      \
for(                                                                \
    PCB_ArenaMark* PCB_MANGLE(m) = PCB_Arena_mark(arena);           \
    PCB_MANGLE(m) != NULL;                                          \
    PCB_Arena_restore(arena, PCB_MANGLE(m)), PCB_MANGLE(m) = NULL   \
)
#endif //PCB_Arena_scope
/**
 * @brief Deallocates `length` bytes from the back of `arena`.
 *
 * Note that if the last non-empty node has less bytes allocated than `length`,
 * then the remaining bytes will be deallocated from previous nodes.
 * This may not be what you want - in that case use `PCB_Arena_dealloc_once` instead.
 * The actual number of bytes deallocated will be rounded up to pointer size.
 *
 * @return number of bytes actually deallocated - always
 * a multiple of pointer size
 */
PCBAPI size_t PCBCALL PCB_Arena_dealloc(PCB_Arena* arena, size_t length) PCB_Nonnull_Arg(1);
/**
 * @brief Deallocates `length` bytes from the back of `arena`,
 * but only from the last non-empty node, i.e. if the last non-empty node
 * has less bytes allocated than `length`, then that amount will be deallocated.
 * The actual number of bytes deallocated will be rounded up to pointer size.
 *
 * @return number of bytes actually deallocated - always
 * a multiple of pointer size
 */
PCBAPI size_t PCBCALL PCB_Arena_dealloc_once(PCB_Arena* arena, size_t length) PCB_Nonnull_Arg(1);
/**
 * @brief Resets `arena` as if nothing was allocated.
 */
PCBAPI void PCBCALL PCB_Arena_reset(PCB_Arena* arena) PCB_Nonnull_Arg(1);
/**
 * @brief Destroys `arena`, i.e. frees blocks contained within it.
 * After this call, `arena` becomes a dangling pointer!
 *
 * Do not call this function if `arena` was created with `PCB_Arena_init_in`.
 * In that case the caller is responsible for managing the backing store.
 * You still need to call this function on `PCB_Arena_next(arena)`.
 */
PCBAPI void PCBCALL PCB_Arena_destroy(PCB_Arena* arena);
/**
 * @brief `sprintf`s a new string in `arena`, variadic version.
 * @return pointer to the allocated string or NULL on error.
 * @sa PCB_Arena_alloc
 *
 * This function is only available if the library was compiled with stdio.h present.
 * Otherwise it always returns NULL.
 */
PCBAPI char* PCBCALL PCB_Arena_asprintf(
    PCB_Arena* arena,
    const char* fmt,
    ...
) PCB_Printf_Format(2, 3) PCB_Nonnull_Arg(1, 2);
/**
 * @brief `sprintf`s a new string in `arena`, argument version.
 * @return pointer to the allocated string or NULL on error.
 * @sa PCB_Arena_alloc
 *
 * This function is only available if the library was compiled with stdio.h present.
 * Otherwise it always returns NULL.
 */
PCBAPI char* PCBCALL PCB_Arena_vasprintf(
    PCB_Arena* arena,
    const char* fmt,
    va_list args
) PCB_Nonnull_Arg(1, 2);
/**
 * @brief Duplicates `str` in `arena`.
 * @return pointer to the duplicated string or NULL if `str == NULL` or if
 * allocation failed.
 * @sa PCB_Arena_alloc
 */
PCBAPI char* PCBCALL PCB_Arena_strdup(
    PCB_Arena* arena,
    const char* str
) PCB_Nonnull_Arg(1, 2);
/**
 * @brief Duplicates `str` in `arena`, copying at most `n` bytes.
 * @return pointer to the duplicated string or NULL if `str == NULL` or if
 * allocation failed.
 * @sa PCB_Arena_alloc
 */
PCBAPI char* PCBCALL PCB_Arena_strndup(
    PCB_Arena* arena,
    const char* str,
    size_t n
) PCB_Nonnull_Arg(1, 2);


/**
 * @brief Get the number of cores in the system.
 * A value of 0 is returned on platforms that are not supported.
 */
PCBAPI size_t PCBCALL PCB_getNumberOfCores(void);


/**
 * @brief Get the string version of the C standard from an integer value
 * `standard` (for example, `199901` for "c99").
 * @return a pointer to a static read-only string or NULL if a match wasn't found.
 */
PCBAPI const char* PCBCALL PCB_GetCStandardStr(long standard);
/**
 * @brief Get the string version of the C standard from an integer value
 * `standard` (for example, `202002` for "c++20").
 * @return a pointer to a static read-only string or NULL if a match wasn't found.
 */
PCBAPI const char* PCBCALL PCB_GetCppStandardStr(long standard);

/**
 * @brief Get the integer version of the C standard from a C string.
 *
 * @param standard C string containing "cX" where "X" identifies
 * the standard (for example "c99" for `199901`).
 * 'c' in "cX" MUST be lowercase, such is the requirement of GCC/Clang
 * for "-std=" flag.
 * @return non-zero integer value of a standard or 0 if a match wasn't found.
 * For C89, the value returned is exceptionally `1` since it didn't have
 * a specific value associated.
 */
PCBAPI long PCBCALL PCB_GetCStandardInt(const char* standard) PCB_Nonnull_Arg(1);
/**
 * @brief Get the integer version of the C++ standard from a C string
 *
 * @param standard C string containing "c++X", where "X" identifies
 * the standard (for example "c++20" for `202002`).
 * 'c' in "c++X" MUST be lowercase, such is the requirement of GCC/Clang
 * for "-std=" flag.
 * @return non-zero integer value of a standard or 0 if a match wasn't found.
 */
PCBAPI long PCBCALL PCB_GetCppStandardInt(const char* standard) PCB_Nonnull_Arg(1);
/**
 * @brief Format `*target` into platform-specific name, based on `bt`.
 *
 * If `bt == PCB_BUILDTYPE_EXEC` and we're not on Windows, no change is made.
 * If `*target` has a file extension, no change is made.
 * Otherwise a new string is allocated in `arena`, formatted based on
 * the platform's conventions and supersedes `*target`.
 *
 * For example, if `bt == PCB_BUILDTYPE_DYNAMICLIB`, `*target` is "bin/foo" and
 * we're on Linux, `*target` is replaced with "bin/libfoo.so".
 *
 * You may get strange results if `*target` is a weird filepath, like "/..////./".
 *
 * @return false if allocation fails, true otherwise.
 */
PCBAPI bool PCBCALL PCB_BuildType_formatName(PCB_BuildType bt, const char** target, PCB_Arena* arena) PCB_Nonnull_Arg(2, 3);
/**
 * @brief Appends a flag to `cmd` that specifies "compile without linking" based on `s`.
 */
PCBAPI void PCBCALL PCB_build_flag_cwl(PCB_ShellCommand* cmd, PCB_ArgvSyntax s) PCB_Nonnull_Arg(1);
/**
 * @brief Appends a flag to `cmd` that specifies "output to this filepath" based on `s`.
 */
PCBAPI void PCBCALL PCB_build_flag_output(PCB_ShellCommand* cmd, PCB_ArgvSyntax s) PCB_Nonnull_Arg(1);
/**
 * @brief Appends a flag to `cmd` that specifies the language `standard` based on
 * `compiler`. The flag is allocated in `arena`.
 * If `gnu`, GNU extensions are enabled, unless `compiler` doesn't support them
 * (for example `PCB_COMPILERT_RT_MSVC`).
 * If `cpp`, the C++ version is used. Otherwise the C version is used.
 *
 * @return false if allocation failed, true otherwise.
 * If `standard` is not a valid C/C++ standard integer value or
 * `compiler == PCB_COMPILER_RT_UNKNOWN`, true is returned and a warning is issued.
 */
PCBAPI bool PCBCALL PCB_build_flag_standard(
    PCB_ShellCommand* cmd,
    long standard,
    PCB_Arena* arena,
    PCB_Compiler_RT compiler,
    bool cpp,
    bool gnu
) PCB_Nonnull_Arg(1, 3);
/**
 * @brief Appends warning flags to `cstrs` chosen by the author of this library.
 * The specific set of warnings depend on the `compiler`, whether we're in `cpp`,
 * the compiler version and whether strict ISO conformance is requested.
 */
PCBAPI void PCBCALL PCB_build_flags_diagnostics_default(
    PCB_CStrings* cstrs,
    PCB_Compiler_RT compiler,
    bool cpp,
    int compilerVersion,
    bool strict_iso
) PCB_Nonnull_Arg(1);
#ifndef PCB_build_flags_diagnostics_current_default
#ifdef __cplusplus
#define PCB_build_flags_diagnostics_current_default(cstrs) \
    PCB_build_flags_diagnostics_default(cstrs, PCB_COMPILER_RT_CURRENT, true, PCB_COMPILER_VERSION, PCB_STRICT_ISO)
#else
#define PCB_build_flags_diagnostics_current_default(cstrs) \
    PCB_build_flags_diagnostics_default(cstrs, PCB_COMPILER_RT_CURRENT, false, PCB_COMPILER_VERSION, PCB_STRICT_ISO)
#endif //C++?
#endif //PCB_build_flags_diagnostics_current_default


/**
 * @brief Initializes the passed `context`.
 *
 * @param flags PCB_BuildOptions OR'ed together
 * @return 0 on success or non-zero value on error
 */
PCBAPI int PCBCALL PCB_BuildContext_init(PCB_BuildContext* context, uint64_t flags) PCB_Nonnull_Arg(1);
/**
 * @brief Create a PCB_BuildContext struct.
 *
 * This function exists for lazy users. It is generally
 * recommended to zero-initialize a `PCB_BuildContext`
 * and pass it to `PCB_BuildContext_init`, because it
 * allows for detecting initialization errors.
 *
 * @param flags PCB_BuildOptions OR'ed together
 * @return an initialized `PCB_BuildContext` struct.
 * Keep in mind that this function may silently fail,
 * which can cause subtle bugs.
 */
PCBAPI PCB_BuildContext PCBCALL PCB_BuildContext_create(uint64_t flags);
/**
 * @brief Resets `context`.
 * Identical to `PCB_BuildContext_destroy`, but memory is not deallocated.
 */
PCBAPI void PCBCALL PCB_BuildContext_reset(PCB_BuildContext* context) PCB_Nonnull_Arg(1);
/**
 * @brief Destroys the passed `context`.
 * Frees any memory allocated by PCB functions that
 * accept a `PCB_BuildContext` struct 
 * and resets every field to 0.
 *
 * Note that if you allocated memory in this structure
 * manually, you have to also free it manually.
 *
 * @param context build context
 */
PCBAPI void PCBCALL PCB_BuildContext_destroy(PCB_BuildContext* context) PCB_Nonnull_Arg(1);
/**
 * @brief Build the target stored in `context`.
 * @return 0 on success,
 * a negative value on internal error,
 * a positive value on external (i.e. inside a command) error
 */
PCBAPI int PCBCALL PCB_build_fromContext(PCB_BuildContext* context);

/**
 * @brief Gather source files for the compilation step.
 * This function is called automatically by `PCB_build_fromContext`,
 * but you can use it prior to that together with
 * `PCB_BuildContext_needsRebuild` for watching files in a live application.
 * @return a non-negative integer of source files found on success or
 * a negative integer otherwise
 */
PCBAPI int PCBCALL PCB_BuildContext_gatherSources(PCB_BuildContext* context) PCB_Nonnull_Arg(1);

/**
 * @brief Whether some source in the build context needs to be rebuilt.
 * @return `true` if something needs to be rebuilt, `false` if not, a negative
 * error code otherwise
 */
PCBAPI int PCBCALL PCB_BuildContext_needsRebuild(PCB_BuildContext* context) PCB_Nonnull_Arg(1);

/**
 * @brief Attempt to identify the compiler from `path`.
 *
 * @param path path/to/some/executable
 * @return an identified compiler
 * or `PCB_COMPILER_RT_UNKNOWN` if identification failed
 */
PCBAPI PCB_Compiler_RT PCBCALL PCB_getCompiler(const char* path) PCB_Nonnull_Arg(1);


/**
 * @brief This function does some nice shit, like
 * automatically rebuilding the build system.
 *
 * To use it, simply put it as the first thing in main:
 * ```c
int main(int argc, char *argv[]) {
    PCB_REBUILD_THIS_SHIT(argc, argv);
    ...
}
```
 * After that and building the file, every subsequent change will be
 * detected and the file will be automatically rebuilt when the program is ran.
 * This way, you only need to build the build system manually once.
 *
 * The idea is shamelessly stole-I mean inspired by tsoding's nob.h's
 * "Go Rebuild Urself™ Technology" because it's simple and utterly brilliant.
 *
 * It is recommended to not use this function directly as it is
 * subject to change and sort of clunky. The vast majority of use cases are
 * handled by `PCB_REBUILD_THIS_SHIT`.
 *
 * @param src path/to/the/file to be rebuilt, only relevant if it
 * calls this function as the first statement in main(int, char*[]),
 * using it outside that will provide strange results.
 */
PCBAPI void PCBCALL PCB_rebuild_shit(int argc, char** argv, const char* src) PCB_Nonnull_Arg(2, 3);

//name shamelessly stolen from nob.h...or not
#ifndef PCB_REBUILD_THIS_SHIT
#define PCB_REBUILD_THIS_SHIT(argc, argv) PCB_rebuild_shit(argc, argv, __FILE__)
#endif //PCB_REBUILD_THIS_SHIT


/* --------------------------------------------------------------- */
/*--------------------  libc fallbacks  ---------------------------*/
/* --------------------------------------------------------------- */

#ifndef PCB_memcpy
PCBAPI void* PCBCALL PCB_memcpy(
    void* PCB_restrict dest, const void* PCB_restrict src, size_t n
);
#endif //PCB_memcpy unavailable externally

#ifndef PCB_memmove
PCBAPI void* PCBCALL PCB_memmove(void* dest, const void* src, size_t n);
#endif //PCB_memmove unavailable externally

#ifndef PCB_memset
PCBAPI void* PCBCALL PCB_memset(void* dest, int v, size_t n);
#endif //PCB_memset unavailable externally

#ifndef PCB_memcmp
PCBAPI int PCBCALL PCB_memcmp(const void* p1, const void* p2, size_t n);
#endif //PCB_memcmp

#ifndef PCB_strcmp
PCBAPI int PCBCALL PCB_strcmp(const char *s1, const char *s2);
#endif //PCB_strcmp

#ifndef PCB_strncmp
PCBAPI int PCBCALL PCB_strncmp(const char *s1, const char *s2, size_t n);
#endif //PCB_strncmp

#ifndef PCB_strncasecmp
PCBAPI int PCBCALL PCB_strncasecmp(const char *s1, const char *s2, size_t n);
#endif //PCB_strncasecmp

#ifndef PCB_strlen
PCBAPI size_t PCBCALL PCB_strlen(const char *s);
#endif //PCB_strlen

#ifndef PCB_strnlen
PCBAPI size_t PCBCALL PCB_strnlen(const char *s, size_t n);
#endif //PCB_strnlen

#ifndef PCB_isspace
PCBAPI int PCBCALL PCB_isspace(int ch);
#endif //PCB_isspace

#if !defined(PCB__ASSERT_HANDLED)
PCBAPI PCB_NoReturn void PCBCALL PCB__assert_fail(
    const char* exprStr, const char* file, unsigned int line, const char* func
);
#endif //PCB_HAS_ASSERT_H

/* --------------------------------------------------------------- */
/* -- Convenience inline functions for different argument types -- */
/* --------------------------------------------------------------- */



#endif //PCB_NO_DECLARATIONS



//Section 3: Implementation

#ifdef PCB_IMPLEMENTATION

#ifndef PCB_IMPLEMENTATION_LOG
#define PCB_IMPLEMENTATION_LOG
#endif //PCB_IMPLEMENTATION_LOG

#ifndef PCB_IMPLEMENTATION_ERR
#define PCB_IMPLEMENTATION_ERR
#endif //PCB_IMPLEMENTATION_ERR

#ifndef PCB_IMPLEMENTATION_FS
#define PCB_IMPLEMENTATION_FS
#endif //PCB_IMPLEMENTATION_FS

#ifndef PCB_IMPLEMENTATION_STRING
#define PCB_IMPLEMENTATION_STRING
#endif //PCB_IMPLEMENTATION_STRING

#ifndef PCB_IMPLEMENTATION_PROCESS
#define PCB_IMPLEMENTATION_PROCESS
#endif //PCB_IMPLEMENTATION_PROCESS

#ifndef PCB_IMPLEMENTATION_ARENA
#define PCB_IMPLEMENTATION_ARENA
#endif //PCB_IMPLEMENTATION_ARENA

#ifndef PCB_IMPLEMENTATION_BUILD
#define PCB_IMPLEMENTATION_BUILD
#endif //PCB_IMPLEMENTATION_BUILD

#ifndef PCB_IMPLEMENTATION_CONSTANTS
#define PCB_IMPLEMENTATION_CONSTANTS
#endif //PCB_IMPLEMENTATION_CONSTANTS

#ifndef PCB_IMPLEMENTATION_LIBC_FALLBACKS
#define PCB_IMPLEMENTATION_LIBC_FALLBACKS
#endif //PCB_IMPLEMENTATION_LIBC_FALLBACKS

#endif //PCB_IMPLEMENTATION


/* ---------------------------------------------------------------- */
/* ------------------------ Private macros ------------------------ */
/* ---------------------------------------------------------------- */
#define PCB__defer_l(val, label) PCB_defer_varl(result, val, label)
#define PCB__return_defer(val)   PCB_defer_varl(result, val, defer)

#ifdef PCB_DEBUG_SELF
#define PCB__logTrace PCB_logTrace
#else
#define PCB__logTrace(...)
#endif //PCB_DEBUG_SELF

#if defined(PCB_DEBUG_SELF) && PCB_DEBUG_SELF+0
#define PCB__logDebug PCB_logDebug
#else
#define PCB__logDebug(...)
#endif //PCB_DEBUG_SELF

//these functions should be moved somewhere else, for now they're here
#ifdef PCB_IMPLEMENTATION_GLOBALS
const int8_t PCB_SAFETY_CHECKS_LEVEL = PCB_SAFETY_CHECKS;

#ifdef PCB_UNICODE_CONFORMANT
const bool PCB_STRICT_UNICODE_CONFORMANCE = true;
#else
const bool PCB_STRICT_UNICODE_CONFORMANCE = false;
#endif //PCB_UNICODE_CONFORMANT

const uint32_t PCB_LIB_VERSION = PCB_VERSION;
#endif //PCB_IMPLEMENTATION_GLOBALS

#ifdef PCB_IMPLEMENTATION_LIBC_FALLBACKS
#ifndef PCB_HAS_ERRNO_H
int errno_stub = 11;
#endif //PCB_HAS_ERRNO_H
#ifndef PCB_strcmp
int PCB_strcmp(const char* s1, const char* s2) {
    const unsigned char* x1 = (const unsigned char*)s1;
    const unsigned char* x2 = (const unsigned char*)s2;
    while(*x1 && *x1 == *x2) { ++x1; ++x2; }
    return (*x1 > *x2) - (*x1 < *x2);
}
#define PCB_strcmp PCB_strcmp
#endif //PCB_strcmp

#ifndef PCB_strncmp
int PCB_strncmp(const char* s1, const char* s2, size_t n) {
    const unsigned char* x1 = (const unsigned char*)s1;
    const unsigned char* x2 = (const unsigned char*)s2;
    while(n > 0 && *x1 && *x1 == *x2) { ++x1; ++x2; --n; }
    return n == 0 ? 0 : ((*x1 > *x2) - (*x1 < *x2));
}
#define PCB_strncmp PCB_strncmp
#endif //PCB_strncmp

#ifndef PCB_strncasecmp
static int PCB_toupper(int ch) { return (ch >= 'a' && ch <= 'a') ? ch - 'a' - 'A' : ch; }
static int PCB_tolower(int ch) { return (ch >= 'A' && ch <= 'Z') ? ch + 'a' - 'A' : ch; }

//https://stackoverflow.com/questions/7299119/source-code-for-strncasecmp-function
int PCB_strncasecmp(const char* s1, const char* s2, size_t n) {
    if(n == 0) return 0;
    const unsigned char* x1 = (const unsigned char*)s1;
    const unsigned char* x2 = (const unsigned char*)s2;
    while(n > 0 && PCB_tolower(*x1) == PCB_tolower(x2)) { ++x1; ++x2; --n; }
    unsigned char c1 = PCB_tolower(*x1), c2 = PCB_tolower(*x2);
    return n == 0 ? 0 : ((c1 > c2) - (c1 < c2));
}
#define PCB_strncasecmp PCB_strncasecmp
#endif //PCB_strncasecmp

#ifndef PCB_strlen
size_t PCB_strlen(const char* s) {
    const char* cursor = s; while(*cursor++);
    return (size_t)(cursor - s);
}
#define PCB_strlen PCB_strlen
#endif //PCB_strlen

#ifndef PCB_strnlen
size_t PCB_strnlen(const char* s, size_t n) {
    const char* cursor = s;
    while(n > 0 && *cursor) { ++cursor; --n; }
    return (size_t)(cursor - s);
}
#define PCB_strnlen PCB_strnlen
#endif //PCB_strnlen

#ifndef PCB_memcpy
void* PCB_memcpy(void* PCB_restrict dest, const void* PCB_restrict src, size_t n) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while(n > 0) *d++ = *s++, --n;
    return dest;
}
#define PCB_memcpy PCB_memcpy
#endif //PCB_memcpy

#ifndef PCB_memmove
void* PCB_memmove(void* dest, const void* src, size_t n) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    if(s < d && d < s + n) { //overlap check?
        s += n; d += n;
        while(n-- > 0) *--d = *--s;
    }
    else while(n-- > 0) *d++ = *s++;
    return dest;
}
#define PCB_memmove PCB_memmove
#endif //PCB_memmove

#ifndef PCB_memset
void* PCB_memset(void* s, int v, size_t n) {
    char* p = (char*)s;
    while(n-- > 0) *p++ = (char)v;
    return s;
}
#define PCB_memset PCB_memset
#endif //PCB_memset

#ifndef PCB_memcmp
int PCB_memcmp(const void* p1, const void* p2, size_t n) {
    const unsigned char* x1 = (const unsigned char*)p1;
    const unsigned char* x2 = (const unsigned char*)p2;
    while(*x1 == *x2 && n > 0) { ++x1; ++x2; --n; }
    return (*x1 > *x2) - (*x1 < *x2);
}
#define PCB_memcmp PCB_memcmp
#endif //PCB_memcmp

#ifndef PCB_isspace
int PCB_isspace(int ch) {
    switch(ch) {
        case ' ' : case '\t': case '\n':
        case '\r': case '\v': case '\f':
            return 1;
    } return 0;
}
#define PCB_isspace PCB_isspace
#endif //PCB_isspace

#if !defined(PCB__ASSERT_HANDLED)
void PCB__assert_fail(
    const char* exprStr, const char* file, unsigned int line, const char* func
) {
#if defined(PCB_HAS_STDIO_H) && defined(PCB_HAS_STDLIB_H)
    fprintf(stderr, "%s:%d:%s: assertion \"%s\" failed.", file, line, func, exprStr);
    abort();
#else
    (void)exprStr; (void)file; (void)line; (void)func;
#error "PCB Error: Assertions cannot be provided or are not implemented for the current platform."
}
#endif //sources of assertions
#endif //PCB_HAS_ASSERT_H
#endif //PCB_IMPLEMENTATION_LIBC_FALLBACKS


#if defined(PCB_IMPLEMENTATION_LOG) || defined(PCB_IMPLEMENTATION_ERR)
#ifndef PCB_fprintf
#error "PCB Error: PCB requires PCB_fprintf defined, but none is available. Perhaps you can't use libc, in which case you need to #define it manually."
#define PCB_fprintf(stream, fmt, ...) //stub
#endif //PCB_fprintf
#ifndef PCB_vfprintf
#define PCB_vfprintf(stream, fmt, args) //stub
#error "PCB Error: PCB requires PCB_vfprintf defined, but none is available. Perhaps you can't use libc, in which case you need to #define it manually."
#endif //PCB_vfprintf

#ifndef PCB_stdout
#define PCB_stdout 0 //stub
#error "PCB Error: PCB requires PCB_stdout, but none is available. Perhaps you can't use libc, in which case you need to #define it manually."
#endif //PCB_stdout
#ifndef PCB_stderr
#define PCB_stderr 0 //stub
#error "PCB Error: PCB requires PCB_stderr, but none is available. Perhaps you can't use libc, in which case you need to #define it manually."
#endif //PCB_stderr
#endif //PCB_IMPLEMENTATION_LOG, PCB_IMPLEMENTATION_ERR

#if defined(PCB_IMPLEMENTATION_LOG)
#ifndef PCB_fflush
#define PCB_fflush(stream) //stub
#error "PCB Error: PCB requires PCB_fflush, but none is available. Perhaps you can't use libc, in which case you need to #define it manually."
#endif //PCB_fflush
#endif //PCB_IMPLEMENTATION_LOG

//Section 3.1: Logging, messages, error handling
#ifdef PCB_IMPLEMENTATION_LOG
void PCB_log(PCB_LogLevel level, const char* fmt, ...) {
#if PCB_PLATFORM_WINDOWS
    enum { ANSI_DUNNO, ANSI_ON, ANSI_OFF, BAD_HANDLE, OTHER_ERR };
    //"ANSI escape sequences available"
    static char Aeqa[2] = { ANSI_DUNNO, ANSI_DUNNO }; //stdout and stderr separately
    if(Aeqa[0] == ANSI_DUNNO) do {
        HMODULE ntdll = GetModuleHandleA("ntdll.dll");
        if (ntdll != NULL && GetProcAddress(ntdll, "wine_get_version") != NULL) {
            Aeqa[0] = Aeqa[1] = ANSI_OFF; break;
            //for some reason the code below doesn't work under Wine
        }
        HANDLE hStds[2] = { GetStdHandle(STD_OUTPUT_HANDLE), GetStdHandle(STD_ERROR_HANDLE) };
        for(int i = 0; i < 2; i++) {
            if(hStds[i] == NULL) { Aeqa[i] = BAD_HANDLE; continue; }
            DWORD dw = GetFileType(hStds[i]);
            if(dw != FILE_TYPE_CHAR) { Aeqa[i] = ANSI_OFF; continue; }
            char errmsg[32] = "Failed to GetConsoleMode of std";
            if(!GetConsoleMode(hStds[i], &dw))
                goto other_err;
            errmsg[10] = 'S';
            if(!SetConsoleMode(hStds[i], dw | ENABLE_VIRTUAL_TERMINAL_PROCESSING))
                goto other_err;
            Aeqa[i] = ANSI_ON; continue;
        other_err:
            WriteFile(hStds[i], errmsg, 31, NULL, NULL);
            WriteFile(hStds[i], i == 0 ? "out\n" : "err\n", 4, NULL, NULL);
            Aeqa[i] = OTHER_ERR; continue;
        }
    } while(0);
    //Implement those 2 ifs.
#endif //detect VT100 compatibility on Windows/Wine
    switch(level) {
      case PCB_LOGLEVEL_NONE:
      case PCB_LOGLEVEL_NONE_NL:
        break;
      case PCB_LOGLEVEL_TRACE:
      case PCB_LOGLEVEL_TRACE_NL:
#if PCB_PLATFORM_WINDOWS
        if(Aeqa[0] != ANSI_ON) PCB_fprintf(PCB_stdout, "[Trace] "); else
#endif //Windows scheiße
        PCB_fprintf(PCB_stdout, "[\033[38;5;238mTrace\033[0m] ");
        break;
      case PCB_LOGLEVEL_DEBUG:
      case PCB_LOGLEVEL_DEBUG_NL:
#if PCB_PLATFORM_WINDOWS
        if(Aeqa[0] != ANSI_ON) PCB_fprintf(PCB_stdout, "[Debug] "); else
#endif //Windows scheiße
        PCB_fprintf(PCB_stdout, "[\033[38;5;51mDebug\033[0m] ");
        break;
      case PCB_LOGLEVEL_INFO:
      case PCB_LOGLEVEL_INFO_NL:
        PCB_fprintf(PCB_stdout, "[Info]  ");
        break;
      case PCB_LOGLEVEL_WARN:
      case PCB_LOGLEVEL_WARN_NL:
#if PCB_PLATFORM_WINDOWS
        if(Aeqa[0] != ANSI_ON) PCB_fprintf(PCB_stdout, "[Warn]  "); else
#endif //Windows scheiße
        PCB_fprintf(PCB_stdout, "[\033[38;5;214mWarn\033[0m]  ");
        break;
      case PCB_LOGLEVEL_ERROR:
      case PCB_LOGLEVEL_ERROR_NL:
#if PCB_PLATFORM_WINDOWS
        if(Aeqa[1] != ANSI_ON) PCB_fprintf(PCB_stderr, "[Error] "); else
#endif //Windows scheiße
        PCB_fprintf(PCB_stderr, "[\033[38;5;9mError\033[0m] ");
        break;
      case PCB_LOGLEVEL_FATAL:
      case PCB_LOGLEVEL_FATAL_NL:
#if PCB_PLATFORM_WINDOWS
        if(Aeqa[1] != ANSI_ON) PCB_fprintf(PCB_stderr, "[Fatal] "); else
#endif //Windows scheiße
        PCB_fprintf(PCB_stderr, "[\033[1m\033[38;5;1mFatal\033[0m] ");
        break;
    }
    va_list args;
    va_start(args, fmt);
    //Yes, dear reader. This switch-case shouldn't be written like this.
    //However, I hate excessive whitespace.
    switch(level) {
      case PCB_LOGLEVEL_NONE:  case PCB_LOGLEVEL_NONE_NL:
      case PCB_LOGLEVEL_TRACE: case PCB_LOGLEVEL_TRACE_NL:
      case PCB_LOGLEVEL_DEBUG: case PCB_LOGLEVEL_DEBUG_NL:
      case PCB_LOGLEVEL_INFO:  case PCB_LOGLEVEL_INFO_NL:
      case PCB_LOGLEVEL_WARN:  case PCB_LOGLEVEL_WARN_NL:
        PCB_vfprintf(PCB_stdout, fmt, args); break;
      case PCB_LOGLEVEL_ERROR: case PCB_LOGLEVEL_ERROR_NL:
      case PCB_LOGLEVEL_FATAL: case PCB_LOGLEVEL_FATAL_NL:
        PCB_vfprintf(PCB_stderr, fmt, args); break;
    }
    va_end(args);
    switch(level) {
      case PCB_LOGLEVEL_NONE:  case PCB_LOGLEVEL_TRACE:
      case PCB_LOGLEVEL_DEBUG: case PCB_LOGLEVEL_INFO:
      case PCB_LOGLEVEL_WARN:
        PCB_fprintf(PCB_stdout, "\n"); break;
      case PCB_LOGLEVEL_ERROR: case PCB_LOGLEVEL_FATAL:
        PCB_fprintf(PCB_stderr, "\n"); break;
      case PCB_LOGLEVEL_NONE_NL:  case PCB_LOGLEVEL_TRACE_NL:
      case PCB_LOGLEVEL_DEBUG_NL: case PCB_LOGLEVEL_INFO_NL:
      case PCB_LOGLEVEL_WARN_NL:
#ifdef PCB_DEBUG
        PCB_fflush(PCB_stdout);
#endif  //explicitly flush if debugging is enabled
        //fallthrough since stderr isn't buffered
      case PCB_LOGLEVEL_ERROR_NL: case PCB_LOGLEVEL_FATAL_NL:
            break;
    }
}
#endif //PCB_IMPLEMENTATION_LOG


#ifdef PCB_IMPLEMENTATION_ERR
int PCB_GetError(void) {
#if PCB_PLATFORM_WINDOWS
    if(errno == 0) return (int)GetLastError();
    else return -errno;
#elif PCB_PLATFORM_POSIX
    return -errno;
#else
    return 0; //stub
#endif //platform
}

void PCB_ClearError(void) {
#if PCB_PLATFORM_WINDOWS
    SetLastError(0);
#endif //Windows scheiße
    errno = 0;
}

int PCB_GetErrorString(int errnum, char* buf, size_t bufSize) {
#if PCB_PLATFORM_WINDOWS
    if(errnum < 0) {
#if defined(__STDC_VERSION__) && __STDC_VERSION__+0 >= 201112L && defined(__STDC_LIB_EXT1__)
        return -strerror_s(buf, bufSize, -errnum);
#else
        PCB_snprintf(buf, bufSize, "%s", strerror(-errnum));
        return 0;
#endif //C11 shenanigans
    }
    DWORD err = GetLastError(); //preserve last error
    DWORD l = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errnum, 0, buf, (DWORD)bufSize, NULL
    );
    if(l == 0) {
        DWORD newErr = GetLastError();
        SetLastError(err);
        return (int)newErr;
    }
    return 0;
#elif PCB_PLATFORM_POSIX
    //PCB_GetError() maps errno values to their negative
    //counterparts for cross-platformness
    errnum = -errnum;
//this code right here is a very good example of xkcd 927
#ifdef _GNU_SOURCE
    char* errStr = strerror_r(errnum, buf, bufSize);
    if(buf != errStr) PCB_snprintf(buf, bufSize, "%s", errStr);
    return 0;
#elif defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE+0 >= 200112L
    int err = errno; //preserve last error
    int code = strerror_r(errnum, buf, bufSize);
    if(code >= 0) return -code; //glibc >= 2.13
    int newErr = errno; errno = err; return newErr; //glibc < 2.13
#elif defined(__STDC_VERSION__) && __STDC_VERSION__+0 >= 201112L && defined(__STDC_LIB_EXT1__)
    return -strerror_s(buf, bufSize, errnum);
#else
    PCB_snprintf(buf, bufSize, "%s", strerror(errnum));
    return 0;
#endif //this is really annoying...
#endif //platform
}

void PCB_logLatestError(const char* fmt, ...) {
    char buf[256] = PCB_ZEROED;
#if PCB_PLATFORM_WINDOWS
    bool addNl = PCB_GetError() <= 0;
#endif //error strings from WinAPI are ended with '\n'
    if(PCB_GetErrorString(
        PCB_GetError(), buf, sizeof(buf)
    ) != 0) {
        PCB_log(PCB_LOGLEVEL_ERROR, "Failed to get error string...");
        return; //wtf are we supposed to do if getting the error string fails...
    }
    PCB_log(PCB_LOGLEVEL_ERROR_NL, "%s", ""); //quick'n'dirty hack
    va_list args;
    va_start(args, fmt);
    PCB_vfprintf(PCB_stderr, fmt, args);
    va_end(args);
#if PCB_PLATFORM_WINDOWS
    if(addNl) PCB_fprintf(PCB_stderr, ": %s\n", buf);
    else PCB_fprintf(PCB_stderr, ": %s", buf);
#else
    PCB_fprintf(PCB_stderr, ": %s\n", buf);
#endif
}
#endif //PCB_IMPLEMENTATION_ERR


//Section 3.2: Platform-independent (sort of) filesystem functions
#ifdef PCB_IMPLEMENTATION_FS
bool PCB_mkdir(const char* path) {
#if PCB_PLATFORM_POSIX
    if(mkdir(path, 0775) == -1) {
        switch(errno) {
          case EEXIST: //not an error if already exists
            return true;
          default:
            PCB_logLatestError("Failed to create directory \"%s\"", path);
            return false;
        }
    }
    return true;
#elif PCB_PLATFORM_WINDOWS
    errno = 0;
    if(!CreateDirectory(path, NULL)) {
        DWORD err = GetLastError();
        if(err == ERROR_ALREADY_EXISTS) return true;
        PCB_logLatestError("Failed to create directory \"%s\"", path);
        return false;
    }
    return true;
#endif //platform
}

int PCB_FS_Exists(const char* path) {
    PCB_FileType type = PCB_FS_GetType(path);
    if(type == PCB_FILETYPE_ERROR) return -1;
    return type != PCB_FILETYPE_NONE;
}

int PCB_FS_ExistsPath(const char* target, PCB_StringView* PATH, PCB_Arena* arena) {
#if PCB_PLATFORM_WINDOWS
    PCB_CHECK_NULL(target, -ERROR_INVALID_ADDRESS);
    PCB_CHECK_NULL(PATH  , -ERROR_INVALID_ADDRESS);
#elif PCB_PLATFORM_POSIX
    PCB_CHECK_NULL(target, -EFAULT);
    PCB_CHECK_NULL(PATH  , -EFAULT);
#else
    (void)target; (void)PATH; return -1;
#endif //platforms (this is horrible)
#if PCB_PLATFORM_WINDOWS || PCB_PLATFORM_POSIX
    if(PCB_String_isEmpty(PATH)) {
        *PATH = PCB_StringView_from_cstr(getenv("PATH"));
        if(PCB_String_isEmpty(PATH)) {
#if PCB_PLATFORM_WINDOWS
            return -ERROR_BAD_ENVIRONMENT;
#elif PCB_PLATFORM_POSIX
            return -ESRCH; //POSIX doesn't have an appropiate errno for such a case
#endif //platforms
        }
    }

    int result = 1;
    PCB_ArenaMark* mark = NULL;
    char* path = NULL;
    if(arena != NULL) {
        mark = PCB_Arena_mark(arena);
        if(mark == NULL)
#if PCB_PLATFORM_WINDOWS
            return -ERROR_NOT_ENOUGH_MEMORY;
#elif PCB_PLATFORM_POSIX
            return -ENOMEM;
#endif //platforms
    }
    const size_t tL = PCB_strlen(target);
    while(!PCB_String_isEmpty(PATH)) {
        char sep[2] = { PCB_FS_PATH_DELIM, '\0' };
        //PATH component
        PCB_StringView comp = PCB_StringView_findCharFrom_cstr(*PATH, sep);
        bool last = false;
        if(PCB_String_isEmpty(&comp)) {
            comp = *PATH; last = true;
        } else {
            comp.length = (size_t)(comp.data - PATH->data);
            comp.data = PATH->data;
        }
        const size_t allocSize = sizeof(*path) * (comp.length + tL + 1 /*'\0'*/ + 1 /*'/'*/);
        if(comp.length == 0) goto next; //`comp` is malformed, skip
        //TODO: This should be optimized to only alloc if `path == NULL` or
        //`path`'s capacity is less than `allocSize` to avoid allocating
        //in a loop.
        if(arena != NULL) path = (char*)PCB_Arena_alloc(arena, allocSize);
        else              path = (char*)PCB_realloc(NULL, allocSize);
        if(path == NULL) {
#if PCB_PLATFORM_WINDOWS
            PCB__return_defer(-ERROR_OUTOFMEMORY);
#elif PCB_PLATFORM_POSIX
            PCB__return_defer(-ENOMEM);
#endif //platforms
        }
        PCB_memcpy(path, comp.data, sizeof(*comp.data) * comp.length);
        if(comp.data[comp.length - 1] == '/') {
            PCB_memcpy(path + comp.length, target, sizeof(*target) * (tL + 1) /*'\0'*/);
        } else {
            path[comp.length] = '/';
            PCB_memcpy(path + comp.length + 1, target, sizeof(*target) * (tL + 1));
        }
        result = PCB_FS_Exists(path);
        if(arena == NULL) PCB_free(path);
        if(result < 0) goto defer;

        if(arena != NULL) PCB_Arena_restore_to(arena, mark);
        path = NULL;

        if(result) return (int)((ssize_t)comp.length);
    next:;
        const size_t shiftAmount = comp.length + (last ? 0 : 1);
        PATH->data   += shiftAmount;
        PATH->length -= shiftAmount;
    }
defer:
    if(arena != NULL) PCB_Arena_restore(arena, mark);
    return result;
#endif //Windows || POSIX
}

PCB_FileType PCB_FS_GetType(const char* path) {
#if PCB_PLATFORM_WINDOWS
    PCB_CHECK_NULL(path, (SetLastError(0), errno = EFAULT, PCB_FILETYPE_ERROR));
#elif PCB_PLATFORM_POSIX
    PCB_CHECK_NULL(path, (errno = EFAULT, PCB_FILETYPE_ERROR));
#else
    (void)path; return PCB_FILETYPE_ERROR; //stub
#endif
#if PCB_PLATFORM_WINDOWS
    PCB_FileType type;
    HANDLE f = CreateFileA(
        path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL
    );
    if(f == INVALID_HANDLE_VALUE) {
        errno = 0;
        type = GetLastError() == ERROR_FILE_NOT_FOUND ? PCB_FILETYPE_NONE : PCB_FILETYPE_ERROR;
        goto end;
    }
    switch(GetFileType(f)) {
      case FILE_TYPE_CHAR: type = PCB_FILETYPE_CHAR; break;
      case FILE_TYPE_PIPE: type = PCB_FILETYPE_STREAM; break;
      case FILE_TYPE_UNKNOWN:
        type = GetLastError() == NO_ERROR ? PCB_FILETYPE_UNKNOWN : PCB_FILETYPE_ERROR;
        break;
      case FILE_TYPE_DISK: {
        FILE_ATTRIBUTE_TAG_INFO tags;
        if(!GetFileInformationByHandleEx(
            f, FileAttributeTagInfo, &tags, sizeof(tags)
        )) {
            errno = 0; type = PCB_FILETYPE_ERROR; break;
        }
        if(tags.FileAttributes == INVALID_FILE_ATTRIBUTES) {
            errno = 0; type = PCB_FILETYPE_ERROR; break;
        }
          if(tags.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            type = PCB_FILETYPE_DIR;
        else type = PCB_FILETYPE_REG;
      } break;
      default: type = PCB_FILETYPE_UNKNOWN; break;
    }
    end:
    CloseHandle(f);
    return type;
#elif PCB_PLATFORM_POSIX
    struct stat s;
    if(lstat(path, &s) == -1) {
        if(errno == ENOENT) { errno = 0; return PCB_FILETYPE_NONE; }
        else return PCB_FILETYPE_ERROR;
    }
    if(S_ISLNK(s.st_mode)) {
        if(stat(path, &s) == -1) {
            if(errno == ENOENT) { errno = 0; return PCB_FILETYPE_NONE_SYM; }
            else return PCB_FILETYPE_ERROR;
        }
        if(S_ISREG(s.st_mode)) return PCB_FILETYPE_REG_SYM;
        if(S_ISDIR(s.st_mode)) return PCB_FILETYPE_DIR_SYM;
        if(S_ISCHR(s.st_mode)) return PCB_FILETYPE_CHAR_SYM;
        if(S_ISBLK(s.st_mode)) return PCB_FILETYPE_BLK_SYM;
        if(S_ISFIFO(s.st_mode) || S_ISSOCK(s.st_mode)) return PCB_FILETYPE_STREAM_SYM;
        return PCB_FILETYPE_UNKNOWN_SYM;
    }
    if(S_ISREG(s.st_mode)) return PCB_FILETYPE_REG;
    if(S_ISDIR(s.st_mode)) return PCB_FILETYPE_DIR;
    if(S_ISCHR(s.st_mode)) return PCB_FILETYPE_CHAR;
    if(S_ISBLK(s.st_mode)) return PCB_FILETYPE_BLK;
    if(S_ISFIFO(s.st_mode) || S_ISSOCK(s.st_mode)) return PCB_FILETYPE_STREAM;
    return PCB_FILETYPE_UNKNOWN;
#endif //platform
}

int PCB_FS_IsDirectory(const char* path) {
    PCB_FileType type = PCB_FS_GetType(path);
    if(type == PCB_FILETYPE_NONE || type == PCB_FILETYPE_ERROR) return -1;
    return type == PCB_FILETYPE_DIR;
}

uint64_t PCB_FS_GetModificationTime(const char* path) {
    errno = 0;
#if PCB_PLATFORM_WINDOWS
    HANDLE hFile = CreateFileA(
        path, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL
    );
    if(hFile == INVALID_HANDLE_VALUE)
        return (uint64_t)(GetLastError() == ERROR_FILE_NOT_FOUND);

    BY_HANDLE_FILE_INFORMATION fileinfo = PCB_ZEROED;
    BOOL b = GetFileInformationByHandle(hFile, &fileinfo);
    CloseHandle(hFile);
    if(!b) return 0;

    SetLastError(0);
    uint64_t modTime = fileinfo.ftLastWriteTime.dwLowDateTime;
    modTime += (uint64_t)(fileinfo.ftLastWriteTime.dwHighDateTime) << 32;
    return modTime;
#elif PCB_PLATFORM_POSIX
    struct stat fileinfo = PCB_ZEROED;
    if(stat(path, &fileinfo) == -1) return (uint64_t)(errno == ENOENT);
    uint64_t modTime;
#if defined(__GLIBC__) && __GLIBC__+0 >= 2 && __GLIBC_MINOR__+0 >= 12 && ( \
    (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE+0 >= 200809L) || \
    (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE+0 >= 700) \
)
    modTime = (uint64_t)fileinfo.st_ctim.tv_sec * 1000000000 + (uint64_t)fileinfo.st_ctim.tv_nsec;
#else
    modTime = (uint64_t)fileinfo.st_ctime       * 1000000000;
#endif //pesky, but useful GNU extensions
    return modTime;
#endif //platform
}

bool PCB_FS_ReadEntireFile(const char* path, PCB_String* buf) {
    PCB_CHECK_NULL(path, false);
    PCB_CHECK_NULL(buf, false);
    bool success = false;
    FILE* f; int64_t s;
    f = fopen(path, "rb");
    if(f == NULL) return false;
    if(fseek(f, 0, SEEK_END) == -1) {
#if PCB_PLATFORM_WINDOWS
        SetLastError(0);
#endif
        goto end;
    }
#if PCB_PLATFORM_WINDOWS
    s = _ftelli64(f);
#else
    s = ftell(f);
#endif
    if(s == -1) goto end;
    if(fseek(f, 0, SEEK_SET) == -1) {
#if PCB_PLATFORM_WINDOWS
        SetLastError(0);
#endif
        goto end;
    }
    if(!PCB_String_reserve(buf, (size_t)s)) goto end;
    fread(buf->data + buf->length, 1, (size_t)s, f);
    if(ferror(f)) goto end;
    buf->data[buf->length += (size_t)s] = '\0';
    success = true;
    end:
    if(f != NULL) fclose(f);
    return success;
}

//skip until not separator
static PCB_StringView PCB__FS_SUNS(PCB_StringView path) {
    while(path.length > 0) { //skip duplicated separators
        if(path.data[path.length-1] == PCB_FS_DIR_DELIM) goto cont;
#if PCB_PLATFORM_WINDOWS
        if(path.data[path.length-1] == '/') goto cont;
#endif
        break;
    cont:
        --path.length;
    }
    return path;
}

PCB_StringView PCB_FS_Basename(PCB_StringView path) {
    if(PCB_String_isEmpty(&path)) return PCB_StringView_from_cstr(".");
    const char sep[2] = { PCB_FS_DIR_DELIM, '\0' };
#if PCB_PLATFORM_WINDOWS
    const char sep_alt[2] = "/";
#endif
    PCB_StringView dirsep = PCB_StringView_rsubcstr(path, sep);
#if PCB_PLATFORM_WINDOWS
    //CMD allows for paths with '/'. Dunno if it internally replaces them with '\'.
    //We'll assume Windows is chill with '/' as a directory separator.
    PCB_StringView dirsep_alt = PCB_StringView_rsubcstr(path, sep_alt);
    if(PCB_String_isEmpty(&dirsep)) dirsep = dirsep_alt;
    else if(!PCB_String_isEmpty(&dirsep_alt) && dirsep_alt.data > dirsep.data)
        dirsep = dirsep_alt; //use rightmost separator
#endif
    if(PCB_String_isEmpty(&dirsep)) return path;
    PCB_StringView base = {
        dirsep.data + 1,
        path.length - (size_t)(dirsep.data - path.data) - 1
    };
    if(base.length > 0) return base;
    path = PCB__FS_SUNS(path);
    //`path` was just separators
    //return the leftmost one, it shouldn't matter which one is actually returned
    if(path.length == 0) { path.length = 1; return path; }
    //Find *previous* separator, i.e. apply identical logic a 2nd time and not more.
    //A loop here would imply that we could potentially do so more than twice, but
    //we don't. DO NOT factor it out into a loop.
    dirsep = PCB_StringView_rsubcstr(path, sep);
#if PCB_PLATFORM_WINDOWS
    dirsep_alt = PCB_StringView_rsubcstr(path, sep_alt);
    if(PCB_String_isEmpty(&dirsep)) dirsep = dirsep_alt;
    else if(!PCB_String_isEmpty(&dirsep_alt) && dirsep_alt.data > dirsep.data)
        dirsep = dirsep_alt;
#endif
    if(PCB_String_isEmpty(&dirsep)) return path;
    return PCB_CLITERAL(PCB_StringView){
        dirsep.data + 1,
        path.length - (size_t)(dirsep.data - path.data) - 1
    };
}

PCB_StringView PCB_FS_Dirname(PCB_StringView path) {
    if(PCB_String_isEmpty(&path)) return PCB_StringView_from_cstr(".");
    char sep[2]=  { PCB_FS_DIR_DELIM, '\0' };
#if PCB_PLATFORM_WINDOWS
    char sep_alt[2] = "/";
#endif
    PCB_StringView dirsep = PCB_StringView_rsubcstr(path, sep);
#if PCB_PLATFORM_WINDOWS
    PCB_StringView dirsep_alt = PCB_StringView_rsubcstr(path, sep_alt);
    if(PCB_String_isEmpty(&dirsep)) dirsep = dirsep_alt;
    //use rightmost separator
    else if(!PCB_String_isEmpty(&dirsep_alt) && dirsep_alt.data > dirsep.data)
        dirsep = dirsep_alt;
#endif
    if(PCB_String_isEmpty(&dirsep)) return PCB_StringView_from_cstr(".");
    path.length = (size_t)(dirsep.data - path.data);
    path = PCB__FS_SUNS(path);
    if(path.length == 0) path.length = 1; //`path` was just separators
    return path;
}

PCB_StringView PCB_FS_Extension(PCB_StringView path) {
    path = PCB_FS_Basename(path);
    PCB_assert(!PCB_String_isEmpty(&path));
    return PCB_FS_Extension_base(path);
}

PCB_StringView PCB_FS_Extension_base(PCB_StringView path) {
    PCB_StringView last_dot = PCB_StringView_rsubcstr(path, ".");
    if(PCB_String_isEmpty(&last_dot)) return last_dot;
    PCB_StringView ext = {
        last_dot.data + 1,
        path.length - (size_t)(last_dot.data - path.data) - 1
    };
    //It's better to crash than have OOB reads, as seen in Mongobleed.
    if(ext.length == 0) ext.data = NULL;
    return ext;
}
#endif //PCB_IMPLEMENTATION_FS


//Section 3.3: Strings, string views, vectors of strings...
#ifdef PCB_IMPLEMENTATION_STRING
static void PCB__StringViews_append(PCB_StringViews* views, PCB_StringView sv) { PCB_Vec_append(views, sv); }
static void PCB__Strings_append(PCB_Strings* strs, PCB_String* str) { PCB_Vec_append(strs, *str); }

size_t PCB_strlen_char8 (const PCB_char8*  str) { const PCB_char8*  s = str; while(*s++) {} return (size_t)(s - str); }
size_t PCB_strlen_char16(const PCB_char16* str) { const PCB_char16* s = str; while(*s++) {} return (size_t)(s - str); }
size_t PCB_strlen_char32(const PCB_char32* str) { const PCB_char32* s = str; while(*s++) {} return (size_t)(s - str); }

#ifndef PCB_strlen_PCB_char8
#define PCB_strlen_PCB_char8 PCB_strlen_char8
#endif //PCB_strlen_PCB_char8
#ifndef PCB_strlen_PCB_char16
#define PCB_strlen_PCB_char16 PCB_strlen_char16
#endif //PCB_strlen_PCB_char16
#ifndef PCB_strlen_PCB_char32
#define PCB_strlen_PCB_char32 PCB_strlen_char32
#endif //PCB_strlen_PCB_char32

//this is gonna be ugly af, but I'm not copy-pasting the same code 5 times for 50 functions
//poor man's C++ templates
#define PCB__Str_destroy(Type) \
void Type##_destroy(Type* PCB_restrict str) { \
    PCB_CHECK_SELF(str,); \
    PCB_Vec_destroy(str); \
}
PCB__Str_destroy(PCB_String)    //PCB_String_destroy
PCB__Str_destroy(PCB_WString)   //PCB_WString_destroy
PCB__Str_destroy(PCB_U8String)  //PCB_U8String_destroy
PCB__Str_destroy(PCB_U16String) //PCB_U16String_destroy
PCB__Str_destroy(PCB_U32String) //PCB_U32String_destroy
#undef PCB__Str_destroy

#define PCB__Str_reserve(Type, charType) \
bool Type##_reserve(Type* PCB_restrict str, const size_t howMany) { \
    PCB_CHECK_SELF(str, false); \
    const size_t newSize = str->length + howMany + 1; /*'\0'*/ \
    if(newSize <= str->capacity) return true; \
    if(newSize > (SIZE_MAX)/2 / sizeof(*str->data)) return false; \
    size_t newCapacity = str->capacity == 0 ? PCB_VEC_INITIAL_CAPACITY : str->capacity; \
    while(newSize > newCapacity) newCapacity *= 2; \
    if(newCapacity > (SIZE_MAX/2) / sizeof(*str->data)) return false; \
    charType* newData = (charType*)PCB_realloc(str->data, newCapacity * sizeof(*str->data)); \
    if(newData == NULL) return false; \
    str->data = newData; str->capacity = newCapacity; \
    return true; \
}
PCB__Str_reserve(PCB_String,    char)       //PCB_String_reserve
PCB__Str_reserve(PCB_WString,   wchar_t)    //PCB_WString_reserve
PCB__Str_reserve(PCB_U8String,  PCB_char8)  //PCB_U8String_reserve
PCB__Str_reserve(PCB_U16String, PCB_char16) //PCB_U16String_reserve
PCB__Str_reserve(PCB_U32String, PCB_char32) //PCB_U32String_reserve
#undef PCB__Str_reserve

#define PCB__Str_reserve_to(Type, charType) \
bool Type##_reserve_to(Type* PCB_restrict str, const size_t cap) { \
    PCB_CHECK_SELF(str, false); \
    if(cap <= str->capacity) return true; \
    if(cap > (SIZE_MAX)/2 / sizeof(*str->data)) return false; \
    size_t newCapacity = str->capacity == 0 ? PCB_VEC_INITIAL_CAPACITY : str->capacity; \
    while(cap > newCapacity) newCapacity *= 2; \
    if(newCapacity > (SIZE_MAX/2) / sizeof(*str->data)) return false; \
    charType* newData = (charType*)PCB_realloc(str->data, newCapacity * sizeof(*str->data)); \
    if(newData == NULL) return false; \
    str->data = newData; str->capacity = newCapacity; \
    return true; \
}
PCB__Str_reserve_to(PCB_String,    char)        //PCB_String_reserve_to()
PCB__Str_reserve_to(PCB_WString,   wchar_t)     //PCB_WString_reserve_to()
PCB__Str_reserve_to(PCB_U8String,  PCB_char8)   //PCB_U8String_reserve_to()
PCB__Str_reserve_to(PCB_U16String, PCB_char16)  //PCB_U16String_reserve_to()
PCB__Str_reserve_to(PCB_U32String, PCB_char32)  //PCB_U32String_reserve_to()

#define PCB__Str_resize(Type) \
bool Type##_resize(Type* PCB_restrict str, const size_t targetLength) { \
    PCB_CHECK_SELF(str, false); \
    if(targetLength == str->length) return true; \
    else if(targetLength < str->length) { \
        str->data[str->length = targetLength] = '\0'; \
        return true; \
    } \
    return Type##_reserve(str, targetLength - str->length); \
}
PCB__Str_resize(PCB_String)    //PCB_String_resize()
PCB__Str_resize(PCB_WString)   //PCB_WString_resize()
PCB__Str_resize(PCB_U8String)  //PCB_U8String_resize()
PCB__Str_resize(PCB_U16String) //PCB_U16String_resize()
PCB__Str_resize(PCB_U32String) //PCB_U32String_resize()
#undef PCB__Str_resize

#define PCB__Str_append(Type) \
bool Type##_append(Type* str, const Type* other) { \
    PCB_CHECK_SELF(str, false); \
    PCB_CHECK_NULL(other, false); \
    if(PCB_String_isEmpty(other)) return true; \
    if(!Type##_reserve(str, other->length)) return false; /*with '\0'*/ \
    PCB_memcpy( \
        str->data + str->length, \
        other->data, \
        other->length * sizeof(*str->data) \
    ); \
    str->data[str->length += other->length] = '\0'; \
    return true; \
}
PCB__Str_append(PCB_String)    //PCB_String_append()
PCB__Str_append(PCB_WString)   //PCB_WString_append()
PCB__Str_append(PCB_U8String)  //PCB_U8String_append()
PCB__Str_append(PCB_U16String) //PCB_U16String_append()
PCB__Str_append(PCB_U32String) //PCB_U32String_append()
#undef PCB__Str_append

#define PCB__Str_append_sv(Type, svType, charType) \
bool Type##_append_sv(Type* PCB_restrict str, svType sv) { \
    PCB_CHECK_SELF(str, false); \
    if(PCB_String_isEmpty(&sv)) return true; \
    const charType* PCB_maybe_restrict data = sv.data; \
    PCB_CHECK(str->data <= data && data <= str->data + str->length, false); \
    if(!Type##_reserve(str, sv.length)) return false; \
    PCB_memcpy(str->data + str->length, data, sv.length * sizeof(*str->data)); \
    str->data[str->length += sv.length] = '\0'; \
    return true; \
}
PCB__Str_append_sv(PCB_String,    PCB_StringView,    char)       //PCB_String_append_sv()
PCB__Str_append_sv(PCB_WString,   PCB_WStringView,   wchar_t)    //PCB_WString_append_sv()
PCB__Str_append_sv(PCB_U8String,  PCB_U8StringView,  PCB_char8)  //PCB_U8String_append_sv()
PCB__Str_append_sv(PCB_U16String, PCB_U16StringView, PCB_char16) //PCB_U16String_append_sv()
PCB__Str_append_sv(PCB_U32String, PCB_U32StringView, PCB_char32) //PCB_U32String_append_sv()
#undef PCB__Str_append_sv

#define PCB__Str_append_cstr(Type, charType) \
bool Type##_append_cstr( \
    Type* PCB_restrict str, const charType* PCB_maybe_restrict cstr \
) { \
    PCB_CHECK_SELF(str, false); \
    PCB_CHECK_NULL(cstr, false); \
    PCB_CHECK(str->data <= cstr && cstr <= str->data + str->length, false); \
    size_t len = PCB_strlen_##charType(cstr); \
    if(len == 0) return true; \
    if(!Type##_reserve(str, len)) return false; \
    PCB_memcpy(str->data + str->length, cstr, (len + 1) * sizeof(*str->data)); \
    str->length += len; \
    return true; \
}
PCB__Str_append_cstr(PCB_String,    char)       //PCB_String_append_cstr()
PCB__Str_append_cstr(PCB_WString,   wchar_t)    //PCB_WString_append_cstr()
PCB__Str_append_cstr(PCB_U8String,  PCB_char8)  //PCB_U8String_append_cstr()
PCB__Str_append_cstr(PCB_U16String, PCB_char16) //PCB_U16String_append_cstr()
PCB__Str_append_cstr(PCB_U32String, PCB_char32) //PCB_U32String_append_cstr()
#undef PCB__Str_append_cstr

#define PCB__Str_append_cstrs(Type, svType, charType) \
ssize_t Type##_append_cstrs( \
    Type* PCB_restrict str, svType cstrs \
) { \
    PCB_CHECK_SELF(str, -1); \
    if(PCB_Vec_isEmpty(&cstrs)) return 0; \
    size_t cstrsLength = 0; \
    PCB_Vec_forEach_it(&cstrs, it, const charType* const) { \
        if(*it == NULL) continue; \
        if(str->data <= *it && *it <= str->data + str->length) continue; \
        cstrsLength += PCB_strlen_##charType(*it); \
        if((cstrsLength + str->length) * sizeof(*str->data) > SIZE_MAX/2) return -1; \
    } \
    if(cstrsLength == 0) return 0; \
    if(!Type##_reserve(str, cstrsLength)) return -1; \
    charType* cursor = str->data + str->length; \
    ssize_t appended = 0; \
    PCB_Vec_forEach_it(&cstrs, it, const charType* const) { \
        if(*it == NULL) continue; \
        if(str->data <= *it && *it <= str->data + str->length) continue; \
        size_t l = PCB_strlen_##charType(*it); \
        PCB_memcpy(cursor, *it, l * sizeof(*str->data)); \
        cursor += l; ++appended; \
    } \
    str->data[str->length += cstrsLength] = '\0'; \
    return appended; \
}
PCB__Str_append_cstrs(PCB_String,    PCB_CStringsView,    char)       //PCB_String_append_cstrs()
PCB__Str_append_cstrs(PCB_WString,   PCB_WCStringsView,   wchar_t)    //PCB_WString_append_cstrs()
PCB__Str_append_cstrs(PCB_U8String,  PCB_U8CStringsView,  PCB_char8)  //PCB_U8String_append_cstrs()
PCB__Str_append_cstrs(PCB_U16String, PCB_U16CStringsView, PCB_char16) //PCB_U1String_append_cstrs()
PCB__Str_append_cstrs(PCB_U32String, PCB_U32CStringsView, PCB_char32) //PCB_U32String_append_cstrs()
#undef PCB__Str_append_cstrs

ssize_t PCB_String_append_cstr_v(PCB_String* PCB_restrict str, ...) {
    PCB_CHECK_SELF(str, -1);
    va_list args;
    size_t argsLength = 0;
    va_start(args, str);
    PCB_VA_forEach_until(args, const char*, NULL, arg) {
        if(str->data <= arg && arg <= str->data + str->length) continue;
        argsLength += PCB_strlen(arg);
        if(argsLength + str->length > SIZE_MAX/2) return -1;
    }
    va_end(args);
    if(argsLength == 0) return 0;

    if(!PCB_String_reserve(str, argsLength)) return -1;

    ssize_t appended = 0;
    char* cursor = str->data + str->length;
    va_start(args, str);
    PCB_VA_forEach_until(args, const char*, NULL, arg) {
        if(str->data <= arg && arg <= str->data + str->length) continue;
        size_t l = PCB_strlen(arg);
        PCB_memcpy(cursor, arg, l);
        cursor += l; ++appended;
    }
    va_end(args);

    str->data[str->length += argsLength] = '\0';
    return appended;
}

bool PCB_String_append_chars(
    PCB_String* PCB_restrict str, const char c, const size_t howManyTimes
) {
    PCB_CHECK_SELF(str, false);
    if(c == '\0') return true;
    if(!PCB_String_reserve(str, howManyTimes)) return false;
    PCB_memset(str->data + str->length, c, howManyTimes);
    str->data[str->length += howManyTimes] = '\0';
    return true;
}

bool PCB_String_appendf(
    PCB_String* PCB_restrict str, const char* PCB_maybe_restrict fmt, ...
) {
#ifdef PCB_HAS_STDIO_H
    PCB_CHECK_SELF(str, false);
    PCB_CHECK_NULL(fmt, false);
    PCB_CHECK(str->data <= fmt && fmt <= str->data + str->length, false);

    va_list args;
    va_start(args, fmt);
    //'\0' is implicitly stored at the end
    const size_t lengthRequired = (size_t)PCB_vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    if(lengthRequired == 0) return true;

    if(!PCB_String_reserve(str, lengthRequired)) return false;

    va_start(args, fmt); //                               '\0'
    PCB_vsnprintf(str->data + str->length, lengthRequired + 1, fmt, args);
    va_end(args);
    str->length += lengthRequired;
    return true;
#else
    (void)str; (void)fmt;
    return false;
#endif //PCB_HAS_STDIO_H
}

bool PCB_String_append_codepoint(
    PCB_String* PCB_restrict str, int32_t codepoint
) {
    PCB_CHECK_SELF(str, false);
    PCB_CHECK(!PCB_IsValidUnicode(codepoint), false);
    uint8_t l = PCB_GetUTF8Length_unchecked(codepoint);
    if(!PCB_String_reserve(str, l)) return false;
    PCB_StoreUTF8Codepoint(str->data + str->length, codepoint);
    str->data[str->length += l] = '\0';
    return true;
}

ssize_t PCB_String_append_codepoints(
    PCB_String* PCB_restrict str, PCB_U32StringView codepoints
) {
    PCB_CHECK_SELF(str, -1);
    if(PCB_String_isEmpty(&codepoints)) return 0;
    size_t neededLen = 0;
    for(size_t i = 0; i < codepoints.length; i++) {
        neededLen += (size_t)PCB_GetUTF8Length((int32_t)codepoints.data[i]);
        if(neededLen + str->length > SIZE_MAX/2) return -1;
    }
    if(neededLen == 0) return 0;
    if(!PCB_String_reserve(str, neededLen)) return -1;

    char* cursor = str->data + str->length;
    ssize_t appended = 0;
    PCB_Vec_forEach_it(&codepoints, c, const PCB_char32) {
        if(!PCB_IsValidUnicode((int32_t)*c)) continue;
        cursor = PCB_StoreUTF8Codepoint(cursor, (int32_t)*c);
        ++appended;
    }
    str->data[str->length += neededLen] = '\0';
    return appended;
}

bool PCB_String_insert(
    PCB_String* PCB_maybe_restrict str, const PCB_String* PCB_maybe_restrict other,
    size_t position
) {
    PCB_CHECK_SELF(str, false);
    PCB_CHECK(position > str->length, false);
    if(PCB_String_isEmpty(other)) return true; //the latter should be impossible
    PCB_CHECK(str == other || str->data == other->data, false); // <---

    if(!PCB_String_reserve(str, other->length)) return false;
    PCB_memmove(str->data + position + other->length, str->data + position, str->length - position);
    PCB_memcpy(str->data + position, other->data, other->length);
    str->data[str->length += other->length] = '\0';
    return true;
}

bool PCB_String_insert_sv(
    PCB_String* PCB_restrict str,
    PCB_StringView sv,
    size_t position
) {
    PCB_CHECK_SELF(str, false);
    PCB_CHECK(position > str->length, false);
    if(PCB_String_isEmpty(&sv)) return true;
    const char* PCB_maybe_restrict data = sv.data;
    PCB_CHECK(str->data <= data && data <= str->data + str->length, false);

    if(!PCB_String_reserve(str, sv.length)) return false;
    PCB_memmove(str->data + position + sv.length, str->data + position, str->length - position);
    PCB_memcpy(str->data + position, data, sv.length);
    str->data[str->length += sv.length] = '\0';
    return true;
}

bool PCB_String_insert_cstr(
    PCB_String* PCB_restrict str, const char* PCB_maybe_restrict cstr,
    size_t position
) {
    PCB_CHECK_SELF(str, false);
    PCB_CHECK_NULL(cstr, false);
    PCB_CHECK(position > str->length, false);
    PCB_CHECK(str->data <= cstr && cstr <= str->data + str->length, false);

    size_t len = PCB_strlen(cstr);
    if(len == 0) return true;
    if(!PCB_String_reserve(str, len)) return false;
    PCB_memmove(str->data + position + len, str->data + position, str->length - position);
    PCB_memcpy(str->data + position, cstr, len);
    str->data[str->length += len] = '\0';
    return true;
}

ssize_t PCB_String_insert_cstrs(
    PCB_String* PCB_restrict str, PCB_CStringsView cstrs, size_t position
) {
    PCB_CHECK_SELF(str, -1);
    PCB_CHECK(position > str->length, -1);
    if(PCB_Vec_isEmpty(&cstrs)) return 0;

    size_t cstrsLength = 0;
    PCB_Vec_forEach_it(&cstrs, it, const char* const) {
        if(*it == NULL) continue;
        if(str->data <= *it && *it <= str->data + str->length) continue;
        cstrsLength += PCB_strlen(*it);
        if(cstrsLength + str->length > SIZE_MAX/2) return -1;
    }
    if(cstrsLength == 0) return 0;

    if(!PCB_String_reserve(str, cstrsLength)) return -1;
    PCB_memmove(
        str->data + position + cstrsLength,
        str->data + position,
        str->length - position
    );

    char* cursor = str->data + position;
    ssize_t appended = 0;
    PCB_Vec_forEach_it(&cstrs, it, const char* const) {
        if(*it == NULL) continue;
        if(str->data <= *it && *it <= str->data + str->length) continue;
        size_t l = PCB_strlen(*it);
        PCB_memcpy(cursor, *it, l);
        cursor += l; ++appended;
    }
    str->data[str->length += cstrsLength] = '\0';
    return appended;
}

ssize_t PCB_String_insert_cstr_v(
    PCB_String* PCB_restrict str, size_t position, ...
) {
    PCB_CHECK_SELF(str, -1);
    va_list args;
    size_t argsLength = 0;
    va_start(args, position);
    PCB_VA_forEach_until(args, const char*, NULL, arg) {
        if(str->data <= arg && arg <= str->data + str->length) continue;
        argsLength += PCB_strlen(arg);
        if(argsLength + str->length > SIZE_MAX/2) return -1;
    }
    va_end(args);
    if(argsLength == 0) return 0;

    if(!PCB_String_reserve(str, argsLength)) return -1;
    PCB_memmove(
        str->data + position + argsLength,
        str->data + position,
        str->length - position
    );

    ssize_t inserted = 0;
    char* cursor = str->data + position;
    va_start(args, position);
    PCB_VA_forEach_until(args, const char*, NULL, arg) {
        if(str->data <= arg && arg <= str->data + str->length) continue;
        size_t l = PCB_strlen(arg);
        PCB_memcpy(cursor, arg, l);
        cursor += l; ++inserted;
    }
    va_end(args);
    str->data[str->length += argsLength] = '\0';
    return inserted;
}

bool PCB_String_insert_chars(
    PCB_String* PCB_restrict str, const char c,
    size_t howManyTimes, size_t position
) {
    PCB_CHECK_SELF(str, false);
    PCB_CHECK(position > str->length, false); PCB_CHECK(c == '\0', false);
    if(howManyTimes == 0) return true;
    if(!PCB_String_reserve(str, howManyTimes)) return false;
    PCB_memmove(str->data + position + howManyTimes, str->data + position, str->length - position);
    PCB_memset(str->data + position, c, howManyTimes);
    str->data[str->length += howManyTimes] = '\0';
    return true;
}

bool PCB_String_insertf(
    PCB_String* PCB_restrict str, const char* PCB_maybe_restrict fmt,
    size_t position, ...
) {
#ifdef PCB_HAS_STDIO_H
    PCB_CHECK_SELF(str, false);
    PCB_CHECK(position > str->length, false);
    PCB_CHECK(str->data <= fmt && fmt <= str->data + str->length, false);

    va_list args;
    va_start(args, position);
    const size_t lengthRequired = (size_t)PCB_vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    if(lengthRequired == 0) return true;
    if(!PCB_String_reserve(str, lengthRequired)) return false;

    //PCB_vsnprintf will override the last character with '\0', so we need to save it
    char overridden = str->data[position];
    PCB_memmove(str->data + position + lengthRequired, str->data + position, str->length - position);
    va_start(args, position);
    PCB_vsnprintf(str->data + position, lengthRequired + 1, fmt, args);
    va_end(args);
    str->data[position + lengthRequired] = overridden;
    str->data[str->length += lengthRequired] = '\0';
    return true;
#else
    (void)str; (void)fmt; (void)position;
    return false;
#endif //PCB_HAS_STDIO_H
}

bool PCB_String_insert_codepoint(
    PCB_String* PCB_restrict str, int32_t codepoint, size_t position
) {
    PCB_CHECK_SELF(str, false);
    PCB_CHECK(position > str->length, false);
    PCB_CHECK(!PCB_IsValidUnicode(codepoint), false);
    uint8_t l = PCB_GetUTF8Length_unchecked(codepoint);

    if(!PCB_String_reserve(str, l)) return false;
    PCB_memmove(
        str->data + position + l,
        str->data + position,
        str->length - position
    );

    PCB_StoreUTF8Codepoint(str->data + position, codepoint);
    str->data[str->length += l] = '\0';
    return true;
}

ssize_t PCB_String_insert_codepoints(
    PCB_String* PCB_restrict str, PCB_U32StringView codepoints, size_t position
) {
    PCB_CHECK_SELF(str, -1);
    PCB_CHECK(position > str->length, -1);
    if(PCB_Vec_isEmpty(&codepoints)) return 0;
    size_t neededLen = 0;
    PCB_Vec_forEach_it(&codepoints, c, const PCB_char32) {
        neededLen += (size_t)PCB_GetUTF8Length((int32_t)*c);
        if(neededLen + str->length > SIZE_MAX/2) return -1;
    }
    if(neededLen == 0) return 0;
    if(!PCB_String_reserve(str, neededLen)) return 0;
    PCB_memmove(str->data + position + neededLen, str->data + position, str->length - position);

    char* cursor = str->data + str->length;
    ssize_t inserted = 0;
    PCB_Vec_forEach_it(&codepoints, c, const PCB_char32) {
        if(!PCB_IsValidUnicode((int32_t)*c)) continue;
        cursor = PCB_StoreUTF8Codepoint(cursor, (int32_t)*c);
        ++inserted;
    }
    str->data[str->length += neededLen] = '\0';
    return inserted;
}

bool PCB_String_replace_range(
    PCB_String* PCB_restrict str,
    size_t start,
    size_t length,
    PCB_StringView other
) {
    PCB_CHECK_SELF(str, false);
    PCB_CHECK(start >= str->length, false);
    PCB_CHECK(start + length > str->length, false);
    if(PCB_String_isEmpty(&other)) return PCB_String_remove_range(str, start, length);
    PCB_CHECK(str->data <= other.data && other.data <= str->data + str->length, false);
    char* const after = str->data + start + length;
    if(other.length > length) {
        const size_t diff = other.length - length;
        if(!PCB_String_reserve(str, diff)) return false;
        PCB_memmove(after + diff, after, str->length - (start + length));
        PCB_memcpy(str->data + start, other.data, other.length);
        str->data[str->length += diff] = '\0';
    }
    else if(other.length < length) {
        const size_t diff = length - other.length;
        PCB_memcpy(str->data + start, other.data, other.length);
        PCB_memmove(after - diff, after, str->length - (start + length));
        str->data[str->length -= diff] = '\0';
    } else {
        PCB_memcpy(str->data + start, other.data, length);
    }
    return true;
}

bool PCB_String_replace_range_chars(
    PCB_String* str,
    size_t start,
    size_t length,
    char c
) {
    PCB_CHECK_SELF(str, false);
    PCB_CHECK(start >= str->length, false);
    PCB_CHECK(start + length > str->length, false);
    PCB_CHECK(c == '\0', false);
    PCB_memset(str->data + start, c, length);
    return true;
}

bool PCB_String_remove_range(
    PCB_String* PCB_restrict str, size_t start, size_t length
) {
    PCB_CHECK_SELF(str, false);
    PCB_CHECK(start >= str->length, false);
    PCB_CHECK(start + length > str->length, false);
    if(length == 0) return true; //would cause a redundant memmove
    PCB_memmove(
        str->data + start,
        str->data + start + length,
        str->length - (start + length) + 1 //'\0'
    );
    str->length -= length; return true;
}

bool PCB_String_setSuffix_char(
    PCB_String* PCB_restrict str, const char c
) {
    PCB_CHECK_SELF(str, false);
    if(str->capacity == 0 && !PCB_String_reserve(str, 1)) return false;
    if(str->length == 0) {
        str->data[0] = c; str->data[++str->length] = '\0';
        return true;
    }
    if(str->data[str->length - 1] != c) {
        if(!PCB_String_reserve(str, 1)) return false;
        str->data[str->length] = c;
        str->data[++str->length] = '\0';
    }
    return true;
}

bool PCB_String_truncate_until_char(
    PCB_String* PCB_restrict str, const char c
) {
    PCB_CHECK_SELF(str, false);
    if(PCB_String_isEmpty(str)) return true;
    if(str->data[str->length - 1] == c) return true;
    const char* cursor = str->data + str->length - 1;
    while(cursor != str->data && *cursor != c) { --cursor; }
    if(cursor == str->data) {
        if(*cursor != c) return false;
        str->data[str->length = 1] = '\0';
        return true;
    }
    const size_t newLength = (size_t)(cursor + 1 - str->data);
    str->data[str->length = newLength] = '\0';
    return true;
}

PCB_String PCB_String_clone(const PCB_String* PCB_restrict str) {
    PCB_CHECK_SELF(str, PCB_ZEROED_T(PCB_String));
    PCB_CHECK(PCB_String_isEmpty(str), PCB_ZEROED_T(PCB_String));
    PCB_String s = PCB_ZEROED;
    s.data = (char*)PCB_realloc(NULL, str->length + 1);
    if(s.data == NULL) return s;
    s.length = str->length;
    s.capacity = str->capacity;
    PCB_memcpy(s.data, str->data, s.length + 1);
    return s;
}

int PCB_String_compare(const PCB_String* a, const PCB_String* b) {
    PCB_CHECK_SELF(a, 0); PCB_CHECK_SELF(b, 0);
    // if(a->data == NULL && b->data == NULL) return 0;
    // else if(a->data == NULL) return 1;
    // else if(b->data == NULL) return -1;
    if(a->data == NULL) return b->data != NULL;
    else if(b->data == NULL) return -1;
    return a->length == b->length
        ? PCB_memcmp(a->data, b->data, a->length)
        : PCB_strcmp(a->data, b->data);
}

int PCB_String_compare_ci(const PCB_String* a, const PCB_String* b) {
    PCB_CHECK_SELF(a, 0); PCB_CHECK_SELF(b, 0);
    if(a->data == NULL) return b->data != NULL;
    else if(b->data == NULL) return -1;
    return PCB_strncasecmp(a->data, b->data, a->length);
}

int PCB_String_compare_cstr(
    const PCB_String* PCB_restrict a, const char* PCB_restrict b
) {
    PCB_CHECK_SELF(a, 0); PCB_CHECK_SELF(b, 0);
    if(a->data == NULL) return 1;
    // if(a->data == NULL) return b != NULL;
    // else if(b == NULL) return -1;
    return PCB_strcmp(a->data, b);
}

int PCB_String_compare_cstr_ci(
    const PCB_String* PCB_restrict a, const char* PCB_restrict b
) {
    PCB_CHECK_SELF(a, 0); PCB_CHECK_SELF(b, 0);
    if(a->data == NULL) return 1;
    // if(a->data == NULL) return b != NULL;
    // else if(b == NULL) return -1;
    return PCB_strncasecmp(a->data, b, a->length);
}

bool PCB_String_eq(const PCB_String* a, const PCB_String* b) {
    PCB_CHECK_SELF(a, false); PCB_CHECK_SELF(b, false);
    if(a->data == NULL || b->data == NULL) return false;
    if(a->length != b->length) return false;
    return PCB_memcmp(a->data, b->data, a->length);
}

bool PCB_String_startsWith(const PCB_String* str, const PCB_String* other) {
    PCB_CHECK_SELF(str, false);
    PCB_CHECK_NULL(other, false);

    if(str == other) return true;
    if(PCB_String_isEmpty(str) || PCB_String_isEmpty(other)) return false;
    if(other->length > str->length) return false;
    return !PCB_memcmp(str->data, other->data, other->length);
}

bool PCB_String_startsWith_cstr(
    const PCB_String* PCB_restrict str, const char* PCB_restrict other
) {
    PCB_CHECK_SELF(str, false);
    PCB_CHECK_NULL(other, false);

    if(PCB_String_isEmpty(str)) return false;
    const size_t len = PCB_strlen(other);
    if(len > str->length) return false;
    return !PCB_memcmp(str->data, other, len);
}

bool PCB_String_endsWith(const PCB_String* str, const PCB_String* other) {
    PCB_CHECK_SELF(str, false);
    PCB_CHECK_NULL(other, false);

    if(str == other) return true;
    if(PCB_String_isEmpty(str) || PCB_String_isEmpty(other)) return false;
    if(other->length > str->length) return false;
    return !PCB_memcmp(
        str->data + str->length - other->length,
        other->data, other->length
    );
}

bool PCB_String_endsWith_cstr(
    const PCB_String* PCB_restrict str, const char* PCB_restrict other
) {
    PCB_CHECK_SELF(str, false);
    PCB_CHECK_NULL(other, false);

    if(PCB_String_isEmpty(str)) return false;
    const size_t len = PCB_strlen(other);
    if(len > str->length) return false;
    return !PCB_memcmp(str->data + str->length - len, other, len);
}

void PCB_String_toUpperCase(PCB_String* PCB_restrict str) {
    PCB_CHECK_SELF(str,);
    for(size_t i = 0; i < str->length; i++) {
        if(str->data[i] >= 'a' && str->data[i] <= 'z') {
            str->data[i] -= 'a' - 'A';
        }
    }
}

void PCB_String_toLowerCase(PCB_String* PCB_restrict str) {
    PCB_CHECK_SELF(str,);
    for(size_t i = 0; i < str->length; i++) {
        if(str->data[i] >= 'A' && str->data[i] <= 'Z') {
            str->data[i] += 'a' - 'A';
        }
    }
}

PCB_String PCB_String_toUpperCase_copy(const PCB_String* PCB_restrict str) {
    PCB_CHECK_SELF(str, PCB_ZEROED_T(PCB_String));
    PCB_String copy = PCB_String_clone(str);
    PCB_String_toUpperCase(&copy);
    return copy;
}

PCB_String PCB_String_toLowerCase_copy(const PCB_String* PCB_restrict str) {
    PCB_CHECK_SELF(str, PCB_ZEROED_T(PCB_String));
    PCB_String copy = PCB_String_clone(str);
    PCB_String_toLowerCase(&copy);
    return copy;
}

char PCB_String_pop(PCB_String* PCB_restrict str) {
    PCB_CHECK_SELF(str, 0x15); //NAK, as if "Why are you passing NULL here? I'm disappointed."
    if(str->length == 0) return '\0';
    char c = str->data[str->length - 1];
    str->data[--str->length] = '\0';
    return c;
}

size_t PCB_String_pop_many(
    PCB_String* PCB_restrict str, size_t howMany, char* PCB_restrict out
) {
    PCB_CHECK_SELF(str, 0);
    if(howMany == 0) return 0;
    if(PCB_String_isEmpty(str)) return 0;
    if(howMany > str->length) howMany = str->length;
    if(out != NULL) {
        PCB_CHECK(str->data <= out && out <= str->data + str->length, 0);
        PCB_memcpy(out, str->data + str->length - howMany, howMany + 1);
    }
    str->data[str->length -= howMany] = '\0';
    return howMany;
}

size_t PCB_String_removeSuffix(PCB_String* str, const PCB_String* other) {
    PCB_CHECK_SELF(str, 0);
    PCB_CHECK_NULL(other, 0);
    if(PCB_String_endsWith(str, other))
        str->data[str->length -= other->length] = '\0';
    return str->length;
}

void PCB_String_trim_left(PCB_String* PCB_restrict str) {
    PCB_CHECK_SELF(str,);
    if(PCB_String_isEmpty(str)) return;
    const char* cursor = str->data;
    while(PCB_isspace(*cursor)) ++cursor;
    size_t firstNonWsp = (size_t)(cursor - str->data);
    if(firstNonWsp == 0) return; //redundant memmove
    PCB_memmove(
        str->data,
        str->data + firstNonWsp,
        str->length - firstNonWsp + 1 //'\0'
    );
    str->length -= firstNonWsp;
}

void PCB_String_trim_right(PCB_String* PCB_restrict str) {
    PCB_CHECK_SELF(str,);
    if(PCB_String_isEmpty(str)) return;
    const char* cursor = str->data + str->length - 1;
    while((cursor - str->data) >= 0 && PCB_isspace(*cursor)) --cursor;
    size_t newLen = (size_t)(cursor + 1 - str->data);
    str->data[str->length = newLen] = '\0';
}

bool PCB_String_repeat(PCB_String* PCB_restrict str, size_t n) {
    PCB_CHECK_SELF(str, false);
    PCB_CHECK(n == 0, false);
    if(n == 1 || PCB_String_isEmpty(str)) return true;
    const size_t startlen = str->length;
    if(!PCB_String_reserve(str, startlen * (n - 1))) return false;
    char* end = str->data + str->length;
    //by copying progressively more each iteration we can leverage memcpy's
    //vectorization for an easy speedup
    size_t x = n; //first copy x times for 2^x total repetitions
    x |= x >> 1;  x |= x >> 2;  x |= x >> 4;  //round x down to the nearest
    x |= x >> 8;  x |= x >> 16; x |= x >> 32; //power of 2
    x -= x >> 1;
    n -= x; x >>= 1;
    while(x) {
        PCB_memcpy(end, str->data, str->length);
        str->length *= 2;
        end = str->data + str->length;
        x >>= 1;
    }
    x = 1; //repurpose x to find the MSB set in `n`
    while(n >= x) x <<= 1;
    x >>= 1;
    while(x) {
        size_t additional = startlen * (n & x);
        if(additional != 0) {
            PCB_memcpy(end, str->data, additional);
            str->length += additional;
            end += additional;
        }
        x >>= 1;
    }
    str->data[str->length] = '\0';
    return true;
}

PCB_String PCB_String_from_StringView(PCB_StringView sv) {
    if(sv.data == NULL) return PCB_ZEROED_T(PCB_String);
    size_t capacity = PCB_VEC_INITIAL_CAPACITY;
    while(capacity < (sv.length + 1)) capacity *= 2; //+1 for '\0'
    PCB_String s = PCB_ZEROED;
    s.data = (char*)PCB_realloc(NULL, capacity);
    if(s.data == NULL) return s;
    s.length = sv.length;
    s.capacity = capacity;
    PCB_memcpy(s.data, sv.data, s.length);
    s.data[s.length] = '\0';
    return s;
}

PCB_String PCB_String_from_CStrings(
    const PCB_CStrings* PCB_restrict cstrs,
    const char* PCB_restrict delimiter
) {
    PCB_String str = PCB_ZEROED;
    PCB_CHECK_NULL(cstrs, str);
    PCB_CHECK_NULL(delimiter, str);
    if(PCB_Vec_isEmpty(cstrs)) return str;

    size_t totalLength = 0;
    for(size_t i = 0; i < cstrs->length; totalLength += PCB_strlen(cstrs->data[i++]));
    //delimiter isn't placed at the end                   v    '\0'
    totalLength += PCB_strlen(delimiter) * (cstrs->length - 1) + 1;
    size_t capacity = PCB_VEC_INITIAL_CAPACITY;
    while(capacity <= totalLength) capacity *= 2;

    str.data = (char*)PCB_realloc(NULL, capacity);
    if(str.data == NULL) return str;
    str.length = totalLength - 1; //we don't count the '\0'
    str.capacity = capacity;

    char* cursor = str.data;
    for(size_t i = 0; i < cstrs->length - 1; i++) {
        const char* current = cstrs->data[i];
        for(; *current; *cursor++ = *current++);  //this is cursed...
        for(current = delimiter; *current; *cursor++ = *current++);
    }

    for(const char* current = cstrs->data[cstrs->length - 1]; *current; *cursor++ = *current++);
    *cursor = '\0';
    return str;
}

PCB_StringView PCB_StringView_substr_n(
    PCB_StringView sv, const PCB_StringView sub, size_t n
) {
    PCB_CHECK(n == 0, PCB_ZEROED_T(PCB_StringView));
    if(PCB_String_isEmpty(&sv))  return PCB_ZEROED_T(PCB_StringView);
    if(PCB_String_isEmpty(&sub)) return PCB_ZEROED_T(PCB_StringView);
    PCB_StringView s = sub;
    while(n > 0) {
        while(sv.length > 0 && sv.data[0] != s.data[0]) {
            sv.data++; sv.length--;
        }
        if(sv.length == 0) return PCB_ZEROED_T(PCB_StringView);
        while(s.length > 0 && sv.length > 0 && sv.data[0] == s.data[0]) {
            sv.data++; sv.length--;
            s.data++;  s.length--;
        }
        if(s.length == 0) n -= 1;
        else if(sv.length == 0) return PCB_ZEROED_T(PCB_StringView);
        s = sub; //search again
    }
    sv.data -= sub.length;
    sv.length = sub.length;
    return sv;
}

PCB_StringView PCB_StringView_rsubstr_n(
    PCB_StringView sv, const PCB_StringView sub, size_t n
) {
    PCB_CHECK(n == 0, PCB_ZEROED_T(PCB_StringView));
    if(PCB_String_isEmpty(&sv))  return PCB_ZEROED_T(PCB_StringView);
    if(PCB_String_isEmpty(&sub)) return PCB_ZEROED_T(PCB_StringView);
    PCB_StringView s = sub;
    while(n > 0) {
        while(sv.length > 0 && sv.data[sv.length-1] != s.data[s.length-1]) {
            --sv.length;
        }
        if(sv.length == 0) return PCB_ZEROED_T(PCB_StringView);
        while(s.length > 0 && sv.length > 0 && sv.data[sv.length-1] == s.data[s.length-1]) {
            --sv.length; --s.length;
        }
        if(s.length == 0) n -= 1;
        else if(sv.length == 0) return PCB_ZEROED_T(PCB_StringView);
        s = sub;
    }
    sv.data += sv.length;
    sv.length = sub.length;
    return sv;
}

PCB_StringViews PCB_StringView_split(
    PCB_StringView sv,
    PCB_StringView delim
) {
    PCB_StringViews views = PCB_ZEROED;
    PCB_CHECK(PCB_String_isEmpty(&delim), views);

    PCB_StringView cur = PCB_StringView_substr(sv, delim);
    while(cur.data != NULL && cur.length > 0) {
        size_t slice_len = (size_t)(&cur.data[0] - &sv.data[0]);
        PCB__StringViews_append(
            &views, PCB_CLITERAL(PCB_StringView){ sv.data, slice_len }
        );
        sv.length -= slice_len + cur.length;
        sv.data   += slice_len + cur.length;
        cur = PCB_StringView_substr(sv, delim);
    }
    if(sv.length > 0) PCB_Vec_append(&views, sv);
    return views;
}

PCB_Strings PCB_StringView_split_copy(
    PCB_StringView sv, PCB_StringView delim
) {
    PCB_Strings strs = PCB_ZEROED;
    PCB_CHECK(PCB_String_isEmpty(&delim), strs);

    PCB_StringView cur = PCB_StringView_substr(sv, delim);
    PCB_String str;
    while(cur.data != NULL && cur.length > 0) {
        str = PCB_ZEROED_T(PCB_String);
        size_t slice_len = (size_t)(&cur.data[0] - &sv.data[0]);
        PCB_String_append_sv(&str, PCB_CLITERAL(PCB_StringView){sv.data, slice_len});
        PCB__Strings_append(&strs, &str);
        sv.length -= slice_len + cur.length;
        sv.data   += slice_len + cur.length;
        cur = PCB_StringView_substr(sv, delim);
    }
    if(sv.length > 0) {
        str = PCB_ZEROED_T(PCB_String);
        PCB_String_append_sv(&str, sv);
        PCB__Strings_append(&strs, &str);
    }
    return strs;
}

PCB_StringViews PCB_StringView_split_whitespace(PCB_StringView sv) {
    PCB_CHECK(PCB_String_isEmpty(&sv), PCB_ZEROED_T(PCB_StringViews));
    PCB_StringViews views = PCB_ZEROED;
    const char*     cur = NULL;
    while(true) {
        while(sv.length > 0 &&  PCB_isspace(sv.data[0])) { sv.data++; sv.length--; }
        if(sv.length == 0) break;
        cur = sv.data;
        while(sv.length > 0 && !PCB_isspace(sv.data[0])) { sv.data++; sv.length--; }
        PCB__StringViews_append(
            &views,
            PCB_CLITERAL(PCB_StringView){cur, (size_t)(sv.data - cur)}
        );
    }
    return views;
}

PCB_Strings PCB_StringView_split_whitespace_copy(PCB_StringView sv) {
    PCB_CHECK(PCB_String_isEmpty(&sv), PCB_ZEROED_T(PCB_Strings));
    PCB_Strings strs = PCB_ZEROED;
    const char* cur  = NULL;
    PCB_String str;
    while(true) {
        str = PCB_ZEROED_T(PCB_String);
        while(sv.length > 0 &&  PCB_isspace(sv.data[0])) { sv.data++; sv.length--; }
        if(sv.length == 0) break;
        cur = sv.data;
        while(sv.length > 0 && !PCB_isspace(sv.data[0])) { sv.data++; sv.length--; }
        PCB_String_append_sv(&str, PCB_CLITERAL(PCB_StringView){ cur, (size_t)(sv.data - cur) });
        PCB__Strings_append(&strs, &str);
    }
    return strs;
}

PCB_StringView PCB_StringView_findCharFrom_n(
    PCB_StringView sv, PCB_StringView accept, size_t n
) {
    PCB_CHECK(PCB_String_isEmpty(&sv),     PCB_ZEROED_T(PCB_StringView));
    PCB_CHECK(PCB_String_isEmpty(&accept), PCB_ZEROED_T(PCB_StringView));
    PCB_CHECK(n == 0,                      PCB_ZEROED_T(PCB_StringView));
    PCB_StringView cur = sv;
    while(true) {
        for(size_t i = 0; i < accept.length; i++) {
            if(*cur.data == accept.data[i]) {
                n--; break;
            }
        }
        if(n == 0) break;
        else {
            cur.data++; cur.length--;
            if(cur.length == 0) return PCB_ZEROED_T(PCB_StringView);
        }
    }
    return cur;
}

PCB_StringView PCB_StringView_findCharNotFrom_n(
    PCB_StringView sv, PCB_StringView accept, size_t n
) {
    PCB_CHECK(PCB_String_isEmpty(&sv),     PCB_ZEROED_T(PCB_StringView));
    PCB_CHECK(PCB_String_isEmpty(&accept), PCB_ZEROED_T(PCB_StringView));
    PCB_CHECK(n == 0,                      PCB_ZEROED_T(PCB_StringView));
    PCB_StringView cur = sv;
    while(true) {
        bool anyOfAccept = false;
        for(size_t i = 0; i < accept.length; i++) {
            if(*cur.data == accept.data[i]) {
                anyOfAccept = true; break;
            }
        }
        if(!anyOfAccept) { --n; }
        if(n == 0) break;
        else {
            cur.data++; cur.length--;
            if(cur.length == 0) return PCB_ZEROED_T(PCB_StringView);
        }
    }
    return cur;
}

//PCB_CodepointLengthFromFirstCharacter_UTF8
//Pay close attention to return values before use.
static PCB_ForceInline uint8_t PCB__CPLFFC_UTF8(unsigned int ch) {
//NOTE: `ch` is NOT a Unicode codepoint, it is a zero-extended 1st byte of
//the input string
#if PCB_COMPILER_GCC || PCB_COMPILER_CLANG
#define PCB__unlikely(cond) __builtin_expect(!!(cond), 0)
#else
#define PCB__unlikely(cond) (cond)
#endif
#if defined(PCB_UTF8_FULL_RANGE) && !defined(PCB_UNICODE_CONFORMANT)
    if(PCB__unlikely(ch > 0xFD)) return 255; //1111111-, invalid
#else
    if(PCB__unlikely(ch > 0xF4)) return 255; //111110xx (>U+10FFFF), 1111110x
#endif //PCB_UTF8_FULL_RANGE
    if(PCB__unlikely(ch == 0xC0 || ch == 0xC1)) return 254;
#undef PCB__unlikely
#if PCB_ARCH_x86_64
#if PCB_COMPILER_GCC || PCB_COMPILER_CLANG
    __asm__ __volatile__ goto (
        "mov{l} {$2147483649, %%eax | eax, 2147483649}\n\t"
        "cpuid\n\t"
        "test{l} {$32, %%ecx | ecx, 32}\n\t"
        "jz %l0"
        ::: "eax", "ecx", "edx", "cc" : no_lzcnt
    );
    __asm__ __volatile__(
        "not{l} %k0\n\t"
        "{sall $24, %k0 | shl %k0, 24}\n\t"
        "lzcnt{l} %k0, %k0\n\t"
        "cmp{l $1, %k0 | %k0, 1}\n\t"
        "jg ret%=\n\t"
        "xor{l $1, %k0 | %k0, 1}\n" /* 0->1 (ASCII), 1->0 (continuation byte) */
        "ret%=:\n\t"
        : "+r" (ch) :: "cc"
    );
    return (uint8_t)ch;
#elif PCB_COMPILER_MSVC
    //https://learn.microsoft.com/en-us/cpp/intrinsics/cpuid-cpuidex
    int cpuinfo[4];
    __cpuid(cpuinfo, 0x80000001);
    if(!(cpuinfo[2] & (1 << 5))) goto no_lzcnt;
    ch = __lzcnt(~ch << 24);
    return (ch > 1 ? ch : ch ^ 1); //0->1 (ASCII), 1->0 (continuation byte)
#endif //compilers/x86_64
#endif //architectures
    if(0) goto no_lzcnt; //suppress "unused label" warnings
    no_lzcnt:
    if(ch <= 0x7F) return 1; /* ASCII */
    if(ch <= 0xBF) return 0; /* 10xxxxxx, continuation byte, invalid input */
    if(ch <= 0xDF) return 2; /* 110xxxxx */
    if(ch <= 0xEF) return 3; /* 1110xxxx */
#if defined(PCB_UTF8_FULL_RANGE) && !defined(PCB_UNICODE_CONFORMANT)
    if(ch <= 0xF7) return 4; /* 11110xxx */
    if(ch <= 0xFB) return 5; /* 111110xx */
    if(ch <= 0xFD) return 6; /* 1111110x */
#else
    if(ch <= 0xF4) return 4; /* 11110xxx (<=U+10FFFF)*/
#endif
    PCB_Unreachable;
}

uint8_t PCB_StringView_GetCodepointLength(PCB_StringView sv, size_t index) {
    PCB_CHECK(PCB_String_isEmpty(&sv), 0);
    PCB_CHECK(index >= sv.length, 0);
    return PCB_StringView_GetCodepointLength_unchecked(sv, index);
}

uint8_t PCB_StringView_GetCodepointLength_unchecked(
    PCB_StringView sv, size_t index
) {
    uint8_t len = PCB__CPLFFC_UTF8((unsigned char)sv.data[index]);
    if(len == 0 || len >= 254) return 0;
    return len;
}

#ifndef PCB_UNICODE_CONFORMANT
static const int32_t PCB__MINIMAL_CODEPOINT_UTF8[] = {
    0, 0x80, 0x800, 0x10000,
#ifdef PCB_UTF8_FULL_RANGE
    0x200000, 0x4000000,
#endif //PCB_UTF8_FULL_RANGE
};
#endif //!PCB_UNICODE_CONFORMANT

PCB_Codepoint PCB_StringView_GetCodepoint(PCB_StringView sv, size_t index) {
    PCB_CHECK(PCB_String_isEmpty(&sv), (PCB_CLITERAL(PCB_Codepoint){ -16, 0 }));
    PCB_CHECK(index >= sv.length, (PCB_CLITERAL(PCB_Codepoint){ -17, 0 }));
    PCB_CHECK((uintptr_t)sv.data > (uintptr_t)-1 - sv.length, (PCB_CLITERAL(PCB_Codepoint){ -18, 0 }));
    return PCB_StringView_GetCodepoint_unchecked(sv, index);
}

PCB_Codepoint PCB_StringView_GetCodepoint_unchecked(PCB_StringView sv, size_t index) {
#define PCB__ISCONT(byte) ((byte & 0xC0) == 0x80)
#define PCB__CP_ERR(code, bytesToSkip) PCB_CLITERAL(PCB_Codepoint){ code, bytesToSkip }
    const unsigned char* cursor = (const unsigned char*)(sv.data + index);
    const unsigned char* const end = (const unsigned char*)(sv.data + sv.length);
    uint8_t len = PCB__CPLFFC_UTF8(*cursor);
    if(len == 0) return PCB__CP_ERR(-1, 1);
    else if(len == 254) return PCB__CP_ERR(-6, 1);
    else if(len == 255) return PCB__CP_ERR(-3, 1);

    if((uintptr_t)cursor > (uintptr_t)-1 - len) return PCB__CP_ERR(-18, 0);
#ifndef PCB_UNICODE_CONFORMANT
    if(cursor + len > end) return PCB__CP_ERR(-2, 1);
#endif //!PCB_UNICODE_CONFORMANT
    if(len == 1) return PCB_CLITERAL(PCB_Codepoint){ *cursor, 1 }; //ASCII

    const uint32_t mask = (1u << (8u - len)) - 1;
    uint32_t codepoint = (uint32_t)*cursor & mask;

#ifdef PCB_UNICODE_CONFORMANT
    if(cursor + 1 == end) return PCB__CP_ERR(-2, 1);
    //https://www.unicode.org/versions/Unicode6.0.0/ch03.pdf, table 3-7
    switch(cursor[0]) {
      case 0xE0: if(cursor[1] < 0xA0) { return PCB__CP_ERR(-3, 1); } break;
      case 0xED: if(cursor[1] > 0x9F) { return PCB__CP_ERR(-3, 1); } break;
      case 0xF0: if(cursor[1] < 0x90) { return PCB__CP_ERR(-3, 1); } break;
      case 0xF4: if(cursor[1] > 0x8F) { return PCB__CP_ERR(-3, 1); } break;
      default: break;
    }
    ++cursor;

    switch(len) {
      case 4:
        if(!PCB__ISCONT(*cursor)) return PCB__CP_ERR(-4, 1);
        codepoint = (codepoint << 6) + (*cursor++ & 0x3F);
        //fallthrough
      case 3:
        if(!PCB__ISCONT(*cursor)) return PCB__CP_ERR(-4, len - 2u);
        if(cursor == end)         return PCB__CP_ERR(-2, len - 2u);
        codepoint = (codepoint << 6) + (*cursor++ & 0x3F);
        //fallthrough
      case 2:
        if(!PCB__ISCONT(*cursor)) return PCB__CP_ERR(-4, len - 1u);
        if(cursor == end)         return PCB__CP_ERR(-2, len - 1u);
        codepoint = (codepoint << 6) + (*cursor & 0x3F);
        return PCB_CLITERAL(PCB_Codepoint){ (int32_t)codepoint, len };
      default: PCB_Unreachable;
    }
#else
    ++cursor;
    for(uint8_t l = len - 1; l > 0; ++cursor, --l) {
        if(!PCB__ISCONT(*cursor)) return PCB__CP_ERR(-4, (uint32_t)(len - l));
        codepoint = (codepoint << 6) + (*cursor & 0x3F);
    }
    // const int32_t errval = -(int32_t)len - 2;
    if(0xD800 <= codepoint && codepoint <= 0xDFFF)
        return PCB__CP_ERR(-5, len); //surrogates
    if((int32_t)codepoint < PCB__MINIMAL_CODEPOINT_UTF8[len - 1])
        return PCB__CP_ERR(-3, len);
#ifndef PCB_UTF8_FULL_RANGE
    if(codepoint > 0x10FFFF) return PCB__CP_ERR(-3, len);
#endif //!PCB_UTF8_FULL_RANGE
    return PCB_CLITERAL(PCB_Codepoint){ (int32_t)codepoint, len };
#endif //PCB_UNICODE_CONFORMANT
#undef PCB__ISCONT
#undef PCB__CP_ERR
}

bool PCB_IsValidUnicode(int32_t codepoint) {
    if(0xD800 <= codepoint && codepoint <= 0xDFFF) return false; //surrogates

    //https://www.unicode.org/versions/corrigendum9.html
    // if(0xFDD0 <= codepoint && codepoint <= 0xFDEF) return false; //reserved
    // const int32_t ls2b = codepoint & 0xFFFF;
    // if(ls2b == 0xFFFE || ls2b == 0xFFFF) return false; //also reserved

    if(codepoint < 0x000000) return false;
#if !(defined(PCB_UTF8_FULL_RANGE) && !defined(PCB_UNICODE_CONFORMANT))
    if(codepoint > 0x10FFFF) return false;
#endif //!PCB_UTF8_FULL_RANGE
    return true;
}

uint8_t PCB_GetUTF8Length_unchecked(int32_t codepoint) {
    if(codepoint <= 0x000007F) return 1;
    if(codepoint <= 0x00007FF) return 2;
    if(codepoint <= 0x000FFFF) return 3;
#if defined(PCB_UTF8_FULL_RANGE) && !defined(PCB_UNICODE_CONFORMANT)
    if(codepoint <= 0x01FFFFF) return 4;
    if(codepoint <= 0x3FFFFFF) return 5;
    return 6;
#else
    return 4;
#endif //PCB_UTF8_FULL_RANGE
}

uint8_t PCB_GetUTF8Length(int32_t codepoint) {
    if(!PCB_IsValidUnicode(codepoint)) return 0;
    return PCB_GetUTF8Length_unchecked(codepoint);
}

//modified from https://gist.github.com/tylerneylon/9773800
char* PCB_StoreUTF8Codepoint(char* buf, int32_t codepoint) {
    if(codepoint <= 0x7F) {
        *(uint8_t*)buf = (uint8_t)((uint32_t)codepoint & 0x7F);
        return buf + 1;
    }
    uint8_t c[6] = PCB_ZEROED; int i = 0; uint8_t first_max = 0x1F;
    while(codepoint > first_max) {
        c[i++] = (uint8_t)((uint32_t)codepoint & 0x3F) | 0x80;
        codepoint >>= 6; first_max >>= 1;
    }
    c[i++] = ((uint8_t)((uint32_t)codepoint) & first_max) | (~first_max << 2);
    while(i > 0) *(uint8_t*)buf++ = c[--i];
    return buf;
}

uint16_t* PCB_StoreUTF16Codepoint(uint16_t* buf, int32_t codepoint) {
    if(codepoint < 0x10000) {
        *buf = (uint16_t)codepoint;
        return buf + 1;
    }
    codepoint -= 0x10000;
    uint16_t high = 0xD800 + (codepoint / 0x400),
             low  = 0xDC00 + (codepoint % 0x400);
    *buf++ = high;
    *buf++ = low;
    return buf;
}
#endif //PCB_IMPLEMENTATION_STRING

#if !defined(PCB_NO_INLINE_EXPORTS) || (defined(PCB_NO_INLINE_EXPORTS) && defined(PCB_IMPLEMENTATION_STRING))
#define PCB__Str_trim(Type) \
PCB_maybe_inline void Type##_trim(Type* PCB_restrict str) { \
    Type##_trim_left (str); \
    Type##_trim_right(str); \
}
PCB__Str_trim(PCB_String)    //PCB_String_trim()
PCB__Str_trim(PCB_WString)   //PCB_WString_trim()
#undef PCB__Str_trim

PCB_maybe_inline PCB_StringView PCB_StringView_from_String(
    const PCB_String* PCB_restrict str
) {
    PCB_CHECK_SELF(str, PCB_ZEROED_T(PCB_StringView));
    return PCB_View_Vec_A_T(str, PCB_StringView);
}

PCB_maybe_inline PCB_StringView PCB_StringView_from_cstr(
    const char* PCB_restrict str
) {
    PCB_CHECK_NULL(str, PCB_ZEROED_T(PCB_StringView));
    return PCB_CLITERAL(PCB_StringView){ str, PCB_strlen(str) };
}

PCB_maybe_inline bool PCB_String_replace_range_cstr(
    PCB_String* PCB_restrict str,
    size_t start, size_t length,
    const char* PCB_restrict cstr
) { return PCB_String_replace_range(str, start, length, PCB_StringView_from_cstr(cstr)); }


PCB_maybe_inline PCB_StringView PCB_StringView_substr(
    PCB_StringView sv, PCB_StringView sub
) { return PCB_StringView_substr_n(sv, sub, 1); }

PCB_maybe_inline PCB_StringView PCB_StringView_subcstr(
    PCB_StringView sv, const char* sub
) { return PCB_StringView_substr(sv, PCB_StringView_from_cstr(sub)); }

PCB_maybe_inline PCB_StringView PCB_StringView_subcstr_n(
    PCB_StringView sv, const char* sub, size_t n
) { return PCB_StringView_substr_n(sv, PCB_StringView_from_cstr(sub), n); }

PCB_maybe_inline PCB_StringView PCB_String_substr(
    const PCB_String* str, const PCB_String* sub
) { return PCB_StringView_substr(PCB_StringView_from_String(str), PCB_StringView_from_String(sub)); }

PCB_maybe_inline PCB_StringView PCB_String_subcstr(
    const PCB_String* str, const char* sub
) { return PCB_StringView_subcstr(PCB_StringView_from_String(str), sub); }

PCB_maybe_inline PCB_StringView PCB_String_substr_n(
    const PCB_String* str, const PCB_String* sub, size_t n
) { return PCB_StringView_substr_n(PCB_StringView_from_String(str), PCB_StringView_from_String(sub), n); }

PCB_maybe_inline PCB_StringView PCB_String_subcstr_n(
    const PCB_String* str, const char* sub, size_t n
) { return PCB_StringView_subcstr_n(PCB_StringView_from_String(str), sub, n); }


PCB_maybe_inline PCB_StringView PCB_StringView_rsubstr(
    PCB_StringView sv, PCB_StringView sub
) { return PCB_StringView_rsubstr_n(sv, sub, 1); }

PCB_maybe_inline PCB_StringView PCB_StringView_rsubcstr(
    PCB_StringView sv, const char* sub
) { return PCB_StringView_rsubstr(sv, PCB_StringView_from_cstr(sub)); }

PCB_maybe_inline PCB_StringView PCB_StringView_rsubcstr_n(
    PCB_StringView sv, const char* sub, size_t n
) { return PCB_StringView_rsubstr_n(sv, PCB_StringView_from_cstr(sub), n); }

PCB_maybe_inline PCB_StringView PCB_String_rsubstr(
    const PCB_String* str, const PCB_String* sub
) { return PCB_StringView_rsubstr(PCB_StringView_from_String(str), PCB_StringView_from_String(sub)); }

PCB_maybe_inline PCB_StringView PCB_String_rsubcstr(
    const PCB_String* str, const char* sub
) { return PCB_StringView_rsubcstr(PCB_StringView_from_String(str), sub); }

PCB_maybe_inline PCB_StringView PCB_String_rsubstr_n(
    const PCB_String* str, const PCB_String* sub, size_t n
) { return PCB_StringView_rsubstr_n(PCB_StringView_from_String(str), PCB_StringView_from_String(sub), n); }

PCB_maybe_inline PCB_StringView PCB_String_rsubcstr_n(
    const PCB_String* str, const char* sub, size_t n
) { return PCB_StringView_rsubcstr_n(PCB_StringView_from_String(str), sub, n); }


/*-----------------split, no copy-----------------*/

PCB_maybe_inline PCB_StringViews PCB_StringView_split_cstr(
    PCB_StringView sv, const char* delim
) {
    return PCB_StringView_split(sv, PCB_StringView_from_cstr(delim));
}

PCB_maybe_inline PCB_StringViews PCB_StringView_split_char(
    PCB_StringView sv, const char delim
) {
    char c[2] = { delim, '\0' };
    return PCB_StringView_split(sv, PCB_StringView_from_cstr(c));
}

PCB_maybe_inline PCB_StringViews PCB_String_split(
    const PCB_String* str, const PCB_String* delim
) {
    return PCB_StringView_split(
        PCB_StringView_from_String(str), PCB_StringView_from_String(delim)
    );
}

PCB_maybe_inline PCB_StringViews PCB_String_split_cstr(
    const PCB_String* str, const char* delim
) {
    return PCB_StringView_split(
        PCB_StringView_from_String(str), PCB_StringView_from_cstr(delim)
    );
}

PCB_maybe_inline PCB_StringViews PCB_String_split_char(
    const PCB_String* str, const char delim
) {
    char c[2] = { delim, '\0' };
    return PCB_StringView_split(
        PCB_StringView_from_String(str), PCB_StringView_from_cstr(c)
    );
}

PCB_maybe_inline PCB_StringViews PCB_String_split_whitespace(
    const PCB_String* str
) {
    return PCB_StringView_split_whitespace(PCB_StringView_from_String(str));
}

/*-----------------split, do copy-----------------*/

PCB_maybe_inline PCB_Strings PCB_StringView_split_cstr_copy(
    PCB_StringView sv, const char* delim
) {
    return PCB_StringView_split_copy(sv, PCB_StringView_from_cstr(delim));
}

PCB_maybe_inline PCB_Strings PCB_StringView_split_char_copy(
    PCB_StringView sv, const char delim
) {
    char c[2] = { delim, '\0' };
    return PCB_StringView_split_copy(sv, PCB_StringView_from_cstr(c));
}

PCB_maybe_inline PCB_Strings PCB_String_split_copy(
    const PCB_String* str, const PCB_String* delim
) {
    return PCB_StringView_split_copy(
        PCB_StringView_from_String(str),
        PCB_StringView_from_String(delim)
    );
}

PCB_maybe_inline PCB_Strings PCB_String_split_cstr_copy(
    const PCB_String* str, const char* delim
) {
    return PCB_StringView_split_copy(
        PCB_StringView_from_String(str), PCB_StringView_from_cstr(delim)
    );
}

PCB_maybe_inline PCB_Strings PCB_String_split_char_copy(
    const PCB_String* str, const char delim
) {
    char c[2] = { delim, '\0' };
    return PCB_StringView_split_copy(
        PCB_StringView_from_String(str), PCB_StringView_from_cstr(c)
    );
}

PCB_maybe_inline PCB_Strings PCB_String_split_whitespace_copy(
    const PCB_String* str
) {
    return PCB_StringView_split_whitespace_copy(PCB_StringView_from_String(str));
}

/* Similarly for `PCB_StringView_findCharFrom_n`. */

PCB_maybe_inline PCB_StringView PCB_StringView_findCharFrom(
    PCB_StringView sv, PCB_StringView accept
) {
    return PCB_StringView_findCharFrom_n(sv, accept, 1);
}

PCB_maybe_inline PCB_StringView PCB_StringView_findCharFrom_cstr(
    PCB_StringView sv, const char* accept
) {
    return PCB_StringView_findCharFrom_n(
        sv, PCB_StringView_from_cstr(accept), 1
    );
}

PCB_maybe_inline PCB_StringView PCB_StringView_findCharFrom_cstr_n(
    PCB_StringView sv, const char* accept, size_t n
) {
    return PCB_StringView_findCharFrom_n(
        sv, PCB_StringView_from_cstr(accept), n
    );
}

PCB_maybe_inline PCB_StringView PCB_String_findCharFrom(
    const PCB_String* str, const PCB_String* accept
) {
    return PCB_StringView_findCharFrom_n(
        PCB_StringView_from_String(str), PCB_StringView_from_String(accept), 1
    );
}

PCB_maybe_inline PCB_StringView PCB_String_findCharFrom_n(
    const PCB_String* str, const PCB_String* accept, size_t n
) {
    return PCB_StringView_findCharFrom_n(
        PCB_StringView_from_String(str), PCB_StringView_from_String(accept), n
    );
}

PCB_maybe_inline PCB_StringView PCB_String_findCharFrom_cstr(
    const PCB_String* str, const char* accept
) {
    return PCB_StringView_findCharFrom_n(
        PCB_StringView_from_String(str), PCB_StringView_from_cstr(accept), 1
    );
}

PCB_maybe_inline PCB_StringView PCB_String_findCharFrom_cstr_n(
    const PCB_String* str, const char* accept, size_t n
) {
    return PCB_StringView_findCharFrom_cstr_n(
        PCB_StringView_from_String(str), accept, n
    );
}

/* Similarly for `PCB_StringView_findCharNotFrom_n`. */

PCB_maybe_inline PCB_StringView PCB_StringView_findCharNotFrom(
    PCB_StringView sv, PCB_StringView accept
) {
    return PCB_StringView_findCharNotFrom_n(sv, accept, 1);
}

PCB_maybe_inline PCB_StringView PCB_StringView_findCharNotFrom_cstr(
    PCB_StringView sv, const char* accept
) {
    return PCB_StringView_findCharNotFrom_n(
        sv, PCB_StringView_from_cstr(accept), 1
    );
}

PCB_maybe_inline PCB_StringView PCB_StringView_findCharNotFrom_cstr_n(
    PCB_StringView sv, const char* accept, size_t n
) {
    return PCB_StringView_findCharNotFrom_n(
        sv, PCB_StringView_from_cstr(accept), n
    );
}

PCB_maybe_inline PCB_StringView PCB_String_findCharNotFrom(
    const PCB_String* str, const PCB_String* accept
) {
    return PCB_StringView_findCharNotFrom_n(
        PCB_StringView_from_String(str), PCB_StringView_from_String(accept), 1
    );
}

PCB_maybe_inline PCB_StringView PCB_String_findCharNotFrom_n(
    const PCB_String* str, const PCB_String* accept, size_t n
) {
    return PCB_StringView_findCharNotFrom_n(
        PCB_StringView_from_String(str), PCB_StringView_from_String(accept), n
    );
}

PCB_maybe_inline PCB_StringView PCB_String_findCharNotFrom_cstr(
    const PCB_String* str, const char* accept
) {
    return PCB_StringView_findCharNotFrom_n(
        PCB_StringView_from_String(str), PCB_StringView_from_cstr(accept), 1
    );
}

PCB_maybe_inline PCB_StringView PCB_String_findCharNotFrom_cstr_n(
    const PCB_String* str, const char* accept, size_t n
) {
    return PCB_StringView_findCharNotFrom_cstr_n(
        PCB_StringView_from_String(str), accept, n
    );
}
#endif //PCB_IMPLEMENTATION_STRING (inline)

//Section 3.4: Platform-independent (sort of) process functions.
#ifdef PCB_IMPLEMENTATION_PROCESS
PCB_Process PCB_Process_self(void) {
    PCB_TODO("PCB_Process_self");
    return PCB_Process_init();
}

PCB_Process PCB_Process_init(void) {
    PCB_Process p = PCB_ZEROED;
    p.handle = PCB_PROCESS_INVALID_HANDLE;
    return p;
}

bool PCB_Process_isValid(const PCB_Process* process) {
    PCB_CHECK_SELF(process, false);
#if PCB_PLATFORM_WINDOWS
    return process->handle != INVALID_HANDLE_VALUE;
#elif PCB_PLATFORM_POSIX
    return process->handle > (pid_t)0;
#endif
}

bool PCB_Process_waitForExit(PCB_Process* process) {
    PCB_CHECK_SELF(process, false);
#if PCB_PLATFORM_WINDOWS
    DWORD val = WaitForSingleObject(process->handle, INFINITE);
    if(val == WAIT_FAILED) {
        errno = 0; return false;
    } //WAIT_TIMEOUT is impossible since INFINITE is provided as wait time
    return true;
#elif PCB_PLATFORM_POSIX
    pid_t id = -1;
wait:
    id = waitpid(process->handle, &process->status, 0);
    if(id == -1) {
        if(errno == EINTR) goto wait;
        return false;
    }
    PCB_assert(id != 0);
    return true;
#endif //platform
}

int PCB_Process_checkExit(PCB_Process* process) {
    PCB_CHECK_SELF(process, (PCB_ClearError(), errno = EFAULT, -1) );
#if PCB_PLATFORM_WINDOWS
    switch(WaitForSingleObject(process->handle, 0)) {
      case WAIT_FAILED:   errno = 0; return -1;
      case WAIT_TIMEOUT:  return false;
      case WAIT_OBJECT_0: return true;
    }
    PCB_Unreachable;
#elif PCB_PLATFORM_POSIX
    pid_t id = waitpid(process->handle, &process->status, WNOHANG);
    if(id == -1) return -1;
    if(id == 0) return false; //no child changed state
    if(WIFEXITED(process->status) || WIFSIGNALED(process->status)) {
        return true;
    }
    return false;
#endif //platform
}

int PCB_Process_getExitCode(const PCB_Process* process) {
    PCB_CHECK_SELF(process, (PCB_ClearError(), errno = EFAULT, -1) );
#if PCB_PLATFORM_WINDOWS
    DWORD exitCode;
    if(!GetExitCodeProcess(process->handle, &exitCode)) return -1;
    bool abnormalExit =
        exitCode == STATUS_ACCESS_VIOLATION ||
        exitCode == STATUS_HEAP_CORRUPTION;
    if(abnormalExit) {
        const char* cause = NULL;
        switch(exitCode) {
          case STATUS_ACCESS_VIOLATION: cause = "Memory access violation"; break;
          case STATUS_HEAP_CORRUPTION:  cause = "Heap corruption"; break;
          default: PCB_Unreachable;
        }
        PCB_log(
            PCB_LOGLEVEL_ERROR, "Child process %u exited abnormally: %s",
            (uint32_t)GetProcessId(process->handle), cause
        );
    }
    return (int)exitCode;
#elif PCB_PLATFORM_POSIX
    if(WIFEXITED(process->status)) return WEXITSTATUS(process->status);
    if(WIFSIGNALED(process->status)) {
        int signal = WTERMSIG(process->status);
        bool abnormalExit =
            signal == SIGABRT || signal == SIGKILL || signal == SIGSEGV ||
            signal == SIGBUS  || signal == SIGFPE  || signal == SIGILL;
        if(abnormalExit) {
            const char* cause = NULL;
            switch(WTERMSIG(process->status)) {
              case SIGABRT: cause = "Aborted"; break;
              case SIGBUS:  cause = "Bus error"; break;
              case SIGFPE:  cause = "Floating point exception"; break;
              case SIGILL:  cause = "Illegal instruction"; break;
              case SIGSEGV: cause = "Segmentation fault"; break;
              case SIGKILL: cause = "Killed"; break;
              default: PCB_Unreachable;
            }
            PCB_log(
                PCB_LOGLEVEL_ERROR, "Process %u exited abnormally: %s",
                (uint32_t)process->handle, cause
            );
        }
        return -signal - 1;
    }
    return -1;
#endif //platform
}

void PCB_Process_destroy(PCB_Process* process) {
    if(process == NULL) return;
#if PCB_PLATFORM_WINDOWS
    CloseHandle(process->handle);
#elif PCB_PLATFORM_POSIX
    process->status = 0;
#endif //platform
    process->handle = PCB_PROCESS_INVALID_HANDLE;
}

int PCB_Processes_waitForAny(PCB_Processes* processes) {
    PCB_CHECK_SELF(processes, (PCB_ClearError(), errno = EFAULT, -1) );
    PCB_Vec_enumerate(processes, i, v, it, const PCB_Process) {
        if(!PCB_Process_isValid(it.v)) return -(int)it.i - 2;
    }
#if PCB_PLATFORM_WINDOWS
    if(processes->length > MAXIMUM_WAIT_OBJECTS) {
        PCB_log(
            PCB_LOGLEVEL_ERROR,
            "Cannot wait for %llu processes, which exceeds the maximum"
            "of " PCB_STRINGIFY(MAXIMUM_WAIT_OBJECTS) " waitable objects. "
            "This should be implemented, but it currently isn't.",
            processes->length
        );
        return -1;
    }
    PCB_static_assert(sizeof(PCB_Process) == sizeof(HANDLE), "New member field, cannot cast to HANDLE*");
    const HANDLE* handles = (const HANDLE*)processes->data;
    DWORD status = WaitForMultipleObjects(
        (DWORD)processes->length, handles, FALSE, INFINITE
    );
    if(status == WAIT_FAILED) return -1;
    return (int)(status - WAIT_OBJECT_0);
#elif PCB_PLATFORM_POSIX
    while(true) {
        size_t i = 0;
        for(; i < processes->length; i++) {
            switch(PCB_Process_checkExit(&processes->data[i])) {
              case true: return (int)i;
              case false: break;
              case -1: return -1;
            }
        }
        if(i == processes->length) {
            struct timespec t;
            t.tv_nsec = 1 * 1000 * 1000; t.tv_sec = 0;
            nanosleep(&t, NULL);
        }
    }
    PCB_Unreachable;
#endif //platform
}

int PCB_Processes_waitForRange(
    PCB_Processes* PCB_restrict processes,
    size_t start,
    size_t end
) {
    PCB_CHECK_SELF(processes, (PCB_ClearError(), errno = EFAULT, -1) );
    PCB_CHECK(end > processes->length || start >= end, (PCB_ClearError(), errno = EINVAL, -1) );
    PCB_Vec_enumerate(processes, i, v, it, const PCB_Process) {
        if(!PCB_Process_isValid(it.v)) return -(int)it.i - 2;
    }
    for(size_t i = start; i < end; i++) {
        PCB_Process* p = &processes->data[i];
        if(!PCB_Process_waitForExit(p)) {
            //this is an annoying way to signal which entry errored out,
            //but the only one without using any additional structures.
            return -(int)i - 2;
        }
    }
    return 0;
}

int PCB_Processes_waitForAll(PCB_Processes* processes) {
    return PCB_Processes_waitForRange(processes, 0, processes->length);
}

//check whether `vfork` is available according to vfork(2)...I love you glibc <3
#if PCB_PLATFORM_POSIX
#ifdef PCB_HAS_VFORK
#error "PCB_HAS_VFORK macro should not be defined prior to this place"
#else
#ifdef __GLIBC__
#if __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 12
//and now, hell begins
#if (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE+0 >= 500) && !(defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE+0 >= 200809L)
#define PCB_TEMP 1
#else
#define PCB_TEMP 0
#endif //temporary convenience macro
#if __GLIBC_MINOR__ == 19
#if PCB_TEMP && defined(_DEFAULT_SOURCE) && defined(_BSD_SOURCE)
#define PCB_HAS_VFORK
#endif //glibc 2.19 check
#elif __GLIBC_MINOR__ > 19
#if PCB_TEMP && defined(_DEFAULT_SOURCE)
#define PCB_HAS_VFORK
#endif //glibc >2.19 check
#else
#if PCB_TEMP && defined(_BSD_SOURCE)
#define PCB_HAS_VFORK
#endif //glibc 2.12-2.18 check
#endif //glibc 2.12+ checks
#undef PCB_TEMP
#else
#if defined(_BSD_SOURCE) || (defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE+0) >= 500)
#define PCB_HAS_VFORK
#endif //glibc check (<2.12)
#endif //glibc checks
#endif //glibc
#endif //PCB_HAS_VFORK
#endif //POSIX-only thing

PCB_Process PCB_ShellCommand_runBg(PCB_ShellCommand* command) {
    PCB_CHECK_SELF(command, PCB_Process_init());
    if(command->data == NULL || command->length < 1) {
        PCB_log(PCB_LOGLEVEL_ERROR, "%s/Cannot run an empty command", __func__);
        PCB_ClearError(); errno = EINVAL;
        return PCB_Process_init();
    }
#if PCB_PLATFORM_WINDOWS
    STARTUPINFO startupinfo   = PCB_ZEROED; startupinfo.cb = sizeof(startupinfo);
    PROCESS_INFORMATION pInfo = PCB_ZEROED;

    PCB_String s = PCB_ShellCommand_render(command);
    if(PCB_String_isEmpty(&s)) return PCB_Process_init();

    errno = 0;
    BOOL success = CreateProcessA(
        NULL, s.data, NULL, NULL, true, 0, NULL, NULL, &startupinfo, &pInfo
    );
    PCB_String_destroy(&s);
    if(!success) {
        PCB_logLatestError("Failed to create a child process");
        return PCB_Process_init();
    }
    CloseHandle(pInfo.hThread);
    PCB_Process process = PCB_Process_init();
    process.handle = pInfo.hProcess;
    return process;
#elif PCB_PLATFORM_POSIX
    //the caller may depend on the lack of null termination afterwards
    bool hadNullLast = true;
    if(command->data[command->length - 1] != NULL) {
        PCB_ShellCommand_append_arg(command, NULL);
        hadNullLast = false;
    }
    int code = 0;
    PCB_Process child = PCB_Process_init();
#ifdef PCB_HAS_VFORK
    child.handle = vfork();
#else
    //checking what error has occured in the child is impossible with fork(2) without some IPC
    int tmpPipe[2] = { -1, -1 };
    ssize_t r = -1;
    if(pipe(tmpPipe) < 0) {
        PCB_logLatestError("Failed to create a temporary pipe");
        code = -errno; goto end;
    }
    if(fcntl(tmpPipe[1], F_SETFD, FD_CLOEXEC) < 0) {
        PCB_logLatestError("Failed to set the temporary pipe to 'close-on-exec'");
        code = -errno; goto end;
    }
    child.handle = fork();
#endif //PCB_HAS_VFORK?
    if(child.handle == -1) {
        PCB_logLatestError("Failed to create a child process");
        code = -errno; goto end;
    }
    else if(child.handle == 0) {
        close(tmpPipe[0]);
        execvp(command->data[0], (char* const*)command->data);
        code = errno;
#ifdef PCB_HAS_VFORK
        PCB_logLatestError("Failed to execute shell command");
#else
        write(tmpPipe[1], &code, sizeof(code));
        close(tmpPipe[1]);
#endif //PCB_HAS_VFORK?
        _exit(255);
    }
#ifndef PCB_HAS_VFORK
    close(tmpPipe[1]); tmpPipe[1] = -1;
repeat:
    r = read(tmpPipe[0], &code, sizeof(code));
    if(r < 0) {
        if(errno == EINTR) goto repeat;
        PCB_Unreachable; //at least it should be...
    } else if(r > 0) {
        errno = code;
        code = -code;
    } //0 means nothing was written and no error has occured
#endif //PCB_HAS_VFORK
end:
#ifndef PCB_HAS_VFORK
    if(tmpPipe[1] >= 0) close(tmpPipe[1]);
    if(tmpPipe[0] >= 0) close(tmpPipe[0]);
#endif //!PCB_HAS_VFORK?
    if(code != 0) {
        if(child.handle > 0) waitpid(child.handle, NULL, 0); //reap the child on error
        child.handle = -code;
    }
    if(!hadNullLast) --command->length;
    return child;
#endif //platform-dependent way of running a shell command
}

int PCB_ShellCommand_runAndWait(PCB_ShellCommand* command) {
    if(command->length < 1) {
        PCB_log(PCB_LOGLEVEL_ERROR, "Cannot run an empty command");
        PCB_ClearError(); errno = EINVAL; return -1;
    }

    PCB_Process process = PCB_ShellCommand_runBg(command);
    if(!PCB_Process_isValid(&process)) return -1;
    if(!PCB_Process_waitForExit(&process)) {
        PCB_logLatestError("Failed to wait for shell command to exit");
        return -1;
    }
    int code = PCB_Process_getExitCode(&process);
    PCB_Process_destroy(&process);
    return code;
}

PCB_String PCB_ShellCommand_render(const PCB_ShellCommand* command) {
    PCB_CStringsView cv = PCB_View_Vec_A(command);
    PCB_String s = PCB_ZEROED;
    if(PCB_Vec_isEmpty(&cv)) return s;
    //a heuristic prediction to minimize reallocs
    if(!PCB_String_reserve(&s, 8 * cv.length)) goto error;
    PCB_Vec_enumerate(&cv, i, v, it, const char*) {
        if(*it.v == NULL) {
            for(size_t j = it.i+1; j < cv.length; j++) {
                if(cv.data[j] == NULL) continue;
                PCB_log(
                    PCB_LOGLEVEL_ERROR,
                    "%s: Stray NULL in the middle of a command",
                    __func__
                );
#if PCB_PLATFORM_WINDOWS
                SetLastError(0);
#endif //Windows nonsense
                errno = EFAULT; goto error;
            }
            break; //allow multiple NULLs at the end of `command` everywhere for portability
        }
        PCB_StringView arg = PCB_StringView_from_cstr(*it.v);
        //other whitespace characters are nonsensical inside a shell command...right?
        PCB_StringView sv = PCB_StringView_findCharFrom_cstr(arg, " \t\v");
        bool needs_quotes = !PCB_String_isEmpty(&sv);
        if(needs_quotes)
            if(!PCB_String_append_chars(&s, '"', 1)) goto error;
        while(arg.length > 0) {
            sv = PCB_StringView_findCharFrom_cstr(arg, "\"\'\\");
            if(PCB_String_isEmpty(&sv)) {
                if(!PCB_String_append_sv(&s, arg)) goto error;
                break;
            }
            ptrdiff_t l = sv.data - arg.data;
            if(!PCB_String_appendf(&s, "%.*s\\%c", (int)l, arg.data, *sv.data)) goto error;
            arg.data   += (size_t)l + 1;
            arg.length -= (size_t)l + 1;
        }
        if(needs_quotes)
            if(!PCB_String_append_chars(&s, '"', 1)) goto error;

        if(!PCB_String_append_chars(&s, ' ', 1)) goto error;
    }
    PCB_String_pop(&s); //remove trailing ' '
    return s;
error:
    PCB_String_destroy(&s);
    return PCB_ZEROED_T(PCB_String);
}
#endif //PCB_IMPLEMENTATION_PROCESS



//Section 3.5: other platform-independent stuff
#ifdef PCB_IMPLEMENTATION_ARENA
struct PCB_ArenaMark {
    size_t length;
    size_t lengths[1];
};

static inline size_t PCB__Arena_nodes(PCB_Arena* arena) {
    PCB_Arena_Prefix* current = (PCB_Arena_Prefix*)arena;
    PCB_Arena_Prefix* next = (PCB_Arena_Prefix*)current->next;
    size_t i = 1;
    while(next != NULL) {
        ++i;
        current = next;
        next = (PCB_Arena_Prefix*)next->next;
    }
    return i;
}

static inline PCB_Arena_Prefix* PCB__Arena_marknode(
    PCB_Arena* arena, PCB_ArenaMark* mark
) {
    const size_t marklen = sizeof(mark->length) + mark->length * sizeof(*mark->lengths);
    PCB_Arena_Prefix* marknode = NULL; //node that houses `mark`
    PCB_Arena_Prefix* current  = (PCB_Arena_Prefix*)arena;
    PCB_Arena_Prefix* next     = (PCB_Arena_Prefix*)current->next;
    while(true) {
        char* start = (char*)current + sizeof(*current);
        char* end = start + current->capacity * sizeof(void*);
        if((char*)mark >= start && (char*)mark + marklen <= end) {
            marknode = current; break;
        }
        if(next == NULL) break;
        current = next;
        next = (PCB_Arena_Prefix*)next->next;
    }
    return marknode;
}

static inline void PCB__Arena_restore(PCB_Arena* arena, PCB_ArenaMark* mark) {
    PCB_Arena_Prefix* current = (PCB_Arena_Prefix*)arena;
    PCB_Arena_Prefix* next    = (PCB_Arena_Prefix*)current->next;
    //NOTE: this code assumes the number of nodes in `arena` cannot be
    //`< mark->length`, which may not hold true if `arena` gains the capability
    //of dropping nodes
    for(size_t i = 0; i < mark->length; i++) {
        next = (PCB_Arena_Prefix*)current->next;
        current->length = mark->lengths[i];
        current = next;
    }
    //if new nodes were allocated after `mark`'s creation, they should be reset
    if(next != NULL) PCB_Arena_reset((PCB_Arena*)next);
}

PCB_Arena* PCB_Arena_init(size_t size) {
    PCB_CHECK(size == 0, NULL);
    size_t capacity = 1;
    while(capacity < size) capacity *= 2;
    PCB_Arena_Prefix* arena = (PCB_Arena_Prefix*)PCB_realloc(NULL, capacity + sizeof(*arena));
    if(arena == NULL) {
#if PCB_PLATFORM_WINDOWS
        SetLastError(0);
#endif
        return NULL;
    }
    arena->length = 0;
    arena->capacity = capacity / sizeof(void*);
    arena->next = NULL;
    return (PCB_Arena*)arena;
}

PCB_Arena* PCB_Arena_init_in(void* mem, size_t memsize) {
    PCB_CHECK_NULL(mem, NULL);
    if(memsize <= sizeof(PCB_Arena_Prefix)) return NULL;
    PCB_Arena_Prefix* arena = (PCB_Arena_Prefix*)mem;
    arena->length = 0;
    arena->capacity = (memsize - sizeof(PCB_Arena_Prefix)) / sizeof(void*);
    arena->next = NULL;
    return (PCB_Arena*)arena;
}

void* PCB_Arena_alloc(PCB_Arena* arena, size_t size) {
    PCB_CHECK_SELF(arena, NULL);
    //size rounded to a multiple of pointer size
    size = (size + sizeof(void*) - 1) & ~(sizeof(void*) - 1);
    if(size == 0) return NULL;
    PCB_Arena_Prefix* a = (PCB_Arena_Prefix*)arena;
    try_alloc:
    if(a->length + (size / sizeof(void*)) > a->capacity) {
        if(a->next != NULL)  {
            a = (PCB_Arena_Prefix*)a->next;
            goto try_alloc;
        }
        size_t capacity = a->capacity;
        while(capacity < size) capacity *= 2;
        a->next = PCB_Arena_init(capacity);
        if(a->next == NULL) return NULL;
        a = (PCB_Arena_Prefix*)a->next;
    }
    void* data = (void*)((char*)a + sizeof(*a) + a->length * sizeof(void*));
    a->length += size / sizeof(void*);
    return data;
}

void* PCB_Arena_calloc(PCB_Arena* arena, size_t size) {
    void* mem = PCB_Arena_alloc(arena, size);
    if(mem != NULL) PCB_memset(mem, 0, size);
    return mem;
}

void* PCB_Arena_aligned_alloc(PCB_Arena* arena, size_t size, size_t alignment) {
    PCB_CHECK_SELF(arena, NULL);
    if(alignment == 0) return NULL;
    const bool pow2 = (alignment & (alignment - 1)) == 0;
    if(!pow2) return NULL;
    if(alignment % sizeof(void*) != 0) return NULL;
    if(size % alignment != 0) return NULL;
    size = (size + sizeof(void*) - 1) & ~(sizeof(void*) - 1);
    if(size == 0) return NULL;

    PCB_Arena_Prefix* a = (PCB_Arena_Prefix*)arena;
    void* data; size_t pad;
    try_alloc:
    data = (void*)((char*)a + sizeof(*a) + a->length * sizeof(void*));
    pad  = alignment - (uintptr_t)data % alignment;
    if(a->length + ((size + pad) / sizeof(void*)) > a->capacity) {
        if(a->next != NULL)  {
            a = (PCB_Arena_Prefix*)a->next;
            goto try_alloc;
        }
        size_t capacity = a->capacity;
        while(capacity < size) capacity *= 2;
        a->next = PCB_Arena_init(capacity);
        if(a->next == NULL) return NULL;
        a = (PCB_Arena_Prefix*)a->next;
    }

    a->length += (size + pad) / sizeof(void*);
    return (void*)((char*)data + pad);
}

void* PCB_Arena_aligned_calloc(PCB_Arena* arena, size_t size, size_t alignment) {
    void* mem = PCB_Arena_aligned_alloc(arena, size, alignment);
    if(mem != NULL) PCB_memset(mem, 0, size);
    return mem;
}

bool PCB_Arena_alloc_whole(PCB_Arena* arena, void** ptr, size_t* size) {
    PCB_CHECK_SELF(arena, false);
    PCB_CHECK_NULL(size, false);
    PCB_Arena_Prefix* a = (PCB_Arena_Prefix*)arena;
    *size = (a->capacity - a->length) * sizeof(void*);
    if(ptr != NULL) {
        if(*size > 0) {
            *ptr = (void*)((char*)a + sizeof(*a) + a->length * sizeof(void*));
            a->length = a->capacity;
        }
        else {
            *ptr = NULL;
        }
    }
    return true;
}

PCB_Arena* PCB_Arena_next(PCB_Arena* arena) {
    PCB_CHECK_SELF(arena, NULL);
    PCB_Arena_Prefix* a = (PCB_Arena_Prefix*)arena;
    return a->next;
}

size_t PCB_Arena_allocated(PCB_Arena* arena) {
    PCB_CHECK_SELF(arena, (size_t)-1);
    PCB_Arena_Prefix* a = (PCB_Arena_Prefix*)arena;
    size_t allocated = 0;
    do {
        allocated += a->length * sizeof(void*);
        a = (PCB_Arena_Prefix*)a->next;
    } while(a != NULL);
    return allocated;
}

size_t PCB_Arena_allocatable(PCB_Arena* arena) {
    PCB_CHECK_SELF(arena, (size_t)-1);
    PCB_Arena_Prefix* a = (PCB_Arena_Prefix*)arena;
    size_t allocatable = 0;
    do {
        allocatable += (a->capacity - a->length) * sizeof(void*);
        a = (PCB_Arena_Prefix*)a->next;
    } while(a != NULL);
    return allocatable;
}

size_t PCB_Arena_capacity(PCB_Arena* arena) {
    PCB_CHECK_SELF(arena, (size_t)-1);
    PCB_Arena_Prefix* a = (PCB_Arena_Prefix*)arena;
    size_t capacity = 0;
    do {
        capacity += a->capacity * sizeof(void*);
        a = (PCB_Arena_Prefix*)a->next;
    } while(a != NULL);
    return capacity;
}

PCB_ArenaMark* PCB_Arena_mark(PCB_Arena* arena) {
    PCB_CHECK_SELF(arena, NULL);
    PCB_Arena_Prefix* current = (PCB_Arena_Prefix*)arena;
    PCB_Arena_Prefix* next    = (PCB_Arena_Prefix*)current->next;

    size_t i = PCB__Arena_nodes(arena);
    PCB_ArenaMark* mark = (PCB_ArenaMark*)PCB_Arena_alloc(
        arena, sizeof(mark->length) + i * sizeof(mark->lengths[0])
    );
    if(mark == NULL) return NULL;

    mark->length = i; i = 0;
    current = (PCB_Arena_Prefix*)arena;
    next = (PCB_Arena_Prefix*)current->next;
    while(true) {
        mark->lengths[i++] = current->length;
        if(next == NULL) break;
        current = next;
        next = (PCB_Arena_Prefix*)next->next;
    }
    return mark;
}

bool PCB_Arena_restore_to(PCB_Arena* arena, PCB_ArenaMark* mark) {
    PCB_CHECK_SELF(arena, false);
    PCB_CHECK_NULL(mark, false);

    PCB_Arena_Prefix* marknode = PCB__Arena_marknode(arena, mark);
    if(marknode == NULL) return false;
    PCB__Arena_restore(arena, mark);
    return true;
}

bool PCB_Arena_restore(PCB_Arena* arena, PCB_ArenaMark* mark) {
    PCB_CHECK_SELF(arena, false);
    PCB_CHECK_NULL(mark, false);

    const size_t marklen = sizeof(mark->length) + mark->length * sizeof(*mark->lengths);
    PCB_Arena_Prefix* marknode = PCB__Arena_marknode(arena, mark);
    if(marknode == NULL) return false;
    PCB__Arena_restore(arena, mark);
    //dealloc `mark` since we know it's the last thing that was allocated
    marknode->length -= marklen / sizeof(void*);
    return true;
}

size_t PCB_Arena_dealloc(PCB_Arena* arena, size_t length) {
    PCB_CHECK_SELF(arena, 0);
    PCB_Arena_Prefix* a = (PCB_Arena_Prefix*)arena;
    size_t deallocated = 0;
    if(a->next != NULL) {
        deallocated = PCB_Arena_dealloc(a->next, length);
        if(deallocated >= length) return deallocated;
    }
    if(a->length == 0) return deallocated;
    const size_t toDealloc = length - deallocated;
    const size_t chunks = ((toDealloc + sizeof(void*) - 1) & ~(sizeof(void*) - 1)) / sizeof(void*);
    if(a->length < chunks) {
        deallocated += a->length * sizeof(void*);
        a->length = 0;
    }
    else {
        deallocated += chunks * sizeof(void*);
        a->length -= chunks;
    }
    return deallocated;
}

size_t PCB_Arena_dealloc_once(PCB_Arena* arena, size_t length) {
    PCB_CHECK_SELF(arena, 0);
    PCB_Arena_Prefix* a = (PCB_Arena_Prefix*)arena;
    if(a->next != NULL) {
        size_t deallocated = PCB_Arena_dealloc_once(a->next, length);
        if(deallocated != 0) return deallocated; //we deallocated *something*
    }
    if(a->length == 0) return 0;
    size_t deallocated = 0;
    size_t chunks = ((length + sizeof(void*) - 1) & ~(sizeof(void*) - 1)) / sizeof(void*);
    if(a->length < chunks) {
        deallocated = a->length * sizeof(void*);
        a->length = 0;
    }
    else {
        deallocated = chunks * sizeof(void*);
        a->length -= chunks;
    }
    return deallocated;
}

void PCB_Arena_reset(PCB_Arena* arena) {
    PCB_CHECK_SELF(arena, );
    PCB_Arena_Prefix* current = (PCB_Arena_Prefix*)arena;
    PCB_Arena_Prefix* next    = (PCB_Arena_Prefix*)current->next;
    while(true) {
        current->length = 0;
        if(next == NULL) break;
        current = next;
        next = (PCB_Arena_Prefix*)next->next;
    }
}

void PCB_Arena_destroy(PCB_Arena* arena) {
    if(arena == NULL) return;
    PCB_Arena_Prefix* next = (PCB_Arena_Prefix*)(((PCB_Arena_Prefix*)arena)->next);
    while(true) {
        PCB_free(arena);
        arena = (PCB_Arena*)next;
        if(arena == NULL) break;
        next = (PCB_Arena_Prefix*)next->next;
    }
}

#ifdef PCB_HAS_STDIO_H
char* PCB_Arena_asprintf(PCB_Arena* arena, const char* fmt, ...) {
    PCB_CHECK_SELF(arena, NULL);
    PCB_CHECK_NULL(fmt, NULL);
    va_list args;
    va_start(args, fmt);
    char* str = PCB_Arena_vasprintf(arena, fmt, args);
    va_end(args);
    return str;
}

char* PCB_Arena_vasprintf(PCB_Arena* arena, const char* fmt, va_list ap) {
    PCB_CHECK_SELF(arena, NULL);
    PCB_CHECK_NULL(fmt, NULL);

    va_list args;
    va_copy(args, ap);
    const size_t lengthRequired = (unsigned int)PCB_vsnprintf(NULL, 0, fmt, args) + 1;
    va_end(args);

    char* text = (char*)PCB_Arena_alloc(arena, lengthRequired);
    if(text == NULL) return NULL;
    va_copy(args, ap);
    const size_t printed = (unsigned int)PCB_vsnprintf(text, lengthRequired, fmt, args);
    PCB_assert(printed + 1 == lengthRequired);
    va_end(args);
    return text;
}
#else
char* PCB_Arena_asprintf(PCB_Arena* arena, const char* fmt, ...) {
    (void)arena; (void)fmt;
    return NULL;
}

char* PCB_Arena_vasprintf(PCB_Arena* arena, const char* fmt, va_list ap) {
    (void)arena; (void)fmt; (void)ap;
    return NULL;
}
#endif //PCB_HAS_STDIO_H?

char* PCB_Arena_strdup(PCB_Arena* arena, const char* str) {
    PCB_CHECK_SELF(arena, NULL);
    PCB_CHECK_NULL(str, NULL);
    size_t len = PCB_strlen(str) + 1; // '\0'
    char* text = (char*)PCB_Arena_alloc(arena, len);
    if(text == NULL) return NULL;
    PCB_memcpy(text, str, len);
    return text;
}

char* PCB_Arena_strndup(PCB_Arena* arena, const char* str, size_t n) {
    PCB_CHECK_SELF(arena, NULL);
    PCB_CHECK_NULL(str, NULL);
    size_t len = PCB_strnlen(str, n); //           '\0'
    char* text = (char*)PCB_Arena_alloc(arena, len + 1);
    if(text == NULL) return NULL;
    PCB_memcpy(text, str, len);
    text[len] = '\0'; //`str` may not end with '\0'
    return text;
}
#endif //PCB_IMPLEMENTATION_ARENA

//uncategorized functions should be put here
#ifdef PCB_IMPLEMENTATION
size_t PCB_getNumberOfCores(void) {
#if PCB_PLATFORM_WINDOWS
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
#elif PCB_PLATFORM_POSIX
    return (size_t)sysconf(_SC_NPROCESSORS_ONLN);
#else
    return 0;
#endif //platform
}
#endif //PCB_IMPLEMENTATION

//Section 3.6: build capability
#ifdef PCB_IMPLEMENTATION_BUILD
static void PCB__CStrings_append(PCB_CStrings* cstrs, const char* cstr) {
    PCB_CStrings_append(cstrs, cstr);
}

const char* PCB_GetCStandardStr(long standard) {
    switch(standard) {
      case 1L:      return "c89"; //see below why 1
//https://sourceforge.net/p/predef/wiki/Standards
#if PCB_COMPILER_GCC || PCB_COMPILER_CLANG
      case 199409L: return "iso9899:199409";
#endif //GCC/Clang have this as a flag for C95, dunno about MSVC
      case 199901L: return "c99";
      case 201112L: return "c11";
      case 201710L: return "c17";
#if PCB_COMPILER_GCC >= 130900
      case 202000L: return "c2x";
#endif //gcc's "c2x", deprecated in GCC14
      case 202311L: return "c23";
#if   PCB_COMPILER_GCC >= 150000
      case 202500L: return "c2y";
#elif PCB_COMPILER_CLANG >= 190000
      case 202400L: return "c2y";
#endif //"c2y"
      default:      return NULL;
    }
    PCB_Unreachable;
}

const char* PCB_GetCppStandardStr(long standard) {
    switch(standard) {
      case 199711L: return "c++98";
      case 201103L: return "c++11"; //there was no C++03 macro definition
      case 201402L: return "c++14";
      case 201703L: return "c++17";
      case 202002L: return "c++20";
      case 202302L: return "c++23";
      default:      return NULL;
    }
    PCB_Unreachable;
}

long PCB_GetCStandardInt(const char* standard) {
    PCB_CHECK_NULL(standard, 0);
    const char* cursor = standard + 1;
    if(standard[0] != 'c') {
#if PCB_COMPILER_GCC || PCB_COMPILER_CLANG
        if(PCB_strncmp(standard, "iso9899:", 8) != 0) return 0;
        cursor = standard + 8;
#else
        return 0;
#endif //GCC/Clang recognize "-std=iso9899:*", not the case with C++ for some reason
    }
    long v = 0;
    while(*cursor) {
        if(*cursor >= '0' && *cursor <= '9') {
            v = v * 10 + (*cursor - '0'); ++cursor;
        } else return 0; //invalid character
    }
    switch(v) {
      //C89 doesn't have __STDC_VERSION__, but it's a valid
      //"-std=", hence a special value of 1
      case 89:
      case 90:   return 1L; //C89 = ANSI C <=> C90 = ISO C90
      //when `standard` starts with "c"
      case 99:   return 199901L;
      case 11:   return 201112L;
      case 17:   return 201710L;
      case 23:   return 202311L;
      //when `standard` starts with "iso9899:", only for GCC/Clang
#if PCB_COMPILER_GCC || PCB_COMPILER_CLANG
      case 1990: return 1L;
      case 1999: return 199901L;
      case 2011: return 201112L;
      case 2017:
      case 2018: return 201710L;
#if PCB_COMPILER_GCC
      case 2024: return 202311L;
#endif //for some reason Clang doesn't have "-std=iso9899:2024"
#endif //GCC/Clang recognize "-std=iso9899:*"
      case 199409: return 199409L;
      default: return 0;
    }
    PCB_Unreachable;
}

long PCB_GetCppStandardInt(const char* standard) {
    PCB_CHECK_NULL(standard, 0);
    size_t len = PCB_strlen(standard);
    //no "c++" at the start
    if(len < 3 || PCB_strncmp(standard, "c++", 3) != 0) return 0;
    const char* cursor = standard + 3;
    long v = 0;
    while(*cursor) {
        if(*cursor >= '0' && *cursor <= '9') {
            v = v * 10 + (*cursor - '0'); ++cursor;
        } else return 0; //invalid character
    }
    switch(v) {
      case 98: return 199711L;
      case 11: return 201103L;
      case 14: return 201402L;
      case 17: return 201703L;
      case 20: return 202002L;
      case 23: return 202302L;
      default: return 0;
    }
    PCB_Unreachable;
}

bool PCB_BuildType_formatName(PCB_BuildType bt, const char** target, PCB_Arena* arena) {
    PCB_CHECK_NULL(target, false);
    PCB_CHECK_NULL(arena, false);
    PCB_CHECK(*target == NULL, false);
    PCB_StringView sv = PCB_StringView_from_cstr(*target);
    switch(bt) {
      case PCB_BUILDTYPE_EXEC: {
#if PCB_PLATFORM_WINDOWS
        PCB_StringView ext = PCB_FS_Extension(sv);
        if(!PCB_String_isEmpty(&ext)) break;
        char* formatted = PCB_Arena_asprintf(arena, "%s.exe", *target);
        if(formatted == NULL) return false;
        *target = formatted;
#endif //on Windows, executable files end with ".exe", unlike other platforms
      } break;
      case PCB_BUILDTYPE_STATICLIB: {
#if PCB_PLATFORM_WINDOWS
        PCB_StringView ext = PCB_FS_Extension(sv);
        if(!PCB_String_isEmpty(&ext)) break;
        char* formatted = PCB_Arena_asprintf(arena, "%s.lib", *target);
#else //outside of Windows this is much more involved
        PCB_StringView base = PCB_FS_Basename(sv);
        PCB_StringView ext  = PCB_FS_Extension_base(base);
        if(!PCB_String_isEmpty(&ext)) break;
        PCB_StringView dir  = PCB_FS_Dirname(sv);
        char* formatted = PCB_Arena_asprintf(
            arena,
            PCB_SV_Fmt "%c" "lib" PCB_SV_Fmt ".a",
            PCB_SV_Arg(dir), PCB_FS_DIR_DELIM, PCB_SV_Arg(base)
        );
#endif //platforms
        if(formatted == NULL) return false;
        *target = formatted;
      } break;
      case PCB_BUILDTYPE_DYNAMICLIB: {
#if PCB_PLATFORM_WINDOWS
        PCB_StringView ext = PCB_FS_Extension(sv);
        if(!PCB_String_isEmpty(&ext)) break;
        char* formatted = PCB_Arena_asprintf(arena, "%s.dll", *target);
#else
        PCB_StringView base = PCB_FS_Basename(sv);
        PCB_StringView ext  = PCB_FS_Extension_base(base);
        if(!PCB_String_isEmpty(&ext)) break;
        PCB_StringView dir  = PCB_FS_Dirname(sv);
#if PCB_PLATFORM_MACOS
#define PCB__DYNAMICLIB_EXT ".dylib"
#else
#define PCB__DYNAMICLIB_EXT ".so"
#endif //the rich kid is acting up again...
        char* formatted = PCB_Arena_asprintf(
            arena,
            PCB_SV_Fmt "%c" "lib" PCB_SV_Fmt PCB__DYNAMICLIB_EXT,
            PCB_SV_Arg(dir), PCB_FS_DIR_DELIM, PCB_SV_Arg(base)
        );
#undef PCB__DYNAMICLIB_EXT
#endif //platforms
        if(formatted == NULL) return false;
        *target = formatted;
      } break;
    }
    return true;
}

void PCB_build_flag_cwl(PCB_ShellCommand* cmd, PCB_ArgvSyntax s) {
    PCB_CHECK_NULL(cmd,);
    switch(s) {
      case PCB_ARGVSYNTAX_POSIX:
        PCB_ShellCommand_append_arg(cmd, "-c"); break;
      case PCB_ARGVSYNTAX_MS:
        PCB_ShellCommand_append_arg(cmd, "/c"); break;
      case PCB_ARGVSYNTAX_UNKNOWN:
        PCB_log(
            PCB_LOGLEVEL_WARN,
            "Cannot add a flag to specify \"compile only\", skipping"
        ); break;
      case PCB_ARGVSYNTAX_COUNT:
        PCB_Unreachable;
    }
}

void PCB_build_flag_output(PCB_ShellCommand* cmd, PCB_ArgvSyntax s) {
    PCB_CHECK_NULL(cmd,);
    switch(s) {
      case PCB_ARGVSYNTAX_POSIX:
        PCB_ShellCommand_append_arg(cmd, "-o");   break;
      case PCB_ARGVSYNTAX_MS:
        PCB_ShellCommand_append_arg(cmd, "/Fo:"); break;
      case PCB_ARGVSYNTAX_UNKNOWN:
        PCB_log(
            PCB_LOGLEVEL_WARN,
            "Cannot add a flag to specify the output path, skipping"
        ); break;
      case PCB_ARGVSYNTAX_COUNT:
        PCB_Unreachable;
    }
}

bool PCB_build_flag_standard(
    PCB_ShellCommand* cmd,
    long standard,
    PCB_Arena* arena,
    PCB_Compiler_RT compiler,
    bool cpp,
    bool gnu
) {
    if(standard == 0) return true;
    const char* standardStr;
    //TODO: move this to runtime
    if(cpp) standardStr = PCB_GetCppStandardStr(standard);
    else    standardStr = PCB_GetCStandardStr(standard);
    if(standardStr == NULL) {
        PCB_log(
            PCB_LOGLEVEL_WARN,
            "Unknown language standard integer value: %ld",
            standard
        ); return true;
    }
#define PCB__MAX_STD_STR_LEN 24
    char* flag = (char*)PCB_Arena_alloc(arena, PCB__MAX_STD_STR_LEN);
    //set the standard if the flag can be parsed
    if(flag == NULL) return false;
    switch(compiler) {
      case PCB_COMPILER_RT_GCC:
      case PCB_COMPILER_RT_CLANG:
        PCB_snprintf(
            flag, PCB__MAX_STD_STR_LEN, "-std=%s%s",
            gnu ? "gnu" : "",
            standardStr + (gnu ? 1 : 0)
        ); break;
      case PCB_COMPILER_RT_MSVC:
        PCB_snprintf(flag, PCB__MAX_STD_STR_LEN, "/std:%s", standardStr);
        break;
      case PCB_COMPILER_RT_UNKNOWN:
        PCB_log(PCB_LOGLEVEL_WARN, "Cannot parse the language standard");
        return true;
    }
#undef PCB__MAX_STD_STR_LEN
    PCB_ShellCommand_append_arg(cmd, flag);
    //MSVC does NOT set "__cplusplus" unless "/Zc:__cplusplus" is added for...reasons.
    if(compiler == PCB_COMPILER_RT_MSVC)
        PCB_ShellCommand_append_arg(cmd, "/Zc:__cplusplus");
    return true;
}

static void PCB__build_flags_diagnostics_default_gcc(
    PCB_CStrings* cstrs, bool cpp, int compilerVersion
) {
    //https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
    PCB_CStrings_append_many(
        cstrs,
        "-Werror=array-bounds=1",
        "-Wformat-signedness",
        "-Wmissing-declarations",
        "-Winit-self",
        "-Wformat=2"
    );
    if(compilerVersion >= 30306) {
        PCB__CStrings_append(cstrs, "-Werror=nonnull");
    }
    if(compilerVersion >= 60100) {
        PCB__CStrings_append(cstrs, "-Wduplicated-cond");
        PCB__CStrings_append(cstrs, "-Wnull-dereference");
    }
    if(compilerVersion >= 70100) {
        PCB__CStrings_append(cstrs, "-Werror=stringop-overflow");
        PCB__CStrings_append(cstrs, "-Walloc-zero");
        PCB__CStrings_append(cstrs, "-Wduplicated-branches");
    }
    if(compilerVersion >= 110100) {
        PCB__CStrings_append(cstrs, "-Werror=stringop-overread");
    }
    if(compilerVersion >= 120100) {
        PCB__CStrings_append(cstrs, "-Werror=use-after-free=2");
    }
    if(compilerVersion >= 140100) {
        PCB__CStrings_append(cstrs, "-Wcalloc-transposed-args");
        PCB__CStrings_append(cstrs, "-Werror=alloc-size");
    }
    if(cpp) {
        //placeholder for future C++-only diagnostics
    } else {
        PCB__CStrings_append(cstrs, "-Wbad-function-cast");
        PCB__CStrings_append(cstrs, "-Wmissing-prototypes");
    }
}

static void PCB__build_flags_diagnostics_default_clang(
    PCB_CStrings* cstrs, bool cpp, int compilerVersion
) {
    //https://clang.llvm.org/docs/DiagnosticsReference.html
    //Clang <4.0.0 seems to be underdocumented.
    //https://releases.llvm.org/18.1.1/tools/clang/docs/DiagnosticsReference.html
    if(compilerVersion >= 40000) {
        PCB_CStrings_append_many(cstrs,
            "-Werror=nonnull",
            "-Werror=array-bounds",
            "-Werror=invalid-noreturn",
            "-Wassign-enum",
            "-Warray-bounds-pointer-arithmetic",
            "-Wbad-function-cast",
            "-Wconversion",
            "-Widiomatic-parentheses",
            "-Winfinite-recursion",
            "-Wmismatched-tags",
            "-Wmissing-variable-declarations",
            "-Wsign-compare",
            "-Wshorten-64-to-32",
            "-Wfloat-conversion",
            "-Wstring-conversion",
            "-Wthread-safety",
            "-Wunused",
            "-Wmissing-prototypes"
        );
    }
    if(compilerVersion >= 100000) {
        PCB__CStrings_append(cstrs, "-Wbool-operation");
        PCB__CStrings_append(cstrs, "-Wformat-type-confusion");
    }
    if(compilerVersion >= 130000) {
        PCB__CStrings_append(cstrs, "-Wcast-function-type");
    }
    if(compilerVersion >= 150000) {
        PCB__CStrings_append(cstrs, "-Warray-parameter");
    }
    if(compilerVersion >= 190000) {
        PCB__CStrings_append(cstrs, "-Wformat-signedness");
    }
    if(cpp) {
        PCB_CStrings_append_many(
            cstrs,
            "-Wpessimizing-move", "-Wself-move",
            "-Wsuggest-destructor-override", "-Wsuggest-override",
            "-Wuninitialized-const-reference"
        );
    } else {
        //placeholder for future C-only diagnostics
    }
}

static void PCB__build_flags_diagnostics_default_msvc(
    PCB_CStrings* cstrs, bool cpp, int compilerVersion
) {
    (void)cpp; (void)compilerVersion;
    //https://learn.microsoft.com/en-us/cpp/build/reference/compiler-option-warning-level
    PCB_CStrings_append_many(
        cstrs,
        "/W4", "/w44062", "/w44388", "/w25219", "/w15247",
        "/w45263", "/w34191"
    );
}

void PCB_build_flags_diagnostics_default(
    PCB_CStrings* cstrs,
    PCB_Compiler_RT compiler,
    bool cpp,
    int compilerVersion,
    bool strict_iso
) {
    PCB_CHECK_NULL(cstrs,);
    PCB__logTrace("Adding default diagnostic flags");
    //these are shared between GCC and Clang
    if(compiler == PCB_COMPILER_RT_GCC || compiler == PCB_COMPILER_RT_CLANG) {
        PCB_CStrings_append_many(
            cstrs,
            "-Wall", "-Wextra",
            //NOTE: The meaning of this diagnostic is not the same.
            //In Clang, it diagnoses implicit conversions to integer types.
            //In GCC, however, it diagnoses reducing precision of double to float.
            "-Werror=float-conversion",
            "-Werror=uninitialized",
            "-Wsign-conversion",
            "-Wundef"
        );
        if(strict_iso) {
            PCB__CStrings_append(cstrs, "-Wpedantic");
            //We may later place warnings about version compatibility here.
        }
    }
    switch(compiler) {
      case PCB_COMPILER_RT_GCC:
        PCB__build_flags_diagnostics_default_gcc(cstrs, cpp, compilerVersion);
        break;
      case PCB_COMPILER_RT_CLANG:
        PCB__build_flags_diagnostics_default_clang(cstrs, cpp, compilerVersion);
        break;
      case PCB_COMPILER_RT_MSVC:
        PCB__build_flags_diagnostics_default_msvc(cstrs, cpp, compilerVersion);
        break;
      case PCB_COMPILER_RT_UNKNOWN:
        break;
    }
}

static void PCB__BuildContext_addDefaultWarnings(PCB_BuildContext* context) {
    PCB_build_flags_diagnostics_current_default(&context->warningFlags);
}

static bool PCB__BuildContext_parseFlags_single(
    PCB_BuildContext* context, PCB_CStringsView strs,
    const char* const fmt,
    const char* const hrn /* human-readable name (for diagnostics) */
) {
    PCB_Vec_enumerate(&strs, i, val, it, const char*) {
        const char* v = *it.val;
        if(v == NULL) {
            PCB_log(
                PCB_LOGLEVEL_WARN, "%s #%" PRIu64 " is NULL, skipping",
                hrn, it.i
            ); continue;
        }
        bool parse = true;
        if(v[0] == '\033') { parse = false; ++v; }
        if(fmt == NULL)    { parse = false; }
        if(!parse) {
            PCB_ShellCommand_append_arg(&context->commandBuffer, v);
            PCB__logTrace("+ %s \"%s\"", hrn, v);
            continue;
        }
#if PCB_COMPILER_GCC
_Pragma("GCC diagnostic push");
_Pragma("GCC diagnostic ignored \"-Wformat-nonliteral\"");
#endif //temporarily disable GCC's "-Wformat-nonliteral" warning since it's fine here
        char* flag = (char*)PCB_Arena_asprintf(context->arena, fmt, v);
#if PCB_COMPILER_GCC
_Pragma("GCC diagnostic pop");
#endif //reenable the warning above
        if(flag == NULL) return false;
        PCB_ShellCommand_append_arg(&context->commandBuffer, flag);
        PCB__logTrace("+ %s \"%s\"", hrn, flag);
    }
    return true;
}

static bool PCB__BuildContext_parseFlags_pairs(
    PCB_BuildContext* context, PCB_CStringPairsView strs,
    const char* const fmts[2],
    const char* hrn /* human-readable name (for diagnostics)*/
) {
    PCB_Vec_enumerate(&strs, i, val, it, const PCB_CStringPair) {
        PCB_CStringPair v = *it.val;
        if(v.key == NULL) {
            PCB_log(
                PCB_LOGLEVEL_WARN, "%s #%" PRIu64 "'s key is NULL, skipping",
                hrn, it.i
            ); continue;
        }
        char* flag = NULL;
#if PCB_COMPILER_GCC
_Pragma("GCC diagnostic push");
_Pragma("GCC diagnostic ignored \"-Wformat-nonliteral\"");
#endif //temporarily disable GCC's "-Wformat-nonliteral" warning since it's fine here
        if(v.value == NULL) {
            if(fmts[0] == NULL) {
                PCB_ShellCommand_append_arg(&context->commandBuffer, v.key);
                PCB__logTrace("+ %s \"%s\"", hrn, v.key);
                continue;
            }
            flag = PCB_Arena_asprintf(context->arena, fmts[0], v.key);
        } else {
            flag = PCB_Arena_asprintf(context->arena, fmts[1], v.key, v.value);
        }
#if PCB_COMPILER_GCC
_Pragma("GCC diagnostic pop");
#endif //reenable the warning above
        if(flag == NULL) return false;
        PCB_ShellCommand_append_arg(&context->commandBuffer, flag);
        PCB__logTrace("+ %s \"%s\"", hrn, flag);
    } return true;
}

int PCB_BuildContext_init(PCB_BuildContext* context, uint64_t flags) {
    PCB_CHECK_SELF(context, 1);
    if(context->arena == NULL) { //user may have provided their own
        context->arena = PCB_Arena_init(1 << 20);
        if(context->arena == NULL) return -ENOMEM;
    }
    if(flags & PCB_BUILDOPTION_DEFAULT_PATHS) {
        context->buildPath = "build/";
        PCB__CStrings_append(&context->sources, "src/");
        PCB__CStrings_append(&context->includes, "include/");
    }
    if(flags & PCB_BUILDOPTION_DEFAULT_COMPILER) {
#if PCB_COMPILER_GCC || PCB_COMPILER_CLANG
        PCB_BuildContext_flags(context).argvSyntax = PCB_ARGVSYNTAX_POSIX;
#elif PCB_COMPILER_MSVC
        PCB_BuildContext_flags(context).argvSyntax = PCB_ARGVSYNTAX_MS;
#endif //compilers
        PCB_BuildContext_flags(context).compilerUsed = PCB_COMPILER_RT_CURRENT;
        context->compilerPath = PCB_COMPILER_PATH;
#ifdef __cplusplus
        context->standard = __cplusplus;
#else
        context->standard = __STDC_VERSION__;
#endif //C++
#if !defined(__STRICT_ANSI__) && defined(__GNUC__)
        PCB_BuildContext_flags(context).gnu = true;
#endif //pesky but useful GNU extensions
    }
    if(flags & PCB_BUILDOPTION_DEFAULT_WARNINGS)
        PCB__BuildContext_addDefaultWarnings(context);
    if(flags & PCB_BUILDOPTION_OPTIMIZE) {
#if PCB_COMPILER_GCC || PCB_COMPILER_CLANG
        PCB__CStrings_append(&context->optimizationFlags, "-O2");
#elif PCB_COMPILER_MSVC
        PCB__CStrings_append(&context->optimizationFlags, "/O2");
#endif //compilers
        if(flags & PCB_BUILDOPTION_LOCAL_SYSTEM) {
#if PCB_COMPILER_GCC || PCB_COMPILER_CLANG
            PCB__CStrings_append(&context->optimizationFlags, "-march=native");
#elif PCB_COMPILER_MSVC
#if PCB_ARCH_x64
            do {
                int cpuinfo[4]; //EAX, EBX, ECX, EDX
                __cpuidex(cpuinfo, 0x7, 0x0);
                if(cpuinfo[1] & (1 << 5)) {
                    PCB__CStrings_append(&context->optimizationFlags, "/arch:AVX2");
                    break;
                }
                __cpuid(cpuinfo, 0x1);
                if(cpuinfo[2] & (1 << 28)) {
                    PCB__CStrings_append(&context->optimizationFlags, "/arch:AVX");
                    break;
                }
                if(cpuinfo[2] & (1 << 20)) {
                    PCB__CStrings_append(&context->optimizationFlags, "/arch:SSE4.2");
                    break;
                }
            } while(0);
#endif //architectures; TODO: architectures other than x64
#endif //compilers
        }
    }
    if(!(flags & PCB_BUILDOPTION_NODEBUG)) { //enable debug info by default
#if PCB_COMPILER_GCC || PCB_COMPILER_CLANG
        PCB__CStrings_append(&context->debugFlags, "-g");
#elif PCB_COMPILER_MSVC
        //TODO: think about how to integrate PDB related stuff here
        PCB__CStrings_append(&context.debugFlags, "/Zi");
#endif
        if(!(flags & PCB_BUILDOPTION_OPTIMIZE)) {
#if PCB_COMPILER_GCC && 0
            PCB__CStrings_append(&context->optimizationFlags, "-Og");
#endif //-Og is supposedly better for debugging on GCC than -O0
        }
    }
    if(flags & PCB_BUILDOPTION_TSAN) {
        if(flags & PCB_BUILDOPTION_ASAN) {
            PCB_log(
                PCB_LOGLEVEL_WARN,
                "Conflicting options: cannot use TSan with ASan"
                ", will use TSan."
            );
            flags &= (uint64_t)~PCB_BUILDOPTION_ASAN;
        }
        if(flags & PCB_BUILDOPTION_LSAN) {
            PCB_log(
                PCB_LOGLEVEL_WARN,
                "Conflicting options: cannot use TSan with LSan"
                ", will use TSan."
            );
            flags &= (uint64_t)~PCB_BUILDOPTION_LSAN;
        }
#if (PCB_COMPILER_GCC && !PCB_PLATFORM_WINDOWS) || PCB_COMPILER_CLANG
        PCB__CStrings_append(&context->debugFlags, "-fsanitize=thread");
#else
        PCB_log(
            PCB_LOGLEVEL_WARN,
            "TSan is not available and will not be linked."
        );
#endif
    }
    if(flags & PCB_BUILDOPTION_ASAN) {
#if (PCB_COMPILER_GCC && !PCB_PLATFORM_WINDOWS) || PCB_COMPILER_CLANG
        PCB__CStrings_append(&context->debugFlags, "-fsanitize=address");
        PCB__CStrings_append(&context->otherLinkerFlags, "-fsanitize=address");
#elif PCB_COMPILER_MSVC
        PCB__CStrings_append(&context->debugFlags, "/fsanitize=address");
        PCB__CStrings_append(&context->otherLinkerFlags, "/fsanitize=address");
#else
        PCB_log(
            PCB_LOGLEVEL_WARN,
            "ASan is not available and will not be linked."
        );
#endif
    }
    if(flags & PCB_BUILDOPTION_LSAN) {
#if (PCB_COMPILER_GCC && !PCB_PLATFORM_WINDOWS) || PCB_COMPILER_CLANG
        PCB__CStrings_append(&context->debugFlags, "-fsanitize=leak");
        PCB__CStrings_append(&context->otherLinkerFlags, "-fsanitize=leak");
#else
        PCB_log(
            PCB_LOGLEVEL_WARN,
            "LSan is not available and will not be linked."
        );
#endif
    }
    if(flags & PCB_BUILDOPTION_UBSAN) {
#if (PCB_COMPILER_GCC && !PCB_PLATFORM_WINDOWS) || PCB_COMPILER_CLANG
        PCB__CStrings_append(&context->debugFlags, "-fsanitize=undefined");
        PCB__CStrings_append(&context->otherLinkerFlags, "-fsanitize=undefined");
#else
        PCB_log(
            PCB_LOGLEVEL_WARN,
            "UBSan is not available and will not be linked."
        );
#endif
    }

    return 0;
}

PCB_BuildContext PCB_BuildContext_create(uint64_t flags) {
    PCB_BuildContext context = PCB_ZEROED;
    PCB_BuildContext_init(&context, flags);
    return context;
}

void PCB_BuildContext_reset(PCB_BuildContext* context) {
    PCB_CHECK_SELF(context, );
    context->compilerPath = NULL;
    context->buildPath = NULL;
    context->outputPath = NULL;
    PCB_Vec_reset(&context->sources);
    PCB_Vec_reset(&context->sourcesBlacklist);
    PCB_Vec_reset(&context->includes);
    PCB_Vec_reset(&context->libs);
    PCB_Vec_reset(&context->staticLibs);
    PCB_Vec_reset(&context->librarySearchPaths);
    PCB_Vec_reset(&context->warningFlags);
    PCB_Vec_reset(&context->debugFlags);
    PCB_Vec_reset(&context->optimizationFlags);
    PCB_Vec_reset(&context->preprocessorFlags.defines);
    PCB_Vec_reset(&context->preprocessorFlags.undefines);
    PCB_Vec_reset(&context->otherCompilerFlags);
    PCB_Vec_reset(&context->otherLinkerFlags);
    PCB_String_reset(&context->currentSourcePath);
    PCB_String_reset(&context->currentBuildPath);
    PCB_Vec_reset(&context->commandBuffer);
    PCB_Vec_reset(&context->processes);
    PCB_Vec_reset(&context->sourceFiles);
    PCB_Vec_reset(&context->objectFiles);
    context->standard = 0;
    context->flags.all = 0;
    memset(&context->btso, 0, sizeof(context->btso));

    PCB_Arena_reset(context->arena);
}

void PCB_BuildContext_destroy(PCB_BuildContext* context) {
    PCB_CHECK_SELF(context, );
    //copy pasta lets goooooo
    context->compilerPath = NULL;
    context->buildPath = NULL;
    context->outputPath = NULL;
    PCB_Vec_destroy(&context->sources);
    PCB_Vec_destroy(&context->sourcesBlacklist);
    PCB_Vec_destroy(&context->includes);
    PCB_Vec_destroy(&context->libs);
    PCB_Vec_destroy(&context->staticLibs);
    PCB_Vec_destroy(&context->librarySearchPaths);
    PCB_Vec_destroy(&context->warningFlags);
    PCB_Vec_destroy(&context->debugFlags);
    PCB_Vec_destroy(&context->optimizationFlags);
    PCB_Vec_destroy(&context->preprocessorFlags.defines);
    PCB_Vec_destroy(&context->preprocessorFlags.undefines);
    PCB_Vec_destroy(&context->otherCompilerFlags);
    PCB_Vec_destroy(&context->otherLinkerFlags);
    PCB_String_destroy(&context->currentSourcePath);
    PCB_String_destroy(&context->currentBuildPath);
    PCB_Vec_destroy(&context->commandBuffer);
    PCB_Vec_destroy(&context->processes);
    PCB_Vec_destroy(&context->sourceFiles);
    PCB_Vec_destroy(&context->objectFiles);
    context->standard = 0;
    context->flags.all = 0;
    memset(&context->btso, 0, sizeof(context->btso));

    PCB_Arena_destroy(context->arena);
    context->arena = NULL;
}

static int PCB__BuildContext_needsRebuild_single(
    PCB_BuildContext* context,
    const char* src,
    const char* out
) {
    uint64_t tB = 0, tS = 0;
    //skip the "whether the source is newer than the cached object file" step
    if(PCB_BuildContext_flags(context).alwaysBuild) return true;
    switch(PCB_FS_Exists(out)) {
      case true: break;
      case false: return true;
      default:
        PCB_logLatestError("Error checking if %s exists", out);
        return -1;
    }
    tB = PCB_FS_GetModificationTime(out);
    if(tB == 0) {
        PCB_logLatestError("Error getting info about file %s", out);
        return -2;
    }
    tS = PCB_FS_GetModificationTime(src);
    if(tS == 0) {
        PCB_logLatestError("Error getting info about file %s", src);
        return -2;
    }
    if(tB >= tS) return false;
    return true;
}

static int PCB__build_file(
    PCB_BuildContext* context,
    const char* src,
    const char* obj,
    bool addToObjs
) {
    PCB__logTrace("In: %s, out: %s", src, obj);
    int result = 0;
    PCB_Process process;
    //`needsRebuild` was already called on `src` and `obj`
    if(!PCB_BuildContext_flags(context).buildImmediately) goto compile;
    result = PCB__BuildContext_needsRebuild_single(context, src, obj);
    if(result < 0) return result;
    if(!result) goto afterCompile;
compile:
    result = 0;
    context->commandBuffer.data[context->commandBuffer.length - 2] = obj;
    context->commandBuffer.data[context->commandBuffer.length - 1] = src;

    PCB_BuildContext_flags(context).rebuiltAnything = true;

    PCB_log(PCB_LOGLEVEL_INFO, "Building %s...", src);
#if defined(PCB_DEBUG_SELF) && PCB_DEBUG_SELF+0
    {
        PCB_String cmd = PCB_ShellCommand_render(&context->commandBuffer);
        if(PCB_String_isEmpty(&cmd)) return -ENOMEM;
        else PCB_log(PCB_LOGLEVEL_DEBUG, "Running command \"%s\"", cmd.data);
        PCB_String_destroy(&cmd);
    }
#endif //PCB_DEBUG
    process = PCB_ShellCommand_runBg(&context->commandBuffer);
    if(!PCB_Process_isValid(&process)) return -3;
    switch(PCB_BuildContext_flags(context).parallel) {
#ifdef PCB_BUILD_0_AS_PARALLEL
      case 1: { //no parallel
#else
      case 0: { //no parallel
#endif //PCB_BUILD_0_AS_PARALLEL
        if(!PCB_Process_waitForExit(&process)) {
            PCB_logLatestError("Error waiting for child process");
            return -3;
        }
        int code = PCB_Process_getExitCode(&process);
        PCB_Process_destroy(&process);
        if(code != 0) {
            PCB_log(
                PCB_LOGLEVEL_ERROR,
                "Encountered an error during compilation, aborting."
            ); return code;
        } break;
      }
      default: {
        size_t parallel = PCB_BuildContext_flags(context).parallel;
        if(context->processes.length < parallel) {
            PCB_Vec_append(&context->processes, process);
            break;
        }
        const int index = PCB_Processes_waitForAny(&context->processes);
        if(index < 0) {
            PCB_log(PCB_LOGLEVEL_ERROR, "Failed to wait for a child process");
            return -4;
        }
        PCB_Process* p = &context->processes.data[index];
        int code = PCB_Process_getExitCode(p);
        if(code != 0) {
            PCB_log(
                PCB_LOGLEVEL_ERROR,
                "Encountered an error during compilation, aborting."
            ); return code;
        }
        PCB_Process_destroy(p); *p = process;
        break;
      }
    }
    afterCompile:
    if(addToObjs) {
        const char* objectFilepath = PCB_Arena_strdup(context->arena, obj);
        if(objectFilepath == NULL) return -ENOMEM;
        PCB__CStrings_append(&context->objectFiles, objectFilepath);
    }
    return 0;
}

static int PCB__build_directory(PCB_BuildContext* context) {
#if PCB_PLATFORM_WINDOWS
    (void)context;
    PCB_TODO("PCB__build_directory");
#elif PCB_PLATFORM_POSIX
    int result = 0;
    PCB_String *src = &context->currentSourcePath,
               *obj = &context->currentBuildPath;

    PCB__logTrace("Opening directory \"%s\"", src->data);
    PCB__logTrace("Current output directory is \"%s\"", obj->data);
    switch(PCB_FS_Exists(obj->data)) {
      case true: break;
      case false: {
        if(!PCB_mkdir(obj->data)) return -errno;
        break;
      }
      default:
        result = -errno;
        PCB_logLatestError("Cannot check whether %s exists", obj->data);
        return result;
    }
    DIR* cwd = opendir(src->data);
    if(cwd == NULL) {
        result = -errno;
        PCB_logLatestError("Could not open directory %s", src->data);
        return result;
    }
    const size_t srclen = src->length;
    const size_t objlen = obj->length;
    for(
        struct dirent* entry = readdir(cwd);
        entry != NULL;
        entry = readdir(cwd),
        src->data[src->length = srclen] = '\0',
        obj->data[obj->length = objlen] = '\0'
    ) {
        if(!PCB_strcmp(entry->d_name, ".") || !PCB_strcmp(entry->d_name, ".."))
            continue;
        if(!PCB_String_append_cstr(src, entry->d_name))
            PCB__return_defer(-ENOMEM);
        PCB_FileType type = PCB_FS_GetType(src->data);
        switch(type & PCB_FILETYPE_SYMLINK_IGN) {
          case PCB_FILETYPE_ERROR:
            switch(errno) {
              case ELOOP:
              case ENAMETOOLONG:
              case EOVERFLOW:
                PCB_log(
                    PCB_LOGLEVEL_WARN,
                    "Skipping %s: %s",
                    src->data, strerror(errno)
                ); continue;
              case ENOMEM:
                PCB__return_defer(-ENOMEM);
              default: PCB_Unreachable;
            } PCB_Unreachable;
          case PCB_FILETYPE_NONE:
            PCB_log(PCB_LOGLEVEL_WARN, "%s does not exist, skipping", src->data);
            continue;
          case PCB_FILETYPE_UNKNOWN:
            PCB_log(PCB_LOGLEVEL_WARN, "dunno what is %s, skipping", src->data);
            continue;
          case PCB_FILETYPE_DIR: {
            if(!PCB_String_append_cstr(obj, entry->d_name))
                PCB__return_defer(-ENOMEM);
            if(!PCB_String_append_chars(src, '/', 1))
                PCB__return_defer(-ENOMEM);
            if(!PCB_String_append_chars(obj, '/', 1))
                PCB__return_defer(-ENOMEM);
            //recursively build the subdirectory
            if((result = PCB__build_directory(context)) != 0) goto defer;
          } break;
          case PCB_FILETYPE_REG: {
            if(PCB_String_endsWith_cstr(src, ".c")) {
                if(!PCB_String_append_cstr(obj, entry->d_name))
                    PCB__return_defer(-ENOMEM);
                obj->data[obj->length - 1] = 'o';
#ifdef __cplusplus
                if(PCB_BuildContext_flags(context).ccInCpp)
                    context->commandBuffer.data[0] = PCB_COMPILER_PATH_ALT;
#endif //C++?
                if((result = PCB__build_file(context, src->data, obj->data, true)) != 0)
                    goto defer;
#ifdef __cplusplus
                if(PCB_BuildContext_flags(context).ccInCpp)
                    context->commandBuffer.data[0] = context->compilerPath;
#endif //C++?
            } else if(PCB_String_endsWith_cstr(src, ".cpp")) {
#ifndef __cplusplus
                if(PCB_BuildContext_flags(context).ccInCpp) {
                    context->commandBuffer.data[0] = PCB_COMPILER_PATH_ALT;
                } else {
                    PCB_log(
                        PCB_LOGLEVEL_WARN, "C++ source file %s in a C-only "
                        "build, skipping", src->data
                    ); continue;
                }
#endif //!C++?
                if(!PCB_String_append_cstr(obj, entry->d_name))
                    PCB__return_defer(-ENOMEM);
                PCB_String_pop_many(obj, 3, NULL);
                PCB_String_append_chars(obj, 'o', 1);
                if((result = PCB__build_file(context, src->data, obj->data, true)) != 0)
                    goto defer;
#ifndef __cplusplus
                if(PCB_BuildContext_flags(context).ccInCpp)
                    context->commandBuffer.data[0] = context->compilerPath;
#endif //!C++?
            }
          } break;
        }
    }
defer:
    src->data[src->length = srclen] = '\0';
    obj->data[obj->length = objlen] = '\0';
    closedir(cwd);
    PCB__logTrace("Closing directory \"%s\"", src->data);
    return result;
#endif //platform-dependent directory enumeration
}

static int PCB__BuildContext_gatherSources_dir(PCB_BuildContext* context) {
#if PCB_PLATFORM_WINDOWS
    (void)context;
    PCB_TODO("PCB__BuildContext_gatherSources_dir");
#elif PCB_PLATFORM_POSIX
    PCB_String *src = &context->currentSourcePath,
               *obj = &context->currentBuildPath;

    int result = 0;
    PCB__logTrace("Opening directory \"%s\"", src->data);
    PCB__logTrace("Current output directory is \"%s\"", obj->data);
    switch(PCB_FS_Exists(obj->data)) {
      case true: break;
      case false: {
        if(!PCB_mkdir(obj->data)) return -errno;
        break;
      }
      default:
        result = -errno;
        PCB_logLatestError("Cannot check whether %s exists", obj->data);
        return result;
    }
    DIR* cwd = opendir(src->data);
    if(cwd == NULL) {
        result = -errno;
        PCB_logLatestError("Could not open directory %s", src->data);
        return result;
    }
    const size_t srclen = src->length;
    const size_t objlen = obj->length;
    for(
        struct dirent* entry = readdir(cwd);
        entry != NULL;
        entry = readdir(cwd),
        src->data[src->length = srclen] = '\0',
        obj->data[obj->length = objlen] = '\0'
    ) {
        if(!PCB_strcmp(entry->d_name, ".") || !PCB_strcmp(entry->d_name, ".."))
            continue;
        if(!PCB_String_append_cstr(src, entry->d_name))
            PCB__return_defer(-ENOMEM);
        PCB_FileType type = PCB_FS_GetType(src->data);
        switch(type & PCB_FILETYPE_SYMLINK_IGN) {
          case PCB_FILETYPE_ERROR:
            switch(errno) {
              case ELOOP:
              case ENAMETOOLONG:
              case EOVERFLOW:
                PCB_log(
                    PCB_LOGLEVEL_WARN,
                    "Skipping %s: %s",
                    src->data, strerror(errno)
                ); continue;
              case ENOMEM:
                PCB__return_defer(-ENOMEM);
              default: PCB_Unreachable;
            } PCB_Unreachable;
          case PCB_FILETYPE_NONE:
            PCB_log(PCB_LOGLEVEL_WARN, "%s does not exist, skipping", src->data);
            continue;
          case PCB_FILETYPE_UNKNOWN:
            PCB_log(PCB_LOGLEVEL_WARN, "dunno what is %s, skipping", src->data);
            continue;
          case PCB_FILETYPE_DIR: {
            if(!PCB_String_append_cstr(obj, entry->d_name))
                PCB__return_defer(-ENOMEM);
            if(!PCB_String_append_chars(src, '/', 1))
                PCB__return_defer(-ENOMEM);
            if(!PCB_String_append_chars(obj, '/', 1))
                PCB__return_defer(-ENOMEM);
            if((result = PCB__BuildContext_gatherSources_dir(context)) != 0)
                goto defer;
          } break;
          case PCB_FILETYPE_REG: {
            if(PCB_String_endsWith_cstr(src, ".c")) {
                if(!PCB_String_append_cstr(obj, entry->d_name))
                    PCB__return_defer(-ENOMEM);
                obj->data[obj->length - 1] = 'o'; //.c -> .o
            } else if(PCB_String_endsWith_cstr(src, ".cpp")) {
#ifndef __cplusplus
                if(!PCB_BuildContext_flags(context).ccInCpp) {
                    PCB_log(
                        PCB_LOGLEVEL_WARN, "C++ source file %s in a C-only "
                        "build, skipping", src->data
                    ); break;
                }
#endif //!C++?
                if(!PCB_String_append_cstr(obj, entry->d_name))
                    PCB__return_defer(-ENOMEM);
                PCB_String_pop_many(obj, 3, NULL); //.cpp -> .
                obj->data[obj->length++] = 'o';    //. -> .o
            } else continue;
            PCB_CStrings *srcs = &context->sourceFiles,
                         *objs = &context->objectFiles;
            const char* obj_ = PCB_Arena_strdup(context->arena, obj->data);
            if(obj_ == NULL) PCB__return_defer(-ENOMEM);
            if(!PCB_BuildContext_flags(context).deferModChecks) {
                result = PCB__BuildContext_needsRebuild_single(
                    context, src->data, obj->data
                );
                if(result < 0) goto defer;
                if(!result) {
                    if(objs->length == 0)
                        PCB__CStrings_append(objs, obj_);
                    else
                        PCB_Vec_insert(objs, obj_, objs->length - srcs->length);
                    break;
                }
                result = 0;
            }
            const char* src_ = PCB_Arena_strdup(context->arena, src->data);
            if(src_ == NULL) PCB__return_defer(-ENOMEM);
            PCB__CStrings_append(srcs, src_);
            PCB__CStrings_append(objs, obj_);
          } break;
          default:
            break;
        }
    }
defer:
    src->data[src->length = srclen] = '\0';
    obj->data[obj->length = objlen] = '\0';
    closedir(cwd);
    PCB__logTrace("Closing directory \"%s\"", src->data);
    return result;
#endif //platform-dependent directory enumeration
}

static int PCB__BuildContext_isSingleFile(PCB_BuildContext* context) {
    PCB_CHECK_SELF(context, -1);
    if(context->sources.length == 1) {
        const char* src = context->sources.data[0];
        switch(PCB_FS_GetType(src) & PCB_FILETYPE_SYMLINK_IGN) {
          case PCB_FILETYPE_ERROR:
            PCB_logLatestError(
                "Failed to check whether the only source specified (%s) is "
                "a regular file", src
            ); return -1;
          case PCB_FILETYPE_REG: return true;
          default: return false;
        }
    }
    return false;
}

static void PCB__BuildContext_logutd(PCB_BuildContext* context) {
    if(PCB_BuildContext_flags(context).noutd) return;
    const char* out = context->outputPath;
    if(out == NULL) out = "Target";
    PCB_log(PCB_LOGLEVEL_INFO, "%s is up-to-date.", out);
}

static int PCB__BuildContext_logCommandTemplate(PCB_BuildContext* context) {
    PCB_String cmd = PCB_ShellCommand_render(&context->commandBuffer);
    if(PCB_String_isEmpty(&cmd)) return ENOMEM;
    PCB_log(
        PCB_LOGLEVEL_INFO,
        "Command template used: \"%s <output path> <source path>\"",
        cmd.data
    );
    PCB_String_destroy(&cmd);
    return 0;
}

static int PCB__BuildContext_logCommand(PCB_BuildContext* context) {
    PCB_String cmd = PCB_ShellCommand_render(&context->commandBuffer);
    if(PCB_String_isEmpty(&cmd)) return ENOMEM;
    PCB_log(PCB_LOGLEVEL_INFO, "Running command \"%s\"", cmd.data);
    PCB_String_destroy(&cmd);
    return 0;
}

static int PCB__BuildContext_parseCompilerFlags(PCB_BuildContext* context) {
    if(context->compilerPath == NULL) {
        PCB_log(
            PCB_LOGLEVEL_INFO,
            "No compiler path specified, will default to \""
            PCB_COMPILER_PATH "\""
        );
        context->compilerPath = PCB_COMPILER_PATH;
    }

    /*
     * The order of arguments passed to the compiler is as follows:
     * <compiler path> <standard> <warnings> <debug flags>
     * <optimization flags> <preprocessor flags> <includes>
     * <other flags> (whatever else...)
     */

    //1. Compiler path
    if(PCB_BuildContext_flags(context).compilerUsed == PCB_COMPILER_RT_MSVC) {
#if !PCB_PLATFORM_WINDOWS
        PCB_log(PCB_LOGLEVEL_ERROR, "MSVC is not available outside of Windows.");
        return -261;
#endif //compiling on Windows?
    }
    const int i = PCB_BuildContext_flags(context).argvSyntax;
    PCB_ShellCommand_append_arg(&context->commandBuffer, context->compilerPath);
    //1½: MSVC scheiße
    if(PCB_BuildContext_flags(context).compilerUsed == PCB_COMPILER_RT_MSVC) {
        PCB_ShellCommand_append_args(
            &context->commandBuffer, "/nologo", "/EHsc"
            //"/options:strict" TODO: only in VS2022 17.0
        );
        //EHsc is used for compatibility with other compilers as EHa is not portable
    }
    //2. The specified language standard
    if(!PCB_build_flag_standard(
        &context->commandBuffer,
        context->standard,
        context->arena,
        PCB_BuildContext_flags(context).compilerUsed,
#ifdef __cplusplus
        true,
#else
        false,
#endif
        PCB_BuildContext_flags(context).gnu
    )) return -ENOMEM;
    //3. Warning flags
    if(!PCB__BuildContext_parseFlags_single(
        context, PCB_View_Vec_A_T(&context->warningFlags, PCB_CStringsView),
        NULL, "Warning flag"
    )) return false;
    //4. Debug flags
    if(!PCB__BuildContext_parseFlags_single(
        context, PCB_View_Vec_A_T(&context->debugFlags, PCB_CStringsView),
        NULL, "Debug flag"
    )) return false;
    //5. Optimization flags
    if(!PCB__BuildContext_parseFlags_single(
        context, PCB_View_Vec_A_T(&context->optimizationFlags, PCB_CStringsView),
        NULL, "Optimization flag"
    )) return false;
    //6. Preprocessor flags
    PCB_static_assert(PCB_ARGVSYNTAX_COUNT == 3,);
    static const char* const FMTS_DEFINE[3][2] = {
        { NULL, "%s%s" }, { "-D%s", "-D%s=%s" }, { "/D%s", "/D%s=%s" }
    };
    if(!PCB__BuildContext_parseFlags_pairs(
        context, PCB_View_Vec_A_T(&context->preprocessorFlags.defines, PCB_CStringPairsView),
        FMTS_DEFINE[i], "Preprocessor define flag"
    )) return false;
    static const char* const FMTS_UNDEFINE[3] = { NULL, "-U%s", "/U%s" };
    if(!PCB__BuildContext_parseFlags_single(
        context, PCB_View_Vec_A_T(&context->preprocessorFlags.undefines, PCB_CStringsView),
        FMTS_UNDEFINE[i], "Preprocessor undefine flag"
    )) return false;
    //7. Include directories
    static const char* const FMTS_INCLUDE[3] = { NULL, "-I%s", "/I%s" };
    if(!PCB__BuildContext_parseFlags_single(
        context, PCB_View_Vec_A_T(&context->includes, PCB_CStringsView),
        FMTS_INCLUDE[i], "Include directory flag"
    )) return false;

    //8. Other flags
    if(PCB_BuildContext_flags(context).buildType == PCB_BUILDTYPE_DYNAMICLIB) {
        switch(PCB_BuildContext_flags(context).compilerUsed) {
          case PCB_COMPILER_RT_UNKNOWN: break;
          case PCB_COMPILER_RT_GCC:
          case PCB_COMPILER_RT_CLANG:
            PCB_ShellCommand_append_arg(&context->commandBuffer, "-fPIC");
            break;
          case PCB_COMPILER_RT_MSVC: break; //DLLs don't use PIC
        }
    }
    if(!PCB__BuildContext_parseFlags_single(
        context, PCB_View_Vec_A_T(&context->otherCompilerFlags, PCB_CStringsView),
        NULL, "Other compiler flag"
    )) return false;
    return 0;
}

static int PCB__BuildContext_parseArchiverFlags(PCB_BuildContext* context) {
    const PCB_Compiler_RT c = PCB_BuildContext_flags(context).compilerUsed;
    switch(c) {
      case PCB_COMPILER_RT_GCC:
      case PCB_COMPILER_RT_CLANG:
        PCB_ShellCommand_append_args(&context->commandBuffer, "ar", "rcs");
        break;
      case PCB_COMPILER_RT_MSVC:
        PCB_ShellCommand_append_args(&context->commandBuffer, "lib", "/nologo");
        break;
      case PCB_COMPILER_RT_UNKNOWN: break;
    }
    if(c == PCB_COMPILER_RT_MSVC) {
        char* out = PCB_Arena_asprintf(context->arena, "/out:%s", context->outputPath);
        if(out == NULL) return ENOMEM;
        PCB_ShellCommand_append_arg(&context->commandBuffer, out);
    } else {
        PCB_ShellCommand_append_arg(&context->commandBuffer, context->outputPath);
    }
    PCB_ShellCommand_append_n_args(
        &context->commandBuffer,
        context->objectFiles.data, context->objectFiles.length
    );
    return 0;
}

static int PCB__BuildContext_parseLinkerFlags(PCB_BuildContext* context) {
    const PCB_Compiler_RT c = PCB_BuildContext_flags(context).compilerUsed;
    int isSingleFile = PCB__BuildContext_isSingleFile(context);
    if(isSingleFile < 0) return isSingleFile;
    //1. Dependent on whether we are building just 1 file or more. If not,
    //the executable to run would be added here prior to calling this function.

    //Linking 1 file in MSVC doesn't require separately invoking the linker,
    //but requires specifying that subsequent flags are linker flags.
    if(isSingleFile && c == PCB_COMPILER_RT_MSVC)
        PCB_ShellCommand_append_arg(&context->commandBuffer, "/link");
    if(PCB_BuildContext_flags(context).buildType == PCB_BUILDTYPE_DYNAMICLIB) {
        switch(c) {
          case PCB_COMPILER_RT_UNKNOWN: break;
          case PCB_COMPILER_RT_GCC: //fallthrough
          case PCB_COMPILER_RT_CLANG:
            //so as not to add the same flag twice,
            //unnecessary but otherwise looks weird
            if(!isSingleFile) PCB_ShellCommand_append_arg(&context->commandBuffer, "-fPIC");
            PCB_ShellCommand_append_arg(&context->commandBuffer, "-shared");
            break;
          case PCB_COMPILER_RT_MSVC:
            PCB_ShellCommand_append_arg(&context->commandBuffer, "/DLL");
            break;
        }
    }
    const int i = PCB_BuildContext_flags(context).argvSyntax;
     //2. Object files, if any
    PCB_ShellCommand_append_n_args(
        &context->commandBuffer,
        context->objectFiles.data, context->objectFiles.length
    );
    //3. Library search paths
    static const char* const FMTS_LIBSEARCHPATHS[3] = { NULL, "-L%s", "/LIBPATH:\"%s\"" };
    if(!PCB__BuildContext_parseFlags_single(
        context, PCB_View_Vec_A_T(&context->librarySearchPaths, PCB_CStringsView),
        FMTS_LIBSEARCHPATHS[i], "Library search path"
    )) return -ENOMEM;

    static const char* const FMTS_LIBS[3] = { NULL, "-l%s", "%s.lib" };
    //4. Static libraries
    if(context->staticLibs.length > 0) {
        switch(i) {
          case PCB_ARGVSYNTAX_UNKNOWN: break;
          case PCB_ARGVSYNTAX_POSIX:
            PCB_ShellCommand_append_arg(&context->commandBuffer, "-Wl,-Bstatic");
            break;
          case PCB_ARGVSYNTAX_MS: break;
          case PCB_ARGVSYNTAX_COUNT:
            PCB_Unreachable;
        }
        if(!PCB__BuildContext_parseFlags_single(
            context, PCB_View_Vec_A_T(&context->staticLibs, PCB_CStringsView),
            FMTS_LIBS[i], "Static library"
        )) return -ENOMEM;
    }
    //5. Dynamic libraries
    if(context->libs.length > 0) {
        switch(i) {
          case PCB_ARGVSYNTAX_UNKNOWN: break;
          case PCB_ARGVSYNTAX_POSIX:
            if(context->staticLibs.length > 0) //override the above
                PCB_ShellCommand_append_arg(&context->commandBuffer, "-Wl,-Bdynamic");
            break;
          case PCB_ARGVSYNTAX_MS: break;
          case PCB_ARGVSYNTAX_COUNT:
            PCB_Unreachable;
        }
        if(!PCB__BuildContext_parseFlags_single(
            context, PCB_View_Vec_A_T(&context->libs, PCB_CStringsView),
            FMTS_LIBS[i], "Library"
        )) return -ENOMEM;
    }
    //6. Other flags
    if(!PCB__BuildContext_parseFlags_single(
        context, PCB_View_Vec_A_T(&context->otherLinkerFlags, PCB_CStringsView),
        NULL, "Other linker flag"
    )) return -ENOMEM;
    //7.Flag to specify "output name", but only if output path is specified
    if(context->outputPath != NULL) {
        switch(PCB_BuildContext_flags(context).argvSyntax) {
          case PCB_ARGVSYNTAX_POSIX:
            PCB_ShellCommand_append_args(
                &context->commandBuffer,
                "-o", context->outputPath
            ); break;
          case PCB_ARGVSYNTAX_MS: { //TODO: above
            const char* outpath = PCB_Arena_asprintf(
                context->arena, "/out:%s", context->outputPath
            ); if(outpath == NULL) return -ENOMEM;
            PCB_ShellCommand_append_arg(&context->commandBuffer, outpath);
        break;
          }
          case PCB_ARGVSYNTAX_UNKNOWN:
            PCB_log(
                PCB_LOGLEVEL_WARN,
                "Cannot add a flag to specify the output path, skipping"
            ); break;
          case PCB_ARGVSYNTAX_COUNT:
            PCB_Unreachable;
        }
    }
    return 0;
}

static int PCB__build_fromContext_single(PCB_BuildContext* context) {
    PCB__logTrace("Building a single file...");
    //parallel building doesn't make sense for a single file
#ifdef PCB_BUILD_0_AS_PARALLEL
    PCB_BuildContext_flags(context).parallel = 1;
#else
    PCB_BuildContext_flags(context).parallel = 0;
#endif

    if(context->outputPath == NULL) {
        PCB_log(
            PCB_LOGLEVEL_ERROR,
            "Builds with 1 source file require specifying the output name."
        ); return -260;
    }
    //may be for whatever reason set to `true` prior to here
    PCB_BuildContext_flags(context).rebuiltAnything = false;
    if(!PCB_BuildType_formatName(
        PCB_BuildContext_flags(context).buildType,
        &context->outputPath,
        context->arena
    )) return -ENOMEM;

    const char* src = context->sources.data[0];
    int code = 0;
    switch(code = PCB__BuildContext_needsRebuild_single(context, src, context->outputPath)) {
      case false:
        PCB__BuildContext_logutd(context);
        return 0;
      case true: break;
      default: //error
        return code;
    }

    if((code = PCB__BuildContext_parseCompilerFlags(context)) != 0) return code;
    if(PCB_BuildContext_flags(context).buildType == PCB_BUILDTYPE_STATICLIB) {
        PCB_build_flag_cwl(&context->commandBuffer, PCB_BuildContext_flags(context).argvSyntax);
        PCB_build_flag_output(&context->commandBuffer, PCB_BuildContext_flags(context).argvSyntax);
        //TODO: Create a temporary file instead...maybe
        PCB_StringView base = PCB_FS_Basename(PCB_StringView_from_cstr(src));
        PCB_StringView dot  = PCB_StringView_rsubcstr(base, ".");
        if(!PCB_String_isEmpty(&dot)) base.length = (size_t)(dot.data - base.data);
        char* obj = PCB_Arena_asprintf(
            context->arena,
            "%s%c" PCB_SV_Fmt ".o",
            context->buildPath, PCB_FS_DIR_DELIM, PCB_SV_Arg(base)
        );
        if(obj == NULL) return ENOMEM;
        PCB_ShellCommand_append_args(&context->commandBuffer, obj, src);
        if((code = PCB__BuildContext_logCommand(context)) != 0) return code;
        if((code = (int)PCB_ShellCommand_runAndWait(&context->commandBuffer)) != 0) return code;

        PCB_Vec_reset(&context->commandBuffer);
        PCB__CStrings_append(&context->objectFiles, obj);
        if((code = PCB__BuildContext_parseArchiverFlags(context)) != 0) return code;
    } else {
        PCB_ShellCommand_append_arg(&context->commandBuffer, src);
        if((code = PCB__BuildContext_parseLinkerFlags(context)) != 0) return code;
    }
    if((code = PCB__BuildContext_logCommand(context)) != 0) return code;
    if((code = (int)PCB_ShellCommand_runAndWait(&context->commandBuffer)) != 0) return code;
    PCB_BuildContext_flags(context).rebuiltAnything = true;
    return 0;
}

int PCB_build_fromContext(PCB_BuildContext* context) {
    PCB_CHECK_SELF(context, -256);
    if(context->sources.length == 0)
        return PCB_log(PCB_LOGLEVEL_ERROR, "No source specified"), -257;
    int isSingleFile = PCB__BuildContext_isSingleFile(context);
    if(isSingleFile < 0) return -258;
    else if(isSingleFile) return PCB__build_fromContext_single(context);
    if(context->buildPath == NULL)
        return PCB_log(PCB_LOGLEVEL_ERROR, "No build path specified"), -259;
    if(PCB_BuildContext_flags(context).deferModChecks)
        return PCB_log(
            PCB_LOGLEVEL_ERROR,
            "Deferring modification checks not implemented"
        ), -259;

    //may be for whatever reason set to `true` prior to here
    PCB_BuildContext_flags(context).rebuiltAnything = false;
    if(!PCB_BuildType_formatName(
        PCB_BuildContext_flags(context).buildType,
        &context->outputPath,
        context->arena
    )) return ENOMEM;
    int result = 0;
    //This bizzare looking code is for saving `PCB_BuildContext_flags(context).parallel`
    //and replacing it with the number of cores if it's 1 to be able to refer to
    //it in other functions without an additional variable.
    //Yes, it's stupid.
    uint8_t parallel = PCB_BuildContext_flags(context).parallel;
#ifdef PCB_BUILD_0_AS_PARALLEL
#define PCB__PARALLELIZE_VAL 0
#else
#define PCB__PARALLELIZE_VAL 1
#endif
    if(PCB_BuildContext_flags(context).parallel == PCB__PARALLELIZE_VAL) {
        //TODO: there are CPUs with more than 255 cores
        size_t cores = PCB_getNumberOfCores();
        if(cores > UINT8_MAX) cores = UINT8_MAX;
        PCB_BuildContext_flags(context).parallel = (uint8_t)cores;
        parallel = PCB__PARALLELIZE_VAL;
    }
#undef PCB__PARALLELIZE_VAL
    PCB_Vec_reserve(&context->processes, PCB_BuildContext_flags(context).parallel);

    if(!PCB_mkdir(context->buildPath)) PCB__return_defer(errno);
    switch(PCB_BuildContext_flags(context).argvSyntax) {
      case PCB_ARGVSYNTAX_POSIX:
      case PCB_ARGVSYNTAX_MS: //supported
        break;
      case PCB_ARGVSYNTAX_UNKNOWN:
        PCB_log(
            PCB_LOGLEVEL_WARN,
            "Building with unknown argument syntax, "
            "flags will be passed directly without preprocessing "
            "and certain features won't work. "
            "You are on your own."
        ); break;
      case PCB_ARGVSYNTAX_COUNT:
        PCB_Unreachable;
    }

    //Setting up the compile command
    //1-8. Here.
    PCB__BuildContext_parseCompilerFlags(context);
    //9. Flag to specify "compile only"
    PCB_build_flag_cwl(&context->commandBuffer, PCB_BuildContext_flags(context).argvSyntax);
    //10. Flag to specify "output name"
    PCB_build_flag_output(&context->commandBuffer, PCB_BuildContext_flags(context).argvSyntax);
    /*
     * NOTE: the only thing that changes
     * between building files is "source path" and "output path",
     * i.e. args[args.length - 1] and args[args.length - 2],
     * so changing the command is rather easy.
     */
    //11.Source path, will be replaced when building a file
    PCB_ShellCommand_append_arg(&context->commandBuffer, NULL);
    //12. Output path, will be replaced when building a file
    PCB_ShellCommand_append_arg(&context->commandBuffer, NULL);

    //TODO: This needs to be moved to another function,
    //like `PCB__BuildContext_iterateSources`.
    if(!PCB_String_append_cstr(
        &context->currentBuildPath, context->buildPath
    )) PCB__return_defer(ENOMEM);
    if(!PCB_String_setSuffix_char(
        &context->currentBuildPath, '/'
    )) PCB__return_defer(ENOMEM);
    if(context->sources.length == 1) {
        if(!PCB_String_append_cstr(
            &context->currentSourcePath, context->sources.data[0]
        )) PCB__return_defer(ENOMEM);
        if(!PCB_String_setSuffix_char(
            &context->currentSourcePath, '/'
        )) PCB__return_defer(ENOMEM);

        if(PCB_BuildContext_flags(context).buildImmediately) {
            //NOTE: Always logged since we don't know whether anything needs
            //to be rebuilt. Leaving the comment below as future reference.
            //DONE (was a todo): this should only be logged if something is not up-to-date.
            //Current implementation relies on `rebuiltAnything` flag, but this is
            //a posteriori. We need to know that a priori by gathering sources and
            //removing those that don't need to be rebuilt.
            if((result = PCB__BuildContext_logCommandTemplate(context)) != 0)
                goto defer;
            if((result = PCB__build_directory(context)) != 0)
                goto defer;

        } else {
            if(context->sourceFiles.length > 0) {
                PCB_log(
                    PCB_LOGLEVEL_ERROR,
                    "sourceFiles field must be empty prior to calling %s."
                    "You've either forgot to reset or have added your"
                    "own source files here. The latter is not allowed - use"
                    "sources instead.",
                    __func__
                );
                PCB__return_defer(-263);
            }
            if((result = PCB__BuildContext_gatherSources_dir(context)) != 0)
                goto defer;
            if(PCB_BuildContext_flags(context).deferModChecks)
                PCB_Unreachable; //not implemented
            if(context->sourceFiles.length == 0) {
                PCB__BuildContext_logutd(context);
                return 0;
            }
            const size_t L = context->sourceFiles.length;
            if(L > 0 && (result = PCB__BuildContext_logCommandTemplate(context)) != 0)
                goto defer;
            //`objectFiles` may contain additional user-provided files, skip them
            const size_t offset = context->objectFiles.length - L;
            for(size_t i = 0; i < L; i++) {
                const char* src = context->sourceFiles.data[i];
                const char* obj = context->objectFiles.data[i + offset];
                if((result = PCB__build_file(context, src, obj, false)) != 0)
                    goto defer;
            }
        }
    } else {
        PCB_TODO("PCB_build_fromContext/multiple sources");
    }
    PCB_Vec_forEach_it(&context->processes, it, PCB_Process) {
        if(!PCB_Process_isValid(it)) continue;
        if(!PCB_Process_waitForExit(it)) {
            PCB_logLatestError("Waiting for child process to exit failed");
            result = -260;
            continue;
        }
        if(result != 0) continue;
        result = PCB_Process_getExitCode(it);
    }

    if(result != 0) {
        PCB_log(
            PCB_LOGLEVEL_ERROR,
            "Encountered an error during compilation, aborting."
        ); return result;
    }
    if(!PCB_BuildContext_flags(context).rebuiltAnything) {
        PCB__BuildContext_logutd(context);
        return 0;
    }

    if(context->objectFiles.length == 0) {
        if(PCB_BuildContext_flags(context).buildType == PCB_BUILDTYPE_STATICLIB) {
            PCB_log(PCB_LOGLEVEL_ERROR, "No object files to archive, aborting.");
        } else {
            PCB_log(PCB_LOGLEVEL_ERROR, "No object files to link, aborting.");
        }
        return -261;
    }

    PCB_BuildContext_flags(context).parallel = parallel;
    //Setting up the link command
    PCB_Vec_reset(&context->commandBuffer);
    //PCB_Arena_reset(context->arena);

    if(PCB_BuildContext_flags(context).buildType == PCB_BUILDTYPE_STATICLIB) {
        if((result = PCB__BuildContext_parseArchiverFlags(context)) != 0) goto defer;
        PCB_log(PCB_LOGLEVEL_INFO, "Attempting to archive...");
    } else {
        //1. Compiler path
        PCB_ShellCommand_append_arg(&context->commandBuffer, context->compilerPath);
        //2-6. Here.
        if((result = PCB__BuildContext_parseLinkerFlags(context)) != 0) goto defer;
        PCB_log(PCB_LOGLEVEL_INFO, "Attempting to link...");
    }
    if((result = PCB__BuildContext_logCommand(context)) != 0) goto defer;
    result = (int)PCB_ShellCommand_runAndWait(&context->commandBuffer);
    if(result != 0) {
        if(PCB_BuildContext_flags(context).buildType == PCB_BUILDTYPE_STATICLIB)
            PCB_log(PCB_LOGLEVEL_ERROR, "Encountered an error during archival, aborting.");
        else
            PCB_log(PCB_LOGLEVEL_ERROR, "Encountered an error during linkage, aborting.");
        PCB__return_defer(-262);
    }
    if(PCB_BuildContext_flags(context).buildType == PCB_BUILDTYPE_STATICLIB)
        PCB_log(PCB_LOGLEVEL_INFO, "Archived successfully.");
    else
        PCB_log(PCB_LOGLEVEL_INFO, "Linked successfully.");

    PCB_fflush(PCB_stdout);
    PCB_fflush(PCB_stderr);

defer:
    return result;
}

int PCB_BuildContext_gatherSources(PCB_BuildContext* context) {
    (void)context;
    PCB_log(PCB_LOGLEVEL_ERROR, PCB_LOC":%s: Not implemented", __func__);
    return -1;
#if PCB_PLATFORM_WINDOWS
#elif PCB_PLATFORM_POSIX

#endif
}

PCB_Compiler_RT PCB_getCompiler(const char* path) {
    PCB_CHECK_NULL(path, PCB_COMPILER_RT_UNKNOWN);
    /* Note: `path` can be arbitrary, i.e. for example, "gcc" may,
     * in a highly unusual situation be an alias for "clang". Therefore we
     * cannot rely on string comparison as a shortcut.
     */

    PCB_Compiler_RT compiler = PCB_COMPILER_RT_UNKNOWN;
    PCB_ShellCommand cmd = PCB_ZEROED;
    FILE* f = NULL;
    int code = 0;
    ssize_t retVal;
    PCB__logTrace("Checking if %s exists", path);
    const char* testSourceCode =
    "#if defined(__GNUC__) && !defined(__llvm__) && !defined(__INTEL_COMPILER)\n"
    "#define VAL 1\n"
    "#elif defined(__clang__)\n"
    "#define VAL 2\n"
    "#elif defined(_MSC_VER) && !defined(__clang__)\n"
    "#define VAL 3\n"
    "#else\n"
    "#define VAL 0\n"
    "#endif\n"
    "int main(void) { return VAL; }\n";
    const char* testPath = "./_test_.c";
#if PCB_PLATFORM_WINDOWS
    const char* testOutPath = "./_test_.exe";
#elif PCB_PLATFORM_POSIX
    const char* testOutPath = "./a.out";
#endif //platform
    PCB__logTrace("Checking if %s exists", testPath);
    switch(PCB_FS_Exists(testPath)) {
      case true: break;
      case false:
        PCB_log(PCB_LOGLEVEL_ERROR, "Aborting the compiler test: %s exists", testPath);
        goto defer;
      default:
        PCB_logLatestError("Failed to check whether %s exists", testPath);
        goto defer;
    }
    f = fopen(testPath, "w");
    if(f == NULL) {
        PCB_logLatestError("Failed to open _test_.c");
        goto defer;
    }
    fwrite(testSourceCode, 1, PCB_strlen(testSourceCode), f);
    PCB_fflush(f);
    PCB__CStrings_append(&cmd, path); PCB__CStrings_append(&cmd, testPath);
    if(PCB_ShellCommand_runAndWait(&cmd) != 0) goto defer;
    cmd.length = 0;
    code = PCB_FS_Exists(testOutPath);
    if(code != true) {
        testOutPath = "./a.exe";
        code = PCB_FS_Exists(testOutPath);
        if(code != true) goto defer;
#if !PCB_PLATFORM_WINDOWS
        PCB__CStrings_append(&cmd, "wine");
#endif
    }
    PCB__CStrings_append(&cmd, testOutPath);
    retVal = PCB_ShellCommand_runAndWait(&cmd);
    if(retVal < 0) goto defer;
    code = (int)retVal;
    switch(code) {
      case 0: break;
      case 1: compiler = PCB_COMPILER_RT_GCC;   break;
      case 2: compiler = PCB_COMPILER_RT_CLANG; break;
      case 3: compiler = PCB_COMPILER_RT_MSVC;  break;
    }

    defer:
        if(f != NULL) fclose(f);
        remove(testPath);
        remove(testOutPath);
        PCB_Vec_free(&cmd);
        return compiler;
}

void PCB_rebuild_shit(int argc, char** argv, const char* src) {
    if(argc == 0) {
        PCB_log(PCB_LOGLEVEL_ERROR, "%s/Invalid argc (%d)", __func__, argc);
        return;
    }
    PCB_CHECK_NULL(argv,);
    PCB_CHECK_NULL(src,);
    PCB__logDebug("Rebuilding myself (maybe)...");
    PCB_BuildContext context = PCB_ZEROED;
    context.outputPath = argv[0];
    if(PCB_BuildContext_init(
        &context, PCB_BUILDOPTION_DEFAULT_COMPILER | PCB_BUILDOPTION_DEFAULT_WARNINGS
    ) != 0) {
        PCB_log(
            PCB_LOGLEVEL_FATAL,
            "Failed to initialize a build context for rebuilding itself."
        ); exit(1);
    }
    PCB_BuildContext_flags(&context).noutd = true;
#if defined(__MINGW32__)
    HMODULE ntdllMod = GetModuleHandleA("ntdll.dll");
    if (ntdllMod != NULL && GetProcAddress(ntdllMod, "wine_get_version") != NULL) {
#ifdef __cplusplus
        context.compilerPath = "/usr/bin/x86_64-w64-mingw32-g++";
#else
        context.compilerPath = "/usr/bin/x86_64-w64-mingw32-gcc";
#endif //C++
    }
#endif //a hackish way to detect MinGW outside of Windows
    PCB__CStrings_append(&context.sources, src);
//these `kvTmp*` variables have to exist instead of
//`(PCB_CStringPair){ .key = "...", .value = "..." } for C++ compatibility
#if defined(PCB_DEBUG_SELF) && PCB_DEBUG_SELF+0 != 0 //checking for 0 if one wants to override the CLI argument
    PCB_CStringPair kvTmp1 = { "PCB_DEBUG_SELF", NULL };
    PCB_Vec_append(&context.preprocessorFlags.defines, kvTmp1);
#endif //subsequent rebuilds will retain the request for debugging the library
#ifdef PCB_DEBUG
    PCB_CStringPair kvTmp2;
    kvTmp2.key = "PCB_DEBUG";
#if PCB_DEBUG+0 > 1
    kvTmp2.value = PCB_STRINGIFY(PCB_DEBUG);
#else
    kvTmp2.value = NULL;
#endif //PCB_DEBUG > 1?
    PCB_Vec_append(&context.preprocessorFlags.defines, kvTmp2);
#endif //subsequent rebuilds will retain the request for debug logging
    PCB_CStrings_append_many(&context.includes, ".", "include/");
#ifdef PCB_LIB_PATH
    PCB__CStrings_append(&context.includes, PCB_LIB_PATH "");
#endif //PCB_LIB_PATH
    if(PCB_build_fromContext(&context) != 0) {
        PCB_log(PCB_LOGLEVEL_FATAL, "Failed to rebuild itself.");
        exit(2);
    }
    if(!PCB_BuildContext_flags(&context).rebuiltAnything) {
        PCB__logTrace("No need to rebuild myself.");
        PCB_BuildContext_destroy(&context);
        return;
    }
    PCB__logDebug("Rebuilt myself successfully.");
    PCB_Arena_reset(context.arena);
    PCB_StringView executable = PCB_StringView_from_cstr(argv[0]);
#if PCB_PLATFORM_WINDOWS
    if(!PCB_String_endsWith_cstr((PCB_String*)&executable, ".exe")) {
        executable.data = PCB_Arena_asprintf(context.arena, "%s.exe", argv[0]);
    }
#endif //Windows executables may end with ".exe"
    PCB_ShellCommand cmd = PCB_ZEROED;
    PCB_ShellCommand_append_arg(&cmd, executable.data);
    PCB_Vec_append_multiple(&cmd, argv + 1, (size_t)(argc - 1));
    int code = PCB_ShellCommand_runAndWait(&cmd);
    if(code != 0) exit(code > 0 ? code : 1);
    exit(0);
}
#endif //PCB_IMPLEMENTATION_BUILD

#ifdef __cplusplus
}
#endif //C++

//Remove all locally defined, potentially conflicting macros

#ifdef PCB_BOOL_LOCALLY_DEFINED
#undef bool
#undef true
#undef false
#endif //PCB_BOOL_LOCALLY_DEFINED

//Appendix 1: Changelog
/**
 * Version 0.2.0:
 * - Removed PCB_* global variables and PCB_build function,
 * - Renamed PCB_build_* to PCB__build_*, PCB__ will mark functions not intended
 *   for public use
 * Version 0.1.13:
 * - Added PCBAPI macro for future use in dynamic linking of PCB, PCBCALL macro
 *   for declaring the calling convention, PCB_BUILD_DYN for setting appropiate
 *   things for symbol export,
 * - Added PCB_Printf_Format macro for printf-like functions (only works with GCC/Clang),
 * - Added PCB_Cleanup macro for use in C as an explicit C++ destructor; only
 *   available if PCB_WANT_CLEANUP is declared since it's not supported everywhere and
 *   its use fundamentally changes the code,
 * - Renamed PCB_CStringVec to PCB_CStrings,
 * - Removed PCB_ShellCommand_run_and_wait_old function,
 * - Added PCB_FileType enum & PCB_FS_GetType function,
 * - Added PCB_FS_GetModificationTime function,
 * - Added PCB_FS_Exists function,
 * Version 0.1.12:
 * - Removed PCB_roundUpToPowerOf2_32/64 macros, added PCB_TODO, PCB_ARRAY_LEN,
 *   PCB_SHIFT macros
 * Version 0.1.11:
 * - Added explicit notice that C89 is not supported.
 * Version 0.1.10:
 * - Added platform-agnostic way of error handling.
 * Version 0.1.9:
 * - Added `PCB_logDebug` and `PCB_logTrace` for debugging the library.
 * Version 0.1.8:
 * - Added `PCB_LOGLEVEL_*_NL` so that "\n" may be omitted in `PCB_log`,
 *   non-error log levels are directed to stdout instead of stderr.
 * Version 0.1.7:
 * - Fixed detection of MinGW, Clang, MSVC and C23 in `PCB_NoReturn`,
 *   added `PCB_NoReturn` for Clang.
 * Version 0.1.6:
 * - Added fallbacks for `bool`, `true`, `false`, `strcmp`, `strncmp` and `strlen`.
 * Version 0.1.5:
 * - Made `PCB_Vec_reserve` realloc-safe, i.e. it won't crash on insufficient memory.
 *   This is a double-edged sword, however; consider overriding it with feature macros.
 * Version 0.1.4:
 * - Changed `PCB_VERSION` to accomodate 1000 minor & patch versions instead of 10.
 * Version 0.1.3:
 * - Changed course on how to approach builds:
 *   Instead of a set of global variables and macros around them to add compiler flags,
 *   PCB will build from a "build context", which allows for multiple build types
 *   and more configurability.
 * Version 0.1.2:
 * - Fixed PCB_VERSION_* defines to set them to the actual version
 * - Fixed a missing #endif at "#ifndef PCB_PLATFORM_POSIX"
 * - Fixed the lack of NULL-termination in PCB_ShellCommand_runBg/POSIX
 * - Removed old PCB_Vec_append
 * - Added comments for some #endif's, added whitespace in some places for readability
 * Version 0.1.1:
 * - Added PCB_HAS_STRING_H macro for libc's string.h header detection
 * - Added PCB_memcpy, PCB_memmove, PCB_memset, PCB_memcmp macros
 *   which map to libc if available, otherwise they map to
 *   equivalent functions with the same name implemented within
 *   PCB itself; memcpy, memmove, memset, memcmp are therefore
 *   substituted with PCB_ versions
 * Version 0.1.0:
 * - Moved platform identification to section 1.1
 * - Added support for some Apple platforms
 * - Added identification of POSIX-compliant platforms
 * - Moved compiler identification to section 1.2
 * - Moved compiler-specific macros to section 1.3
 * - Changed "#error" statement start with "PCB Error"
 * - Added strings.h header for case-insensitive C-strings
 * - Changed Linux-specific implementations to POSIX-specific
 * - Added PCB_String_setSuffix_char function
 * - Added a missing #endif at "#ifdef PCB_BUILD_CAPABILITY"
 * - Reinforced PCB_build_directory for errors, now returns a status code
 * Version 0.0.1: Initial version
 */

#endif //PCB_H
