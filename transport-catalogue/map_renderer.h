#pragma once

#include <vector>
#include <unordered_set>
#include "svg.h"
#include "domain.h"
#include "geo.h"

namespace trans_cat {

struct RenderSettings {
	std::optional<double> width;
	std::optional<double> height;
	std::optional<double> padding;
	std::optional<double> line_width;
	std::optional<double> stop_radius;
	std::optional<uint32_t> bus_label_font_size;
	std::optional<svg::Point> bus_label_offset;
	std::optional<uint32_t> stop_label_font_size;
	std::optional<svg::Point> stop_label_offset;
	std::optional<svg::Color> underlayer_color;
	std::optional<double> underlayer_width;
	std::optional<std::vector<svg::Color>> color_palette;
};

class MapRenderer {
public:
	MapRenderer();
	virtual void RenderMap(std::ostream& os) = 0;
	virtual ~MapRenderer() = default;
};

using BusList = std::vector<Bus>;
class MapRendererSVG {
public:
	
	MapRendererSVG(const RenderSettings& render_settings, BusList bus_list) : rs_(render_settings), bus_list_ (bus_list) {
	}
	
	void RenderMap(std::ostream& os);
private:
	RenderSettings rs_;
	BusList bus_list_;
};

}
