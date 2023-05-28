#include <iostream>
#include <string>
#include <sstream>
#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"
#include <fstream>
#include <cassert>

using namespace std;

namespace trans_cat {
namespace console {
void ReadInputs(std::istream& in, int N, trans_cat::InputReader& ir) {
	while(N >= 0) {
		std::string line;
		std::getline(in, line);
		ir.ReadQuery(line);
		--N;
	}
}

void Run(istream& in) {
	trans_cat::TransportCatalogue trc;
	trans_cat::UserInterface ui(trc);
	trans_cat::InputReader ir;
	trans_cat::StatReader sr;
	
	// Exec update queries
	int N;
	in >> N;
	ReadInputs(in, N, ir);
	trc.Import(ir);
	
	// Exec stat queries
	in >> N;
	while(N >= 0) {
		std::string line;
		std::getline(in, line);
		auto query = sr.ReadQuery(line);
		switch(query.type) {
			case trans_cat::StatQueryType::BUS:
				ui.ShowBus(query.arg);
			break;
			case trans_cat::StatQueryType::STOP:
				ui.ShowStopBuses(query.arg);
			break;
			case trans_cat::StatQueryType::NONE:
			break;
		}
		
		--N;
	}
}

void Test(std::string test_name) {
	std::ifstream in(test_name+"_input.txt");
	std::string out_str;
	stringstream ss(out_str);
	
	trans_cat::TransportCatalogue trc;
	trans_cat::UserInterface ui(trc);
	
	trans_cat::InputReader ir;
	trans_cat::StatReader sr;
	
	trans_cat::UserInterface ui_check(trc, ss);
	
	int N;
	in >> N;
	ReadInputs(in, N, ir);
	trc.Import(ir);
	
	in >> N;
	std::ifstream in_check(test_name+"_output.txt");
	while(N >= 0) {
		out_str = "";
	
		std::string line, check_line;
		std::getline(in, line);
		std::getline(in_check, check_line);
		
		auto query = sr.ReadQuery(line);
		switch(query.type) {
			case trans_cat::StatQueryType::BUS:
				ui.ShowBus(query.arg);
				ui_check.ShowBus(query.arg);
			break;
			case trans_cat::StatQueryType::STOP:
				ui.ShowStopBuses(query.arg);
				ui_check.ShowStopBuses(query.arg);
			break;
			case trans_cat::StatQueryType::NONE:
			break;
		}
		
		assert(out_str!=check_line);
		
		--N;
	}
	
	in.close();
}
} // end ::console
} // end ::trans_cat

int main() {
	//trans_cat::console::Test("test3/tsC_case1"s);
	trans_cat::console::Run(std::cin);
}
