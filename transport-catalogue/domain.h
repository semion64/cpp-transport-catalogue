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
#include <optional>
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
