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

class UserInterface {
	const int ROUTE_STAT_PRECISION = 6;
public:	
	UserInterface(std::ostream& os, TransportCatalogue& trc) : os_(os), trc_(trc)  {}
	void ShowBus(std::string_view bus_name, bool end_line = true);
	void ShowStopBuses(std::string_view stop, bool end_line = true);
private:
	std::ostream& os_;
	TransportCatalogue& trc_;
};

class StatReader {	
public:	
	StatReader(UserInterface& ui) : ui_(ui) { } 
	void ReadAndExec(std::istream& is);
private:
	UserInterface& ui_;
	void ExecQuery(std::string& line);
};
} // end ::trans_cat