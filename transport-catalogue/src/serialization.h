#include "transport_catalogue.h"
//#include <transport_catalogue.pb.h>
#include <transport_catalogue.pb.h>
struct SerializationSettings {
	std::string file;
};

void SerializeTransportCatalogue(const trans_cat::TransportCatalogue& trc, std::ostream& output);
bool DeserializeTransportCatalogue(trans_cat::TransportCatalogue* trc, std::istream& input);
