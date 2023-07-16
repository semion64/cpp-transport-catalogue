#include "transport_router.h"

namespace trans_cat {
size_t TransportRouter::GetVertexWaitId(size_t stop_id) const {
	return stop_id + trc_.GetStops().size();
}

graph::Edge<RouteItem> TransportRouter::GetEdge(size_t id) const {
	return gr.GetEdge(id);
}	

double TransportRouter::CalcTime(int di) {
	return di / (rs_.bus_velocity / 0.06);
}

void TransportRouter::AddEdgesForBus(const Bus& bus, size_t start_stop, size_t end_stop, bool take_last_stop) {
	for(int i = start_stop; i < end_stop; ++i) {
		size_t from_id = GetVertexWaitId(bus.stops[i]->id);
		int di = 0;
		for(int j = i + 1; j < end_stop + take_last_stop; ++j) { // + take_last_stop чтобы учесть конечную остановку для некольцевого маршрута
			di += trc_.GetDistanceBetweenStops(bus.stops[j-1], bus.stops[j]);
			gr.AddEdge(graph::Edge<RouteItem> {
				from_id, 
				bus.stops[j]->id, 
				RouteItem {
					RouteItemType::BUS,
					CalcTime(di),
					bus.name,
					j - i
				}
			});
		}
	}
}
const graph::DirectedWeightedGraph<RouteItem>& TransportRouter::BuildGraph() {
	gr = graph::DirectedWeightedGraph<RouteItem>(trc_.GetStops().size() * 2);
	auto stops = trc_.GetStops();
	for(int i = 0; i < stops.size(); ++i) {
		gr.AddEdge(graph::Edge<RouteItem> {
			stops[i].id, 
			GetVertexWaitId(stops[i].id), 
			RouteItem { 
				RouteItemType::WAIT, 
				rs_.bus_wait_time, 
				stops[i].name, 
				0
			} 
		});
	}	
			
	for(const auto& bus : trc_.GetBuses()) {
		if(bus.is_ring) {
			AddEdgesForBus(bus, 0, bus.stops.size(), 0);
		} 
		else {
			AddEdgesForBus(bus, 0, bus.stops.size() / 2, 1);
			AddEdgesForBus(bus, bus.stops.size() / 2, bus.stops.size(), 0);
		}
	}
	
	return gr;
}

} // end ::trans_cat

