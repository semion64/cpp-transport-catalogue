#include "request_handler.h"

namespace trans_cat {
namespace request {
using namespace std::literals;	
//---------------------------HandlerBase--------------------------------------------------------------------------------------------------------------------------

void HandlerBase::DoAndPrint(UserInterface* ui) {
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

const RenderSettings& Manager::GetSettingsMapRenderer() {
	return render_settings_;
}

const RouterSettings& Manager::GetSettingsRouter() {
	return router_settings_;
}

} // end ::request
} // end ::trans_cat

/*
 
 #include "parser.h"

using namespace std;

namespace queries {
	class ComputeIncome : public ComputeQuery {
	public:
		using ComputeQuery::ComputeQuery;
		ReadResult Process(const BudgetManager& budget) const override {
			return { budget.ComputeSum(GetFrom(), GetTo()) };
		}

		class Factory : public QueryFactory {
		public:
			std::unique_ptr<Query> Construct(std::string_view config) const override {
				auto parts = Split(config, ' ');
				return std::make_unique<ComputeIncome>(Date::FromString(parts[0]), Date::FromString(parts[1]));
			}
		};
	};

	class Alter : public ModifyQuery {
	public:
		Alter(Date from, Date to, DayInOut amount)
			: ModifyQuery(from, to)
			, amount_(amount) {
		}

		void Process(BudgetManager& budget) const override {

			DayInOut d;
			d.incoming = amount_.incoming / (Date::ComputeDistance(GetFrom(), GetTo()) + 1);
			//d.outcoming = amount_.outcoming / (Date::ComputeDistance(GetFrom(), GetTo()) + 1);

			budget.AddBulkOperation(GetFrom(), GetTo(), BulkMoneyAdder{d});
		}

		class Factory : public QueryFactory {
		public:
			std::unique_ptr<Query> Construct(std::string_view config) const override {
				auto parts = Split(config, ' ');
				double payload = std::stod(std::string(parts[2]));
				return std::make_unique<Alter>(Date::FromString(parts[0]), Date::FromString(parts[1]), DayInOut{ payload, 0.0 });
			}
		};

	private:
		DayInOut amount_;
	};

	class Spend : public ModifyQuery {
	public:
		Spend(Date from, Date to, DayInOut amount)
			: ModifyQuery(from, to)
			, amount_(amount) {
		}

		void Process(BudgetManager& budget) const override {

			DayInOut d;
			//d.incoming = amount_.incoming / (Date::ComputeDistance(GetFrom(), GetTo()) + 1);
			d.outcoming = amount_.outcoming / (Date::ComputeDistance(GetFrom(), GetTo()) + 1);

			budget.AddBulkOperation(GetFrom(), GetTo(), BulkMoneyAdder{ d });
		}

		class Factory : public QueryFactory {
		public:
			std::unique_ptr<Query> Construct(std::string_view config) const override {
				auto parts = Split(config, ' ');
				double payload = std::stod(std::string(parts[2]));
				return std::make_unique<Spend>(Date::FromString(parts[0]), Date::FromString(parts[1]), DayInOut{ 0.0, payload });
			}
		};

	private:
		DayInOut amount_;
	};

	class PayTax : public ModifyQuery {
	public:
		PayTax(Date from, Date to, double rate)
			: ModifyQuery(from, to)
			, rate_(rate) {
		}

		using ModifyQuery::ModifyQuery;

		void Process(BudgetManager& budget) const override {
			double f = (100 - rate_) / 100.0;
			budget.AddBulkOperation(GetFrom(), GetTo(), BulkTaxApplier{ f });
		}

		class Factory : public QueryFactory {
		public:
			std::unique_ptr<Query> Construct(std::string_view config) const override {
				auto parts = Split(config, ' ');
				if (parts.size() == 2)
					return std::make_unique<PayTax>(Date::FromString(parts[0]), Date::FromString(parts[1]));
				else
					return std::make_unique<PayTax>(Date::FromString(parts[0]), Date::FromString(parts[1]), stoi(string(parts[2])));
			}
		};

	private:
		double rate_ = 13;
	};

}  // namespace queries

const QueryFactory& QueryFactory::GetFactory(std::string_view id) {
	static queries::ComputeIncome::Factory compute_income;
	static queries::Alter::Factory earn;
	static queries::Spend::Factory spend;
	static queries::PayTax::Factory pay_tax;
	static std::unordered_map<std::string_view, const QueryFactory&> factories
		= { {"ComputeIncome"sv, compute_income}, {"Earn"sv, earn}, {"PayTax"sv, pay_tax}, { "Spend"sv, spend } };

	return factories.at(id);
}

 
base_requests (Stop, Bus)
stat_requests (Bus, Stop, Route)
routing_settings
render_settings

const QueryFactory& QueryFactory::GetFactory(std::string_view id) {
	static queries::ComputeIncome::Factory compute_income;
	static queries::Alter::Factory earn;
	static queries::Spend::Factory spend;
	static queries::PayTax::Factory pay_tax;
	static std::unordered_map<std::string_view, const QueryFactory&> factories
		= { {"ComputeIncome"sv, compute_income}, {"Earn"sv, earn}, {"PayTax"sv, pay_tax}, { "Spend"sv, spend } };

	return factories.at(id);
}
 
 */
