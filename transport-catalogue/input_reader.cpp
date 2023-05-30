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
		name = AddName(name, stop_names_);
		auto args = detail::parser::SplitIntoWords(args_line, ',');
		std::unordered_map<std::string_view, int> stop_di;
		for(int i = 2; i < args.size(); ++i) {
			auto [dim, to_stop_line] = detail::parser::Split(args[i], ' ');     //args[i] look as "2134m to StopName"
			auto [to, to_stop] = detail::parser::Split(to_stop_line, ' '); 		//to_stop_line look as "to StopName"
			dim.remove_suffix(1);
			to_stop = detail::parser::LRstrip(to_stop);
			stop_di_[name][AddName(to_stop, stop_names_)] = detail::parser::fromString<int>(std::string(dim));
		}
		
		stops_.push_back({ name,
								{	detail::parser::fromString<double>(std::string(detail::parser::LRstrip(args[0]))), 
									detail::parser::fromString<double>(std::string(detail::parser::LRstrip(args[1]))),
								}
							});
	}
	else if(type == "Bus") {
		name = AddName(name, bus_names_);
		std::vector<std::string_view> stops;
		bool is_ring = ParseStopList(args_line, stops);
		bus_stops_[name] = std::move(stops);
		buses_.push_back({ name, {}, is_ring});
	}
	//else {
		//throw ExceptionWrongQueryType(std::string(type));
	//} 	 	
}

void InputReader::Export(TransportCatalogue& trc) {
	trc.ImportStopNames(std::move(stop_names_));
	trc.ImportBusNames(std::move(bus_names_));
	
	for(auto& stop : stops_) {
		trc.AddStop(std::move(stop));
	}
	
	// add distance between stops
	for(const auto& from_stop : trc.GetStops()) {
		// ==ВОПРОС: остались ли индексы в map::stop_di_ (string_view указывает на строку из stop_names_) валидными после перемещения вектора stop_names_: trc.ImportStopNames(std::move(stop_names_));
		for(const auto& [to_stop, di]: stop_di_[from_stop.name]) { 
			trc.SetDistance(&trc.GetStop(from_stop.name), &trc.GetStop(to_stop), di);
		}
		// ==ВОПРОС: Может лучше отказаться от множеств stop_names_ и bus_names_ и хранить строки в struct Stop и struct Bus?
	}
		
	for(auto& bus : buses_) {
		// fill bus stops vector by pointers
		auto stops = bus_stops_[bus.name];
		std::for_each(stops.begin(), stops.end(), [&stops, &trc, &bus, this](auto& stop_name) {
				bus.stops.push_back(&trc.GetStop(stop_name));
			});
		
		trc.AddBus(std::move(bus));
	}
}

bool InputReader::ParseStopList(std::string_view args_line, std::vector<std::string_view>& stops_view) {
	bool is_ring = true;
	if(args_line.find('-') != std::string::npos) {
		stops_view = detail::parser::SplitIntoWords(args_line, '-');
		is_ring = false;
	}
	else {
		stops_view = detail::parser::SplitIntoWords(args_line, '>');
	}

	stops_view = LoadNames(stops_view, stop_names_);

	if(!is_ring) {
		for(int i = stops_view.size() - 2; i >= 0; --i) { // backward if not ring route
			stops_view.push_back(stops_view[i]);
		}
	}
	
	return is_ring;
}

std::string_view InputReader::AddName(std::string_view& view, std::unordered_set<std::string>& set) {
	return *(set.insert(std::string(view)).first);
}

std::vector<std::string_view> InputReader::LoadNames(std::vector<std::string_view>& view, std::unordered_set<std::string>& set) {
	std::vector<std::string_view> res;
	std::for_each(view.begin(), view.end(), [&res, &set, this](auto v) {
			std::string name(v);
			if(set.find(name) == set.end()) {
				res.push_back(AddName(v, set));
				return;
			}
			
			res.push_back(*set.find(std::string(v)));
		});
	return res;
}

std::vector<std::string_view> InputReader::AddNames(std::vector<std::string_view>& view, std::unordered_set<std::string>& set) {
	std::vector<std::string_view> res;
	std::for_each(view.begin(), view.end(), [&res, &set](auto v) {
			res.push_back(*(set.insert(std::string(v)).first));
		});
	return res;
}

} // end ::trans_cat
