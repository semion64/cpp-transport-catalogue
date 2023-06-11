
 
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"
#include "json_reader.h"
#include "transport_catalogue.h"

using namespace std;

namespace trans_cat {
namespace tests {
/*
13
Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino
Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino
Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye
Bus 750: Tolstopaltsevo - Marushkino - Marushkino - Rasskazovka
Stop Rasskazovka: 55.632761, 37.333324, 9500m to Marushkino
Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam
Stop Biryusinka: 55.581065, 37.64839, 750m to Universam
Stop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, 900m to Biryulyovo Tovarnaya
Stop Biryulyovo Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo Passazhirskaya
Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to Biryulyovo Zapadnoye
Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye
Stop Rossoshanskaya ulitsa: 55.595579, 37.605757
Stop Prazhskaya: 55.611678, 37.603831
6
Bus 256
Bus 750
Bus 751
Stop Samara
Stop Prazhskaya
Stop Biryulyovo Zapadnoye 
 
Вывод
Bus 256: 6 stops on route, 5 unique stops, 5950 route length, 1.36124 curvature
Bus 750: 7 stops on route, 3 unique stops, 27400 route length, 1.30853 curvature
Bus 751: not found
Stop Samara: not found
Stop Prazhskaya: no buses
Stop Biryulyovo Zapadnoye: buses 256 828

--------------------------------------------------------------------------------------

std::string query = 
	"Stop Tolstopaltsevo: 55.611087, 37.20829, 1000m to Marushkino\n"
	"Stop Marushkino: 55.595884, 37.209755, 2000m to Tolstopaltsevo\n"
	"Bus 128: Tolstopaltsevo > Marushkino > Tolstopaltsevo\n"
	"Bus 256: Tolstopaltsevo - Marushkino\n";
	
std::stringstream ss(query);
 struct Stop {
	std::string_view name;
	geo::Coordinates coord;
	
};

struct Bus {
	std::string_view name;
	std::vector<const Stop*> stops;
	bool is_ring;
};

struct RouteStat {
	size_t stops_count;
	size_t unique_stops;
	int distance;
	double curvature;
	bool is_ring;
};




std::string ReadFile(std::string file_name) {
	std::ifstream in(file_name);
	std::string str;
	std::getline(in, str);
	in.close();
	return str;
}				
void HandlerBase(std::istream& is, trans_cat::TransportCatalogue& trc, trans_cat::RequestHandlerBase& handler_base) {
	handler_base.Read(is);
	handler_base.DoQueries();
	assert((trc.GetStops().size() == 2));
	assert((trc.GetBuses().size() == 2));
	
	auto marush = trc.GetStop("Marushkino");
	assert((marush.name == "Marushkino"));
	assert((marush.coord.lat == 55.595884));
	assert((marush.coord.lng == 37.209755));
		
	auto bus_128 = trc.GetBus("128");
	assert((bus_128.name == "128"));
	assert((bus_128.is_ring == true));
	assert((bus_128.stops[0]->name == "Tolstopaltsevo"));
	assert((bus_128.stops[1]->name == "Marushkino"));
	assert((bus_128.stops[2]->name == "Tolstopaltsevo"));
	
	auto bus_256 = trc.GetBus("256");
	assert((bus_256.name == "256"));
	assert((bus_256.is_ring == false));
	assert((bus_256.stops.size() == 3));
	
	assert((trc.GetRouteStat(bus_128).distance == 3000));	
	assert((trc.GetRouteStat(bus_256).distance == 3000));
}

void BaseStd() {
	std::cerr << "\tBaseStd: ";
	std::ifstream in("tests/std_test_in.txt");
	trans_cat::TransportCatalogue trc;
	trans_cat::InputReaderStd hb(trc);
	HandlerBase(in, trc, hb);
	std::cerr << "\tok" << std::endl;
}

void BaseJSON() {
	std::cerr << "\tBaseJSON: ";
	std::ifstream in("tests/json_test_in.txt");
	trans_cat::TransportCatalogue trc;
	trans_cat::InputReaderJSON hb(trc);
	HandlerBase(in, trc, hb);
	std::cerr << "\tok" << std::endl;
}

void StatStd() {
	std::cerr << "\tStatStd: ";
	std::ifstream in("tests/std_test_in.txt");
	
	trans_cat::TransportCatalogue trc;
	trans_cat::InputReaderStd hb(trc);
	hb.Read(in);
	hb.DoQueries();
	
	std::stringstream ss_out;
	trans_cat::UserInterfaceStd ui(ss_out, trc);
	trans_cat::StatReaderStd hs(trc, ui);
	hs.Read(in);
	hs.DoQueries();	
	
	assert((ReadFile("tests/std_test_out.txt").compare(ss_out.str())));
	std::cerr << "\tok" << std::endl;
}

void StatJSON() {
	std::cerr << "\tStatJSON: ";
	std::ifstream in("tests/json_test_in.txt");
	trans_cat::TransportCatalogue trc;
	trans_cat::InputReaderJSON hb(trc);
	auto doc = json::Load(in);
	const json::Node& data = doc.GetRoot();
	hb.ReadJSON(data);
	hb.DoQueries();
	
	std::stringstream ss_out;
	trans_cat::UserInterfaceJSON ui(ss_out, trc);
	trans_cat::StatReaderJSON hs(trc, ui);
	hs.ReadJSON(data);
	hs.DoQueries();	
	//std::cerr << ss_out.str();
	assert((ReadFile("tests/json_test_out.txt").compare(ss_out.str())));
	std::cerr << "\tok" << std::endl;
}

void BaseStatStd() {
	std::cerr << "\tBaseStatStd: ";
	std::ifstream in("tests/std_test_in.txt");
	std::stringstream ss_out;
	
	trans_cat::TransportCatalogue trc;
	trans_cat::UserInterfaceStd ui(ss_out, trc);
	trans_cat::RequestStd std_request(trc, ui);
	std_request.Read(in);
	std_request.DoQueries();
	
	assert((ReadFile("tests/std_test_out.txt").compare(ss_out.str())));
	std::cerr << "\tok" << std::endl;
	
	in.close();
}

void BaseStatJSON() {
	std::cerr << "\tBaseStatJSON: ";
	std::ifstream in("tests/json_test_in.txt");
	std::stringstream ss_out;
	
	trans_cat::TransportCatalogue trc;
	trans_cat::UserInterfaceJSON ui(ss_out, trc);
	trans_cat::RequestJSON json_request(trc, ui);
	json_request.Read(in);
	json_request.DoQueries();
	
	assert((ReadFile("tests/json_test_out.txt").compare(ss_out.str())));
	std::cerr << "\tok" << std::endl;
	
	in.close();
}
} // end ::tests
} // end ::trans_cat

void Tests() {
	std::cerr << "Tests_Start" << std::endl;
	trans_cat::tests::BaseStd();
	trans_cat::tests::StatStd();
	trans_cat::tests::BaseJSON();
	trans_cat::tests::StatJSON();
	trans_cat::tests::BaseStatJSON();
	std::cerr << "Tests_End" << std::endl;
}

void RunStd(std::istream& is = std::cin) {
	trans_cat::TransportCatalogue trc;
	trans_cat::InputReaderStd ir(trc);
	trans_cat::UserInterfaceStd ui(std::cout, trc);
	trans_cat::StatReaderStd sr(trc, ui);
	
	ir.Read(is);
	ir.DoQueries();
	sr.Read(is);
	sr.DoQueries();
}
*/

void RunJSON(std::istream& is = std::cin, std::ostream& os = std::cout) {
	trans_cat::TransportCatalogue trc;
	trans_cat::UserInterfaceJSON ui(os, trc);
	trans_cat::RequestJSON json_request(trc, ui);
	json_request.Read(is);
	json_request.DoQueries();
}

int main() {
	//std::ifstream f("test1/tsA_case1_input.txt");
	RunJSON();
	//Tests();
}
