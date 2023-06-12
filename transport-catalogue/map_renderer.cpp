#include "map_renderer.h"

namespace trans_cat {
void MapRendererSVG::RenderMap(std::ostream& os) {
	svg::Document doc_;
	// Точки, подлежащие проецированию
	std::vector<geo::Coordinates> geo_coords;
	for(const Bus& bus: bus_list_) {
		std::for_each(bus.stops.begin(), bus.stops.end(), [&geo_coords](const Stop* stop) { geo_coords.push_back(stop->coord); });
	}
	// Создаём проектор сферических координат на карту
	const geo::SphereProjector proj{
		geo_coords.begin(), geo_coords.end(), *rs_.width, *rs_.height, *rs_.padding
	};
	int curr_color = 0;
	for(const Bus& bus: bus_list_) {
		if(bus.stops.size() > 0) {
			svg::Polyline route_polilyne;
			std::for_each(bus.stops.begin(), bus.stops.end(), [&route_polilyne, &proj](const Stop* stop) {
				route_polilyne.AddPoint(proj(stop->coord));
			});
			
			if(rs_.line_width) route_polilyne.SetStrokeWidth(*rs_.line_width); 
			if(rs_.color_palette) route_polilyne.SetStrokeColor((*rs_.color_palette)[curr_color]);
			route_polilyne
				.SetFillColor("none")
				.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
			doc_.Add(route_polilyne);
			
			if(rs_.color_palette && ++curr_color == static_cast<int>((*rs_.color_palette).size())) {
				curr_color = 0;
			}
		}
	}
	
	doc_.Render(os);
}
}
