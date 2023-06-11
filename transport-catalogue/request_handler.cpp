#include "request_handler.h"

namespace trans_cat {
	
//---------------------------RequestHandlerBase--------------------------------------------------------------------------------------------------------------------------

void RequestHandlerBase::DoQueries() {
	AddStops(stop_di_);
	AddBuses();
	AddDistanceBetweenStops();
}

void RequestHandlerBase::AddDistanceBetweenStops() {
	for(const auto& from_stop : trc_.GetStops()) {
		for(const auto& [to_stop, di]: stop_di_[from_stop.name]) { 
			trc_.SetDistance(&trc_.GetStop(from_stop.name), &trc_.GetStop(to_stop), di);
		}
	}
	
	stop_di_.clear();
}

//---------------------------RequestHandlerStat--------------------------------------------------------------------------------------------------------------------------

void RequestHandlerStat::DoQueries() {
	ui_.ShowQueriesResult(queries_);
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

//---------------------------RequestHandlerBaseStat--------------------------------------------------------------------------------------------------------------------------


RequestHandlerBaseStat::RequestHandlerBaseStat(RequestHandlerBase* handler_base, RequestHandlerStat* handler_stat) 
	: handler_base_(handler_base), handler_stat_(handler_stat){ }
	
void RequestHandlerBaseStat::DoBaseQueries() {
	handler_base_->DoQueries();
}

void RequestHandlerBaseStat::DoStatQueries() {
	handler_stat_->DoQueries();
}

void RequestHandlerBaseStat::DoQueries() {
	DoBaseQueries();
	DoStatQueries();
}

} // end ::trans_cat
