#include "serialization.h"

namespace serialize{
trc_serialize::Point PointToProto(svg::Point point) {
	trc_serialize::Point proto_point;
	proto_point.set_x(point.x);
	proto_point.set_y(point.y);
	return proto_point;
}

trc_serialize::Color ColorToProto(svg::Color clr) {
	trc_serialize::Color proto_clr;
	std::visit(ColorSetter(proto_clr), clr);
	return proto_clr;
}

svg::Point ProtoToPoint(trc_serialize::Point proto_point) {
	return { 
		proto_point.x(), 
		proto_point.y() 
	};
}

svg::Color ProtoToColor(trc_serialize::Color proto_clr) {
	switch(static_cast<ColorFormat>(proto_clr.format())) {
		case ColorFormat::NAMED:
			return proto_clr.name();
		case ColorFormat::RGB:
			return svg::Rgb{ 
				static_cast<uint8_t>(proto_clr.r()), 
				static_cast<uint8_t>(proto_clr.g()), 
				static_cast<uint8_t>(proto_clr.b())
			};
		case ColorFormat::RGBA:
			return svg::Rgba{ 
				static_cast<uint8_t>(proto_clr.r()), 
				static_cast<uint8_t>(proto_clr.g()), 
				static_cast<uint8_t>(proto_clr.b()), 
				proto_clr.opacity() 
			};
		default:
			return svg::Color {};
	}
}


trc_serialize::Weight WeightToProto(const trans_cat::RouteItem& weight) {
	trc_serialize::Weight proto_weight;
	proto_weight.set_item_type(static_cast<uint32_t>(weight.type));
	proto_weight.set_time(weight.time);
	proto_weight.set_item_id(weight.GetId());
	proto_weight.set_span(weight.span);
	return proto_weight;
}

using IncidenceList = std::vector<graph::EdgeId>;
bool Router::Save() const {
	
	trc_serialize::TransportRouter proto_trans_router;
	trc_serialize::Graph proto_graph;
	
	// router settings
	trc_serialize::RouterSettings proto_settings;
	auto settings = trans_router_->GetSettings();
	proto_settings.set_bus_wait_time(settings.bus_wait_time);
	proto_settings.set_bus_velocity(settings.bus_velocity);
	*proto_trans_router.mutable_settings() = proto_settings;
	
	//Graph
	std::vector<graph::Edge<trans_cat::RouteItem>> edges;
    std::vector<IncidenceList> incidence_lists;
	const std::vector<graph::Edge<trans_cat::RouteItem>>& edges = trans_router_->GetGraph().GetEdges(); 
	auto incident_lists = trans_router_->GetGraph().GetIncidentLists();
	

	for(const auto& edge : edges) {
		trc_serialize::Edge proto_edge;
		
		trc_serialize::Weight proto_weight = WeightToProto(edge.weight);
		proto_edge.set_from(edge.from);
		proto_edge.set_to(edge.to);
		*proto_edge.mutable_weight() = proto_weight;
		
		*proto_graph.add_edges() = proto_edge;
		//proto_weight.set_item_type()
	}
	
	trc_serialize::IncidenceList proto_incidence;
	for(const auto& list : incidence_lists) {
		trc_serialize::IncidenceList proto_incidence;
		for(const auto& edge_id : list) {
			proto_incidence.add_edge_id(edge_id);
		}
		*proto_graph.add_incidence_lists() = proto_incidence;
	}
	/*
	 * 
	 * enum class RouteItemType {
	NONE,
	WAIT,
	BUS
};
// RouteItem
struct Weight {
	RouteItemType type = RouteItemType::NONE;
	double time = 0;
	std::string_view name;
	int span = 0;
	* 
	 * GRAPH
struct Edge {
    VertexId from;
    VertexId to;
    Weight weight;
};

    using IncidenceList = std::vector<EdgeId>;
    using IncidentEdgesRange = ranges::Range<typename IncidenceList::const_iterator>;
    * 
    * 
	 *  std::vector<Edge<Weight>> edges_;
    std::vector<IncidenceList> incidence_lists_;
    */
    
	*proto_trans_router.mutable_graph() = proto_graph;
    // Router
    trc_serialize::Router proto_router;
	const auto internal_data_list = trans_router_->GetRouter().GetInternalData(); 
		
	for(const auto& internal_datas : internal_data_list) {
		trc_serialize::RouteInternalDataList proto_internal_data_list;
		for(const auto& data : internal_datas) {
			trc_serialize::RouteInternalData proto_internal_data;
			*proto_internal_data.weight() = GetWeight(data.weight);
			if(data.prev_edge) {
				*proto_internal_data.prev_edge() = data.prev_edge;
			}
			*proto_internal_data_list.add_list() = proto_internal_data;
		}
		*proto_router.add_routes_internal_data() = proto_internal_data_list;
	}
	
	
    /*
    * ROUTER
    * 
	  struct RouteInternalData {
        Weight weight;
        std::optional<EdgeId> prev_edge;
    };
	 using RoutesInternalData = std::vector<std::vector<std::optional<RouteInternalData>>>;
	 RoutesInternalData routes_internal_data_;
	 * */
	 
	*proto_trans_router.mutable_router() = proto_router;
	
	*proto_trans_cat_->mutable_router() = proto_trans_router;
	
	
	return true;
}

bool Router::Load() {
	trc_serialize::TransportRouter proto_trans_router = proto_trans_cat_->router();
	
	// router settings
	trc_serialize::RouterSettings proto_settings = proto_trans_router.settings();
	trans_router_->SetSettings(trans_cat::RouterSettings{proto_settings.bus_wait_time(), proto_settings.bus_velocity()});
		
	return true;
}

bool TransportCatalogue::Save() const {
	// add sotps & buses names
	trc_serialize::NameIndex proto_name_index;
	std::map <std::string_view, int> tmp_stop_index, tmp_bus_index;
	
	int i = 0;
	for(const auto& stop_name : trc_->GetStopNames()) { 
		(*proto_name_index.mutable_stop_name())[++i] = stop_name;
		tmp_stop_index[stop_name] = i;
		
	}
	i = 0;
	for(const auto& bus_name : trc_->GetBusNames()) { 
		(*proto_name_index.mutable_bus_name())[++i] = bus_name;
		tmp_bus_index[bus_name] = i;
	}
	
	*proto_trans_cat_->mutable_name_index() = proto_name_index;
	
	// add stops
	for(const auto& stop : trc_->GetStops()) {
		trc_serialize::Stop proto_stop;
		proto_stop.set_name_index(tmp_stop_index[stop.name]);
		proto_stop.set_id(stop.id);
		trc_serialize::Coord proto_coord;
		proto_coord.set_lat(stop.coord.lat);
		proto_coord.set_lng(stop.coord.lng);
		(*proto_stop.mutable_coord()) = proto_coord;
		*proto_trans_cat_->add_stop() = proto_stop;
	}
	
	// add buses
	for(const auto& bus : trc_->GetBuses()) {
		trc_serialize::Bus proto_bus;
		proto_bus.set_id(bus.id);
		proto_bus.set_name_index(tmp_bus_index[bus.name]);
		proto_bus.set_is_ring(bus.is_ring);
		for(const auto& stop : bus.stops) {
			proto_bus.add_stop_id(stop->id); 
		}
		
		*proto_trans_cat_->add_bus() = proto_bus;
	}
	
	// add distance between stop
	for(const auto& dist : trc_->GetDistanceBetweenStops()) {
		trc_serialize::StopDistance proto_stop_dist;
		proto_stop_dist.set_stop_from(dist.stop_from->id);
		proto_stop_dist.set_stop_to(dist.stop_to->id);
		proto_stop_dist.set_distance(dist.distance);
		*proto_trans_cat_->add_stop_distance() = proto_stop_dist;		
	}
	
	return true;
}

bool TransportCatalogue::Load() {
	auto proto_bus_index = proto_trans_cat_->name_index().bus_name();
    auto proto_stop_index = proto_trans_cat_->name_index().stop_name();
    
    // add stop
    std::map<int, trans_cat::Stop*> tmp_id_stop;
	for(const auto& proto_stop : proto_trans_cat_->stop()) {
		tmp_id_stop[proto_stop.id()]= 
			&trc_->AddStop(
				proto_stop_index.at(proto_stop.name_index()), 
				geo::Coordinates {
					proto_stop.coord().lat(),
					proto_stop.coord().lng()
				}
			);
	} 
	
	// add bus
	for(const auto& proto_bus : proto_trans_cat_->bus()) {
		// fill bus_stops vector
		std::vector<const trans_cat::Stop*> bus_stops;
		for(auto stop_id : proto_bus.stop_id()) {
			bus_stops.push_back(tmp_id_stop[stop_id]);
		}
		
		trc_->AddBus(
			proto_bus_index.at(proto_bus.name_index()), 
			bus_stops,
			proto_bus.is_ring()
		);
	}
	
	// add distance between stop
	for(const auto& proto_dist : proto_trans_cat_->stop_distance()) {
		trc_->SetDistance(
			tmp_id_stop.at(proto_dist.stop_from()),
			tmp_id_stop.at(proto_dist.stop_to()),
			proto_dist.distance());
	}
	
	return true;
}

bool RenderSettings::Save() const {
	trc_serialize::RenderSettings proto_rs;
	
	proto_rs.set_width(settings_->width);
	proto_rs.set_height(settings_->height);
	proto_rs.set_stop_radius(settings_->stop_radius);
	proto_rs.set_line_width(settings_->line_width);
	
	proto_rs.set_padding(settings_->padding);
	proto_rs.set_bus_label_font_size(settings_->bus_label_font_size);
	*proto_rs.mutable_bus_label_offset() 
		= (PointToProto(settings_->bus_label_offset));
	proto_rs.set_stop_label_font_size(settings_->stop_label_font_size);
	*proto_rs.mutable_stop_label_offset() 
		= (PointToProto(settings_->stop_label_offset));
	*proto_rs.mutable_underlayer_color()
		= (ColorToProto(settings_->underlayer_color));
	proto_rs.set_underlayer_width(settings_->underlayer_width);

	for(const auto& color : settings_->color_palette) {
		*proto_rs.add_color_palette() = ColorToProto(color); 
	}
	
	*proto_trans_cat_->mutable_render_settings() = proto_rs;
	return true;
}

bool RenderSettings::Load() {
	auto proto_rs = proto_trans_cat_->render_settings();
	settings_->width = proto_rs.width();
	settings_->height = proto_rs.height();
	settings_->stop_radius = proto_rs.stop_radius();
	settings_->line_width = proto_rs.line_width();
	
	settings_->padding = proto_rs.padding();
	settings_->bus_label_font_size = proto_rs.bus_label_font_size();
	settings_->bus_label_offset = ProtoToPoint(proto_rs.bus_label_offset());	
	settings_->stop_label_font_size = proto_rs.stop_label_font_size();
	settings_->stop_label_offset = ProtoToPoint(proto_rs.stop_label_offset());	
	settings_->underlayer_color = ProtoToColor(proto_rs.underlayer_color());
	settings_->underlayer_width = proto_rs.underlayer_width();
	
	for(const auto& color : proto_rs.color_palette()) {
		settings_->color_palette.push_back(ProtoToColor(color)); 
	}
	
	return true;
}

} // ::serialize
