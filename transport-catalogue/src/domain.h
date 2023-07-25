#pragma once

#include <algorithm>
#include <deque>
#include <iomanip>
#include <iostream>
#include <optional>
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
	size_t id;
};

struct Bus {
	std::string_view name;
	std::vector<const Stop*> stops;
	bool is_ring;
	size_t id;
};

namespace detail {
struct RouteStat {
	int stops_count;
	int unique_stops;
	int distance;
	double curvature;
	bool is_ring;
};

class DistanceBetweenStopHasher {
public:
	template<class TFirst, class TSecond>
	int operator()(const std::pair<TFirst, TSecond>& p) const noexcept {
		int h = std::hash<TFirst>{}(p.first);
		h <<= sizeof(int);
		h ^= std::hash<TSecond>{}(p.second);
		return std::hash<int>{}(h);
	}
};	

struct compareBusesByName {
	bool operator()(const Bus* lhs, const Bus* rhs) const {
		return lhs->name < rhs->name;
	}
};

enum class StatQueryType {
	NONE,
	BUS,
	STOP,
	MAP,
	ROUTE
};

struct StatQuery {
	int id;
	StatQueryType type;
	std::unordered_map<std::string, std::string> args;
	static StatQueryType GetType(std::string_view type_str);
};

struct DistanceBetweenStopItem {
	const Stop* stop_from;
	const Stop* stop_to;
	int distance;
};

using StopBuses = std::set<const Bus*, compareBusesByName>;
using DistanceBetweenStop = std::unordered_map<std::pair<const Stop*, const Stop*>, int, DistanceBetweenStopHasher>; 
} // end ::detail	
} // end ::trans_cat
