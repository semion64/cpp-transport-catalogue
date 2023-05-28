#include "stat_reader.h"

namespace trans_cat {
StatQuery StatReader::ReadQuery(std::string& line) {
	auto [type_str, arg]= detail::parser::Split(line, ' ');
	StatQueryType type = StatQueryType::NONE;
	if(type_str == "Bus") {
		type = StatQueryType::BUS;
	}
	else if(type_str == "Stop") {
		type = StatQueryType::STOP;
	}
	
	return {type, std::string(arg)};
}
} // end ::trans_cat
