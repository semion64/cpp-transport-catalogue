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
	MapRenderer(TransportCatalogue& trc, const RenderSettings& render_settings) : trc_(trc), rs_(render_settings) { }
	virtual void RenderMap(std::ostream& os_) = 0;
	virtual ~MapRenderer() = default;
	
	virtual void SetRenderSettings(const RenderSettings& rs) {
		rs_ = rs;
	}
	
protected:
	TransportCatalogue& trc_;	
	RenderSettings rs_;
};

using BusesInfo = std::map<std::string_view, const Bus*>;
using StopsInfo = std::map<std::string_view, const Stop*>;

class MapRendererSVG : public MapRenderer {
public:
	MapRendererSVG(TransportCatalogue& trc) : MapRenderer(trc) {
		SetDefaultTemplates();
	}
	
	MapRendererSVG(TransportCatalogue& trc, const RenderSettings& render_settings) : MapRenderer(trc, render_settings) {
		SetDefaultTemplates();
	}
	
	void RenderMap(std::ostream& os_) override;
	void SetRenderSettings(const RenderSettings& rs) override;
private:
	BusesInfo buses_info_;
	StopsInfo stops_info_;
	
	svg::Text bus_label_templ_;
	svg::Text bus_label_templ_bckg_;
	svg::Text stop_label_templ_;
	svg::Text stop_label_templ_bckg_;
	
	std::map<std::string_view, int> buses_color_;	
	
	void SetDefaultTemplates();
	geo::SphereProjector CreateSphereProjector();
	void LoadBusesStopsInfo();
	
	void DrawBusLines(svg::Document& doc, const geo::SphereProjector& proj);
	void DrawBusLabels(svg::Document& doc, const geo::SphereProjector& proj);
	void DrawStopSymbols(svg::Document& doc, const geo::SphereProjector& proj);
	void DrawStopLabels(svg::Document& doc, const geo::SphereProjector& proj);
	
	void DrawLabel(svg::Document& doc, std::string_view name, svg::Point pos, svg::Text& templ, svg::Text& templ_bckg, svg::Color clr_fill);
};

} // end ::trans_cat
