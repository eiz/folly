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

#include <folly/File.h>

#include <fcntl.h>

#if defined(HAVE_UNISTD)
#include <unistd.h>
#endif

#if defined(HAVE_SYS_FILE)
#include <sys/file.h>
#endif

#include <folly/Exception.h>
#include <folly/FileUtil.h>
#include <folly/Format.h>
#include <folly/ScopeGuard.h>

#include <system_error>

#include <glog/logging.h>

namespace folly {

File::File()
  : fd_(platform::kInvalidHandle)
  , ownsFd_(false)
{}

File::File(platform::Handle fd, bool ownsFd)
  : fd_(fd)
  , ownsFd_(ownsFd) {
  CHECK(fd != platform::kInvalidHandle || !ownsFd)
    << "cannot own invalid handle";
}

File::File(const char* name, int flags, mode_t mode)
  : fd_(platform::open(name, flags, mode))
  , ownsFd_(false) {
  if (fd_ == platform::kInvalidHandle) {
    throwSystemError(folly::format("open(\"{}\", {:#o}, 0{:#o}) failed",
                                   name, flags, mode).fbstr());
  }
  ownsFd_ = true;
}

File::File(const std::string& name, int flags, mode_t mode)
  : File(name.c_str(), flags, mode) {}

File::File(StringPiece name, int flags, mode_t mode)
  : File(name.str(), flags, mode) {}

File::File(File&& other) noexcept
  : fd_(other.fd_)
  , ownsFd_(other.ownsFd_) {
  other.release();
}

File& File::operator=(File&& other) {
  closeNoThrow();
  swap(other);
  return *this;
}

File::~File() {
  auto fd = fd_;
  if (!closeNoThrow()) {  // ignore most errors
    DCHECK_NE(errno, EBADF) << "closing fd " << fd << ", it may already "
      << "have been closed. Another time, this might close the wrong FD.";
  }
}

/* static */ File File::temporary() {
  // make a temp file with tmpfile(), dup the fd, then return it in a File.
  FILE* tmpFile = tmpfile();
  checkFopenError(tmpFile, "tmpfile() failed");
  SCOPE_EXIT { fclose(tmpFile); };

  auto fd = platform::dup(platform::fileno(tmpFile));

  if (fd == platform::kInvalidHandle) {
    throwSystemError("dup() failed");
  }

  return File(fd, true);
}

platform::Handle File::release() noexcept {
  auto released = fd_;
  fd_ = platform::kInvalidHandle;
  ownsFd_ = false;
  return released;
}

void File::swap(File& other) {
  using std::swap;
  swap(fd_, other.fd_);
  swap(ownsFd_, other.ownsFd_);
}

void swap(File& a, File& b) {
  a.swap(b);
}

File File::dup() const {
  if (fd_ != platform::kInvalidHandle) {
    auto fd = platform::dup(fd_);

    if (fd == platform::kInvalidHandle) {
      throwSystemError("dup() failed");
    }

    return File(fd, true);
  }

  return File();
}

void File::close() {
  if (!closeNoThrow()) {
    throwSystemError("close() failed");
  }
}

bool File::closeNoThrow() {
  int r = ownsFd_ ? platform::close(fd_) : 0;
  release();
  return r == 0;
}

#if defined(HAVE_FLOCK)
void File::lock() { doLock(LOCK_EX); }
bool File::try_lock() { return doTryLock(LOCK_EX); }
void File::lock_shared() { doLock(LOCK_SH); }
bool File::try_lock_shared() { return doTryLock(LOCK_SH); }

void File::doLock(int op) {
  checkUnixError(flockNoInt(fd_, op), "flock() failed (lock)");
}

bool File::doTryLock(int op) {
  int r = flockNoInt(fd_, op | LOCK_NB);
  // flock returns EWOULDBLOCK if already locked
  if (r == -1 && errno == EWOULDBLOCK) return false;
  checkUnixError(r, "flock() failed (try_lock)");
  return true;
}

void File::unlock() {
  checkUnixError(flockNoInt(fd_, LOCK_UN), "flock() failed (unlock)");
}
void File::unlock_shared() { unlock(); }
#endif

}  // namespace folly
