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
	void DoQueries() override;
protected:
	UserInterface& ui_;
	StatQueryList queries_;
};

class UserInterface {
public:	
	UserInterface(std::ostream& os, TransportCatalogue& trc) : os_(os), trc_(trc)  {}
	virtual void ShowQueriesResult(RequestHandlerStat::StatQueryList queries) = 0;
protected:
	int ROUTE_STAT_PRECISION = 6;
	std::ostream& os_;
	TransportCatalogue& trc_;
};

} // end ::trans_cat
