#pragma once

#include "input_reader.h"
#include <string>
#include <string_view>
#include <algorithm>
#include <unordered_set>

namespace trans_cat {
class ExceptionWrongStatReaderQuery {
public:
	std::string text_;
	ExceptionWrongStatReaderQuery(std::string text = "") : text_(text) { }	
};

enum class StatQueryType {
	NONE,
	BUS,
	STOP
};

struct StatQuery {
	StatQueryType type;
	std::string arg;
};

class StatReader {	
public:	
	StatReader() { } 
	
	StatQuery ReadQuery(std::string& line);
};
} // end ::trans_cat
