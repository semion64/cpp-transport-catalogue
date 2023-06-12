#pragma once

#include <vector>
#include <unordered_set>
#include "svg.h"
#include "domain.h"
#include "geo.h"

namespace trans_cat {
	
struct RenderSettings {
	double width = 1200.0;
	double height = 1200.0;
	double padding = 50.0;

	double line_width =14.0;
	double stop_radius = 5.0;

	double bus_label_font_size = 20;
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
};

class MapRenderer {
public:
	MapRenderer();
	virtual void RenderMap(std::ostream& os) = 0;
	virtual ~MapRenderer() = default;
};

using BusList = std::vector<const Bus*>;
class MapRendererSVG {
public:
	
	MapRendererSVG(const RenderSettings& render_settings, BusList bus_list) : rs_(render_settings), bus_list_ (bus_list) {
	}
	
	void RenderMap(std::ostream& os);
private:
	RenderSettings rs_;
	svg::Document doc_;
	BusList bus_list_;
};

}
