// MeCab -- Yet Another Part-of-Speech and Morphological Analyzer
// Copyright(C) 2001-2006 Taku Kudo <taku@chasen.org>
// Copyright(C) 2004-2006 Nippon Telegraph and Telephone Corporation
#pragma once
#include <cstdio>
#include <string>
#include <unordered_map>
#include <vector>
#include "mecab.hpp"
#include "Stream.hpp"
namespace MeCab {
	struct Option {
		const char *name;
		const char  shortName;
	};
	class Param {
		private:
			std::unordered_map<std::string, std::string> conf_;
			std::vector<std::string> rest_;
		public:
			explicit Param() {}
			~Param() {}
			bool open(int argc, char **argv, const Option *opts) noexcept {
				if (argc <= 0) return true; // this is not error
				for (auto ind = 1; ind < argc; ++ind) {
					if (argv[ind][0] == '-') {
						// long options
						if (argv[ind][1] == '-') {
							char *s;
							for (s = &argv[ind][2]; *s != '\0' && *s != '='; ++s);
							const size_t len = static_cast<size_t>(s - &argv[ind][2]);
							if (!len) return true; // stop the scanning
							bool hit = false;
							size_t i;
							for (i = 0; opts[i].name; ++i) {
								if (std::strncmp(&argv[ind][2], opts[i].name, len) == 0) {
									hit = true;
									break;
								}
							}
							if (!hit) {
								std::cerr << "unrecognized option `" << argv[ind] << "`\n";
								return false;
							}
							if (*s == '=')
								set(opts[i].name, s + 1);
							else if (argc > (ind + 1))
								set(opts[i].name, argv[++ind]);
							else {
								std::cerr << "`" << argv[ind] << "` requires an argument\n";
								return false;
							}
						}
						// short options
						else if (argv[ind][1] != '\0') {
							bool hit = false;
							size_t i;
							for (i = 0; opts[i].name; ++i) {
								if (opts[i].shortName == argv[ind][1]) {
									hit = true;
									break;
								}
							}
							if (!hit) {
								std::cerr << "unrecognized option `" << argv[ind] << "`\n";
								return false;
							}
							if (argv[ind][2] != '\0')
								set(opts[i].name, &argv[ind][2]);
							else if (argc > (ind + 1))
								set(opts[i].name, argv[++ind]);
							else {
								std::cerr << "`" << argv[ind] << "` requires an argument\n";
								return false;
							}
						}
					} else
						rest_.emplace_back(argv[ind]); // others
				}
				return true;
			}
			const std::vector<std::string> &restArgs() const noexcept { return rest_; }

			bool loadDictionaryResource() noexcept {
				auto rcfile = get("rcfile");
				if (rcfile.empty()) {
					const std::string homedir{getenv("HOME")};
					if (homedir.size()) {
						const std::string file{homedir + "/.mecabrc"};
						std::ifstream is(file);
						if (is) rcfile = file;
					}
				}
				if (!load(rcfile)) return false;

				auto dicdir = get("dicdir");
				if (dicdir.empty()) dicdir = "."; // current
				dicdir += '/';
				set("dicdir", dicdir);

				dicdir += DICRC;
				return load(dicdir);
			}
			std::string get(const std::string &key, const std::string &def = "") const noexcept {
				const auto it = conf_.find(key);
				return it == conf_.end() ? def : it->second;
			}
			void dumpConfig(std::ostream *os) const noexcept {
				for (auto&& it : conf_)
					*os << it.first << ": " << it.second << std::endl;
			}
		private:
			void set(const char *key, const std::string &val) noexcept {
				conf_.insert_or_assign(key, val);
			}
			bool load(std::string &filename) noexcept {
				std::ifstream is(filename);
				if (!is) {
					std::cerr << "open failed: " << filename << std::endl;
					return false;
				}
				for (std::string line; std::getline(is, line);) {
					if (line.empty() || line[0] == '#' || line[0] == ';') continue;
					const auto pos = line.find('=');
					if (pos == std::string::npos) {
						std::cerr << "format error: " << line << std::endl;
						return false;
					}
					size_t s;
					for (s = pos - 1; s > 0 && isspace(line[s]); --s);
					const auto key = line.substr(0, s + 1);
					for (s = pos + 1; s < line.size() && isspace(line[s]); ++s);
					const auto val = line.substr(s);
					conf_.try_emplace(key, val); // Add value if not exist
				}
				return true;
			}
	};
}
// vim:set ts=2 sts=2 sw=2 noet:
