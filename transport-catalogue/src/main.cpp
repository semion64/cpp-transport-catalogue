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

// #define __TRANS_CAT_DEBUG__

using namespace std::literals;
void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

void MakeBase(std::istream& is) {
	trans_cat::TransportCatalogue trc;
	trans_cat::ManagerMakeBaseJSON json_make(trc);
	json_make.Read(is); 
	json_make.DoBase();  
	std::ofstream output(json_make.GetSettingsSerialization().file, std::ios::binary);
	
	trans_cat::TransportRouter trans_router(trc, json_make.GetSettingsRouter());
	trans_cat::RenderSettings render_settings = json_make.GetSettingsRender();
	
	trans_router.BuildGraph();
	
	serialize::Manager serial_mng;
	serial_mng.AddTask(new serialize::TransportCatalogue(&trc));
	serial_mng.AddTask(new serialize::RenderSettings(&render_settings));
	serial_mng.AddTask(new serialize::Router(&trans_router));
	if(!serial_mng.Save(output)) {
		std::cerr << "Serialization error" << std::endl;
	}
	
#ifdef __TRANS_CAT_DEBUG__ 	
	std::cout << "Serialize done" << std::endl;
#endif
}

void ProcessRequests(std::istream& is, std::ostream& os) {
	trans_cat::TransportCatalogue trc;
	trans_cat::ManagerProcessRequestsJSON json_process(trc); 
	trans_cat::RenderSettings render_settings;
	trans_cat::RouterSettings router_settings;
	json_process.Read(is); 
	trans_cat::TransportRouter router(trc, router_settings);
	
	std::ifstream input(json_process.GetSettingsSerialization().file, std::ios::binary);
	
	serialize::Manager serial_mng;
	serial_mng.AddTask(new serialize::TransportCatalogue(&trc));
	serial_mng.AddTask(new serialize::RenderSettings(&render_settings));
	serial_mng.AddTask(new serialize::Router(&router));
	if(!serial_mng.Load(input)) {
		std::cerr << "Deserialization error" << std::endl;
	}
	
#ifdef __TRANS_CAT_DEBUG__ 	
	std::cout << "Deserialize done" << std::endl;
#endif
	trans_cat::MapRendererSVG map_renderer(trc, render_settings);
	
	
	trans_cat::UserInterfaceJSON ui(os, trc, &router, &map_renderer);
	json_process.DoStat(ui);  
}
int main(int argc, char* argv[]) {
	
#ifdef __TRANS_CAT_DEBUG__ 
	std::cout << "DEBUGGING_START..." << std::endl;
	std::ifstream is("open_test/s14_3_opentest_2_make_base.json");
	MakeBase(is);
	
	std::ifstream is2("open_test/s14_3_opentest_2_process_requests.json");
	std::ofstream os("res.json");
	ProcessRequests(is2, os);
	std::cout << "DEBUGGING_OK..." << std::endl;
	return 0;
#endif
	
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

