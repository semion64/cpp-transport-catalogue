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
void ManagerBase::DoBase() {
	handler_base_->DoAndPrint(nullptr);
}

void ManagerStat::DoStat(UserInterface& ui) {
	handler_stat_->DoAndPrint(&ui);
}

void ManagerBase::SetBase(HandlerBase* base) {
	handler_base_.reset(base);
}
void ManagerStat::SetStat(HandlerStat* stat) {
	handler_stat_.reset(stat);
}

const RenderSettings& ManagerBase::GetSettingsRender() const {
	return render_settings_;
}



const RouterSettings& ManagerBase::GetSettingsRouter() const {
	return router_settings_;
}

const serialize::Settings& ManagerBase::GetSettingsSerialization() const {
	return serialization_settings_;
}

const serialize::Settings& ManagerStat::GetSettingsSerialization() const {
	return serialization_settings_;
}

} // end ::request
} // end ::trans_cat
