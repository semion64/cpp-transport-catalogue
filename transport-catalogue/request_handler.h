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

#include "domain.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

namespace trans_cat {
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

struct RouterSettings {
	int bus_wait_time;
	double bus_velocity;
};

class UserInterface;
class RequestReader;
class RequestHandler;
class RequestHandlerBase;
class RequestHandlerStat;

class UserInterface {
public:	
	UserInterface(std::ostream& os, TransportCatalogue& trc, MapRenderer* map_renderer = nullptr) : os_(os), trc_(trc), map_renderer_(map_renderer)  {}
	virtual void ShowQueriesResult(const std::list<detail::StatQuery>& queries) const = 0;
protected:
	int ROUTE_STAT_PRECISION = 6;
	std::ostream& os_;
	TransportCatalogue& trc_;
	MapRenderer* map_renderer_;
};

class RequestReader {
public:	
	RequestReader(TransportCatalogue& trc) : trc_(trc) { }
	virtual void Read(std::istream& is) = 0;
	virtual ~RequestReader() = default;
protected:
	TransportCatalogue& trc_;
};

class RequestHandler : public RequestReader {
public:	
	RequestHandler(TransportCatalogue& trc) : RequestReader(trc) { }
	virtual void Do() = 0;
};

class RequestHandlerBase : public RequestHandler {
public:	
	RequestHandlerBase(TransportCatalogue& trc) : RequestHandler(trc) {	}
	void Do() override;
protected:
	using MapDiBetweenStops = std::unordered_map<std::string_view, std::unordered_map<std::string, int>>;
	virtual void AddStops(MapDiBetweenStops& stop_di) = 0; // function must fill stop_di_ map
	virtual void AddBuses() = 0;
private:
	MapDiBetweenStops stop_di_;
	void AddDistanceBetweenStops();
};

class RequestHandlerStat : public RequestHandler {
public:
	using StatQueryList = std::list<detail::StatQuery>;
	RequestHandlerStat(TransportCatalogue& trc, UserInterface* ui) : RequestHandler(trc), ui_(ui) {	}
	void Do() override;
protected:
	UserInterface* ui_;
	StatQueryList queries_;
};

class RequestHandlerRenderSettings : public RequestHandler {
public:
	RequestHandlerRenderSettings(TransportCatalogue& trc) : RequestHandler(trc) {	}
	RenderSettings GetRenderSettings() {
		return rs_;
	}
protected:
	RenderSettings rs_;
};

class RequestHandlerRouterSettings : public RequestHandler {
public:
	RequestHandlerRouterSettings(TransportCatalogue& trc) : RequestHandler(trc) {	}
	RouterSettings GetRouterSettings() {
		return rs_;
	}
protected:
	RouterSettings rs_;
};

class RequestManager : public RequestReader {
public: 
	RequestManager(TransportCatalogue& trc, UserInterface* ui) : RequestReader(trc), ui_(ui) { }
	void DoBase();
	void DoStat();
	RenderSettings GetSettingsMapRenderer();
	~RequestManager() override {
		if(handler_base_) delete handler_base_;
		if(handler_stat_) delete handler_stat_;
		if(handler_render_) delete handler_render_;
		if(handler_router_) delete handler_router_;
	}
protected:
	UserInterface* ui_;
	RequestHandlerBase* handler_base_;
    RequestHandlerStat* handler_stat_;
    RequestHandlerRenderSettings* handler_render_;
    RequestHandlerRouterSettings* handler_router_;
};
} // end ::trans_cat
