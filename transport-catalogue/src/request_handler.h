#pragma once

#include <algorithm>
#include <iostream>
#include <list>
#include <stdexcept>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>
#include <memory>
#include <optional>

#include "domain.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "serialization.h"

namespace trans_cat {
	
class UserInterface {
public:	
	UserInterface(std::ostream& os, TransportCatalogue& trc, TransportRouter* tr_router, MapRenderer* map_renderer) 
		: os_(os), trc_(trc), tr_router_(tr_router), map_renderer_ (map_renderer) { }
	virtual void ShowQueriesResult(const std::list<detail::StatQuery>& queries) const = 0;
protected:
	int ROUTE_STAT_PRECISION = 6;
	std::ostream& os_;
	TransportCatalogue& trc_;
	TransportRouter* tr_router_;
	MapRenderer* map_renderer_;
};


namespace request {

class ExceptionWrongQueryType : public std::logic_error {
public:	
	ExceptionWrongQueryType(std::string what = "") : logic_error(what) { }
};

class ExceptionWrongStatReaderQuery : public std::logic_error{
public:
	ExceptionWrongStatReaderQuery(std::string what = "") : logic_error(what) { }
};

class ExceptionWrongInputFormat : public std::logic_error {
public:	
	ExceptionWrongInputFormat(std::string what = "") : logic_error(what) { }
};

class ExceptionMapRendererNullPtr : public std::invalid_argument {
public:	
	ExceptionMapRendererNullPtr(std::string what = "") : invalid_argument(what) { }
};

class Reader;
class Handler;
class HandlerBase;
class HandlerStat;

class Reader {
public:	
	Reader(TransportCatalogue& trc) : trc_(trc) { }
	virtual void Read(std::istream& is) = 0;
	virtual ~Reader() = default;
protected:
	TransportCatalogue& trc_;
};

class Handler : public Reader {
public:	
	Handler(TransportCatalogue& trc) : Reader(trc) { }
	virtual void DoAndPrint(UserInterface* ui = nullptr) = 0;
};

class HandlerBase : public Handler {
public:	
	HandlerBase(TransportCatalogue& trc) : Handler(trc) {	}
	virtual void DoAndPrint(UserInterface* ui) override;
protected:
	using MapDiBetweenStops = std::unordered_map<std::string_view, std::unordered_map<std::string, int>>;
	virtual void AddStops(MapDiBetweenStops& stop_di) = 0; // function must fill stop_di_ map
	virtual void AddBuses() = 0;
private:
	MapDiBetweenStops stop_di_;
	void AddDistanceBetweenStops();
};

class HandlerStat : public Handler {
public:
	using StatQueryList = std::list<detail::StatQuery>;
	HandlerStat(TransportCatalogue& trc) : Handler(trc) {	}
	virtual void DoAndPrint(UserInterface* ui) override;
protected:
	StatQueryList queries_;
};

template <typename TSettings>
class HandlerSettings : public Reader {
public:
	HandlerSettings(TransportCatalogue& trc) : Reader(trc) { }
	void Set(TSettings&& rs) {
		rs_ = rs;
	}
	TSettings Get() {
		return rs_;
	}
private:
	TSettings rs_;
};
/*
 class Manager : public Reader {
public: 
	Manager(TransportCatalogue& trc) : Reader(trc) { }
	void SetBase(HandlerBase* base);
	void SetStat(HandlerStat* stat);
	void DoBase();
	void DoStat(UserInterface& ui);
	const RenderSettings& GetSettingsMapRenderer() const;
	const RouterSettings& GetSettingsRouter() const;
	const SerializationSettings& GetSettingsSerialization() const;
	~Manager() override { }
protected:

    RenderSettings render_settings_;
    RouterSettings router_settings_;
    SerializationSettings serialization_settings_;
private:
	std::unique_ptr<HandlerBase> handler_base_;
    std::unique_ptr<HandlerStat> handler_stat_;
};
*  
 */
 class ManagerStat : public Reader {
public: 
	ManagerStat(TransportCatalogue& trc) : Reader(trc) { }
	void SetStat(HandlerStat* stat);
	void DoStat(UserInterface& ui);
	const SerializationSettings& GetSettingsSerialization() const;
	~ManagerStat() override { }
protected:
    SerializationSettings serialization_settings_;
private:
    std::unique_ptr<HandlerStat> handler_stat_;
};

class ManagerBase : public Reader {
public: 
	ManagerBase(TransportCatalogue& trc) : Reader(trc) { }
	void SetBase(HandlerBase* base);
	void DoBase();
	const RenderSettings& GetSettingsMapRenderer() const;
	const RouterSettings& GetSettingsRouter() const;
	const SerializationSettings& GetSettingsSerialization() const;
	~ManagerBase() override { }
protected:
    RenderSettings render_settings_;
    RouterSettings router_settings_;
    SerializationSettings serialization_settings_;
private:
	std::unique_ptr<HandlerBase> handler_base_;
};
} // end ::request
} // end ::trans_cat
