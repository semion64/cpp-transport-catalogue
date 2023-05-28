#include "transport_catalogue.h"

namespace trans_cat {
int TransportCatalogue::RouteDi(std::vector<const Stop*>& stops) const {
	int di = 0;
	for(size_t i = 0; i < stops.size() - 1; ++i) {
		di += GetDistance(stops[i], stops[i + 1]);
	}
	
	return di;
}

void TransportCatalogue::AddStop(const detail::QueryStop& stop) {
	stop_.push_back({ stop.name, {stop.lat, stop.lng} });
	stop_index_[stop.name] = &stop_.back();
}

void TransportCatalogue::SetDistance(const Stop* s1, const Stop* s2, int di) {
	stop_di_[{s1, s2}] = di;
}

int TransportCatalogue::GetDistance(const Stop* s1, const Stop*  s2) const {
	if(stop_di_.count({s1, s2})) {
		return stop_di_.at({s1, s2});
	}
	
	return stop_di_.at({s2, s1});
}

void TransportCatalogue::AddBus(const detail::QueryBus& bus) {
	// create real stop vector from string_view names
	std::vector<const Stop*> stops;
	stops.reserve(bus.stops.size());
	std::for_each(bus.stops.begin(), bus.stops.end(), [&stops, this](auto& stop_name) {
			stops.push_back(stop_index_[stop_name]);
		});
	
	// calc geo_length	
	double geo_length = 0;
	for(size_t i = 0; i < stops.size() - 1; ++i) {
		geo_length += geo::ComputeDistance(stops[i]->coord, stops[i + 1]->coord);
	}
	
	auto stops_view = bus.stops;
	// make unique stops vector
	std::sort(stops_view.begin(), stops_view.end());	
	stops_view.erase(std::unique(stops_view.begin(), stops_view.end()), stops_view.end());

	// add bus
	bus_.push_back({ bus.name, {stops, stops_view.size(), geo_length, RouteDi(stops), bus.is_ring} });
	const Bus* insert_bus = &bus_.back();
	
	// add bus to index map
	bus_index_[bus.name] = insert_bus;
	
	// add inserted bus to stop_buses_ map
	std::for_each(stops_view.begin(), stops_view.end(), [this, &insert_bus](const auto stop_name) {
			stop_buses_[stop_index_[stop_name]].insert(insert_bus);
		});
}

void TransportCatalogue::Import(InputReader& ir) {
	// import stops
	std::vector<detail::QueryStop> stop_query;
	stop_names_ = std::move(ir.ExportStops(stop_query));
	for(auto& stop : stop_query) {
		AddStop(stop);
	}
	
	// add distance between stops
	for(auto& from_stop : stop_query) {
		for(auto& [to_stop, di]: from_stop.stop_di) {
			SetDistance(&GetStop(from_stop.name), &GetStop(to_stop), di);
		}
	}

	// import buses
	std::vector<detail::QueryBus> bus_query;
	bus_names_ = std::move(ir.ExportBuses(bus_query));
	for(auto& bus : bus_query) {
		AddBus(bus);
	}
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

const detail::StopBuses& TransportCatalogue::GetStopBuses(const Stop& stop) const {
	if(!stop_buses_.count(&stop)) {
		throw ExceptionBusNotFound();
	}
	
	return stop_buses_.at(&stop);
}

UserInterface::UserInterface(TransportCatalogue& trc, std::ostream& os) : trc_(trc), os_(os) { }

void UserInterface::ShowBus(std::string_view bus_name, bool end_line) {
	os_ << std::setprecision(6);
	
	try {
 		const auto bus = trc_.GetBus(bus_name);
		os_ << "Bus " << bus.name << ": ";
		os_	<< bus.route.size() << " stops on route, " 
					<< bus.route.unique_stops << " unique stops, " 
					<< static_cast<double>(bus.route.distance) << " route length, "
					<< bus.route.curvature() << " curvature"; 
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
