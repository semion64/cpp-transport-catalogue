#pragma once

#include <algorithm>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_set>
#include "request_handler.h"
#include "input_reader.h"

namespace trans_cat {

class StatReaderStd : public RequestHandlerStat {	
public:	
	StatReaderStd(TransportCatalogue& trc, UserInterface& ui) : RequestHandlerStat(trc, ui) { } 
	void Read(std::istream& is) override;
private:
	void ExecQuery(std::string& line);
};

class UserInterfaceStd : public UserInterface {
public:	
	UserInterfaceStd(std::ostream& os, TransportCatalogue& trc) : UserInterface(os, trc) {}
	void ShowBus(std::string_view bus_name) override;
	void ShowStopBuses(std::string_view stop) override;
};

} // end ::trans_cat
