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
#include "map_renderer.h"
#include "json_builder.h"
namespace trans_cat {
	
namespace detail {
svg::Color ParseColor(const json::Node& node_color);
}// end ::detail
	
class BaseJSON;
class StatJSON;
class UserInterfaceJSON;

class LoaderJSON {
public:
	LoaderJSON() {}
	virtual void Read(const json::Node* root) = 0;
protected:
	void ReadFromStream(std::istream& is) {
		doc_ = json::Load(is);
		Read(&doc_.GetRoot());
	}

	void ReadFromJSON(const json::Node* root, std::string request_name = "") {
		if(!root->IsDict()) {
			throw request::ExceptionWrongQueryType("json root node has unsupported format");
		}
		
		if(request_name.empty()) { 
			root_ = root;
			return;
		}
		
		const auto& root_map = root->AsDict();
		if(!root_map.count(request_name)) {
			throw request::ExceptionWrongQueryType("request node (" + std::string(request_name) + ") not find");
		}
		
		root_ = &root_map.at(request_name);		
	}
protected:
	const json::Node* root_;
private:
	json::Document doc_ {nullptr};
};

class BaseJSON : public request::HandlerBase, public LoaderJSON {	
public:	
	BaseJSON(TransportCatalogue& trc) : request::HandlerBase (trc) {	}
	
	void Read(std::istream& is) override { ReadFromStream(is); }
	void Read(const json::Node* root) override;
private:
	std::unordered_set<const json::Dict*> add_stop_queries_;
	std::unordered_set<const json::Dict*> add_bus_queries_;
	void ReadQuery(const json::Node& request);
	void AddStops(MapDiBetweenStops& stop_di) override;
	void AddBuses() override;
	std::vector<const Stop*> ParseStopList(const json::Array& stop_list, bool is_ring);
};

class StatJSON : public request::HandlerStat, public LoaderJSON {	
public:	
	StatJSON(TransportCatalogue& trc) : request::HandlerStat(trc) { } 
	void Read(std::istream& is) override { ReadFromStream(is); }
	void Read(const json::Node* root) override;
};

class RenderSettingsJSON : public request::HandlerSettings<RenderSettings>, public LoaderJSON {
public:
	RenderSettingsJSON(TransportCatalogue& trc) : request::HandlerSettings<RenderSettings>(trc) {	}
	void Read(std::istream& is) override { ReadFromStream(is); }
	void Read(const json::Node* root) override;
};

class RouterSettingsJSON : public request::HandlerSettings<RouterSettings>, public LoaderJSON {
public:
	RouterSettingsJSON(TransportCatalogue& trc) : request::HandlerSettings<RouterSettings>(trc) {	}
	void Read(std::istream& is) override { ReadFromStream(is); }
	void Read(const json::Node* root) override;
};

class UserInterfaceJSON : public UserInterface {
public:	
	UserInterfaceJSON(std::ostream& os, TransportCatalogue& trc, TransportRouter& tr_router, MapRenderer& map_renderer) 
		: UserInterface(os, trc, tr_router, map_renderer) { }	
	void ShowQueriesResult(const request::HandlerStat::StatQueryList& queries) const override;
private:	
	void ShowBus(std::string_view bus_name) const;
	void ShowStopBuses(std::string_view stop_name) const;
	void ShowMap() const;
	void ShowRoute(const TransportRouter& tr_router, graph::Router<RouteItem>& router, std::string_view from, std::string_view to) const;
	mutable json::Builder json_build_;
};

class ManagerJSON : public request::Manager, public LoaderJSON {
public: 
	ManagerJSON(TransportCatalogue& trc) : request::Manager(trc) {	}
	void Read(std::istream& is) override { ReadFromStream(is); }
	void Read(const json::Node* root) override;	
};

} // end ::trans_cat
