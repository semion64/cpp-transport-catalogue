#include "map_renderer.h"

namespace trans_cat {

void MapRendererSVG::DrawBusLabels(svg::Document& doc, const geo::SphereProjector& proj) {
	for(const auto& [bus_name, bus]: buses_info_) {
		if(!bus->stops.size()) { 
			return;
		}
		svg::Text label_start = bus_label_templ_;
		
		/*label_start
			.SetPosition(proj(bus->stops[0]->coord))
			.SetData(std::string(bus_name))
			.SetFillColor();
		*/
		//doc.Add(route_polilyne);
	}
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

void MapRendererSVG::LoadBusesStopsInfo() {
	int curr_color = 0;
	for(const auto& bus : trc_.GetBuses()) {
		if(!bus.stops.size()) {
			continue;
		}
		
		buses_info_[bus.name] = &bus;
		
		buses_color_[bus.name] = curr_color;
		for(const auto* stop : bus.stops) {
			stops_info_[stop->name] = stop;
		}
		
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
	
	doc.Render(os);
}
} // end ::trans_cat
