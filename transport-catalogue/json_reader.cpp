#include "json_reader.h"

namespace trans_cat {
	
void InputReaderJSON::Read(const json::Node& root) { 
	try {	
		ReadFromJSON(root, "base_requests"); 
	}
	catch(ExceptionWrongQueryType&) {
		std::cerr << "request node (base_requests) not find" << std::endl;
		return;
	} 
	
	for(const auto& request : root_.AsArray()) {
		ReadQuery(request);
	}
}

void StatReaderJSON::Read(const json::Node& root) { 
	try {
		ReadFromJSON(root, "stat_requests");
	}
	catch(ExceptionWrongQueryType&) {
		std::cerr << "request node (stat_requests) not find" << std::endl;
		return;
	} 
	
	for(const auto& request : root_.AsArray()) {
		json::Dict m = request.AsMap();
		queries_.push_back({
			m.at("id").AsInt(), 
			StatQuery::GetType(m.at("type").AsString()), 
			std::string(m.at("name").AsString())
		});
	}
}

void RenderSettingsJSON::Read(const json::Node& root) { // , std::string request_name
	try {
		ReadFromJSON(root, "render_settings");
	}
	catch(ExceptionWrongQueryType&) {
		std::cerr << "request node (render_settings) not find" << std::endl;
		return;
	}
	auto m = root_.AsMap();
	
	if(m.count("bus_label_offset")) {
		auto& blo = m.at("bus_label_offset").AsArray();
		rs_.bus_label_offset 	= svg::Point {blo[0].AsDouble(), blo[1].AsDouble()};
	}
	
	if(m.count("stop_label_offset")) {
		auto& slo = m.at("stop_label_offset").AsArray();
		rs_.stop_label_offset 	= svg::Point {slo[0].AsDouble(), slo[1].AsDouble()};
	}
	
	if(m.count("color_palette")) {
		std::vector<svg::Color> color_palette;
		for(auto& node_color : m.at("color_palette").AsArray()) {
			color_palette.push_back(detail::ParseColor(node_color));
		}
		
		rs_.color_palette =  color_palette;
	}
	
	if(m.count("width"))	rs_.width				= m.at("width").AsDouble();
	if(m.count("height")) rs_.height 				= m.at("height").AsDouble();
	if(m.count("padding")) rs_.padding 			= m.at("padding").AsDouble();
	if(m.count("line_width")) rs_.line_width 			= m.at("line_width").AsDouble();
	if(m.count("stop_radius")) rs_.stop_radius 		= m.at("stop_radius").AsDouble();
	if(m.count("bus_label_font_size")) rs_.bus_label_font_size = m.at("bus_label_font_size").AsInt();
	if(m.count("stop_label_font_size")) rs_.stop_label_font_size = m.at("stop_label_font_size").AsInt();
	if(m.count("underlayer_color")) rs_.underlayer_color 	=  detail::ParseColor(m.at("underlayer_color"));
	if(m.count("underlayer_width")) rs_.underlayer_width 	=  m.at("underlayer_width").AsDouble();
}

void RequestManagerJSON::Read(const json::Node& root) {
	ReadFromJSON(root);
	auto handler_base = new InputReaderJSON(trc_); 
	auto handler_stat = new StatReaderJSON(trc_, ui_); 
	auto handler_render = new RenderSettingsJSON(trc_);
	
	handler_base->Read(root_);
	handler_stat->Read(root_);
	handler_render->Read(root_);
	
	handler_base_ = handler_base;
	handler_stat_ = handler_stat;
	handler_render_ = handler_render;
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
	try {
		const auto& bus = trc_.GetBus(bus_name);
		const auto& route = trc_.GetRouteStat(bus);
		os_ << "\"curvature\":" << route.curvature << ","
			<< "\"route_length\":" << static_cast<double>(route.distance) << ","
			<< "\"stop_count\":" << route.stops_count << ","
			<< "\"unique_stop_count\":" << route.unique_stops;
	}
	catch(ExceptionBusNotFound&) {
		os_ << "\"error_message\":" << "\"not found\"";
	}	
}

void UserInterfaceJSON::ShowStopBuses(std::string_view stop_name) const {
	os_ << std::setprecision(ROUTE_STAT_PRECISION);
	try {
		const auto& stop_buses = trc_.GetStopBuses(trc_.GetStop(stop_name));
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

namespace detail {
svg::Color ParseColor(const json::Node& node_color) {
	if(node_color.IsArray()) {
		auto& a = node_color.AsArray();
		if(a.size() == 4) {
			return svg::Rgba(a[0].AsInt(), a[1].AsInt(), a[2].AsInt(), a[3].AsDouble());
		}

		return svg::Rgb(a[0].AsInt(), a[1].AsInt(), a[2].AsInt());
	}
	
	return node_color.AsString();
}
}// end ::detail
 
} // end ::trans_cat
