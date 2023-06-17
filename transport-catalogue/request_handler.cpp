#include "request_handler.h"

namespace trans_cat {
	
//---------------------------RequestHandlerBase--------------------------------------------------------------------------------------------------------------------------

void RequestHandlerBase::Do() {
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

void RequestHandlerStat::Do() {
	ui_.ShowQueriesResult(queries_);
}

StatQueryType StatQuery::GetType(std::string_view type_str) {
	if(type_str == "Bus") {
		return StatQueryType::BUS;
	}
	else if(type_str == "Stop") {
		return StatQueryType::STOP;
	}
	else if(type_str == "Map") {
		return StatQueryType::MAP;
	}
	
	return StatQueryType::NONE;
	//throw ExceptionWrongStatReaderQuery("incorrect query type: " + std::string(type_str));
}

//---------------------------RequestManager--------------------------------------------------------------------------------------------------------------------------
void RequestManager::DoBase() {
	handler_base_->Do();
}

void RequestManager::DoStat() {
	handler_stat_->Do();
}

RenderSettings RequestManager::GetSettingsMapRenderer() {
	handler_render_->Do();
	return handler_render_->GetRenderSettings();
}

} // end ::trans_cat
