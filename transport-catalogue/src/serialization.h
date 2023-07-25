#pragma once
#include <variant>


#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"

#include <transport_catalogue.pb.h>
#include <map_renderer.pb.h>
#include <transport_router.pb.h>

namespace trans_cat {
namespace serialize{
	
struct Settings {
	std::string file;
};

enum class ColorFormat {
	NONE,
	RGB,
	RGBA,
	NAMED
}; 

struct ColorSetter {
public:
	ColorSetter(trc_serialize::Color& proto_color) : proto_color_(proto_color) {}
    void operator()([[maybe_unused]]const std::monostate& none) const {
		using namespace std::literals; 
		proto_color_.set_format(static_cast<int32_t> (ColorFormat::NONE));
	} 
	void operator()(const std::string& value) const {
		using namespace std::literals; 
		proto_color_.set_format(static_cast<int32_t> (ColorFormat::NAMED));
		proto_color_.set_name(value);
	}
    void operator()(const svg::Rgb& value) const { 
		using namespace std::literals; 
		proto_color_.set_format(static_cast<int32_t> (ColorFormat::RGB));
		proto_color_.set_r(value.red);
		proto_color_.set_g(value.green);
		proto_color_.set_b(value.blue);
	}
    void operator()(const svg::Rgba& value) const { 
		using namespace std::literals;
		proto_color_.set_format(static_cast<int32_t> (ColorFormat::RGBA)); 
		proto_color_.set_r(value.red);
		proto_color_.set_g(value.green);
		proto_color_.set_b(value.blue);
		proto_color_.set_opacity(value.opacity);
	}
private:
	trc_serialize::Color& proto_color_;
};

bool Save(std::ostream& output, const TransportCatalogue& trc, std::optional<RenderSettings> rs, const TransportRouter& router);
bool Load(std::istream& input, TransportCatalogue* trc, RenderSettings* rs, TransportRouter* router);

namespace detail{

bool SaveRender(trc_serialize::TransportCatalogue* proto_trans_cat, const RenderSettings& rs);
bool LoadRender(const trc_serialize::TransportCatalogue& proto_trans_cat, RenderSettings* rs);

bool SaveTransport(trc_serialize::TransportCatalogue* proto_trans_cat, const TransportCatalogue& trc);
bool LoadTransport(const trc_serialize::TransportCatalogue& proto_trans_cat, TransportCatalogue* trc);






bool SaveRouter(trc_serialize::TransportCatalogue* proto_trans_cat, const TransportRouter& router);
bool LoadRouter(const trc_serialize::TransportCatalogue& proto_trans_cat, TransportRouter* router);


}

}
}
