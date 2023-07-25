#include "serialization.h"
namespace trans_cat {
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


bool Save(std::ostream& output, const TransportCatalogue& trc, std::optional<RenderSettings> rs) {
	trc_serialize::TransportCatalogue proto_trans_cat;
	
	detail::SaveTransport(&proto_trans_cat, trc);
	
	if(rs) {
		detail::SaveRender(&proto_trans_cat, *rs);
	}
	
	proto_trans_cat.SerializeToOstream(&output);
	
	return true;
}	

bool Load(std::istream& input, TransportCatalogue* trc, RenderSettings* rs) {
	trc_serialize::TransportCatalogue proto_trans_cat;
	
    if (!proto_trans_cat.ParseFromIstream(&input)) {
        return false;
    }
    
	detail::LoadTransport(proto_trans_cat, trc);
   
	if(proto_trans_cat.has_render_settings()) {
		 detail::LoadRender(proto_trans_cat, rs);
	}
	
	return true;
}

namespace detail{
	
bool SaveRouter(trc_serialize::TransportCatalogue* proto_trans_cat) {
	
}

bool SaveRender(trc_serialize::TransportCatalogue* proto_trans_cat, const RenderSettings& rs) {
	trc_serialize::RenderSettings proto_rs;
	proto_rs.set_width(rs.width);
	proto_rs.set_height(rs.height);
	proto_rs.set_stop_radius(rs.stop_radius);
	proto_rs.set_line_width(rs.line_width);
	
	proto_rs.set_padding(rs.padding);
	proto_rs.set_bus_label_font_size(rs.bus_label_font_size);
	*proto_rs.mutable_bus_label_offset() 
		= (PointToProto(rs.bus_label_offset));
	proto_rs.set_stop_label_font_size(rs.stop_label_font_size);
	*proto_rs.mutable_stop_label_offset() 
		= (PointToProto(rs.stop_label_offset));
	*proto_rs.mutable_underlayer_color()
		= (ColorToProto(rs.underlayer_color));
	proto_rs.set_underlayer_width(rs.underlayer_width);

	for(const auto& color :rs.color_palette) {
		*proto_rs.add_color_palette() = ColorToProto(color); 
	}
	*proto_trans_cat->mutable_render_settings() = proto_rs;
	return true;
}

bool SaveTransport(trc_serialize::TransportCatalogue* proto_trans_cat, const TransportCatalogue& trc) {
// add sotps & buses names
	trc_serialize::NameIndex proto_name_index;
	std::map <std::string_view, int> tmp_stop_index, tmp_bus_index;
	
	int i = 0;
	for(const auto& stop_name : trc.GetStopNames()) { 
		(*proto_name_index.mutable_stop_name())[++i] = stop_name;
		tmp_stop_index[stop_name] = i;
		
	}
	i = 0;
	for(const auto& bus_name : trc.GetBusNames()) { 
		(*proto_name_index.mutable_bus_name())[++i] = bus_name;
		tmp_bus_index[bus_name] = i;
	}
	
	*proto_trans_cat->mutable_name_index() = proto_name_index;
	
	// add stops
	for(const auto& stop : trc.GetStops()) {
		trc_serialize::Stop proto_stop;
		proto_stop.set_name_index(tmp_stop_index[stop.name]);
		proto_stop.set_id(stop.id);
		trc_serialize::Coord proto_coord;
		proto_coord.set_lat(stop.coord.lat);
		proto_coord.set_lng(stop.coord.lng);
		(*proto_stop.mutable_coord()) = proto_coord;
		*proto_trans_cat->add_stop() = proto_stop;
	}
	// add buses
	for(const auto& bus : trc.GetBuses()) {
		trc_serialize::Bus proto_bus;
		proto_bus.set_name_index(tmp_bus_index[bus.name]);
		proto_bus.set_is_ring(bus.is_ring);
		for(const auto& stop : bus.stops) {
			proto_bus.add_stop_id(stop->id); 
		}
		
		*proto_trans_cat->add_bus() = proto_bus;
	}
	
	// add distance between stop
	for(const auto& dist : trc.GetDistanceBetweenStops()) {
		trc_serialize::StopDistance proto_stop_dist;
		proto_stop_dist.set_stop_from(dist.stop_from->id);
		proto_stop_dist.set_stop_to(dist.stop_to->id);
		proto_stop_dist.set_distance(dist.distance);
		*proto_trans_cat->add_stop_distance() = proto_stop_dist;		
	}
	return true;
}

bool LoadRender(const trc_serialize::TransportCatalogue& proto_trans_cat, RenderSettings* rs) {
	auto proto_rs = proto_trans_cat.render_settings();
	rs->width = proto_rs.width();
	rs->height = proto_rs.height();
	rs->stop_radius = proto_rs.stop_radius();
	rs->line_width = proto_rs.line_width();
	
	rs->padding = proto_rs.padding();
	rs->bus_label_font_size = proto_rs.bus_label_font_size();
	rs->bus_label_offset = ProtoToPoint(proto_rs.bus_label_offset());	
	rs->stop_label_font_size = proto_rs.stop_label_font_size();
	rs->stop_label_offset = ProtoToPoint(proto_rs.stop_label_offset());	
	rs->underlayer_color = ProtoToColor(proto_rs.underlayer_color());
	rs->underlayer_width = proto_rs.underlayer_width();
	
	for(const auto& color : proto_rs.color_palette()) {
		rs->color_palette.push_back(ProtoToColor(color)); 
	}
	
	return true;
}

bool LoadTransport(const trc_serialize::TransportCatalogue& proto_trans_cat, TransportCatalogue* trc) {
	auto proto_bus_index = proto_trans_cat.name_index().bus_name();
    auto proto_stop_index = proto_trans_cat.name_index().stop_name();
    
    // add stop
    std::map<int, Stop*> tmp_id_stop;
	for(const auto& proto_stop : proto_trans_cat.stop()) {
		tmp_id_stop[proto_stop.id()]= 
			&trc->AddStop(
				proto_stop_index.at(proto_stop.name_index()), 
				geo::Coordinates {
					proto_stop.coord().lat(),
					proto_stop.coord().lng()
				}
			);
	} 
	
	// add bus
	for(const auto& proto_bus : proto_trans_cat.bus()) {
		// fill bus_stops vector
		std::vector<const Stop*> bus_stops;
		for(auto stop_id : proto_bus.stop_id()) {
			bus_stops.push_back(tmp_id_stop[stop_id]);
		}
		
		trc->AddBus(
			proto_bus_index.at(proto_bus.name_index()), 
			bus_stops,
			proto_bus.is_ring()
		);
	}
	
	// add distance between stop
	for(const auto& proto_dist : proto_trans_cat.stop_distance()) {
		trc->SetDistance(
			tmp_id_stop.at(proto_dist.stop_from()),
			tmp_id_stop.at(proto_dist.stop_to()),
			proto_dist.distance());
	}
	
	return true;
}

} // ::detail
} // ::serialize
} // ::trans_cat
