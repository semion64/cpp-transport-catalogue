#include "map_renderer.h"

namespace trans_cat {
void MapRendererSVG::RenderMap(std::ostream& os) {
	// Точки, подлежащие проецированию
	std::vector<geo::Coordinates> geo_coords;
	for(const auto* bus: bus_list_) {
		std::for_each(bus->stops.begin(), bus->stops.end(), [&geo_coords](const auto* stop) { geo_coords.push_back(stop->coord); });
	}
	// Создаём проектор сферических координат на карту
	const geo::SphereProjector proj{
		geo_coords.begin(), geo_coords.end(), rs_.width, rs_.height, rs_.padding
	};
	int curr_color = 0;
	for(const auto* bus: bus_list_) {
		svg::Polyline route_polilyne;
		std::for_each(bus->stops.begin(), bus->stops.end(), [&route_polilyne, &proj](const auto* stop) {
			route_polilyne.AddPoint(proj(stop->coord));
		});
		route_polilyne
			.SetStrokeWidth(rs_.line_width)
			.SetStrokeColor(rs_.color_palette[curr_color])
			.SetFillColor("none")
			.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
			.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		doc_.Add(route_polilyne);
		
		if(++curr_color == rs_.color_palette.size()) {
			curr_color = 0;
		}
	}
	
	
	
	doc_.Render(os);
}
}
