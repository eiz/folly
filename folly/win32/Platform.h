/*
* Copyright 2015 Facebook, Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef FOLLY_WIN32_PLATFORM_H_
#define FOLLY_WIN32_PLATFORM_H_

#include <folly/Portability.h>
#include <folly/Preprocessor.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <intrin.h>

namespace folly {

namespace detail {

uint64_t currentThreadId();

} // namespace detail


inline uint32_t ctz(uint32_t value)
{
  DWORD tail = 0;

  return _BitScanForward(&tail, value) ? tail : 32;
}

inline uint32_t ctzll(uint64_t value)
{
  DWORD tail = 0;

  return _BitScanForward64(&tail, value) ? tail : 64;
}

inline uint32_t clz(uint32_t value)
{
  DWORD head = 0;

  return _BitScanReverse(&head, value) ? 31 - head : 32;
}

inline uint32_t clzll(uint64_t value)
{
  DWORD head = 0;

  return _BitScanReverse64(&head, value) ? 63 - head : 64;
}

typedef DWORD pid_t;
typedef DWORD mode_t;

namespace platform {
using HandleType = HANDLE;

#define O_RDONLY 00000000
#define O_WRONLY 00000001
#define O_RDWR   00000002
#define O_CREAT  00000100
#define O_EXCL   00000200
#define O_TRUNC  00001000
#define O_APPEND 00020000

HANDLE platformOpen(const char *name, int flags, mode_t mode)
{
  // TODO: mode is currently ignored.
  // TODO: directories
  // TODO: is there suitably licensed, minimal code that already does this?
  //       no cygwin or mingw stuff allowed.
  // TODO: error reporting unity
  DWORD desiredAccess = 0, creationDisposition = 0;

  if (flags & O_RDONLY) {
    desiredAccess |= GENERIC_READ;
  }

  if (flags & O_WRONLY) {
    desiredAccess |= GENERIC_WRITE;
  }

  if (flags & O_CREAT) {
    creationDisposition = flags & O_EXCL ?
      (flags & O_TRUNC ? CREATE_ALWAYS : CREATE_NEW) :
      (flags & O_TRUNC ? CREATE_ALWAYS : OPEN_ALWAYS);
  }
  else {
    creationDisposition = flags & O_TRUNC ? TRUNCATE_EXISTING : OPEN_EXISTING;
  }

  HANDLE result = CreateFileA(
    name,
    desiredAccess,
    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
    NULL,
    creationDisposition,
    FILE_FLAG_OVERLAPPED,
    nullptr);

  return result;
}

}

} // namespace folly

#endif // FOLLY_WIN32_PLATFORM_H_
