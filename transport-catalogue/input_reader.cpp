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

void InputReader::Read(std::istream& is) {
	int N;
	is >> N;
	
	while(N >= 0) {
		std::string line;
		std::getline(is, line);
		ReadQuery(line);
		--N;
	}
	
	AddStops();
	AddBuses();
	AddDistanceBetweenStops();
}

void InputReader::ReadQuery(std::string& line) {
	auto [type_line, args_line] = detail::parser::Split(line, ':');
	auto [type, name] = detail::parser::Split(type_line, ' ');
	
	if(type == "Stop") {
		add_stop_queries_[std::string(name)] = std::string(args_line);
	}
	else if(type == "Bus") {
		add_bus_queries_[std::string(name)] = std::string(args_line);
	}
	//else {
		//throw ExceptionWrongQueryType(std::string(type));
	//} 
}
void InputReader::AddStops() {
	for(auto& [name, args_line] : add_stop_queries_) {
		auto args = detail::parser::SplitIntoWords(args_line, ',');
		auto& stop = trc_.AddStop(name, { detail::parser::fromString<double>(std::string(detail::parser::LRstrip(args[0]))), 
							detail::parser::fromString<double>(std::string(detail::parser::LRstrip(args[1])))});
		for(int i = 2; i < args.size(); ++i) {
			auto [dim, to_stop_line] = detail::parser::Split(args[i], ' ');     //args[i] look as "2134m to StopName"
			auto [to, to_stop_name] = detail::parser::Split(to_stop_line, ' '); 		//to_stop_line look as "to StopName"
			dim.remove_suffix(1);
			stop_di_[stop.name][std::string(detail::parser::LRstrip(to_stop_name))] = detail::parser::fromString<int>(std::string(dim));
		}
	}
	
	add_stop_queries_.clear();
}

void InputReader::AddBuses() { 
	for(auto& [name, args_line] : add_bus_queries_) {
		bool is_ring;
		std::vector<const Stop*> bus_stops = ParseStopList(args_line, &is_ring);
		trc_.AddBus(name, bus_stops, is_ring);
	}
	
	add_bus_queries_.clear();
}

void InputReader::AddDistanceBetweenStops() {
	for(const auto& from_stop : trc_.GetStops()) {
		for(const auto& [to_stop, di]: stop_di_[from_stop.name]) { 
			trc_.SetDistance(&trc_.GetStop(from_stop.name), &trc_.GetStop(to_stop), di);
		}
	}
	
	stop_di_.clear();
}

std::vector<const Stop*> InputReader::ParseStopList(std::string_view args_line, bool* is_ring) {
	std::vector<std::string_view> stop_names;
	*is_ring = true;
	if(args_line.find('-') != std::string::npos) {
		stop_names = detail::parser::SplitIntoWords(args_line, '-');
		*is_ring = false;
	}
	else {
		stop_names = detail::parser::SplitIntoWords(args_line, '>');
	}
	
    std::vector<const Stop*> stops;
	stops.reserve(stop_names.size());
	std::for_each(stop_names.begin(), stop_names.end(), [&stops, this](auto& stop_name) {
			stops.push_back(&trc_.GetStop(stop_name));
		});

	if(!(*is_ring)) {
		for(int i = stops.size() - 2; i >= 0; --i) { // backward if not ring route
			stops.push_back(stops[i]);
		}
	}
	
	return stops;
}
} // end ::trans_cat
