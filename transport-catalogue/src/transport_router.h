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
	
	
	graph::Edge<RouteItem> GetEdge(size_t id) const;
	const graph::DirectedWeightedGraph<RouteItem>& GetGraph() const;
	void BuildGraph();
protected:
	TransportCatalogue& trc_;	
	RouterSettings rs_;
	graph::DirectedWeightedGraph<RouteItem> gr;
	
private:
	constexpr static double VELOCITY_TO_M_MIN = 1 / 0.06; 
	size_t GetVertexWaitId(size_t stop_id) const;
	double CalcTime(int distance);
	void FillByStops(const std::deque<Stop>& stops);
	void FillByBuses(const std::deque<Bus>& buses);
	void AddEdgesForBus(const Bus& bus, size_t start_stop, size_t end_stop, bool take_last_stop);
};

} // end ::trans_cat
