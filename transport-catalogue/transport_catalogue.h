#pragma once

#include <string>
#include <string_view>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "input_reader.h"
#include "geo.h"
#include <iostream>
#include <iomanip>
#include <set>

namespace trans_cat {

class ExceptionBusNotFound {};
class ExceptionStopNotFound {
public:
	std::string text_;
 	ExceptionStopNotFound(std::string text = "") : text_ (text) {}
};	

struct Stop {
	std::string_view name;
	geo::Coordinates coord;
	
};

struct Route {
	std::vector<const Stop*> stop;
	size_t unique_stops;
	double geo_length;
	int distance;
	bool is_ring;
	size_t size() const {
		return stop.size();
	}
	double curvature() const {
		return distance / geo_length;
	}
};

struct Bus {
	std::string_view name;
	Route route;
};

namespace detail {
struct compareBusesByName {
	bool operator()(const Bus* lhs, const Bus* rhs) const {
		return lhs->name < rhs->name;
	}
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

using StopBuses = std::set<const Bus*, compareBusesByName>;
using DistanceBetweenStop = std::unordered_map<std::pair<const Stop*, const Stop*>, int, DistanceBetweenStopHasher>;
} // end ::detail	

class TransportCatalogue{
public:
	
	TransportCatalogue() {}
	
	void Import(InputReader& ir) ;
	void AddStop(const detail::QueryStop& stop);	
	void AddBus(const detail::QueryBus& bus);
	
	const Bus& GetBus(std::string_view bus_name) const;
	const Stop& GetStop(std::string_view stop_name) const;
	const std::set<const Bus*, detail::compareBusesByName>& GetStopBuses(const Stop&) const;

	void SetDistance(const Stop* s1, const Stop*  s2, int di); 
	int GetDistance(const Stop* s1, const Stop*  s2) const; 
	
private:
	// bus & stop store
	std::deque<Bus> bus_;
	std::deque<Stop> stop_;
	
	// find by name indexes 
	std::unordered_map <std::string_view, const Bus*> bus_index_;
	std::unordered_map <std::string_view, const Stop*> stop_index_;
	
	std::unordered_map<const Stop*, std::set<const Bus*, detail::compareBusesByName>> stop_buses_;
	detail::DistanceBetweenStop stop_di_;
	
	// string store
	std::unordered_set<std::string> stop_names_;
	std::unordered_set<std::string> bus_names_;
	int RouteDi(std::vector<const Stop*>& stops) const;
};

class UserInterface {
	TransportCatalogue& trc_;
	std::ostream& os_;
	
public:	
	UserInterface(TransportCatalogue& trc, std::ostream& os = std::cout);
	void ShowBus(std::string_view bus_name, bool end_line = true);
	void ShowStopBuses(std::string_view stop, bool end_line = true);
private:
	void EndLine(bool end_line = true);	
};
} // end ::trans_cat
