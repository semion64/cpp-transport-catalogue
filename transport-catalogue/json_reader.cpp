#include "json_reader.h"

namespace trans_cat {

void InputReaderJSON::Read(std::istream& is) {
	root_ = json::Load(is).GetRoot();
	Read(root_);
}

void InputReaderJSON::Read(const json::Node& root) {
	root_ = root;
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
	Read(root_);
}

void StatReaderJSON::Read(const json::Node& root) {
	root_ = root;
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

//---------------------------RequestManagerJSON--------------------------------------------------------------------------------------------------------------------------

void RequestManagerJSON::Read(std::istream& is) {
	doc_ = json::Load(is);
	auto handler_base = new InputReaderJSON(trc_); 
	auto handler_stat = new StatReaderJSON(trc_, ui_); 
	auto handler_render = new RenderSettingsJSON(trc_);
	
	handler_base->Read(doc_.GetRoot());
	handler_stat->Read(doc_.GetRoot());
	handler_render->Read(doc_.GetRoot());
	
	handler_base_ = handler_base;
	handler_stat_ = handler_stat;
	handler_render_ = handler_render;
	
}

void RenderSettingsJSON::Read(std::istream& is) { 
	root_ = json::Load(is).GetRoot();
	Read(root_);
}

void RenderSettingsJSON::Read(const json::Node& root) { // , std::string request_name
	root_ = root;
    if(!root.IsMap()) {
		return;
		throw ExceptionWrongQueryType("json root node has unsupported format");
	}
	
	const auto& root_map = root.AsMap();
	if(!root_map.count("render_settings")) {
		return;
		throw ExceptionWrongQueryType("json root node has unsupported format");
	}
	
	ReadQuery(root_map.at("render_settings"));
}
/*
struct RenderSettings {
	double width = 1200.0;
	double height = 1200.0;
	double padding = 50.0;

	double line_width =14.0;
	double stop_radius = 5.0;

	uint32_t bus_label_font_size = 20;
	svg::Point bus_label_offset {7.0, 15.0};

	uint32_t stop_label_font_size = 20;
	svg::Point stop_label_offset {7.0, -3.0};

	svg::Color underlayer_color = svg::Rgba{255, 255, 255, 0.85};
	double underlayer_width = 3.0;

	std::vector<svg::Color> color_palette = {
		"green",
		svg::Rgb {255, 160, 0},
		"red"
	};
};*/

	
void RenderSettingsJSON::ReadQuery(const json::Node& request) { 
	const json::Dict* m = &request.AsMap();
	
	if(m->count("bus_label_offset")) {
		auto& blo = m->at("bus_label_offset").AsArray();
		rs_.bus_label_offset 	= svg::Point {blo[0].AsDouble(), blo[1].AsDouble()};
	}
	
	if(m->count("stop_label_offset")) {
		auto& slo = m->at("stop_label_offset").AsArray();
		rs_.stop_label_offset 	= svg::Point {slo[0].AsDouble(), slo[1].AsDouble()};
	}
	if(m->count("color_palette")) {
		std::vector<svg::Color> color_palette;
		for(auto& node_color : m->at("color_palette").AsArray()) {
			color_palette.push_back(detail::ParseColor(node_color));
		}
		
		rs_.color_palette =  color_palette;
	}
	
	if(m->count("width"))	rs_.width				= m->at("width").AsDouble();
	if(m->count("height")) rs_.height 				= m->at("height").AsDouble();
	if(m->count("padding")) rs_.padding 			= m->at("padding").AsDouble();
	if(m->count("line_width")) rs_.line_width 			= m->at("line_width").AsDouble();
	if(m->count("stop_radius")) rs_.stop_radius 		= m->at("stop_radius").AsDouble();
	if(m->count("bus_label_font_size")) rs_.bus_label_font_size = m->at("bus_label_font_size").AsInt();
	if(m->count("stop_label_font_size")) rs_.stop_label_font_size = m->at("stop_label_font_size").AsInt();
	if(m->count("underlayer_color")) rs_.underlayer_color 	=  detail::ParseColor(m->at("underlayer_color"));
	if(m->count("underlayer_width")) rs_.underlayer_width 	=  m->at("underlayer_width").AsDouble();
}

/*
 
 {
  "width": 1200.0,
  "height": 1200.0,

  "padding": 50.0,

  "line_width": 14.0,
  "stop_radius": 5.0,

  "bus_label_font_size": 20,
  "bus_label_offset": [7.0, 15.0],

  "stop_label_font_size": 20,
  "stop_label_offset": [7.0, -3.0],

  "underlayer_color": [255, 255, 255, 0.85],
  "underlayer_width": 3.0,

  "color_palette": [
    "green",
    [255, 160, 0],
    "red"
  ]
} 

 */
 
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
