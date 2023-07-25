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
#include "domain.h"
#include "geo.h"
#include <map>

namespace trans_cat {

class TransportCatalogue{
public:
	TransportCatalogue() {}
	
	Stop& AddStop(const std::string& name, const geo::Coordinates coord, size_t id = 0);	
	Bus& AddBus(const std::string& name, std::vector<const Stop*>& stops, bool is_ring, size_t id = 0); 
	
	const Bus& GetBus(std::string_view bus_name) const;
	const Stop& GetStop(std::string_view stop_name) const;
	const std::deque<Bus>& GetBuses() const;
	std::vector<Bus>GetBusesSorted() const;
	const std::deque<Stop>& GetStops() const;
	const detail::StopBuses& GetStopBuses(const Stop&) const;
	const detail::RouteStat GetRouteStat(const Bus& bus) const;
	void SetDistance(const Stop* s1, const Stop*  s2, int di); 
	int GetDistanceBetweenStops(const Stop* s1, const Stop*  s2) const;
	const std::vector<detail::DistanceBetweenStopItem>& GetDistanceBetweenStops() const;
	
	const std::unordered_set<std::string>& GetStopNames() const;
	const std::unordered_set<std::string>& GetBusNames() const;
	
private:
	// bus & stop store
	std::deque<Bus> bus_;
	std::deque<Stop> stop_;
	
	// find by name indexes 
	std::unordered_map <std::string_view, const Bus*> bus_index_;
	std::unordered_map <std::string_view, const Stop*> stop_index_;
	
	std::map <size_t, const Bus*> bus_id_index_;
	std::map <size_t, const Stop*> stop_id_index_;
	
	std::unordered_map<const Stop*, detail::StopBuses> stop_buses_;
	detail::DistanceBetweenStop stop_di_; // unique fast use hasher
	std::vector<detail::DistanceBetweenStopItem> distance_between_stops_vector_;
	
	// string store
	std::unordered_set<std::string> stop_names_;
	std::unordered_set<std::string> bus_names_;
	
	std::string_view AddName(const std::string& name, std::unordered_set<std::string>& set);
	
	int GetDistance(const Bus& bus) const;
	double GetGeoLength(const Bus& bus) const;
	double GetCurvature(const Bus& bus) const;
	size_t GetUniqueStopsCount(const Bus& bus) const;
	size_t last_stop_id = 0;
	size_t last_bus_id = 0;
};
} // end ::trans_cat
