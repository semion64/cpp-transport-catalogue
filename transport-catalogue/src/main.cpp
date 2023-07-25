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
	std::ofstream ofs(json_request.GetSettingsSerialization().file, std::ios::binary);
	trans_cat::serialize::Save(ofs, trc, json_request.GetSettingsRender());
	ofs.close();
}

void ProcessRequests(std::istream& is, std::ostream& os) {
	trans_cat::TransportCatalogue trc2;
	trans_cat::ManagerStatJSON json_request2(trc2); 
	trans_cat::RenderSettings render_settings;
	json_request2.Read(is); 
	
	std::ifstream input(json_request2.GetSettingsSerialization().file, std::ios::binary);
	trans_cat::serialize::Load(input, &trc2, &render_settings);
	input.close();

	trans_cat::MapRendererSVG map_renderer(trc2, render_settings);
	
	trans_cat::UserInterfaceJSON ui(os, trc2, nullptr, &map_renderer);
	json_request2.DoStat(ui);  
}
int main(int argc, char* argv[]) {
	int DEBUG_VER = true;
	if(DEBUG_VER) {
		std::ifstream is("make_1_in.json");
		MakeBase(is);
		
		std::ifstream is2("process_1_in.json");
		std::ofstream os("process_1_out.json");
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

