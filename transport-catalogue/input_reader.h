#pragma once

#include <string>
#include <string_view>
#include <algorithm>
#include <unordered_set>
#include <vector>
#include <sstream>
#include <iostream>
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
	InputReader() {	}
	
	void Export(TransportCatalogue& trc);
	void ReadQuery(std::string& line);

private:
	std::vector<Bus> buses_;
	
	std::unordered_map<std::string_view, std::vector<std::string_view>> bus_stops_;
	std::vector<Stop> stops_;
	std::unordered_set<std::string> stop_names_;
	std::unordered_set<std::string> bus_names_;
	std::unordered_map<std::string_view, std::unordered_map<std::string_view,int>> stop_di_;
	
	bool ParseStopList(std::string_view args_line, std::vector<std::string_view>& stops_view);
	
	std::string_view AddName(std::string_view& view, std::unordered_set<std::string>& set);
	
	std::vector<std::string_view> AddNames(std::vector<std::string_view>& view, std::unordered_set<std::string>& set);
	std::vector<std::string_view> LoadNames(std::vector<std::string_view>& view, std::unordered_set<std::string>& set);
};
} // end ::trans_cat
