// MeCab -- Yet Another Part-of-Speech and Morphological Analyzer
// Copyright(C) 2001-2006 Taku Kudo <taku@chasen.org>
// Copyright(C) 2004-2006 Nippon Telegraph and Telephone Corporation
#pragma once
#include <cstring>
#include <fstream>
#include <iostream>
namespace MeCab {
	class iStream {
		private:
			std::istream *is_;
		public:
			std::istream &operator*() const noexcept { return *is_; }
			explicit iStream(const std::string &filename) {
				is_ = (filename == "-") ? &std::cin : new std::ifstream(filename);
			}
			~iStream() { if (is_ != &std::cin) delete is_; }
	};
	class oStream {
		private:
			std::ostream *os_;
		public:
			std::ostream &operator*() const noexcept { return *os_; }
			explicit oStream(const std::string &filename) {
				os_ = (filename == "-") ? &std::cout : new std::ofstream(filename);
			}
			~oStream() { if (os_ != &std::cout) delete os_; }
	};
}
// vim:set ts=2 sts=2 sw=2 noet:
