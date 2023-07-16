#pragma once

#include "transport_catalogue.h"
#include "router.h"

namespace trans_cat {
	
enum class RouteItemType {
	NONE,
	WAIT,
	BUS
};

struct RouteItem {
	RouteItemType type = RouteItemType::NONE;
	double time = 0;
	std::string_view name;
	int span = 0;
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

struct RouterSettings {
	double bus_wait_time;
	double bus_velocity;
};

class TransportRouter {
public:
	TransportRouter(TransportCatalogue& trc, const RouterSettings& router_settings) : trc_(trc), rs_(router_settings) { }
	
	const graph::DirectedWeightedGraph<RouteItem>& BuildGraph();
	graph::Edge<RouteItem> GetEdge(size_t id) const;
	
protected:
	TransportCatalogue& trc_;	
	RouterSettings rs_;
	graph::DirectedWeightedGraph<RouteItem> gr;
private: 
	size_t GetVertexWaitId(size_t stop_id) const;
};

} // end ::trans_cat