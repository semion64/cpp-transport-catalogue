#include "request_handler.h"

namespace trans_cat {
namespace request {
using namespace std::literals;	
//---------------------------HandlerBase--------------------------------------------------------------------------------------------------------------------------

	
void HandlerBase::DoAndPrint([[maybe_unused]]UserInterface* ui) {
	AddStops(stop_di_);
	AddBuses();
	AddDistanceBetweenStops();
}

void HandlerBase::AddDistanceBetweenStops() {
	for(const auto& from_stop : trc_.GetStops()) {
		for(const auto& [to_stop, di]: stop_di_[from_stop.name]) { 
			trc_.SetDistance(&trc_.GetStop(from_stop.name), &trc_.GetStop(to_stop), di);
		}
	}
	
	stop_di_.clear();
}

//---------------------------HandlerStat--------------------------------------------------------------------------------------------------------------------------

void HandlerStat::DoAndPrint(UserInterface* ui_) {
	ui_->ShowQueriesResult(queries_);
}

//---------------------------Manager--------------------------------------------------------------------------------------------------------------------------
void Manager::DoBase() {
	handler_base_->DoAndPrint(nullptr);
}

void Manager::DoStat(UserInterface& ui) {
	handler_stat_->DoAndPrint(&ui);
}
void Manager::SetBase(HandlerBase* base) {
	handler_base_.reset(base);
}
void Manager::SetStat(HandlerStat* stat) {
	handler_stat_.reset(stat);
}

const RenderSettings& Manager::GetSettingsMapRenderer() {
	return render_settings_;
}

const RouterSettings& Manager::GetSettingsRouter() {
	return router_settings_;
}

} // end ::request
} // end ::trans_cat
