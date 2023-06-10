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

namespace trans_cat {
class ExceptionWrongQueryType : public std::logic_error {
public:	
	ExceptionWrongQueryType(std::string what = "") : logic_error(what) { }
};

class ExceptionWrongStatReaderQuery : public std::logic_error{
public:
	ExceptionWrongStatReaderQuery(std::string what = "") : logic_error(what) { }
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

class RequestHandler {
public:	
	RequestHandler (TransportCatalogue& trc) : trc_ (trc) {	}
	
	virtual void Read(std::istream& is) = 0;
	virtual void DoQueries() = 0;
	
	virtual ~RequestHandler() = default;
protected:
	TransportCatalogue& trc_;
};

class RequestHandlerBase : public RequestHandler {
public:	
	RequestHandlerBase(TransportCatalogue& trc) : RequestHandler(trc) {	}
	void DoQueries() override;
protected:
	virtual void AddStops() = 0;
	virtual void AddBuses() = 0;
	virtual void AddDistanceBetweenStops() = 0;
};

class RequestHandlerStat : public RequestHandler {
public:	
	RequestHandlerStat(TransportCatalogue& trc, UserInterface& ui) : RequestHandler(trc), ui_(ui) {	}
	void DoQueries() override;
protected:
	UserInterface& ui_;
	std::list<StatQuery> queries_;
};

class UserInterface {
public:	
	UserInterface(std::ostream& os, TransportCatalogue& trc) : os_(os), trc_(trc)  {}
	virtual void ShowBus(std::string_view bus_name) = 0;
	virtual void ShowStopBuses(std::string_view stop) = 0;
protected:
	const int ROUTE_STAT_PRECISION = 6;
	std::ostream& os_;
	TransportCatalogue& trc_;
};

} // end ::trans_cat
