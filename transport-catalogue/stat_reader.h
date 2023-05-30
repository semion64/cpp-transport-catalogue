#pragma once

#include <algorithm>
#include <string>
#include <string_view>
#include <unordered_set>

#include "input_reader.h"

namespace trans_cat {
class ExceptionWrongStatReaderQuery {
public:
	std::string text_;
	ExceptionWrongStatReaderQuery(std::string text = "") : text_(text) { }	
};

class StatReader {	
public:	
	StatReader(UserInterface& ui) : ui_(ui) { } 
	
	void ExecQuery(std::string& line);
private:
	UserInterface ui_;
};
} // end ::trans_cat
