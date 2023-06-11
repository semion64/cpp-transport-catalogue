#include "json_reader.h"

namespace trans_cat {
	
void InputReaderJSON::Read(std::istream& is) {
	root_ = json::Load(is).GetRoot();
	ReadJSON(root_);
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

void StatReaderJSON::Read(std::istream& is) {
	root_ = json::Load(is).GetRoot();
	ReadJSON(root_);
}

void StatReaderJSON::ReadJSON(const json::Node& root) {
	if(!root.IsMap()) {
		throw ExceptionWrongQueryType("json root node has unsupported format");
	}
	
	const auto& root_map = root.AsMap();
	if(!root_map.count("stat_requests")) {
		throw ExceptionWrongQueryType("json root node has unsupported format");
	}
	
	for(const auto& request : root_map.at("stat_requests").AsArray()) {
		json::Dict m = request.AsMap();
		queries_.push_back({
			m.at("id").AsInt(), 
			StatQuery::GetType(m.at("type").AsString()), 
			std::string(m.at("name").AsString())
		});
	}
}

void UserInterfaceJSON::ShowQueriesResult(const RequestHandlerStat::StatQueryList& queries) const {
	os_ << "[";
    bool is_first = true;
	for(const auto& q: queries) {
        if(!is_first) {
            os_ << ",";
        }
        else {
            is_first = false;
        }
        
		os_ << "{" << "\"request_id\":" << q.id << ",";
		switch (q.type) {
			case StatQueryType::BUS:
				ShowBus(q.name);
			break;
			case StatQueryType::STOP:
				ShowStopBuses(q.name);
			break;
			default:
				throw ExceptionWrongQueryType("");
			break;
		}
		
		os_ << "}";
	}
	
	os_ << "]";	
    os_ << std::endl;
}

void UserInterfaceJSON::ShowBus(std::string_view bus_name) const {
	/*
	 {
	  "curvature": 2.18604,
	  "request_id": 12345678,
	  "route_length": 9300,
	  "stop_count": 4,
	  "unique_stop_count": 3
	}  
	*/
	try {
		const auto& bus = trc_.GetBus(bus_name);
		const auto& route = trc_.GetRouteStat(bus);
		os_ << "\"curvature\":" << route.curvature << ","
			<< "\"route_length\":" << static_cast<double>(route.distance) << ","
			<< "\"stop_count\":" << route.stops_count << ","
			<< "\"unique_stop_count\":" << route.unique_stops;
	}
	catch(ExceptionBusNotFound&) {
		/*{
		  "request_id": 12345,
		  "error_message": "not found"
		} */
		os_ << "\"error_message\":" << "\"not found\"";
	}	
}

void UserInterfaceJSON::ShowStopBuses(std::string_view stop_name) const {
	os_ << std::setprecision(ROUTE_STAT_PRECISION);
	try {
		const auto& stop_buses = trc_.GetStopBuses(trc_.GetStop(stop_name));
		/*{
		  "buses": [
			  "14", "22к"
		  ],
		  "request_id": 12345
		} */
		os_ << "\"buses\":[";
		bool is_first = false;
		for(const auto& bus : stop_buses) {
			if(!is_first) {
				is_first = true;
			}
			else {
				os_ << ",";
			}
			os_ << "\"" << bus->name << "\"";
		}
		os_ << "]";
	}
	catch(ExceptionBusNotFound&) {
		os_ << "\"buses\":" << "[]";
	}
	catch(ExceptionStopNotFound&) {
		os_ << "\"error_message\":" << "\"not found\"";
	}
}

//---------------------------RequestJSON--------------------------------------------------------------------------------------------------------------------------

void RequestJSON::Read(std::istream& is) {
	doc_ = json::Load(is);
	(dynamic_cast<InputReaderJSON*> (handler_base_))->ReadJSON(doc_.GetRoot());
	(dynamic_cast<StatReaderJSON*> (handler_stat_))->ReadJSON(doc_.GetRoot());
}

} // end ::trans_cat
