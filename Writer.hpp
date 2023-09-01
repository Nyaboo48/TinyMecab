// MeCab -- Yet Another Part-of-Speech and Morphological Analyzer
// Copyright(C) 2001-2011 Taku Kudo <taku@chasen.org>
// Copyright(C) 2004-2006 Nippon Telegraph and Telephone Corporation
#pragma once
#include <algorithm>
#include <charconv>
#include <string>
#include <string_view>
#include <vector>
#include "tmecab.hpp"
#include "Param.hpp"
namespace TMeCab {
	class Writer {
		private:
			std::string norFmt_; // normal node format
			std::string unkFmt_; // unknown node format
			std::string bosFmt_; // BOS node format
			std::string eosFmt_; // EOS node format
			std::string_view sentence_;
		public:
			explicit Writer() {}
			~Writer() {}
			bool open(const Param &param) noexcept {
				std::string norKey = "node-format";
				std::string unkKey = "unk-format";
				std::string bosKey = "bos-format";
				std::string eosKey = "eos-format";
				norFmt_ = param.get(norKey, "%m\\t%H\\n");
				unkFmt_ = param.get(unkKey, norFmt_);
				bosFmt_ = param.get(bosKey, "");
				eosFmt_ = param.get(eosKey, "EOS\\n");

				const auto formatType = param.get("output-format-type");
				if (!formatType.empty()) {
					const std::string fType{"-" + formatType};
					norKey += fType;
					unkKey += fType;
					bosKey += fType;
					eosKey += fType;
					const auto tmp = param.get(norKey);
					if (tmp.empty()) {
						std::cerr << "unkown format type [" << formatType << "]\n";
						return false;
					}
				}
				norFmt_ = param.get(norKey, norFmt_);
				unkFmt_ = param.get(unkKey, norFmt_);
				bosFmt_ = param.get(bosKey, bosFmt_);
				eosFmt_ = param.get(eosKey, eosFmt_);
				return true;
			}
			void setSentence(std::string_view sentence) noexcept {
				sentence_ = sentence;
			}
			bool writeNode(const Node *node, std::string &os) const noexcept {
				switch (node->stat) {
					case NodeStat::MECAB_NOR_NODE: return writeNode(norFmt_, node, os);
					case NodeStat::MECAB_UNK_NODE: return writeNode(unkFmt_, node, os);
					case NodeStat::MECAB_BOS_NODE: return writeNode(bosFmt_, node, os);
					case NodeStat::MECAB_EOS_NODE: return writeNode(eosFmt_, node, os);
				}
				return false;
			}
		private:
			void addString(std::string &os, const auto v) const noexcept {
				char buf[8]{};
				const auto [ptr, ec] = std::to_chars(buf, std::end(buf), v);
				if (ec == std::errc())
					os.append(buf, static_cast<size_t>(ptr - buf));
			}
			bool writeNode(std::string_view format, const Node *node, std::string &os) const noexcept {
				std::vector<std::string> csv;
				for (const char *p = format.data(); *p; ++p) {
					switch (*p) {
						default:
							os += *p;
							break;
						case '\\':
							os += escapedChar(*++p);
							break;
						case '%': // macros
							switch (*++p) {
								default: // unknown meta char
									os += {'%', *p};
									break;
								case '%': // %
									os += '%';
									break;
								case 'S': // input sentence
									os += sentence_;
									break;
								case 'L': // sentence length
									addString(os, sentence_.size());
									break;
								case 'm': // morph
									os.append(node->surface, node->length);
									break;
								case 'M':
									os.append(node->surface - node->rlength + node->length, node->rlength);
									break;
								case 'H':
									os += node->feature;
									break;
								case 'F':
								case 'f':
									if (node->feature[0] == '\0') {
										std::cerr << "no feature information available\n";
										return false;
									}
									if (csv.empty())
										csv = splitCsv(node->feature);
									auto separator = '\t'; // default separator
									if (*p == 'F') // change separator
										separator = (*++p == '\\') ? escapedChar(*++p) : *p;
									if (*++p !='[') {
										std::cerr << "cannot find '['\n";
										return false;
									}
									size_t n = 0;
									bool sep = false;
									for (++p ; *p != ']'; ++p) {
										switch (*p) {
											case '0': case '1': case '2': case '3': case '4':
											case '5': case '6': case '7': case '8': case '9':
												n = n * 10 + static_cast<size_t>(*p - '0');
												break;
											case ',':
												if (n >= csv.size()) {
													std::cerr << "given index is out of range\n";
													return false;
												}
												if (csv[n].at(0) != '*') {
													if (sep) os += separator;
													os += csv[n];
													sep = true;
												} else
													sep = false;
												n = 0;
												break;
											default:
												std::cerr << "cannot find ']'\n";
												return false;
										}
									}
									// ']'
									if (n >= csv.size()) {
										std::cerr << n << "\t" << csv.size() << std::endl;
										std::cerr << "given index is out of range\n";
										return false;
									}
									if (csv[n].at(0) != '*') {
										if (sep) os += separator;
										os += csv[n];
									}
									break;
							} // end switch
							break; // end case '%'
					} // end switch
				}
				return true;
			}
			std::vector<std::string> splitCsv(std::string_view str) const noexcept {
				std::vector<std::string> sv;
				const char *bos = str.data();
				const char *eos = bos + str.size();
				for (; bos < eos; ++bos) {
					if (isspace(*bos)) continue;
					if (*bos == '"') {
						std::string val;
						for (++bos; bos < eos; ++bos) {
							if (*bos == '"' && *++bos != '"') break;
							val += *bos;
						}
						sv.emplace_back(val);
						bos = std::find(bos, eos, ',');
					} else {
						const char *n = std::find(bos, eos, ',');
						sv.emplace_back(std::string(bos, static_cast<size_t>(n - bos)));
						bos = n;
					}
				}
				return sv;
			}
			constexpr char escapedChar(const char p) const noexcept {
				switch (p) {
					case '0':  return '\0';
					case 'a':  return '\a';
					case 'b':  return '\b';
					case 't':  return '\t';
					case 'n':  return '\n';
					case 'v':  return '\v';
					case 'f':  return '\f';
					case 'r':  return '\r';
					case 's':  return ' ';
					case '\\': return '\\';
					default: break;
				}
				return '\0';
			}
	};
}
// vim:set ts=2 sts=2 sw=2 noet:
