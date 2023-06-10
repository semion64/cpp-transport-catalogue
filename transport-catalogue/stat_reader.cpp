#include "stat_reader.h"

namespace trans_cat {

void StatReaderStd::Read(std::istream& is) {
	int N;
	is >> N;
	while(N >= 0) {
		std::string line;
		std::getline(is, line);
		auto [type_str, name]= detail::parser::Split(line, ' ');
		queries_.push_back({0, StatQuery::GetType(type_str), std::string(name)});
		--N;
	}
}

void UserInterfaceStd::ShowBus(std::string_view bus_name) {
	os_ << std::setprecision(ROUTE_STAT_PRECISION);
	
	try {
 		const auto& bus = trc_.GetBus(bus_name);
		const auto& route = trc_.GetRouteStat(bus);
 		os_ << "Bus " << bus.name << ": ";
		os_	<< route.stops_count << " stops on route, " 
					<< route.unique_stops << " unique stops, " 
					<< static_cast<double>(route.distance) << " route length, "
					<< route.curvature << " curvature"; 
	}
	catch(ExceptionBusNotFound) {
		os_ << "Bus " << bus_name << ": not found";
	}
	
	os_ << std::endl;
}

void UserInterfaceStd::ShowStopBuses(std::string_view stop_name) {
	os_ << std::setprecision(ROUTE_STAT_PRECISION);
	
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
	
	os_ << std::endl;
	
}
} // end ::trans_cat