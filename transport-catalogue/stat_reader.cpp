#include "stat_reader.h"

namespace trans_cat {
void StatReader::ExecQuery(std::string& line) {
	auto [type_str, name]= detail::parser::Split(line, ' ');
	if(type_str == "Bus") {
		ui_.ShowBus(name);	
	}
	else if(type_str == "Stop") {
		ui_.ShowStopBuses(name);	
	}
}
} // end ::trans_cat
