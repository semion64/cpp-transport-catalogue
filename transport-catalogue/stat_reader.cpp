#include "stat_reader.h"

namespace trans_cat {
using namespace std::literals;

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

void UserInterfaceStd::ShowQueriesResult(const RequestHandlerStat::StatQueryList& queries) const {
	for(const auto& q: queries) {
		switch (q.type) {
			case StatQueryType::BUS:
				ShowBus(q.name);
			break;
			case StatQueryType::STOP:
				ShowStopBuses(q.name);
			break;
			default:
				//throw ExceptionWrongQueryType("");
			break;
		}
	}
	os_ << std::endl;
}

void UserInterfaceStd::ShowBus(std::string_view bus_name) const {
	os_ << std::setprecision(ROUTE_STAT_PRECISION);
	
	try {
 		const auto& bus = trc_.GetBus(bus_name);
		const auto& route = trc_.GetRouteStat(bus);
 		os_ << "Bus "sv << bus.name << ": "sv;
		os_	<< route.stops_count << " stops on route, "sv 
					<< route.unique_stops << " unique stops, "sv 
					<< static_cast<double>(route.distance) << " route length, "sv
					<< route.curvature << " curvature"sv; 
	}
	catch(ExceptionBusNotFound&) {
		os_ << "Bus "sv << bus_name << ": not found"sv;
	}
}

void UserInterfaceStd::ShowStopBuses(std::string_view stop_name) const {
	os_ << std::setprecision(ROUTE_STAT_PRECISION);
	
	try {
 		const auto& stop_buses = trc_.GetStopBuses(trc_.GetStop(stop_name));
		os_ << "Stop "sv << stop_name << ": buses "sv;
		bool is_first = false;
		for(const auto& bus : stop_buses) {
			if(!is_first) {
				is_first = true;
			}
			else {
				os_ << " "sv;
			}
			os_ << bus->name;
		}
	}
	catch(ExceptionBusNotFound&) {
		os_ << "Stop "sv << stop_name << ": no buses"sv;
	}
	catch(ExceptionStopNotFound&) {
		os_ << "Stop "sv << stop_name << ": not found"sv;
	}	
}

void RequestManagerSTD::Read(std::istream& is) {
	handler_base_ = new InputReaderStd(trc_); 
	handler_stat_ = new StatReaderStd(trc_, ui_); 
	handler_render_ = new RenderSettingsStd(trc_);
	
	handler_base_->Read(is);
	handler_stat_->Read(is);
	handler_render_->Read(is);
}

} // end ::trans_cat
