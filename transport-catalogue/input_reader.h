#pragma once

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "transport_catalogue.h"

namespace trans_cat {
class ExceptionWrongQueryType {
public:	
	std::string text_;
	ExceptionWrongQueryType(std::string  text = "") : text_(text) { }
};

namespace detail {
namespace parser{
template <typename T>
T fromString(std::string str) {
	std::stringstream ss(str);
	T res;
	ss >> res;
	return res;
}

std::pair<std::string_view, std::string_view> Split(std::string_view line, char by);
std::string_view Lstrip(std::string_view line) ;
std::string_view Rstrip(std::string_view line);
std::string_view LRstrip(std::string_view line) ;
std::vector<std::string_view> SplitIntoWords(std::string_view text, char add_delimetr = ' ');
} // end ::parser
} // end ::detail

class InputReader {	
public:	
	InputReader(TransportCatalogue& trc) : trc_ (trc) {	}
	
	void Read(std::istream& is);
private:
	TransportCatalogue& trc_;
	std::vector<Bus> buses_;
	
	std::unordered_map<std::string_view, std::vector<std::string_view>> bus_stops_;
	std::vector<Stop> stops_;
	std::unordered_map<std::string, std::string> add_stop_queries_;
	std::unordered_map<std::string, std::string> add_bus_queries_;
	std::unordered_map<std::string_view, std::unordered_map<std::string,int>> stop_di_;
	
	void ReadQuery(std::string& line);
	void AddStops();
	void AddBuses();
	void AddDistanceBetweenStops();
	std::vector<const Stop*> ParseStopList(std::string_view args_line, bool* is_ring);
};
} // end ::trans_cat
