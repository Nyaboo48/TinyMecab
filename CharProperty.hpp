// MeCab -- Yet Another Part-of-Speech and Morphological Analyzer
// Copyright(C) 2001-2011 Taku Kudo <taku@chasen.org>
// Copyright(C) 2004-2006 Nippon Telegraph and Telephone Corporation
#pragma once
#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>
#include "tmecab.hpp"
#include "Mmap.hpp"
namespace TMeCab {
	struct CharInfo {
		uint32_t type:        18 = 0;
		uint32_t default_type: 8 = 0;
		uint32_t length:       4 = 0;
		uint32_t group:        1 = 0;
		uint32_t invoke:       1 = 0;
		bool isKindOf(CharInfo c) const noexcept { return type & c.type; }
	};
	class CharProperty {
		private:
			Mmap<char>               mmap_;
			std::vector<std::string> clist_;
			const CharInfo          *cinfo_;
		public:
			explicit CharProperty() {}
			~CharProperty() {}
			bool open(const std::string &filename) {
				if (!mmap_.open(filename))
					return false;
				const char *ptr = mmap_.begin();

				const size_t csize = read32u(&ptr);
				const size_t fsize = sizeof(uint32_t) + (csize * 32) + sizeof(CharInfo) * 0xffff;
				if (fsize != mmap_.size()) {
					std::cerr << "invalid file size: " << filename << std::endl;
					return false;
				}
				for (size_t i = 0; i < csize; ++i) {
					clist_.emplace_back(ptr);
					ptr += 32;
				}
				cinfo_ = reinterpret_cast<const CharInfo *>(ptr);
				return true;
			}
			const std::vector<std::string> &list() const noexcept { return clist_; }

			std::tuple<CharInfo, size_t, size_t, size_t> seekToOtherType(std::string_view sv, CharInfo c) const {
				CharInfo fail;
				size_t mlen = 0;
				size_t clen = 0;
				size_t blen = 0;
				while (sv.size()) {
					std::tie(fail, mlen) = getCharInfo(sv);
					if (!c.isKindOf(fail)) break;
					blen += mlen;
					sv.remove_prefix(mlen);
					++clen;
					c = fail;
				}
				return {fail, mlen, clen, blen};
			}
			std::tuple<CharInfo, size_t> getCharInfo(std::string_view str) const noexcept {
				const auto [utf8, mlen] = utf8_to_ucs2(str);
				return {cinfo_[utf8], mlen};
			}
			CharInfo getCharInfo(const uint16_t c) const noexcept { return cinfo_[c]; }
		private:
			// All internal codes are represented in UCS2.
			std::tuple<uint16_t, size_t> utf8_to_ucs2(std::string_view str) const noexcept {
				const auto len = str.size();
				if (static_cast<uint8_t>(str[0]) < 0x80)
					return {static_cast<uint16_t>(str[0]), 1};
				if (len >= 2 && (str[0] & 0xe0) == 0xc0)
					return {static_cast<uint16_t>(((str[0] & 0x1f) << 6) | (str[1] & 0x3f)), 2};
				if (len >= 3 && (str[0] & 0xf0) == 0xe0)
					return {static_cast<uint16_t>(((str[0] & 0x0f) << 12) | ((str[1] & 0x3f) << 6) | (str[2] & 0x3f)), 3};
				// belows are out of UCS2
				if (len >= 4 && (str[0] & 0xf8) == 0xf0)
					return {0, 4};
				if (len >= 5 && (str[0] & 0xfc) == 0xf8)
					return {0, 5};
				if (len >= 6 && (str[0] & 0xfe) == 0xfc)
					return {0, 6};
				return {0, 1};
			}
			uint32_t read32u(const char **ptr) const noexcept {
				const uint32_t *r = reinterpret_cast<const uint32_t *>(*ptr);
				*ptr += sizeof(uint32_t);
				return *r;
			}
	};
}
// vim:set ts=2 sts=2 sw=2 noet:
