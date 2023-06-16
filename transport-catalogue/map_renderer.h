#pragma once

#include <vector>
#include <map>
#include <string_view>
#include "svg.h"
#include "domain.h"
#include "geo.h"
#include "transport_catalogue.h"

namespace trans_cat {

struct RenderSettings {
	double width;
	double height;
	double padding;
	double line_width;
	double stop_radius;
	uint32_t bus_label_font_size;
	svg::Point bus_label_offset;
	uint32_t stop_label_font_size;
	svg::Point stop_label_offset;
	svg::Color underlayer_color;
	double underlayer_width;
	std::vector<svg::Color> color_palette;
};

class MapRenderer {
public:
	MapRenderer(TransportCatalogue& trc) : trc_(trc) { }
	virtual void RenderMap(std::ostream& os_) = 0;
	virtual ~MapRenderer() = default;
protected:
	TransportCatalogue& trc_;	
};

using BusesInfo = std::map<std::string_view, const Bus*>;
using StopsInfo = std::map<std::string_view, const Stop*>;

class MapRendererSVG : public MapRenderer {
public:
	MapRendererSVG(TransportCatalogue& trc, const RenderSettings& render_settings) : MapRenderer(trc), rs_(render_settings) {
		bus_label_templ_
			.SetFillColor("none")
			.SetOffset(rs_.bus_label_offset)
			.SetFontSize(rs_.bus_label_font_size)
			.SetFontFamily("Verdana")
			.SetFontWeight("bold");
		
		bus_label_templ_bckg_ = bus_label_templ_;
		bus_label_templ_bckg_
			.SetFillColor(rs_.underlayer_color)
			.SetStrokeColor(rs_.underlayer_color)
			.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
			.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
	}
	
	void RenderMap(std::ostream& os_) override;
private:
	RenderSettings rs_;
	BusesInfo buses_info_;
	StopsInfo stops_info_;
	svg::Text bus_label_templ_;
	svg::Text bus_label_templ_bckg_;
	std::map<std::string_view, int> buses_color_;	
	
	geo::SphereProjector CreateSphereProjector();
	void LoadBusesStopsInfo();
	
	void DrawBusLabels(svg::Document& doc, const geo::SphereProjector& proj);
	void DrawBusLines(svg::Document& doc, const geo::SphereProjector& proj);
};

}
