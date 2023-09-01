// MeCab -- Yet Another Part-of-Speech and Morphological Analyzer
// Copyright(C) 2001-2011 Taku Kudo <taku@chasen.org>
// Copyright(C) 2004-2006 Nippon Telegraph and Telephone Corporation
#pragma once
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <iostream>
#include <string>
#include "tmecab.hpp"
#ifndef O_BINARY
#define O_BINARY 0
#endif
namespace TMeCab {
	template <class T> class Mmap {
		private:
			T     *buf_;
			size_t size_;
		public:
			const T &operator[](const size_t n) const noexcept { return *(buf_ + n); }
			const T *begin() const noexcept { return buf_; }
			const T *end() const noexcept { return buf_ + size(); }
			size_t size() const noexcept { return size_ / sizeof(T); }

			explicit Mmap(): buf_(nullptr), size_(0) {}
			~Mmap() {
				if (buf_)
					::munmap(reinterpret_cast<char *>(buf_), size_);
			}
			bool open(const std::string &filename) noexcept {
				int fd = ::open(filename.c_str(), O_RDONLY | O_BINARY);
				if (fd < 0) {
					std::cerr << "open failed: " << filename << std::endl;
					return false;
				}

				struct stat st;
				if (::fstat(fd, &st) < 0) {
					std::cerr << "failed to get file size: " << filename << std::endl;
					return false;
				}
				size_ = static_cast<size_t>(st.st_size);

				void *p = ::mmap(nullptr, size_, PROT_READ, MAP_PRIVATE, fd, 0);
				if (p == MAP_FAILED) {
					std::cerr << "mmap() failed: " << filename << std::endl;
					return false;
				}
				buf_ = reinterpret_cast<T *>(p);

				::close(fd);
				return true;
			}
	};
}
// vim:set ts=2 sts=2 sw=2 noet:
