// MeCab -- Yet Another Part-of-Speech and Morphological Analyzer
// Copyright(C) 2001-2011 Taku Kudo <taku@chasen.org>
// Copyright(C) 2004-2006 Nippon Telegraph and Telephone Corporation
#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>
#include "tmecab.hpp"
#include "Mmap.hpp"
namespace TMeCab {
	struct Token {
		uint16_t lcAttr;
		uint16_t rcAttr;
		uint16_t posid; // not used
		int16_t  wcost;
		uint32_t feature;
		uint32_t compound; // not used
	};
	using DA = std::tuple<const Token *, const int32_t, const size_t>;
	class Dictionary {
		private:
			const Token *token_;
			const char  *feature_;
			Mmap<char>   mmap_;
			struct unit_t {
				int32_t    base;
				uint32_t   check;
			};
			const unit_t *array_;
			const uint32_t DictionaryMagicID = 0xef718f77u;
		public:
			explicit Dictionary() {}
			~Dictionary() {}
			bool open(const std::string &filename) noexcept {
				if (!mmap_.open(filename))
					return false;
				if (mmap_.size() < 100) {
					std::cerr << "dictionary file is broken: " << filename << std::endl;
					return false;
				}

				const char *ptr = mmap_.begin();
				const auto magic = read32u(&ptr);
				if ((magic ^ DictionaryMagicID) != mmap_.size()) {
					std::cerr << "invalid file size: " << filename << std::endl;
					return false;
				}
				const auto version = read32u(&ptr);
				if (version != DIC_VERSION) {
					std::cerr << "incompatible version: " << version << std::endl;
					return false;
				}

				ptr += sizeof(uint32_t) * 4; // type, lexsize, lsize, rsize
				const auto dsize = read32u(&ptr);
				const auto tsize = read32u(&ptr);
				const auto fsize = read32u(&ptr);
				ptr += sizeof(uint32_t); // dummy
				ptr += 32; // skip charset

				array_ = reinterpret_cast<const unit_t *>(ptr);
				ptr += dsize;

				token_ = reinterpret_cast<const Token *>(ptr);
				ptr += tsize;

				feature_ = ptr;
				ptr += fsize;

				if (ptr != mmap_.end()) {
					std::cerr << "dictionary file is broken: " << filename << std::endl;
					return false;
				}
				return true;
			}
			DA exactMatchSearch(std::string_view key) const noexcept {
				const size_t len = key.size();
				uint32_t p;
				uint32_t b = static_cast<uint32_t>(array_[0].base);
				for (size_t i = 0; i < len; ++i) {
					p = b + static_cast<uint8_t>(key[i]) + 1;
					if (b != array_[p].check)
						return {nullptr, 0, 0};
					b = static_cast<uint32_t>(array_[p].base);
				}
				p = b;
				const int32_t n = array_[p].base;
				if (b == array_[p].check && n < 0)
					return {token_ + ((-n-1) >> 8), (-n-1) & 0xff, len}; // found
				return {nullptr, 0, 0};
			}
			std::vector<DA> commonPrefixSearch(std::string_view key) const noexcept {
				const size_t len = key.size();
				std::vector<DA> result;
				int32_t  n;
				uint32_t p;
				uint32_t b = static_cast<uint32_t>(array_[0].base);
				for (size_t i = 0; i < len; ++i) {
					p = b;
					n = array_[p].base;
					if (b == array_[p].check && n < 0)
						result.emplace_back(token_ + ((-n-1) >> 8), (-n-1) & 0xff, i);
					p = b + static_cast<uint8_t>(key[i]) + 1;
					if (b != array_[p].check)
						return result;
					b = static_cast<uint32_t>(array_[p].base);
				}
				p = b;
				n = array_[p].base;
				if (b == array_[p].check && n < 0)
					result.emplace_back(token_ + ((-n-1) >> 8), (-n-1) & 0xff, len);
				return result;
			}
			const char *feature(const Token &t) const noexcept {
				return feature_ + t.feature;
			}
		private:
			uint32_t read32u(const char **ptr) const noexcept {
				const uint32_t *r = reinterpret_cast<const uint32_t *>(*ptr);
				*ptr += sizeof(uint32_t);
				return *r;
			}
	};
}
// vim:set ts=2 sts=2 sw=2 noet:
