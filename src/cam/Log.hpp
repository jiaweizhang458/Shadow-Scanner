// Software developed for the Spring 2016 Brown University course
// ENGN 2502 3D Photography
// Copyright (c) 2016, Daniel Moreno and Gabriel Taubin
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Brown University nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL DANIEL MORENO NOR GABRIEL TAUBIN BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef __LOG_HPP__
#define __LOG_HPP__

#include <string>
#include <stdarg.h>
#include <stdio.h>

#ifdef _MSC_VER 
#   define __FUNC__ __FUNCTION__
#else
#   define __FUNC__ __PRETTY_FUNCTION__
#endif

#define ADD_LOG \
static std::string logName(void)                            \
{                                                           \
    auto pretty = __FUNC__;                                 \
    auto p = strstr(pretty, "::logName");                   \
    if (p) { return std::string(pretty, p-pretty); }        \
    return std::string(pretty);                             \
}                                                           \
static int log(const char * fmt,...)                        \
{                                                           \
    int rv = 0;                                             \
    if (!silent) {                                          \
        FILE * stream = stderr;                             \
        rv += fprintf(stream, "[%s] ", logName().c_str());  \
        va_list args;                                       \
        va_start (args, fmt);                               \
        rv += vfprintf (stderr, fmt, args);                 \
        va_end (args);                                      \
    }                                                       \
    return rv;                                              \
}

#endif  // __LOG_HPP__