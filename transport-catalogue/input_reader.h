#pragma once

#include <string>
#include <string_view>
#include <algorithm>
#include <unordered_set>
#include <vector>
#include <sstream>
#include <iostream>

namespace trans_cat {
class ExceptionWrongQueryType {
public:	
	std::string text_;
	ExceptionWrongQueryType(std::string  text = "") : text_(text) { }
};

namespace detail {
struct QueryStop {
	std::string_view name;
	double lat, lng;
	std::unordered_map<std::string_view, int> stop_di;
};

struct QueryBus {
	std::string_view name;
	std::vector<std::string_view> stops;
	bool is_ring;
};

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
	
	void ReadQuery(std::string& line);

	std::unordered_set<std::string>& ExportStops(std::vector<detail::QueryStop>& stop_query);
	std::unordered_set<std::string>& ExportBuses(std::vector<detail::QueryBus>& bus_query);
	
private:
	std::vector<detail::QueryBus> bus_query_;
	std::vector<detail::QueryStop> stop_query_;
	std::unordered_set<std::string> stop_names_;
	std::unordered_set<std::string> bus_names_;
	
	bool parseStopList(std::string_view args_line, std::vector<std::string_view>& stops_view);
	
	std::string_view addName(std::string_view& view, std::unordered_set<std::string>& set);
	
	std::vector<std::string_view> addNames(std::vector<std::string_view>& view, std::unordered_set<std::string>& set);
	std::vector<std::string_view> loadNames(std::vector<std::string_view>& view, std::unordered_set<std::string>& set);
};
} // end ::trans_cat
