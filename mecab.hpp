// MeCab -- Yet Another Part-of-Speech and Morphological Analyzer
// Copyright(C) 2001-2011 Taku Kudo <taku@chasen.org>
// Copyright(C) 2004-2006 Nippon Telegraph and Telephone Corporation
#pragma once
#include <cstdint>
#define SYS_DIC_FILE       "sys.dic"
#define UNK_DIC_FILE       "unk.dic"
#define MATRIX_FILE        "matrix.bin"
#define CHAR_PROPERTY_FILE "char.bin"
#define DICRC              "dicrc"
#define BOS_KEY            "BOS/EOS"
#define BOS_FEATURE        "BOS/EOS,*,*,*,*,*,*,*,*,*,*,*,*,*,*,*,*"
#define MAX_GROUPING_SIZE  24
namespace MeCab {
	// Parameters for MeCab::Node::stat
	enum NodeStat : uint8_t {
		MECAB_NOR_NODE = 0, // Normal node defined in the dictionary.
		MECAB_UNK_NODE = 1, // Unknown node not defined in the dictionary.
		MECAB_BOS_NODE = 2, // Virtual node representing a begin of the sentence.
		MECAB_EOS_NODE = 3, // Virtual node representing a end of the sentence.
	};
	struct Node {
		struct Node *prev; // pointer to the previous node
		struct Node *next; // pointer to the next node

		// surface string. this value is not 0 terminated.
		// You can get the length with length/rlength members.
		const char *surface;
		const char *feature; // feature string

		uint16_t length;  // length of the surface form
		uint16_t rlength; // length of the surface form including white space before the morph

		uint16_t lcAttr; // left attribute id
		uint16_t rcAttr; // right attribute id

		int64_t  cost; // best accumulative cost from bos node to this node
		int16_t wcost; // word cost

		NodeStat stat; // status of this model.
		uint8_t _padding[5];
	};
}
// vim:set ts=2 sts=2 sw=2 noet:
