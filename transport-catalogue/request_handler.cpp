#include "request_handler.h"

namespace trans_cat {
using namespace std::literals;	
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
	ui_->ShowQueriesResult(queries_);
}

//---------------------------RequestManager--------------------------------------------------------------------------------------------------------------------------
void RequestManager::DoBase() {
	handler_base_->Do();
}

void RequestManager::DoStat() {
	handler_stat_->Do();
}

RenderSettings RequestManager::GetSettingsMapRenderer() {
	return handler_render_->GetRenderSettings();
}

RouterSettings RequestManager::GetSettingsRouter() {
	return handler_router_->GetRouterSettings();
}

} // end ::trans_cat
