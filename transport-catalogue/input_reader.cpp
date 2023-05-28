#include "input_reader.h"

namespace trans_cat {
namespace detail{
namespace parser{
using namespace std;
	pair<string_view, string_view> Split(string_view line, char by) {
		size_t pos = line.find(by);
		string_view left = line.substr(0, pos);

		if (pos < line.size() && pos + 1 < line.size()) {
			return {left, line.substr(pos + 1)};
		} else {
			return {left, string_view()};
		}
	}

	string_view Lstrip(string_view line) {
		while (!line.empty() && isspace(line[0])) {
			line.remove_prefix(1);
		}
		return line;
	}
	
	string_view Rstrip(string_view line) {
		while (!line.empty() && isspace(line[line.size() - 1])) {
			line.remove_suffix(1);
		}
		return line;
	}
	
	string_view LRstrip(string_view line) {
		return Lstrip(Rstrip(line));
	}
	
	std::vector<std::string_view> SplitIntoWords(std::string_view text, char add_delimetr) {
		std::vector<std::string_view> words;
		std::string word;
		size_t start = -1;
		size_t length = 0;
		size_t num_c = 0;
		for (const char c : text) {
			if (c == add_delimetr) {
				if (length > 0) {
					words.push_back(LRstrip(text.substr(start, length)));
					length = 0;
					start = -1;
				}
			}
			else {
				if(start == -1) {
					start = num_c;
				}
				++length;
			}
			
			++num_c;
		}

		if (length > 0) {
			words.push_back(LRstrip(text.substr(start, length)));
		}

		return words;
	}
} // end ::parser
} // end ::detail

void InputReader::ReadQuery(std::string& line) {
	auto [type_line, args_line] = detail::parser::Split(line, ':');
	auto [type, name] = detail::parser::Split(type_line, ' ');
	if(type == "Stop") {
		auto args = detail::parser::SplitIntoWords(args_line, ',');
		std::unordered_map<std::string_view, int> stop_di;
		for(int i = 2; i < args.size(); ++i) {
			auto [dim, to_stop_line] = detail::parser::Split(args[i], ' ');     //args[i] look as "2134m to StopName"
			auto [to, to_stop] = detail::parser::Split(to_stop_line, ' '); 		//to_stop_line look as "to StopName"
			dim.remove_suffix(1);
			to_stop = detail::parser::LRstrip(to_stop);
			stop_di[addName(to_stop, stop_names_)] = detail::parser::fromString<int>(std::string(dim));
		}
		
		stop_query_.push_back({ addName(name, stop_names_),
								detail::parser::fromString<double>(std::string(detail::parser::LRstrip(args[0]))), 
								detail::parser::fromString<double>(std::string(detail::parser::LRstrip(args[1]))),
								stop_di 
							});
	}
	else if(type == "Bus") {
		std::vector<std::string_view> stops;
		bool is_ring = parseStopList(args_line, stops);
		bus_query_.push_back({ addName(name, bus_names_), std::move(stops), is_ring});
	}
	//else {
		//throw ExceptionWrongQueryType(std::string(type));
	//} 	 	
}

std::unordered_set<std::string>& InputReader::ExportStops(std::vector<detail::QueryStop>& stop_query) {
	stop_query = stop_query_;
	return stop_names_;
}

std::unordered_set<std::string>& InputReader::ExportBuses(std::vector<detail::QueryBus>& bus_query) {
	bus_query = bus_query_;
	return bus_names_;
}

bool InputReader::parseStopList(std::string_view args_line, std::vector<std::string_view>& stops_view) {
	bool is_ring = true;
	if(args_line.find('-') != std::string::npos) {
		stops_view = detail::parser::SplitIntoWords(args_line, '-');
		is_ring = false;
	}
	else {
		stops_view = detail::parser::SplitIntoWords(args_line, '>');
	}

	stops_view = loadNames(stops_view, stop_names_);

	if(!is_ring) {
		for(int i = stops_view.size() - 2; i >= 0; --i) { // backward if not ring route
			stops_view.push_back(stops_view[i]);
		}
	}
	
	return is_ring;
}

std::string_view InputReader::addName(std::string_view& view, std::unordered_set<std::string>& set) {
	return *(set.insert(std::string(view)).first);
}

std::vector<std::string_view> InputReader::loadNames(std::vector<std::string_view>& view, std::unordered_set<std::string>& set) {
	std::vector<std::string_view> res;
	std::for_each(view.begin(), view.end(), [&res, &set, this](auto v) {
			std::string name(v);
			if(set.find(name) == set.end()) {
				res.push_back(addName(v, set));
				return;
			}
			
			res.push_back(*set.find(std::string(v)));
		});
	return res;
}

std::vector<std::string_view> InputReader::addNames(std::vector<std::string_view>& view, std::unordered_set<std::string>& set) {
	std::vector<std::string_view> res;
	std::for_each(view.begin(), view.end(), [&res, &set](auto v) {
			res.push_back(*(set.insert(std::string(v)).first));
		});
	return res;
}

} // end ::trans_cat
