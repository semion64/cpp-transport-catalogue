#include "request_handler.h"

namespace trans_cat {
	
void RequestHandlerBase::DoQueries() {
	AddStops(stop_di_);
	AddBuses();
	AddDistanceBetweenStops();
}

void RequestHandlerStat::DoQueries() {
	for(const auto& q: queries_) {
		if(q.type == StatQueryType::BUS) {
			ui_.ShowBus(q.name);	
		}
		else if(q.type == StatQueryType::STOP) {
			ui_.ShowStopBuses(q.name);	
		}
	}
}

StatQueryType StatQuery::GetType(std::string_view type_str) {
	if(type_str == "Bus") {
		return StatQueryType::BUS;
	}
	else if(type_str == "Stop") {
		return StatQueryType::STOP;
	}
	return StatQueryType::NONE;
	//throw ExceptionWrongStatReaderQuery("incorrect query type: " + std::string(type_str));
}

void RequestHandlerBase::AddDistanceBetweenStops() {
	for(const auto& from_stop : trc_.GetStops()) {
		for(const auto& [to_stop, di]: stop_di_[from_stop.name]) { 
			trc_.SetDistance(&trc_.GetStop(from_stop.name), &trc_.GetStop(to_stop), di);
		}
	}
	
	stop_di_.clear();
}
} // end ::trans_cat
