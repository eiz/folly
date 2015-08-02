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

#include <folly/FileUtil.h>

#include <cerrno>
#ifdef __APPLE__
#include <fcntl.h>
#endif

#if defined(__unix__)
#include <sys/file.h>
#include <sys/socket.h>
#endif

#include <folly/detail/FileUtilDetail.h>

namespace folly {

using namespace fileutil_detail;

platform::Handle openNoInt(const char* name, int flags, mode_t mode) {
  platform::Handle fd;

  do {
    fd = platform::open(name, flags, mode);
  } while (fd == platform::kInvalidHandle && errno == EINTR);

  return fd;
}

int closeNoInt(platform::Handle fd) {
  int r = platform::close(fd);
  // Ignore EINTR.  On Linux, close() may only return EINTR after the file
  // descriptor has been closed, so you must not retry close() on EINTR --
  // in the best case, you'll get EBADF, and in the worst case, you'll end up
  // closing a different file (one opened from another thread).
  //
  // Interestingly enough, the Single Unix Specification says that the state
  // of the file descriptor is unspecified if close returns EINTR.  In that
  // case, the safe thing to do is also not to retry close() -- leaking a file
  // descriptor is definitely better than closing the wrong file.
  if (r == -1 && errno == EINTR) {
    r = 0;
  }
  return r;
}

int fsyncNoInt(platform::Handle fd) {
  return wrapNoInt(platform::fsync, fd);
}

platform::Handle dupNoInt(platform::Handle fd) {
  platform::Handle nfd;

  do {
    nfd = platform::dup(fd);
  } while (nfd == platform::kInvalidHandle && errno == EINTR);

  return nfd;
}

#if defined(HAVE_DUP2)
int dup2NoInt(int oldfd, int newfd) {
  return wrapNoInt(dup2, oldfd, newfd);
}
#endif

#if defined(BROKEN_STUFF)
int fdatasyncNoInt(platform::Handle fd) {
#if defined(__APPLE__)
  return wrapNoInt(fcntl, fd, F_FULLFSYNC);
#elif defined(__FreeBSD__) || defined(_MSC_VER)
  return wrapNoInt(platform::fsync, fd);
#else
  return wrapNoInt(fdatasync, fd);
#endif
}

int ftruncateNoInt(platform::Handle fd, off_t len) {
  return wrapNoInt(ftruncate, fd, len);
}

int truncateNoInt(const char* path, off_t len) {
  return wrapNoInt(truncate, path, len);
}

int flockNoInt(platform::Handle fd, int operation) {
  return wrapNoInt(flock, fd, operation);
}

int shutdownNoInt(platform::Handle fd, int how) {
  return wrapNoInt(shutdown, fd, how);
}

ssize_t readNoInt(platform::Handle fd, void* buf, size_t count) {
  return wrapNoInt(read, fd, buf, count);
}

ssize_t preadNoInt(platform::Handle fd, void* buf, size_t count, off_t offset) {
  return wrapNoInt(pread, fd, buf, count, offset);
}

ssize_t readvNoInt(platform::Handle fd, const iovec* iov, int count) {
  return wrapNoInt(writev, fd, iov, count);
}

ssize_t writeNoInt(platform::Handle fd, const void* buf, size_t count) {
  return wrapNoInt(write, fd, buf, count);
}

ssize_t pwriteNoInt(platform::Handle fd, const void* buf, size_t count, off_t offset) {
  return wrapNoInt(pwrite, fd, buf, count, offset);
}

ssize_t writevNoInt(platform::Handle fd , const iovec* iov, int count) {
  return wrapNoInt(writev, fd, iov, count);
}

ssize_t readFull(platform::Handle fd, void* buf, size_t count) {
  return wrapFull(read, fd, buf, count);
}

ssize_t preadFull(platform::Handle fd, void* buf, size_t count, off_t offset) {
  return wrapFull(pread, fd, buf, count, offset);
}

ssize_t writeFull(platform::Handle fd, const void* buf, size_t count) {
  return wrapFull(write, fd, const_cast<void*>(buf), count);
}

ssize_t pwriteFull(platform::Handle fd, const void* buf, size_t count, off_t offset) {
  return wrapFull(pwrite, fd, const_cast<void*>(buf), count, offset);
}

ssize_t readvFull(platform::Handle fd, iovec* iov, int count) {
  return wrapvFull(readv, fd, iov, count);
}

#if FOLLY_HAVE_PREADV
ssize_t preadvFull(platform::Handle fd, iovec* iov, int count, off_t offset) {
  return wrapvFull(preadv, fd, iov, count, offset);
}
#endif

ssize_t writevFull(platform::Handle fd, iovec* iov, int count) {
  return wrapvFull(writev, fd, iov, count);
}

#if FOLLY_HAVE_PWRITEV
ssize_t pwritevFull(platform::Handle fd, iovec* iov, int count, off_t offset) {
  return wrapvFull(pwritev, fd, iov, count, offset);
}
#endif
#endif

}  // namespaces
