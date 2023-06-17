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

namespace trans_cat {
	
namespace detail {
svg::Color ParseColor(const json::Node& node_color);
}// end ::detail
	
class InputReaderJSON;
class StatReaderJSON;
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
		if(!root->IsMap()) {
			throw ExceptionWrongQueryType("json root node has unsupported format");
		}
		
		if(request_name.empty()) { 
			root_ = root;
			return;
		}
		
		const auto& root_map = root->AsMap();
		if(!root_map.count(request_name)) {
			throw ExceptionWrongQueryType("request node (" + std::string(request_name) + ") not find");
		}
		
		root_ = &root_map.at(request_name);		
	}
protected:
	const json::Node* root_;
private:
	json::Document doc_ {nullptr};
};

class InputReaderJSON : public RequestHandlerBase, public LoaderJSON {	
public:	
	InputReaderJSON(TransportCatalogue& trc) : RequestHandlerBase (trc) {	}
	
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

class StatReaderJSON : public RequestHandlerStat, public LoaderJSON {	
public:	
	StatReaderJSON(TransportCatalogue& trc, UserInterface* ui) : RequestHandlerStat(trc, ui) { } 
	void Read(std::istream& is) override { ReadFromStream(is); }
	void Read(const json::Node* root) override;
};

class RenderSettingsJSON : public RequestHandlerRenderSettings, public LoaderJSON {
public:
	RenderSettingsJSON(TransportCatalogue& trc) : RequestHandlerRenderSettings(trc) {	}
	void Read(std::istream& is) override { ReadFromStream(is); }
	void Read(const json::Node* root) override;
	void Do() override {}
};

class UserInterfaceJSON : public UserInterface {
public:	
	UserInterfaceJSON(std::ostream& os, TransportCatalogue& trc, MapRenderer* map_renderer = nullptr) : UserInterface(os, trc, map_renderer) {}
	void ShowQueriesResult(const RequestHandlerStat::StatQueryList& queries) const override;
private:	
	void ShowBus(std::string_view bus_name) const;
	void ShowStopBuses(std::string_view stop_name) const;
	void ShowMap() const;
};

class RequestManagerJSON : public RequestManager, public LoaderJSON {
public: 
	RequestManagerJSON(TransportCatalogue& trc, UserInterface* ui) : RequestManager(trc, ui) {	}
	void Read(std::istream& is) override { ReadFromStream(is); }
	void Read(const json::Node* root) override;	
};

} // end ::trans_cat
