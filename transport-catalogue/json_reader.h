#pragma once

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>
#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"

namespace trans_cat {
namespace detail {
namespace parser{
} // end ::parser
} // end ::detail

class InputReaderJSON : public RequestHandlerBase {	
public:	
	InputReaderJSON(TransportCatalogue& trc) : RequestHandlerBase (trc) {	}
	
	void Read(std::istream& is) override;
	void ReadJSON(const json::Node& root);
private:
	json::Node root_;
	std::unordered_set<const json::Dict*> add_stop_queries_;
	std::unordered_set<const json::Dict*> add_bus_queries_;
	
	void ReadQuery(const json::Node& request);
	
	void AddStops(MapDiBetweenStops& stop_di) override;
	void AddBuses() override;
	
	std::vector<const Stop*> ParseStopList(const json::Array& stop_list, bool is_ring);
};

class StatReaderJSON : public RequestHandlerStat {	
public:	
	StatReaderJSON(TransportCatalogue& trc, UserInterface& ui) : RequestHandlerStat(trc, ui) { } 
	void Read(std::istream& is) override;
	void ReadJSON(const json::Node& root);
private:
	json::Node root_;
};

class UserInterfaceJSON : public UserInterface {
public:	
	UserInterfaceJSON(std::ostream& os, TransportCatalogue& trc) : UserInterface(os, trc) {}
	void ShowQueriesResult(RequestHandlerStat::StatQueryList queries) override;
private:	
	void ShowBus(std::string_view bus_name);
	void ShowStopBuses(std::string_view stop);
};
} // end ::trans_cat