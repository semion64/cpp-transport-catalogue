#include "json_reader.h"

namespace trans_cat {

using namespace std::literals;	

void BaseJSON::Read(const json::Node* root) { 
	try {	
		ReadFromJSON(root, "base_requests"s); 
	}
	catch(request::ExceptionWrongQueryType&) {
		std::cerr << "request node (base_requests) not find"sv << std::endl;
		return;
	} 
	
	for(const auto& request : root_->AsArray()) {
		ReadQuery(request);
	}
}

void StatJSON::Read(const json::Node* root) { 
	try {
		ReadFromJSON(root, "stat_requests"s);
	}
	catch(request::ExceptionWrongQueryType&) {
		std::cerr << "request node (stat_requests) not find"sv << std::endl;
		return;
	} 
	
	for(const auto& request : root_->AsArray()) {
		const auto& m = request.AsDict();
		std::string type = m.at("type"s).AsString();
		std::unordered_map<std::string, std::string> args;
		if(type == "Bus"s || type == "Stop"s){
			args["name"s] = m.at("name"s).AsString();
		}
		else if(type == "Route"s) {
			args["from"s] = m.at("from"s).AsString();
			args["to"s] = m.at("to"s).AsString();
		} 
		
		queries_.push_back({
			m.at("id"s).AsInt(), 
			detail::StatQuery::GetType(type), 
			args
		});
	}
}

void RouterSettingsJSON::Read(const json::Node* root) {
	try {
		ReadFromJSON(root, "routing_settings"s);
	}
	catch(request::ExceptionWrongQueryType&) {
		std::cerr << "request node (routing_settings) not find"sv << std::endl;
		return;
	}
	
	const auto& m = root_->AsDict();
	Set({m.at("bus_wait_time"s).AsDouble(),  m.at("bus_velocity"s).AsDouble()});
}

void SerializeSettingsJSON::Read(const json::Node* root) {
	try {
		ReadFromJSON(root, "serialization_settings"s);
	}
	catch(request::ExceptionWrongQueryType&) {
		std::cerr << "request node (routing_settings) not find"sv << std::endl;
		return;
	}
	
	const auto& m = root_->AsDict();
	Set({m.at("file"s).AsString()});
}

void RenderSettingsJSON::Read(const json::Node* root) {
	try {
		ReadFromJSON(root, "render_settings"s);
	}
	catch(request::ExceptionWrongQueryType&) {
		std::cerr << "request node (render_settings) not find"sv << std::endl;
		return;
	}
	
	const auto& m = root_->AsDict();
	auto& blo = m.at("bus_label_offset"s).AsArray();
	RenderSettings rs;
	rs.bus_label_offset 	= svg::Point {blo[0].AsDouble(), blo[1].AsDouble()};
	
	auto& slo = m.at("stop_label_offset"s).AsArray();
	rs.stop_label_offset 	= svg::Point {slo[0].AsDouble(), slo[1].AsDouble()};
	
	std::vector<svg::Color> color_palette;
	for(auto& node_color : m.at("color_palette"s).AsArray()) {
		color_palette.push_back(detail::ParseColor(node_color));
	}
	
	rs.color_palette 		=  color_palette;
	rs.width				= m.at("width"s).AsDouble();
	rs.height 				= m.at("height"s).AsDouble();
	rs.padding 			= m.at("padding"s).AsDouble();
	rs.line_width 			= m.at("line_width"s).AsDouble();
	rs.stop_radius 		= m.at("stop_radius"s).AsDouble();
	rs.bus_label_font_size = m.at("bus_label_font_size"s).AsInt();
	rs.stop_label_font_size = m.at("stop_label_font_size"s).AsInt();
	rs.underlayer_color 	=  detail::ParseColor(m.at("underlayer_color"s));
	rs.underlayer_width 	=  m.at("underlayer_width"s).AsDouble();
	
	Set(std::move(rs));
}

void BaseJSON::ReadQuery(const json::Node& request) {
	const auto& m = request.AsDict();
	if(m.at("type"s).AsString() == "Stop"s) {
		add_stop_queries_.insert(&m);
	}
	else if(m.at("type"s).AsString() == "Bus"s) {
		add_bus_queries_.insert(&m);
	}
}

void BaseJSON::AddStops(MapDiBetweenStops& stop_di) {
	for(const auto* q : add_stop_queries_) {
		auto& stop = trc_.AddStop(q->at("name"s).AsString(), {q->at("latitude"s).AsDouble(), q->at("longitude"s).AsDouble()});
		for(const auto& [name, di] : q->at("road_distances"s).AsDict()) {
			stop_di[stop.name][name] = di.AsInt();
		}
	}
	
	add_stop_queries_.clear();
}

void BaseJSON::AddBuses() { 
	for(const auto& q : add_bus_queries_) {
		bool is_ring = q->at("is_roundtrip"s).AsBool();
		std::vector<const Stop*> bus_stops = ParseStopList(q->at("stops"s).AsArray(), is_ring);
		trc_.AddBus(q->at("name"s).AsString(), bus_stops, is_ring);
	}
	
	add_bus_queries_.clear();
}

std::vector<const Stop*> BaseJSON::ParseStopList(const json::Array& stop_list, bool is_ring) {
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

void UserInterfaceJSON::ShowQueriesResult(const request::HandlerStat::StatQueryList& queries) const {
	//graph::Router<RouteItem> router(tr_router_->BuildGraph());
	
	//json_build_ = json::Builder();
	json_build_.StartArray();
	for(const auto& q: queries) {
		json_build_.StartDict()
				.Key("request_id").Value(q.id);
		switch (q.type) {
			case detail::StatQueryType::BUS:
				ShowBus(q.args.at("name"s));
			break;
			case detail::StatQueryType::STOP:
				ShowStopBuses(q.args.at("name"s));
			break;
			case detail::StatQueryType::MAP:
				ShowMap();
			break;
			case detail::StatQueryType::ROUTE:
				//ShowRoute(*tr_router_, router, q.args.at("from"s), q.args.at("to"s));
			break;
			default:
				//throw ExceptionWrongQueryType("");
			break;
		}
		
		json_build_.EndDict();
	}
	
	json_build_.EndArray();
	
	json::Print(json::Document{json_build_.Build()}, os_);
}

void UserInterfaceJSON::ShowMap() const {
    std::stringstream ss;
	map_renderer_->RenderMap(ss);
    json_build_.Key("map").Value(ss.str());
    	
}

void UserInterfaceJSON::ShowRoute(const TransportRouter& tr_router, graph::Router<RouteItem>& router, std::string_view from, std::string_view to) const {
	std::optional<graph::Router<RouteItem>::RouteInfo> route = router.BuildRoute(trc_.GetStop(from).id, trc_.GetStop(to).id);
	if(!route) {
		json_build_.Key("error_message").Value("not found");
		return;
	}
	
	json_build_.Key("total_time").Value(route->weight.time);
	json_build_.Key("items").StartArray();
	for(auto edge_id: route->edges) {
		const graph::Edge<RouteItem>& edge = tr_router.GetEdge(edge_id);
		json_build_.StartDict();
		if(edge.weight.type == RouteItemType::WAIT) {	
			json_build_
				.Key("type").Value("Wait")
				.Key("stop_name").Value(std::string(edge.weight.name))
				.Key("time").Value(edge.weight.time);
		}
		else if(edge.weight.type == RouteItemType::BUS) {	
			json_build_
				.Key("type").Value("Bus")
				.Key("Bus").Value(std::string(edge.weight.name))
				.Key("span_count").Value(edge.weight.span)
				.Key("time").Value(edge.weight.time);
		}
		json_build_.EndDict();
	}
	
	json_build_.EndArray();
}

void UserInterfaceJSON::ShowBus(std::string_view bus_name) const {
	try {
		const auto& bus = trc_.GetBus(bus_name);
		const auto& route = trc_.GetRouteStat(bus);
		json_build_
			.Key("curvature").Value(route.curvature)
			.Key("route_length").Value(static_cast<double>(route.distance))
			.Key("stop_count").Value(route.stops_count)
			.Key("unique_stop_count").Value(route.unique_stops);
	}
	catch(ExceptionBusNotFound&) {
		json_build_.Key("error_message").Value("not found");
	}	
}

void UserInterfaceJSON::ShowStopBuses(std::string_view stop_name) const {
	os_ << std::setprecision(ROUTE_STAT_PRECISION);
	try {
		const auto& stop_buses = trc_.GetStopBuses(trc_.GetStop(stop_name));
		json_build_.Key("buses").StartArray();
		for(const auto& bus : stop_buses) {
			json_build_.Value(std::string(bus->name));
		}
		
		json_build_.EndArray();
	}
	catch(ExceptionBusNotFound&) {
		json_build_.Key("buses").StartArray().EndArray();
	}
	catch(ExceptionStopNotFound&) {
		json_build_.Key("error_message").Value("not found");
	}
}

void ManagerBaseJSON::Read(const json::Node* root) {
	ReadFromJSON(root);
	
	RenderSettingsJSON render_settings(trc_);
	RouterSettingsJSON router_settings(trc_);
	SerializeSettingsJSON serialization_settings(trc_);
	
	render_settings.Read(root_);
	router_settings.Read(root_);
	serialization_settings.Read(root_);
	
	render_settings_ = render_settings.Get();
	router_settings_ = router_settings.Get();
	serialization_settings_ = serialization_settings.Get();
	auto base = std::make_unique<BaseJSON>(trc_); 
	base->Read(root_);
	SetBase(base.release());
}

void ManagerStatJSON::Read(const json::Node* root) {
	ReadFromJSON(root);
	
	SerializeSettingsJSON serialization_settings(trc_);
	serialization_settings.Read(root_);
	serialization_settings_ = serialization_settings.Get();
	
	auto stat = std::make_unique<StatJSON>(trc_); 
	stat->Read(root_);
	SetStat(stat.release());
}

/*
void ManagerJSON::Read(const json::Node* root) {
	ReadFromJSON(root);
	
	RenderSettingsJSON render_settings(trc_);
	RouterSettingsJSON router_settings(trc_);
	
	SerializeSettingsJSON serialization_settings(trc_);
	render_settings.Read(root_);
	router_settings.Read(root_);
	serialization_settings.Read(root_);
	
	render_settings_ = render_settings.Get();
	router_settings_ = router_settings.Get();
	serialization_settings_ = serialization_settings.Get();
	
	auto base = std::make_unique<BaseJSON>(trc_); 
	auto stat = std::make_unique<StatJSON>(trc_); 
	base->Read(root_);
	stat->Read(root_);
	SetBase(base.release());
	SetStat(stat.release());
}*/

namespace detail {
svg::Color ParseColor(const json::Node& node_color) {
	if(node_color.IsArray()) {
		const auto& a = node_color.AsArray();
		if(a.size() == 4) {
			return svg::Rgba(a[0].AsInt(), a[1].AsInt(), a[2].AsInt(), a[3].AsDouble());
		}

		return svg::Rgb(a[0].AsInt(), a[1].AsInt(), a[2].AsInt());
	}
	
	return node_color.AsString();
}
}// end ::detail
 
} // end ::trans_cat
