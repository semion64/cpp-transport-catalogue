#include <iostream>

#include <cassert>
#include <sstream>
#include <string>
#include "json_reader.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include <optional>
#include <fstream>

#include <string_view>

using namespace std::literals;
void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

void MakeBase(std::istream& is) {
	trans_cat::TransportCatalogue trc;
	trans_cat::ManagerBaseJSON json_request(trc);
	json_request.Read(is); 
	json_request.DoBase();  
	std::ofstream output(json_request.GetSettingsSerialization().file, std::ios::binary);
	
	trans_cat::TransportRouter trans_router(trc, json_request.GetSettingsRouter());
	trans_cat::RenderSettings render_settings = json_request.GetSettingsRender();
	
	trans_router.BuildGraph();
	
	serialize::Manager serial_mng;
	serial_mng.AddTask(new serialize::TransportCatalogue(&trc));
	serial_mng.AddTask(new serialize::RenderSettings(&render_settings));
	serial_mng.AddTask(new serialize::Router(&trans_router, trc));
	if(!serial_mng.Save(output)) {
		std::cerr << "Serialization error" << std::endl;
	}
}

void ProcessRequests(std::istream& is, std::ostream& os) {
	trans_cat::TransportCatalogue trc;
	trans_cat::ManagerStatJSON json_request(trc); 
	trans_cat::RenderSettings render_settings;
	trans_cat::RouterSettings router_settings;
	json_request.Read(is); 
	trans_cat::TransportRouter router(trc, router_settings);
	
	std::ifstream input(json_request.GetSettingsSerialization().file, std::ios::binary);
	
	serialize::Manager serial_mng;
	serial_mng.AddTask(new serialize::TransportCatalogue(&trc));
	serial_mng.AddTask(new serialize::RenderSettings(&render_settings));
	serial_mng.AddTask(new serialize::Router(&router, trc));
	if(!serial_mng.Load(input)) {
		std::cerr << "Deserialization error" << std::endl;
	}
	
	//std::cout << "Router_settings: " << router.GetSettings().bus_wait_time << ", " << router.GetSettings().bus_velocity << std::endl;
	trans_cat::MapRendererSVG map_renderer(trc, render_settings);
	
	
	trans_cat::UserInterfaceJSON ui(os, trc, &router, &map_renderer);
	json_request.DoStat(ui);  
}
int main(int argc, char* argv[]) {
	int DEBUG_VER = true;
	if(DEBUG_VER) {
		std::ifstream is("open_test/s14_3_opentest_3_make_base.json");
		MakeBase(is);
		
		std::cout << "SERIALIZE_OK!" << std::endl;
		std::ifstream is2("open_test/s14_3_opentest_3_process_requests.json");
		std::ofstream os("res.json");
		ProcessRequests(is2, os);
		std::cout << "DEBUGGING_OK!" << std::endl;
		return 0;
	}
	
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
		MakeBase(std::cin);
    } else if (mode == "process_requests"sv) {
		ProcessRequests(std::cin, std::cout);
    } else {
        PrintUsage();
        return 1;
    }  	
    
}

