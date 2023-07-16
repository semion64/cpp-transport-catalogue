#include "transport_router.h"

namespace trans_cat {
size_t TransportRouter::GetVertexWaitId(size_t stop_id) const {
	return stop_id + trc_.GetStops().size();
}

graph::Edge<RouteItem> TransportRouter::GetEdge(size_t id) const {
	return gr.GetEdge(id);
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
	
	/*for(int i = 0; i < stops.size(); ++i) {
		try{
		int di = trc_.GetDistanceBetweenStops(&stops[i], &stops[i]);
		gr.AddEdge(graph::Edge<RouteItem> {
			GetVertexWaitId(stops[i].id),
			stops[i].id,
			RouteItem { 
				RouteItemType::BUS, 
				di /  (rs_.bus_velocity / 0.06), 
				stops[i].name, 
				0
			} 
		});}
		catch(...){}
	}*/	
			
	for(const auto& bus : trc_.GetBuses()) {
		if(bus.is_ring) {
			for(int i = 0; i < bus.stops.size(); ++i) {
				size_t from_id = GetVertexWaitId(bus.stops[i]->id);
				int di = 0;
				for(int j = i + 1; j < bus.stops.size(); ++j) {
					di += trc_.GetDistanceBetweenStops(bus.stops[j-1], bus.stops[j]);
					gr.AddEdge(graph::Edge<RouteItem> {
						from_id, 
						bus.stops[j]->id, 
						RouteItem {
							RouteItemType::BUS,
							di / (rs_.bus_velocity / 0.06),
							bus.name,
							j - i
						}
					});
				}
			}
		} 
		else {
			for(int i = 0; i < bus.stops.size() / 2; ++i) {
				size_t from_id = GetVertexWaitId(bus.stops[i]->id);
				int di = 0;
				for(int j = i + 1; j <= bus.stops.size() / 2; ++j) {
					di += trc_.GetDistanceBetweenStops(bus.stops[j-1], bus.stops[j]);
					gr.AddEdge(graph::Edge<RouteItem> {
						from_id, 
						bus.stops[j]->id, 
						RouteItem {
							RouteItemType::BUS,
							di / (rs_.bus_velocity / 0.06),
							bus.name,
							j - i
						}
					});
				}
			}
			
			for(int i = bus.stops.size() / 2; i < bus.stops.size(); ++i) {
				size_t from_id = GetVertexWaitId(bus.stops[i]->id);
				int di = 0;
				for(int j = i + 1; j < bus.stops.size(); ++j) {
					di += trc_.GetDistanceBetweenStops(bus.stops[j-1], bus.stops[j]);
					gr.AddEdge(graph::Edge<RouteItem> {
						from_id, 
						bus.stops[j]->id,
						RouteItem {
							RouteItemType::BUS,
							di / (rs_.bus_velocity / 0.06),
							bus.name, 
							j - i
						}
					});
				}
			}
		}
	}
	
	return gr;
}

} // end ::trans_cat

