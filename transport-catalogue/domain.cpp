#include "domain.h"

namespace trans_cat{
namespace detail{
using namespace std::literals;
StatQueryType StatQuery::GetType(std::string_view type_str) {
	if(type_str == "Bus"s) {
		return detail::StatQueryType::BUS;
	}
	else if(type_str == "Stop"s) {
		return detail::StatQueryType::STOP;
	}
	else if(type_str == "Map"s) {
		return detail::StatQueryType::MAP;
	}
	
	return detail::StatQueryType::NONE;
	//throw ExceptionWrongStatReaderQuery("incorrect query type: " + std::string(type_str));
}
} // end detail
} // end trans_cat
