// MeCab -- Yet Another Part-of-Speech and Morphological Analyzer
// Copyright(C) 2001-2006 Taku Kudo <taku@chasen.org>
// Copyright(C) 2004-2006 Nippon Telegraph and Telephone Corporation
#include <cstdlib>
#include <string>
#include <vector>
#include "mecab.hpp"
#include "Param.hpp"
#include "Stream.hpp"
#include "Lattice.hpp"
namespace MeCab {
	const MeCab::Option options[] = {
		{"rcfile",             'r'}, // resource file
		{"dicdir",             'd'}, // system dicdir
		{"output-format-type", 'O'}, // output format type (wakati,none,...)
		{"output",             'o'}, // output file name
		{"node-format",        'F'}, // user-defined node format
		{"unk-format",         'U'}, // user-defined unknown node format
		{"bos-format",         'B'}, // user-defined beginning-of-sentence format
		{"eos-format",         'E'}, // user-defined end-of-sentence format
		{"eon-format",         'S'}, // user-defined end-of-NBest format(NOT USED)
		{"unk-feature",        'x'}, // feature for unknown word
	//{"dump-config",        'P'}, // dump MeCab parameters
		{"input-buffer-size",  'b'}, // IGNORED
		{nullptr, '\0'}
	};
}
int main(int argc, char **argv) {
	MeCab::Param param;
	if (!param.open(argc, argv, MeCab::options))
		return 1;
	if (!param.loadDictionaryResource())
		return 1;

	auto ofilename = param.get("output");
	if (ofilename.empty()) ofilename = "-";
	MeCab::oStream os(ofilename);
	if (!*os) {
		std::cerr << "output failed: " << ofilename << std::endl;
		return 1;
	}

	MeCab::Lattice lattice;
	if (!lattice.open(param)) return 1;

#if 0
	if (param.get("dump-config") == "1") {
		param.dumpConfig(&*os);
		return 1;
	}
#endif

	auto files = param.restArgs();
	if (files.empty()) files.push_back("-");
	std::string str;
	for (auto&& file : files) {
		MeCab::iStream is(file);
		if (!*is) {
			std::cerr << "input failed: " << file << std::endl;
			return 1;
		}
		for (std::string line; std::getline(*is, line);) {
			lattice.setSentence(line);
			lattice.viterbi();
			if (!lattice.stringify(str)) return 1;
			*os << str;
		}
		*os << std::flush;
	}
	return 0;
}
// vim:set ts=2 sts=2 sw=2 noet:
