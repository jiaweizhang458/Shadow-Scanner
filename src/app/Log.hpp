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

#ifndef _Log_hpp_
#define _Log_hpp_

#include <string>
#include <cstring>
#include <stdarg.h>
#include <stdio.h>

#ifdef _MSC_VER 
# define __FUNC__ __FUNCTION__
#else
# define __FUNC__ __PRETTY_FUNCTION__
#endif

#define LOG_CURRNAME \
static std::string logName(void)                            \
{                                                           \
    auto pretty = __FUNC__;                                 \
    const char * p1 = strrchr(pretty, ' ');                 \
    const char * p2 = strstr(pretty, "::logName");          \
    if (!p1 && p2) {return std::string(pretty, p2-pretty);} \
    if ( p2) {return std::string(1+p1, p2-p1-1);}           \
    return std::string(pretty);                             \
}

#define LOG_SETNAME(name) \
  static std::string logName(void) { return std::string(#name); }

#define LOG_PRINTF \
static int log(const char * fmt,...)                        \
{                                                           \
    if (!LOG_ENABLED) { return 0; }                         \
    int rv = 0;                                             \
    FILE * stream = stderr;                                 \
    rv += fprintf(stream, "[%s] ", logName().c_str());      \
    va_list args;                                           \
    va_start (args, fmt);                                   \
    rv += vfprintf (stderr, fmt, args);                     \
    va_end (args);                                          \
    return rv;                                              \
}    

#define LOG_NOLOG static int log(const char * fmt,...) { return 0; }    

#define LOG_ENABLED(v) enum {LOG_ENABLED=v};

#ifdef QT_VERSION
# include <QString>
# define LOG_QSTRING static int log(QString str) { return log(qPrintable(str)); }
# define LOGL_QSTRING static int logL(QString str) { return log(str+"\n"); }
# define qNum(n) QString::number(n)
#else
# define LOG_QSTRING
# define LOGL_QSTRING
#endif //QT_VERSION

#define ADDLOG(enabled) \
  LOG_ENABLED(enabled)  \
  LOG_CURRNAME          \
  LOG_PRINTF            \
  LOG_QSTRING           \
  LOGL_QSTRING

#define ADDLOG_NAME(name,enabled) \
  LOG_ENABLED(enabled)            \
  LOG_SETNAME(name)               \
  LOG_PRINTF                      \
  LOG_QSTRING                     \
  LOGL_QSTRING

#endif  // _Log_hpp_