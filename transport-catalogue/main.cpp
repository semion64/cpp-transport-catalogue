/*
 
	Добрый день. Благодарю за ревью. 
	Появилось пару вопросов по move семантике и структурам данных, описанных в InputReader::Export()
	Рад был бы увидеть комментарии 
 */
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

using namespace std;

namespace trans_cat {
namespace console {
void ReadInputs(trans_cat::TransportCatalogue& trc, std::istream& in) {
	trans_cat::InputReader ir;
	
	// Exec update queries
	int N;
	in >> N;
	
	while(N >= 0) {
		std::string line;
		std::getline(in, line);
		ir.ReadQuery(line);
		--N;
	}
	
	ir.Export(trc);
}

void ReadAndExecStats(trans_cat::TransportCatalogue& trc, std::istream& in) {
	trans_cat::UserInterface ui(trc);
	trans_cat::StatReader sr(ui);
	int N;
	// Exec stat queries
	in >> N;
	while(N >= 0) {
		std::string line;
		std::getline(in, line);
		sr.ExecQuery(line);
		--N;
	}
}

} // end ::console
} // end ::trans_cat

int main() {
	trans_cat::TransportCatalogue trc;
	trans_cat::console::ReadInputs(trc, std::cin);
	trans_cat::console::ReadAndExecStats(trc, std::cin);
}

/*void Test(std::string test_name) {
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
	ir.Export(trc);
	
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
}*/
