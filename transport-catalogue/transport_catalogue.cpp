#include "transport_catalogue.h"

namespace trans_cat {

void TransportCatalogue::AddStop(Stop&& stop) {
	stop_.push_back(stop);
	stop_index_[stop_.back().name] = &stop_.back();
}

void TransportCatalogue::SetDistance(const Stop* s1, const Stop* s2, int di) {
	stop_di_[{s1, s2}] = di;
}

void TransportCatalogue::AddBus(Bus&& bus) {
	// add bus
	bus_.push_back(bus);
	const Bus* insert_bus = &bus_.back();
	
	// add bus to index map
	bus_index_[bus.name] = insert_bus;
	
	// add inserted bus to stop_buses_ map
	std::for_each(insert_bus->stops.begin(), insert_bus->stops.end(), [this, &insert_bus](const auto stop) {
		stop_buses_[stop_index_[stop->name]].insert(insert_bus);
	});
}

const Bus& TransportCatalogue::GetBus(std::string_view bus_name) const {
	if(!bus_index_.count(bus_name)) {
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

void TransportCatalogue::ImportStopNames(std::unordered_set<std::string>&& stop_names) {
	stop_names_ = std::move(stop_names);
}

void TransportCatalogue::ImportBusNames(std::unordered_set<std::string>&& bus_names) {
	bus_names_ = std::move(bus_names);
}

UserInterface::UserInterface(TransportCatalogue& trc, std::ostream& os) : trc_(trc), os_(os) { }

void UserInterface::ShowBus(std::string_view bus_name, bool end_line) {
	os_ << std::setprecision(6);
	
	try {
 		const auto bus = trc_.GetBus(bus_name);
		os_ << "Bus " << bus.name << ": ";
		os_	<< bus.StopsCount() << " stops on route, " 
					<< bus.UniqueStopsCount() << " unique stops, " 
					<< static_cast<double>(trc_.GetDistance(bus)) << " route length, "
					<< trc_.GetCurvature(bus) << " curvature"; 
	}
	catch(ExceptionBusNotFound) {
		os_ << "Bus " << bus_name << ": not found";
	}
	
	EndLine(end_line);
}

void UserInterface::ShowStopBuses(std::string_view stop_name, bool end_line) {
	os_ << std::setprecision(6);
	
	try {
 		const auto& stop_buses = trc_.GetStopBuses(trc_.GetStop(stop_name));
		os_ << "Stop " << stop_name << ": buses ";
		bool is_first = false;
		for(const auto& bus : stop_buses) {
			if(!is_first) {
				is_first = true;
			}
			else {
				os_ << " ";
			}
			os_ << bus->name;
		}
	}
	catch(ExceptionBusNotFound) {
		os_ << "Stop " << stop_name << ": no buses";
	}
	catch(ExceptionStopNotFound) {
		os_ << "Stop " << stop_name << ": not found";
	}
	
	EndLine(end_line);
}

void UserInterface::EndLine(bool end_line) {
	if(end_line) {
		os_ << std::endl;
	}
}
} // end ::trans_cat
