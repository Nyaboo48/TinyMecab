// MeCab -- Yet Another Part-of-Speech and Morphological Analyzer
// Copyright(C) 2001-2006 Taku Kudo <taku@chasen.org>
// Copyright(C) 2004-2006 Nippon Telegraph and Telephone Corporation
#pragma once
#include <forward_list>
#include <limits>
#include <vector>
#include "mecab.hpp"
#include "Param.hpp"
#include "CharProperty.hpp"
#include "Dictionary.hpp"
#include "Writer.hpp"
namespace MeCab {
	class Lattice {
		private:
			std::string                            sentence_;
			std::vector<std::forward_list<Node *>> endNodes_;
			std::forward_list<Node>                nodeList_;
			std::forward_list<Node *>              tokens_;
			Writer          writer_;
			// Tokenize
			Dictionary      sysdic_;
			Dictionary      unkdic_;
			CharProperty    property_;
			CharInfo        space_;
			std::vector<DA> unk_da_;
			// Viterbi
			Mmap<int16_t>   mmap_;
			const int16_t  *matrix_;
			size_t          lSize_;
			size_t          rSize_;
		public:
			explicit Lattice() {}
			~Lattice() {}
			bool open(const Param &param) noexcept {
				const auto dicdir = param.get("dicdir");
				if (!sysdic_.open(dicdir + SYS_DIC_FILE)) return false;
				if (!unkdic_.open(dicdir + UNK_DIC_FILE)) return false;
				if (!property_.open(dicdir + CHAR_PROPERTY_FILE)) return false;
				for (auto&& key : property_.list()) {
					// DEFAULT, SPACE, KANJI, SYMBOL...
					const auto [token, tlen, len] = unkdic_.exactMatchSearch(key);
					if (!token) {
						std::cerr << "cannot find UNK category: " << key << std::endl;
						return false;
					}
					unk_da_.emplace_back(token, tlen, len);
				}
				space_ = property_.getCharInfo(0x20); // ad-hoc
				const auto file = dicdir + MATRIX_FILE;
				if (!mmap_.open(file))
					return false;
				if (!mmap_.begin()) {
					std::cerr << "matrix is NULL\n";
					return false;
				}
				if (mmap_.size() <= 2) {
					std::cerr << "invalid file size: " << file << std::endl;
					return false;
				}
				lSize_ = static_cast<size_t>(mmap_[0]);
				rSize_ = static_cast<size_t>(mmap_[1]);
				if ((lSize_ * rSize_ + 2) != mmap_.size()) {
					std::cerr << "invalid file size: " << file << std::endl;
					return false;
				}
				matrix_ = mmap_.begin() + 2;
				return writer_.open(param);
			}

			void setSentence(const std::string &sentence) noexcept {
				sentence_ = sentence;
				writer_.setSentence(sentence);
				nodeList_.clear();
				for (auto&& it : endNodes_)
					it.clear();
				endNodes_.clear();
				endNodes_.resize(sentence_.size() + 1);
				endNodes_[0].emplace_front(newBosNode());
			}

			void viterbi() noexcept {
				const std::string_view sv{sentence_};
				const auto len = sv.size();
				for (size_t pos = 0; pos < len; ++pos) {
					if (endNodes(pos).empty()) continue;
					tokenize(sv.substr(pos));
					for (auto&& it : tokens_)
						connect(pos, it);
				}
				const auto eosNode = newEosNode();
				for (size_t pos = len + 1; pos--;) { // len..0
					if (endNodes(pos).empty()) continue;
					connect(pos, eosNode);
					break;
				}
				for (auto node = eosNode; node->prev; node = node->prev)
					node->prev->next = node;
			}

			bool stringify(std::string &os) const noexcept {
				os.clear();
				if (endNodes_.empty())
					return true; // not error
				for (auto node = bosNode(); node; node = node->next)
					if (!writer_.writeNode(node, os))
						return false;
				return true;
			}
		private:
			Node *newNode(const NodeStat stat,
				const char *surface, const char *feature,
				const uint16_t len = 0, const uint16_t slen = 0,
				const uint16_t lcAttr = 0, const uint16_t rcAttr = 0,
				const int16_t wcost = 0) noexcept {
				Node node;
				node.prev    = nullptr;
				node.next    = nullptr;
				node.surface = surface;
				node.feature = feature;
				node.length  = len;
				node.rlength = slen + len;
				node.lcAttr  = lcAttr;
				node.rcAttr  = rcAttr;
				node.cost    = 0;
				node.wcost   = wcost;
				node.stat    = stat;
				nodeList_.emplace_front(node);
				return &nodeList_.front();
			}
			Node *newBosNode() noexcept {
				return newNode(NodeStat::MECAB_BOS_NODE, BOS_KEY, BOS_FEATURE);
			}
			Node *newEosNode() noexcept {
				return newNode(NodeStat::MECAB_EOS_NODE, BOS_KEY, BOS_FEATURE);
			}
			Node *bosNode() const noexcept { return endNodes_[0].front(); }
			std::forward_list<Node *> &endNodes(const size_t pos) noexcept {
				return endNodes_[pos];
			}
			void addEndNode(const size_t pos, Node *node) noexcept {
				endNodes_[pos].emplace_front(node);
			}

			void addNor(const DA &da, const char *surface, const size_t slen) noexcept {
				auto [token, tsize, len] = da;
				for (auto i = 0; i < tsize; ++i, ++token)
					tokens_.emplace_front(newNode(NodeStat::MECAB_NOR_NODE,
						surface, sysdic_.feature(*token),
						static_cast<uint16_t>(len), static_cast<uint16_t>(slen),
						token->lcAttr, token->rcAttr, token->wcost));
			}
			void addUnk(const CharInfo cinfo, const char *surface, const size_t len, const size_t slen) noexcept {
				auto [token, tsize, xxx] = unk_da_[cinfo.default_type];
				for (auto i = 0; i < tsize; ++i, ++token)
					tokens_.emplace_front(newNode(NodeStat::MECAB_UNK_NODE,
						surface, unkdic_.feature(*token),
						static_cast<uint16_t>(len), static_cast<uint16_t>(slen),
						token->lcAttr, token->rcAttr, token->wcost));
			}
			void tokenize(std::string_view sv) noexcept {
				const char *end = sv.data() + sv.size();
				tokens_.clear();

				// skip space
				auto [cinfo, mlen, clen, blen] = property_.seekToOtherType(sv, space_);
				if (sv.size() == blen) return; // ends with space
				const auto slen = blen; // space length
				const auto surface = sv.substr(slen);

				// dictionary
				const auto daresult = sysdic_.commonPrefixSearch(surface);
				for (auto&& da : daresult)
					addNor(da, surface.data(), slen);
				if (!tokens_.empty() && !cinfo.invoke) return;

				// Unknown words less than or equal to max-grouping-size characters
				const char *isAdded = nullptr;
				if (cinfo.group) {
					std::tie(std::ignore, std::ignore, clen, blen) = property_.seekToOtherType(surface.substr(mlen), cinfo);
					const size_t ulen = mlen + blen;
					const char *tail = surface.data() + ulen; // Tail of unknown word
					if (clen <= MAX_GROUPING_SIZE)
						addUnk(cinfo, surface.data(), ulen, slen);
					isAdded = tail;
				}

				// First 1 to cinfo.length character
				const char *tail = surface.data() + mlen; // One character after surface
				size_t ulen = mlen;
				for (auto i = 0; i < cinfo.length && tail < end; ++i) {
					if (tail == isAdded) continue;
					addUnk(cinfo, surface.data(), ulen, slen);
					const auto [_cinfo, _mlen] = property_.getCharInfo(surface.substr(ulen));
					if (!cinfo.isKindOf(_cinfo)) break;
					tail += _mlen;
					ulen += _mlen;
				}
			}
			void connect(const size_t pos, Node *rNode) noexcept {
				int64_t bestCost = std::numeric_limits<int64_t>::max();
				Node *bestNode = nullptr;
				for (auto&& lNode : endNodes(pos)) {
					const auto cost = lNode->cost + matrix_[lNode->rcAttr + lSize_ * rNode->lcAttr] + rNode->wcost;
					if (bestCost > cost) {
						bestCost = cost;
						bestNode = lNode;
					}
				}
				rNode->prev = bestNode;
				rNode->next = nullptr;
				rNode->cost = bestCost;
				addEndNode(pos + rNode->rlength, rNode);
			}
	};
}
// vim:set ts=2 sts=2 sw=2 noet:
