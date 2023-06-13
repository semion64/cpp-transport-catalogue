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

enum class StatQueryType {
	NONE,
	BUS,
	STOP
};

struct StatQuery {
	int id;
	StatQueryType type;
	std::string name;
	
	static StatQueryType GetType(std::string_view type_str);
};

class UserInterface;
class RequestReader;
class RequestHandler;
class RequestHandlerBase;
class RequestHandlerStat;

class UserInterface {
public:	
	UserInterface(std::ostream& os, TransportCatalogue& trc) : os_(os), trc_(trc)  {}
	virtual void ShowQueriesResult(const std::list<StatQuery>& queries) const = 0;
protected:
	int ROUTE_STAT_PRECISION = 6;
	std::ostream& os_;
	TransportCatalogue& trc_;
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
	using StatQueryList = std::list<StatQuery>;
	RequestHandlerStat(TransportCatalogue& trc, UserInterface& ui) : RequestHandler(trc), ui_(ui) {	}
	void Do() override;
protected:
	UserInterface& ui_;
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

class RequestManager : public RequestReader {
public: 
	RequestManager(TransportCatalogue& trc, UserInterface& ui) : RequestReader(trc), ui_(ui) { }
	void DoBase();
	void DoStat();
	RenderSettings DoRenderSettings();
	~RequestManager() override {
		if(handler_base_) delete handler_base_;
		if(handler_stat_) delete handler_stat_;
		if(handler_render_) delete handler_render_;
	}
protected:
	UserInterface& ui_;
	RequestHandlerBase* handler_base_;
    RequestHandlerStat* handler_stat_;
    RequestHandlerRenderSettings* handler_render_;
};
/*
class RequestHandlerMap {
public: 
	using HandlerMap = std::unordered_map<std::string, RequestHandler*>;
	RequestHandlerMap(HandlerMap handler_map) : handler_map_ (handler_map) { }
	void AddHandler(std::pair<std::string, RequestHandler*>& handler) {
		handler_map_.insert(handler); 
	}
	
	void DoQueries(std::string_view handler_key) {
		handler_map_.[handler_key].DoQueries(); 
	}
	
	RequestHandler* GetHandler(std::string_view handler_key) {
		handler_map_.[handler_key]; 
	}
	
private:
	HandlerMap handler_map_;
};*/
} // end ::trans_cat
