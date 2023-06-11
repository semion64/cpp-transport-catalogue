#include "json_reader.h"

namespace trans_cat {
	
namespace detail{
} // end ::detail

void InputReaderJSON::Read(std::istream& is) {
	root_ = json::Load(is).GetRoot();
	ReadJSON(root_);
    //throw ExceptionWrongInputFormat("use InputReaderJSON::ReadJSON(const json::Node& root) method instead InputReaderJSON::Read(std::istream& is)");
}

void InputReaderJSON::ReadJSON(const json::Node& root) {
    if(!root.IsMap()) {
		throw ExceptionWrongQueryType("json root node has unsupported format");
	}
	
	const auto& root_map = root.AsMap();
	if(!root_map.count("base_requests")) {
		throw ExceptionWrongQueryType("json root node has unsupported format");
	}
	
	for(const auto& request : root_map.at("base_requests").AsArray()) {
		ReadQuery(request);
	}
}

void InputReaderJSON::ReadQuery(const json::Node& request) {
	const json::Dict* m = &request.AsMap();

	if(m->at("type").AsString() == "Stop") {
		add_stop_queries_.insert(m);
	}
	else if(m->at("type").AsString() == "Bus") {
		add_bus_queries_.insert(m);
	}
	//else {
		//throw ExceptionWrongQueryType(std::string(type));
	//} 
}

void InputReaderJSON::AddStops(MapDiBetweenStops& stop_di) {
	for(const json::Dict* request : add_stop_queries_) {
		auto& stop = trc_.AddStop(request->at("name").AsString(), {request->at("latitude").AsDouble(), request->at("longitude").AsDouble()});
		for(const auto& [name, di] : request->at("road_distances").AsMap()) {
			stop_di[stop.name][name] = di.AsInt();
		}
	}
	
	add_stop_queries_.clear();
}

void InputReaderJSON::AddBuses() { 
	for(const auto& request : add_bus_queries_) {
		bool is_ring = request->at("is_roundtrip").AsBool();
		std::vector<const Stop*> bus_stops = ParseStopList(request->at("stops").AsArray(), is_ring);
		trc_.AddBus(request->at("name").AsString(), bus_stops, is_ring);
	}
	
	add_bus_queries_.clear();
}

std::vector<const Stop*> InputReaderJSON::ParseStopList(const json::Array& stop_list, bool is_ring) {
	std::vector<std::string_view> stop_names;
    std::vector<const Stop*> stops;
	stops.reserve(stop_list.size());
	std::for_each(stop_list.begin(), stop_list.end(), [&stops, this](auto& stop_name) {
			stops.push_back(&trc_.GetStop(stop_name.AsString()));
		});

	if(!is_ring) {
		for(int i = stops.size() - 2; i >= 0; --i) { // backward if not ring route
			stops.push_back(stops[i]);
		}
	}
	
	return stops;
}
} // end ::trans_cat
