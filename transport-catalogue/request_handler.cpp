#include "request_handler.h"

namespace trans_cat {
	
void RequestHandlerBase::DoQueries() {
	AddStops();
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

} // end ::trans_cat
