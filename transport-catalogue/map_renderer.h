#pragma once

#include <vector>
#include <unordered_set>
#include "svg.h"
#include "domain.h"
#include "geo.h"

namespace trans_cat {
	
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
