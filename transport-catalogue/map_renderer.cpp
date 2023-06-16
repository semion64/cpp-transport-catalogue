#include "map_renderer.h"

namespace trans_cat {

void MapRendererSVG::DrawBusLabels() {
	for(const auto& [bus_name, bus]: buses_info_) {
		if(!bus->stops.size()) { 
			return;
		}
		/*svg::Text bus_label;
		bus_label.
			.SetFillColor("none")
			.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
			.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		doc_.Add(route_polilyne);*/
		
		
	}
}
void MapRendererSVG::DrawBusLines(const geo::SphereProjector& proj) {
	int curr_color = 0;
	for(const auto& [bus_name, bus]: buses_info_) {
		svg::Polyline route_polilyne;
		std::for_each(bus->stops.begin(), bus->stops.end(), [&route_polilyne, &proj](const Stop* stop) {
			route_polilyne.AddPoint(proj(stop->coord));
		});
		
		route_polilyne.SetStrokeWidth(rs_.line_width); 
		route_polilyne.SetStrokeColor(rs_.color_palette[curr_color]);
		route_polilyne
			.SetFillColor("none")
			.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
			.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		doc_.Add(route_polilyne);
		
		if(++curr_color == static_cast<int>(rs_.color_palette.size())) {
			curr_color = 0;
		}
		
	}
}

geo::SphereProjector MapRendererSVG::CreateSphereProjector() {
	// Точки, подлежащие проецированию
	std::vector<geo::Coordinates> geo_coords;
	for(const auto& [bus_name, bus]: buses_info_) {
		std::for_each(bus->stops.begin(), bus->stops.end(), [&geo_coords](const Stop* stop) { geo_coords.push_back(stop->coord); });
	}
	
	// Создаём проектор сферических координат на карту
	const geo::SphereProjector proj{
		geo_coords.begin(), geo_coords.end(), rs_.width, rs_.height, rs_.padding
	};
	
	return proj;
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
}

void MapRendererSVG::RenderMap(std::ostream& os) {
	LoadBusesStopsInfo();
	const auto proj = CreateSphereProjector();
	DrawBusLines(proj);
	
	doc_.Render(os);
}
} // end ::trans_cat
