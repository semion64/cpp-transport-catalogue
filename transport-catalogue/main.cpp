/*
	Добрый день. Благодарю за ревью и терпение)
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

int main() {
	trans_cat::TransportCatalogue trc;
	trans_cat::InputReader ir(trc);
	trans_cat::UserInterface ui(std::cout, trc);
	trans_cat::StatReader sr(ui);
	
	ir.Read(std::cin);
	sr.ReadAndExec(std::cin);
}