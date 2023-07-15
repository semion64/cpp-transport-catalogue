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
#include "router.h"

namespace trans_cat {

enum class RouteItemType {
	NONE,
	WAIT,
	BUS
};
struct RouteItem {
	RouteItemType type;
	double time;
	std::string_view name;
	int span;
	bool operator<(const RouteItem& other) const {
		return time < other.time;
	}
	
	bool operator==(const RouteItem& other) const {
		return time == other.time;
	}

	bool operator!=(const RouteItem& other) const {
		return !(time == other.time);
	}

	inline bool operator>(const RouteItem& other) const {
		return !(time < other.time) && time != other.time;
	}
};

inline RouteItem operator+(const RouteItem& l, const RouteItem& r) {
	return RouteItem {RouteItemType::NONE, l.time + r.time, "", 0};
}

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

class RouterBuilder {
public:
	RouterBuilder(TransportCatalogue& trc) : trc_(trc) { }
	RouterBuilder(TransportCatalogue& trc, const RouterSettings& router_settings) : trc_(trc), rs_(router_settings) { }
	//virtual void RouterBuilder(std::ostream& os_) = 0;
	//virtual ~RouterBuilder() = default;
	
	virtual void SetRouterSettings(const RouterSettings& rs) {
		rs_ = rs;
	}
	
	void BuildGraph() {
		gr = graph::DirectedWeightedGraph<RouteItem>(trc_.GetStops().size() * 2);
		std::cout << "bus_velocity: " << rs_.bus_velocity << std::endl;
		std::cout << "bus_wait_time: " << rs_.bus_wait_time << std::endl;
		for(const auto& bus : trc_.GetBuses()) {
			if(bus.is_ring) {
				for(int i = 0; i < bus.stops.size() - 1; ++i) {
					size_t from_id = GetVertexWaitId(bus.stops[i]->id);
					gr.AddEdge(graph::Edge<RouteItem> {
						bus.stops[i]->id, 
						from_id, 
						RouteItem { 
							RouteItemType::WAIT, 
							rs_.bus_wait_time, 
							bus.stops[i]->name, 
							0
						} 
					});
					
					int di = 0;
					for(int j = i + 1; j < bus.stops.size(); ++j) {
						di += trc_.GetDistanceBetweenStops(bus.stops[j-1], bus.stops[j]);
						gr.AddEdge(graph::Edge<RouteItem> {
							from_id, 
							bus.stops[j]->id, 
							RouteItem {
								RouteItemType::BUS,
								di / rs_.bus_velocity,
								bus.name,
								j - i
							}
						});
					}
				}
			} 
			else {
				for(int i = 0; i < bus.stops.size() / 2 - 1; ++i) {
					size_t from_id = GetVertexWaitId(bus.stops[i]->id);
					gr.AddEdge(graph::Edge<RouteItem> {
						bus.stops[i]->id, 
						from_id, 
						RouteItem {
							RouteItemType::WAIT,
							rs_.bus_wait_time,
							bus.stops[i]->name,
							0 
						}
					});
					
					int di = 0;
					for(int j = i + 1; j < bus.stops.size() / 2; ++j) {
						di += trc_.GetDistanceBetweenStops(bus.stops[j-1], bus.stops[j]);
						gr.AddEdge(graph::Edge<RouteItem> {
							from_id, 
							bus.stops[j]->id, 
							RouteItem {
								RouteItemType::BUS,
								di / rs_.bus_velocity,
								bus.name,
								j - i
							}
						});
					}
				}
				
				for(int i = bus.stops.size() / 2; i < bus.stops.size() - 1; ++i) {
					size_t from_id = GetVertexWaitId(bus.stops[i]->id);
					gr.AddEdge(graph::Edge<RouteItem> {
						bus.stops[i]->id, 
						from_id, 
						RouteItem {
							RouteItemType::WAIT,
							rs_.bus_wait_time,
							bus.stops[i]->name,
							0 
						} 
					});
					
					int di = 0;
					for(int j = i + 1; j < bus.stops.size(); ++j) {
						di += trc_.GetDistanceBetweenStops(bus.stops[j-1], bus.stops[j]);
						gr.AddEdge(graph::Edge<RouteItem> {
							from_id, 
							bus.stops[j]->id,
							RouteItem {
								RouteItemType::BUS,
								di / rs_.bus_velocity,
								bus.name, 
								j - i
							}
						});
					}
				}
			}
		}
	}
	graph::Edge<RouteItem> GetEdge(size_t id) {
		return gr.GetEdge(id);
	}
	std::optional<graph::Router<RouteItem>::RouteInfo> BuildRoute(size_t from, size_t to){
		graph::Router<RouteItem> router(gr);
		return router.BuildRoute(from, to);
	}
protected:
	TransportCatalogue& trc_;	
	RouterSettings rs_;
	graph::DirectedWeightedGraph<RouteItem> gr;
private: 
	size_t GetVertexWaitId(size_t stop_id) {
		return stop_id + trc_.GetStops().size();
	}
};


class UserInterface;
class RequestReader;
class RequestHandler;
class RequestHandlerBase;
class RequestHandlerStat;

class UserInterface {
public:	
	UserInterface(std::ostream& os, TransportCatalogue& trc, RouterBuilder* route_builder, MapRenderer* map_renderer = nullptr) : os_(os), trc_(trc),
	route_builder_(route_builder), map_renderer_(map_renderer)  {}
	virtual void ShowQueriesResult(const std::list<detail::StatQuery>& queries) const = 0;
protected:
	int ROUTE_STAT_PRECISION = 6;
	std::ostream& os_;
	TransportCatalogue& trc_;
	MapRenderer* map_renderer_;
	RouterBuilder* route_builder_;
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
	RouterSettings GetSettingsRouter();
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
