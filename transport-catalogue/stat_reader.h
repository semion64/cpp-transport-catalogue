#pragma once

#include <algorithm>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_set>
#include "request_handler.h"
#include "input_reader.h"

namespace trans_cat {
	
class StatReaderStd;
class UserInterfaceStd;

class StatReaderStd : public RequestHandlerStat {	
public:	
	StatReaderStd(TransportCatalogue& trc, UserInterface& ui) : RequestHandlerStat(trc, ui) { } 
	void Read(std::istream& is) override;
};

class UserInterfaceStd : public UserInterface {
public:	
	UserInterfaceStd(std::ostream& os, TransportCatalogue& trc) : UserInterface(os, trc) {}
	void ShowQueriesResult(const RequestHandlerStat::StatQueryList& query) const override;
private:
	void ShowBus(std::string_view bus_name) const;
	void ShowStopBuses(std::string_view stop) const;
};

class RenderSettingsStd : public RequestHandlerRenderSettings {
public:
	RenderSettingsStd (TransportCatalogue& trc) : RequestHandlerRenderSettings(trc) {	}
	void Read(std::istream& is) override { }
	void Do() override {}
};

class RequestManagerSTD : public RequestManager {
public: 
	RequestManagerSTD(TransportCatalogue& trc, UserInterface& ui) : RequestManager(trc, ui) { }
	void Read(std::istream& is);
};

} // end ::trans_cat
