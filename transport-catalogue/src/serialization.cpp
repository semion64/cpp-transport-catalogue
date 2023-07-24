#include "serialization.h"

void SerializeTransportCatalogue(const trans_cat::TransportCatalogue& trc, std::ostream& output) {
	trc_serialize::TransportCatalogue proto_trans_cat;
	
	// add sotps & buses names
	trc_serialize::NameIndex proto_name_index;
	std::map <std::string_view, int> tmp_stop_index, tmp_bus_index;
	
	int i = 0;
	for(const auto& stop_name : trc.GetStopNames()) { 
		(*proto_name_index.mutable_stop_name())[++i] = stop_name;
		tmp_stop_index[stop_name] = i;
		
	}
	i = 0;
	for(const auto& bus_name : trc.GetBusNames()) { 
		(*proto_name_index.mutable_bus_name())[++i] = bus_name;
		tmp_bus_index[bus_name] = i;
	}
	
	*proto_trans_cat.mutable_name_index() = proto_name_index;
	
	// add stops
	for(const auto& stop : trc.GetStops()) {
		trc_serialize::Stop proto_stop;
		proto_stop.set_name_index(tmp_stop_index[stop.name]);
		proto_stop.set_id(stop.id);
		(*proto_stop.mutable_coord()).set_lat(stop.coord.lat);
		(*proto_stop.mutable_coord()).set_lng(stop.coord.lng);
		*proto_trans_cat.add_stop() = proto_stop;
	}
	// add buses
	for(const auto& bus : trc.GetBuses()) {
		trc_serialize::Bus proto_bus;
		proto_bus.set_name_index(tmp_bus_index[bus.name]);
		proto_bus.set_is_ring(bus.is_ring);
		for(const auto& stop : bus.stops) {
			proto_bus.add_stop_id(stop->id); 
		}
		
		*proto_trans_cat.add_bus() = proto_bus;
	}
	
	// add distance between stop
	for(const auto& dist : trc.GetDistanceBetweenStops()) {
		trc_serialize::StopDistance proto_stop_dist;
		//const trans_cat::Stop* stop_from = dist.stop_from;
		proto_stop_dist.set_stop_from(dist.stop_from->id);
		proto_stop_dist.set_stop_to(dist.stop_to->id);
		proto_stop_dist.set_distance(dist.distance);
		*proto_trans_cat.add_stop_distance() = proto_stop_dist;		
	}
	
	proto_trans_cat.SerializeToOstream(&output);
	
}		
bool DeserializeTransportCatalogue(trans_cat::TransportCatalogue* trc, std::istream& input) {
	trc_serialize::TransportCatalogue proto_trans_cat;
	
    if (!proto_trans_cat.ParseFromIstream(&input)) {
        return false;
    }
    
    auto proto_bus_index = proto_trans_cat.name_index().bus_name();
    auto proto_stop_index = proto_trans_cat.name_index().stop_name();
    
    // add stop
    std::map<int, trans_cat::Stop*> tmp_id_stop;
	for(const auto& proto_stop : proto_trans_cat.stop()) {
		tmp_id_stop[proto_stop.id()]= 
			&trc->AddStop(
				proto_stop_index.at(proto_stop.name_index()), 
				geo::Coordinates {
					proto_stop.coord().lat(),
					proto_stop.coord().lng()
				}
			);
	} 
	
	// add bus
	for(const auto& proto_bus : proto_trans_cat.bus()) {
		// fill bus_stops vector
		std::vector<const trans_cat::Stop*> bus_stops;
		for(auto stop_id : proto_bus.stop_id()) {
			bus_stops.push_back(tmp_id_stop[stop_id]);
		}
		std::cout << std::endl;
		trc->AddBus(
			proto_bus_index.at(proto_bus.name_index()), 
			bus_stops,
			proto_bus.is_ring()
		);
	}
	
	// add distance between stop
	for(const auto& proto_dist : proto_trans_cat.stop_distance()) {
		trc->SetDistance(
			tmp_id_stop.at(proto_dist.stop_from()),
			tmp_id_stop.at(proto_dist.stop_to()),
			proto_dist.distance());
	}
	
	return true;
}
