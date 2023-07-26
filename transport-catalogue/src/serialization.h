#pragma once
#include <variant>


#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"

#include <transport_catalogue.pb.h>
#include <map_renderer.pb.h>
#include <transport_router.pb.h>
#include <graph.pb.h>
#include <svg.pb.h>

namespace serialize{
	
using IncidenceList = std::vector<graph::EdgeId>;
	
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
	ColorSetter(serial_svg::Color& proto_color) : proto_color_(proto_color) {}
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
	serial_svg::Color& proto_color_;
};

class Manager;
class Serializable;

class Serializable {
public:
	Serializable() { } 
	virtual bool Save() const = 0;
	virtual bool Load() = 0;
	void SetProtoTransCat(serial_trc::TransportCatalogue* proto_trans_cat) {
		proto_trans_cat_ = proto_trans_cat;
	}
protected:
	mutable serial_trc::TransportCatalogue* proto_trans_cat_ = nullptr;	
};

class Manager {
public:
	Manager() {
	
	}
	void AddTask(Serializable* task) {
		task->SetProtoTransCat(&proto_trans_cat_); // все сериализаторы должны работать с одним proto_trans_cat_
		tasks_.push_back(task);
	}
	
	bool Save(std::ostream& output) {
		for(auto& task : tasks_) {
			if(!task->Save()) {
				return false;
			} 
		}
		
		proto_trans_cat_.SerializeToOstream(&output); // собственно сериализации в файл данных из всех task
		
		return true;
	}
	
	bool Load(std::istream& input) {
		serial_trc::TransportCatalogue proto_trans_cat;
	
		if (!proto_trans_cat_.ParseFromIstream(&input)) {
			return false;
		}
    
		for(auto& task : tasks_) {
			if(!task->Load()) {
				return false;
			} 
		}
		
		return true;
	}
	
private:
	serial_trc::TransportCatalogue proto_trans_cat_;
	std::vector<Serializable*> tasks_;
};

class TransportCatalogue : public Serializable {
public:
	TransportCatalogue(trans_cat::TransportCatalogue* trc)  : trc_(trc) { }
	bool Save() const override;
	bool Load() override;
private:
	trans_cat::TransportCatalogue* trc_;
};

class RenderSettings : public Serializable {
public:
	RenderSettings(trans_cat::RenderSettings* settings) : settings_(settings) { }
	bool Save() const override;
	bool Load() override;
private:
	trans_cat::RenderSettings* settings_;
	
	serial_svg::Point PointToProto(svg::Point point) const;
	serial_svg::Color ColorToProto(svg::Color clr) const;
	svg::Point ProtoToPoint(serial_svg::Point proto_point) const;
	svg::Color ProtoToColor(serial_svg::Color proto_clr) const;
};

class Router : public Serializable {
public:
	Router(trans_cat::TransportRouter* trans_router) : trans_router_(trans_router) { }
	bool Save() const override;
	bool Load() override;
private:
	trans_cat::TransportRouter* trans_router_;
	void SaveSettings(serial_trc::TransportRouter* proto_trans_router) const;
	void SaveGraph(serial_trc::TransportRouter* proto_trans_router) const;
	
	void LoadSettings(const serial_trc::TransportRouter& proto_trans_router);
	void LoadGraph(const serial_trc::TransportRouter& proto_trans_router);
	
	serial_graph::Weight WeightToProto(const trans_cat::RouteItem& weight) const;
	trans_cat::RouteItem ProtoToWeight(const serial_graph::Weight& proto_weight) const;
};

}

