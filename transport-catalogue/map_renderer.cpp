#include "map_renderer.h"

namespace trans_cat {
	
void MapRendererSVG::DrawLabel(
					svg::Document& doc, 
					std::string_view name, 
					svg::Point pos, 
					svg::Text& templ, 
					svg::Text& templ_bckg, 
					svg::Color clr_fill) {
	svg::Text label_bckg = templ_bckg;
	svg::Text label = templ;
	label_bckg
		.SetPosition(pos)
		.SetData(std::string(name));
	label
		.SetPosition(pos)
		.SetData(std::string(name))
		.SetFillColor(clr_fill);
	
	doc.Add(label_bckg);
	doc.Add(label);
}

void MapRendererSVG::DrawBusLines(svg::Document& doc, const geo::SphereProjector& proj) {
	for(const auto& [bus_name, bus]: buses_info_) {
		svg::Polyline route_polilyne;
		std::for_each(bus->stops.begin(), bus->stops.end(), [&route_polilyne, &proj](const Stop* stop) {
			route_polilyne.AddPoint(proj(stop->coord));
		});
		
		route_polilyne.SetStrokeWidth(rs_.line_width); 
		route_polilyne.SetStrokeColor(rs_.color_palette[buses_color_.at(bus_name)]);
		route_polilyne
			.SetFillColor("none")
			.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
			.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		doc.Add(route_polilyne);
	}
}

void MapRendererSVG::DrawBusLabels(svg::Document& doc, const geo::SphereProjector& proj) {
	for(const auto& [bus_name, bus]: buses_info_) {
		auto& clr = rs_.color_palette[buses_color_.at(bus->name)];
		DrawLabel(doc, 
			bus_name, 
			proj(bus->stops.front()->coord), 
			bus_label_templ_, 
			bus_label_templ_bckg_, 
			clr);
		auto end_stop = bus->stops[bus->stops.size() / 2];//rs_.color_palette[buses_color_.at(bus->name)]
		if(!bus->is_ring && bus->stops.front() != end_stop) {
			DrawLabel(doc, 
				bus_name, 
				proj(end_stop->coord), 
				bus_label_templ_, 
				bus_label_templ_bckg_, 
				clr);	
		}
	}
}

void MapRendererSVG::DrawStopSymbols(svg::Document& doc, const geo::SphereProjector& proj) {
	for(const auto& [stop_name, stop]: stops_info_) {
		svg::Circle circle;
		circle
			.SetCenter(proj(stop->coord))
			.SetRadius(rs_.stop_radius)
			.SetFillColor("white");
		doc.Add(circle);
	}
}

void MapRendererSVG::DrawStopLabels(svg::Document& doc, const geo::SphereProjector& proj) {
	for(const auto& [stop_name, stop]: stops_info_) {
		DrawLabel(doc, 
			stop_name, 
			proj(stop->coord), 
			stop_label_templ_, 
			stop_label_templ_bckg_, 
			"black");	
	}
}

geo::SphereProjector MapRendererSVG::CreateSphereProjector() {
	// Точки, подлежащие проецированию
	std::vector<geo::Coordinates> geo_coords;
	for(const auto& [bus_name, bus]: buses_info_) {
		std::for_each(bus->stops.begin(), bus->stops.end(), [&geo_coords](const Stop* stop) { geo_coords.push_back(stop->coord); });
	}
	
	// Создаём проектор сферических координат на карту
	return geo::SphereProjector(
		geo_coords.begin(), geo_coords.end(), rs_.width, rs_.height, rs_.padding
	);
}

void MapRendererSVG::SetDefaultTemplates() {
	bus_label_templ_
			.SetFillColor("none")
			.SetOffset(rs_.bus_label_offset)
			.SetFontSize(rs_.bus_label_font_size)
			.SetFontFamily("Verdana")
			.SetFontWeight("bold");
		
	bus_label_templ_bckg_ = bus_label_templ_;
	bus_label_templ_bckg_
		.SetFillColor(rs_.underlayer_color)
		.SetStrokeWidth(rs_.underlayer_width)
		.SetStrokeColor(rs_.underlayer_color)
		.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
		.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
	
	stop_label_templ_
		.SetFillColor("none")
		.SetOffset(rs_.stop_label_offset)
		.SetFontSize(rs_.stop_label_font_size)
		.SetFontFamily("Verdana");
	
	stop_label_templ_bckg_ = stop_label_templ_;
	stop_label_templ_bckg_
		.SetFillColor(rs_.underlayer_color)
		.SetStrokeWidth(rs_.underlayer_width)
		.SetStrokeColor(rs_.underlayer_color)
		.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
		.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
}

void MapRendererSVG::LoadBusesStopsInfo() {
	
	for(const auto& bus : trc_.GetBuses()) {
		if(!bus.stops.size()) {
			continue;
		}
		
		buses_info_[bus.name] = &bus;
		
		for(const auto* stop : bus.stops) {
			stops_info_[stop->name] = stop;
		}
	}
	
	int curr_color = 0;
	for(const auto& [bus_name, bus] : buses_info_) {
		buses_color_[bus_name] = curr_color;
		if(++curr_color == static_cast<int>(rs_.color_palette.size())) {
			curr_color = 0;
		}
	}
}

void MapRendererSVG::RenderMap(std::ostream& os) {
	svg::Document doc;
	LoadBusesStopsInfo();
	const auto& proj = CreateSphereProjector();
	DrawBusLines(doc, proj);
	DrawBusLabels(doc, proj);
	DrawStopSymbols(doc, proj);
	DrawStopLabels(doc, proj);
	doc.Render(os);
}
} // end ::trans_cat
