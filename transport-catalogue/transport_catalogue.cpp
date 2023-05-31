#include "transport_catalogue.h"

namespace trans_cat {

Stop& TransportCatalogue::AddStop(std::string name, const geo::Coordinates&& coord) {
	stop_.push_back({AddName(std::move(name), stop_names_), std::move(coord)});
	stop_index_[stop_.back().name] = &stop_.back();
	return stop_.back();
}

Bus& TransportCatalogue::AddBus(std::string name, std::vector<const Stop*>&& stops, bool is_ring) {
	// add bus
	bus_.push_back({AddName(std::move(name), bus_names_), std::move(stops), is_ring});
	Bus* insert_bus = &bus_.back();
	
	// add bus to index map
	bus_index_[insert_bus->name] = insert_bus;
	
	// add inserted bus to stop_buses_ map
	std::for_each(insert_bus->stops.begin(), insert_bus->stops.end(), [this, &insert_bus](const auto stop) {
		stop_buses_[stop_index_[stop->name]].insert(insert_bus);
	});
	
	return *insert_bus;
}

std::string_view  TransportCatalogue::AddName(std::string&& name, std::unordered_set<std::string>& set) {
	return *(set.insert(name).first);
}

void TransportCatalogue::SetDistance(const Stop* s1, const Stop* s2, int di) {
	stop_di_[{s1, s2}] = di;
}

const Bus& TransportCatalogue::GetBus(std::string_view bus_name) const {
	if(!bus_index_.count(bus_name)) {
		// исключение перехватывается в методе UserInterface::ShowStopBuses, чтобы вывести информацию, что не существует автобуса или остановки
		throw ExceptionBusNotFound();
	}
	 
	return *bus_index_.at(bus_name);
}

const Stop& TransportCatalogue::GetStop(std::string_view stop_name) const {
	if(!stop_index_.count(stop_name)) {
		
		throw ExceptionStopNotFound(std::string(stop_name));
	}
	
	return *stop_index_.at(stop_name);
}

const std::deque<Bus>& TransportCatalogue::GetBuses() const {
	return bus_;
}

const std::deque<Stop>& TransportCatalogue::GetStops() const {
	return stop_;
}

const detail::StopBuses& TransportCatalogue::GetStopBuses(const Stop& stop) const {
	if(!stop_buses_.count(&stop)) {
		throw ExceptionBusNotFound();
	}
	
	return stop_buses_.at(&stop);
}

int TransportCatalogue::GetDistance(const Bus& bus) const {
	int di = 0;
	for(size_t i = 0; i < bus.stops.size() - 1; ++i) {
		di += GetDistanceBetweenStops(bus.stops[i], bus.stops[i + 1]);
	}
	
	return di;
}

double TransportCatalogue::GetGeoLength(const Bus& bus) const {
	double geo_length = 0;
	for(size_t i = 0; i < bus.stops.size() - 1; ++i) {
		geo_length += geo::ComputeDistance(bus.stops[i]->coord, bus.stops[i + 1]->coord);
	}
	return geo_length;
}

double TransportCatalogue::GetCurvature(const Bus& bus) const {
	return GetDistance(bus) / GetGeoLength(bus);
}

int TransportCatalogue::GetDistanceBetweenStops(const Stop* s1, const Stop*  s2) const {
	if(stop_di_.count({s1, s2})) {
		return stop_di_.at({s1, s2});
	}
	
	return stop_di_.at({s2, s1});
}

size_t TransportCatalogue::GetUniqueStopsCount(const Bus& bus) const {
	auto temp = bus.stops;
	std::sort(temp.begin(), temp.end());	
	return std::unique(temp.begin(), temp.end()) - temp.begin();
}
	
const detail::RouteStat TransportCatalogue::GetRouteStat(const Bus& bus) const {
	return {
		bus.stops.size(),
		GetUniqueStopsCount(bus),
		GetDistance(bus),
		GetCurvature(bus),
		bus.is_ring
	};
}
} // end ::trans_cat
