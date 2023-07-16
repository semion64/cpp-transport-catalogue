#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

// #include "input_reader.h"
// #include "stat_reader.h"

#include "json_reader.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

using namespace std;

/*
namespace trans_cat {
namespace tests {

std::string ReadFile(std::string file_name) {
	std::ifstream in(file_name);
	std::ostringstream sstr;
    sstr << in.rdbuf();
	in.close();
    return sstr.str();
}	
		
void HandlerBase(std::istream& is, trans_cat::TransportCatalogue& trc, trans_cat::RequestHandlerBase& handler_base) {
	handler_base.Read(is);
	handler_base.Do();
	assert((trc.GetStops().size() == 2));
	assert((trc.GetBuses().size() == 2));
	
	auto marush = trc.GetStop("Marushkino"s);
	assert((marush.name == "Marushkino"s));
	assert((marush.coord.lat == 55.595884));
	assert((marush.coord.lng == 37.209755));
		
	auto bus_128 = trc.GetBus("128"s);
	assert((bus_128.name == "128"s));
	assert((bus_128.is_ring == true));
	assert((bus_128.stops[0]->name == "Tolstopaltsevo"s));
	assert((bus_128.stops[1]->name == "Marushkino"s));
	assert((bus_128.stops[2]->name == "Tolstopaltsevo"s));
	
	auto bus_256 = trc.GetBus("256"s);
	assert((bus_256.name == "256"s));
	assert((bus_256.is_ring == false));
	assert((bus_256.stops.size() == 3));
	
	assert((trc.GetRouteStat(bus_128).distance == 3000));	
	assert((trc.GetRouteStat(bus_256).distance == 3000));
}

void BaseStd() {
	std::cerr << "\tBaseStd: "sv;
	std::ifstream in("tests/input.std"s);
	trans_cat::TransportCatalogue trc;
	trans_cat::InputReaderStd hb(trc);
	HandlerBase(in, trc, hb);
	std::cerr << "\tok"sv << std::endl;
}

void BaseJSON() {
	std::cerr << "\tBaseJSON: "sv;
	std::ifstream in("tests/input.json"s);
	trans_cat::TransportCatalogue trc;
	trans_cat::InputReaderJSON hb(trc);
	HandlerBase(in, trc, hb);
	std::cerr << "\tok"sv << std::endl;
}

void StatStd() {
	std::cerr << "\tStatStd: "sv;
	std::ifstream in("tests/input.std"s);
	
	trans_cat::TransportCatalogue trc;
	trans_cat::InputReaderStd hb(trc);
	hb.Read(in);
	hb.Do();
	
	std::stringstream ss_out;
	trans_cat::UserInterfaceStd ui(ss_out, trc);
	trans_cat::StatReaderStd hs(trc, &ui);
	hs.Read(in);
	hs.Do();	
	
	assert((ReadFile("tests/output.std"s).compare(ss_out.str())));
	std::cerr << "\tok"sv << std::endl;
}

void StatJSON() {
	std::cerr << "\tStatJSON: "sv;
	std::ifstream in("tests/input.json"s);
	trans_cat::TransportCatalogue trc;
	trans_cat::InputReaderJSON hb(trc);
	auto doc = json::Load(in);
	hb.Read(&doc.GetRoot());
	hb.Do();
	
	std::stringstream ss_out;
	trans_cat::UserInterfaceJSON ui(ss_out, trc);
	trans_cat::StatReaderJSON hs(trc, &ui);
	hs.Read(&doc.GetRoot());
	hs.Do();	
	assert((ReadFile("tests/uotput.json"s).compare(ss_out.str())));
	std::cerr << "\tok"sv << std::endl;
}

void BaseStatStd() {
	std::cerr << "\tBaseStatStd: "sv;
	std::ifstream in("tests/input.std"s);
	std::stringstream ss_out;
	
	trans_cat::TransportCatalogue trc;
	trans_cat::UserInterfaceStd ui(ss_out, trc);
	trans_cat::RequestManagerSTD std_request(trc, &ui);
	std_request.Read(in);
	std_request.DoBase();
	std_request.DoStat();
	
	assert((ReadFile("tests/output.std"s).compare(ss_out.str())));
	std::cerr << "\tok"sv << std::endl;
	
	in.close();
}

void BaseStatJSON() {
	std::cerr << "\tBaseStatJSON: "sv;
	std::ifstream in("tests/input.json"s);
	std::stringstream ss_out;
	
	trans_cat::TransportCatalogue trc;
	trans_cat::UserInterfaceJSON ui(ss_out, trc);
	trans_cat::RequestManagerJSON json_request(trc, &ui);
	json_request.Read(in);
	json_request.DoBase();
	json_request.DoStat();
	
	assert((ReadFile("tests/output.json"s).compare(ss_out.str()) == 0));
	std::cerr << "\tok"sv << std::endl;
	
	in.close();
}

void RoutesSVG() {
	std::cerr << "\tRoutesSVG: "sv;
	std::ifstream in("tests/input_svg.json"s);
	std::stringstream ss_out;
	
	trans_cat::TransportCatalogue trc;
	trans_cat::UserInterfaceJSON ui(ss_out, trc);
	trans_cat::RequestManagerJSON json_request(trc, &ui);
	json_request.Read(in);
	json_request.DoBase();
	
	trans_cat::RenderSettings rs = json_request.GetSettingsMapRenderer();
	trans_cat::MapRendererSVG* map_renderer = new trans_cat::MapRendererSVG(trc,rs); 
	map_renderer->RenderMap(ss_out);
	assert((ReadFile("tests/output_svg.svg"s).compare(ss_out.str()) == 0));
	std::cerr << "\tok"sv << std::endl;
	in.close();
}

void RoutesAndLabelsSVG() {
	std::cerr << "\tRoutesAndLabelsSVG: "sv;
	std::ifstream in("tests/input_labels.json"s);
	std::stringstream ss_out;
	
	trans_cat::TransportCatalogue trc;
	trans_cat::UserInterfaceJSON ui(ss_out, trc);
	trans_cat::RequestManagerJSON json_request(trc, &ui);
	json_request.Read(in);
	
	json_request.DoBase();
	trans_cat::RenderSettings rs = json_request.GetSettingsMapRenderer();
	trans_cat::MapRendererSVG* map_renderer = new trans_cat::MapRendererSVG(trc, rs); 
	map_renderer->RenderMap(ss_out);
	assert((ReadFile("tests/output_labels.svg"s).compare(ss_out.str()) == 0));
	std::cerr << "\tok"sv << std::endl;
	in.close();
}

void MapStatSVGFinall() {
	std::cerr << "\tMapStatSVGFinall: "sv;
	std::ifstream in("tests/input_stat_map.json"s);
	std::stringstream ss_out;
	
	trans_cat::TransportCatalogue trc;
	trans_cat::MapRendererSVG map_renderer(trc);
	trans_cat::UserInterfaceJSON ui(ss_out, trc, &map_renderer); // new UserInterfaceStd(...)
	trans_cat::RequestManagerJSON json_request(trc, &ui);  // new RequestManagerStd(...)
	json_request.Read(in); // read json text from input stream
	map_renderer.SetRenderSettings(json_request.GetSettingsMapRenderer()); // not forget set render settings, otherwise was use default zero values
	json_request.DoBase(); // fill transport catalogue
	json_request.DoStat(); // do stat queries 
	
	assert((ReadFile("tests/output_stat_map.json"s).compare(ss_out.str()) == 0));
	std::cerr << "\tok"sv << std::endl;
	in.close();
}
} // end ::tests
} // end ::trans_cat

void Tests() {
	std::cerr << "Tests_Start"sv << std::endl;
	trans_cat::tests::BaseStd();
	trans_cat::tests::StatStd();
	trans_cat::tests::BaseJSON();
	trans_cat::tests::StatJSON();
	trans_cat::tests::BaseStatJSON();
	trans_cat::tests::RoutesSVG();
	trans_cat::tests::RoutesAndLabelsSVG();
	trans_cat::tests::MapStatSVGFinall();
	std::cerr << "Tests_End"sv << std::endl;
}

// ---------------------- various ways to use transport catalogue interface--------------------
void RunStd_BASE_STAT(std::istream& is = std::cin) {
	trans_cat::TransportCatalogue trc;
	trans_cat::InputReaderStd ir(trc);
	trans_cat::UserInterfaceStd ui(std::cout, trc);
	trans_cat::StatReaderStd sr(trc, &ui);
	
	ir.Read(is);
	ir.Do();
	sr.Read(is);
	sr.Do();
}

void RunJSON_BASE_STAT(std::istream& is = std::cin, std::ostream& os = std::cout) {
	trans_cat::TransportCatalogue trc;
	trans_cat::UserInterfaceJSON ui(os, trc);
	trans_cat::RequestManagerJSON json_request(trc, &ui);
	json_request.Read(is);
	json_request.DoBase();
	json_request.DoStat();
}

void RunJSON_BASE_SVG(std::istream& is = std::cin, std::ostream& os = std::cout) {
	trans_cat::TransportCatalogue trc;
	trans_cat::UserInterfaceJSON ui(os, trc);
	trans_cat::RequestManagerJSON json_request(trc, &ui);
	json_request.Read(is);
	json_request.DoBase();
	
	trans_cat::MapRendererSVG map_renderer(trc, json_request.GetSettingsMapRenderer()); // using MapRenderer separately from UserInterface
	map_renderer.RenderMap(os);
}

*/
void RunJSON_BASE_STAT_SVG(std::istream& is = std::cin, std::ostream& os = std::cout) {
	trans_cat::TransportCatalogue trc;
	trans_cat::ManagerJSON json_request(trc);  // new RequestManagerStd(...)
	json_request.Read(is); // read json text from input stream
	trans_cat::MapRendererSVG map_renderer(trc, json_request.GetSettingsMapRenderer());
	trans_cat::TransportRouter route_builder(trc, json_request.GetSettingsRouter());
	json_request.DoBase();
	trans_cat::UserInterfaceJSON ui(os, trc, route_builder, map_renderer);
	json_request.DoStat(ui);  
}

int main() {
	//std::ifstream f("test_router/ex_1_in.json");
	//std::ofstream f2("res.json");
	//RunStd_BASE_STAT(f);
	//RunJSON_BASE_STATf);
	//RunJSON_BASE_SVG(f);
	RunJSON_BASE_STAT_SVG();
	//Tests();
}
