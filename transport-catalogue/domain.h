#pragma once

#include <algorithm>
#include <deque>
#include <iomanip>
#include <iostream>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "geo.h"

namespace trans_cat {

class ExceptionBusNotFound : public std::logic_error {
public:
 	ExceptionBusNotFound(std::string what = "") : logic_error(what) {}
};

class ExceptionStopNotFound : public std::logic_error {
public:
 	ExceptionStopNotFound(std::string what = "") : logic_error(what) {}
};	

struct Stop {
	std::string_view name;
	geo::Coordinates coord;
};

struct Bus {
	std::string_view name;
	std::vector<const Stop*> stops;
	bool is_ring;
};

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

namespace detail {
struct RouteStat {
	size_t stops_count;
	size_t unique_stops;
	int distance;
	double curvature;
	bool is_ring;
};

class DistanceBetweenStopHasher {
public:
	template<class TFirst, class TSecond>
	size_t operator()(const std::pair<TFirst, TSecond>& p) const noexcept {
		size_t h = std::hash<TFirst>{}(p.first);
		h <<= sizeof(size_t);
		h ^= std::hash<TSecond>{}(p.second);
		return std::hash<size_t>{}(h);
	}
};	

struct compareBusesByName {
	bool operator()(const Bus* lhs, const Bus* rhs) const {
		return lhs->name < rhs->name;
	}
};

using StopBuses = std::set<const Bus*, compareBusesByName>;
using DistanceBetweenStop = std::unordered_map<std::pair<const Stop*, const Stop*>, int, DistanceBetweenStopHasher>; 
} // end ::detail	
} // end ::trans_cat
