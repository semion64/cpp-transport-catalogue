#pragma once
#include <variant>


#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"

#include <transport_catalogue.pb.h>
#include <map_renderer.pb.h>
#include <transport_router.pb.h>

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

class Manager;
class Serializable;

class Serializable {
public:
	Serializable() { } 
	virtual bool Save() const = 0;
	virtual bool Load() = 0;
	void SetProtoTransCat(trc_serialize::TransportCatalogue* proto_trans_cat) {
		proto_trans_cat_ = proto_trans_cat;
	}
protected:
	mutable trc_serialize::TransportCatalogue* proto_trans_cat_ = nullptr;	
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
		trc_serialize::TransportCatalogue proto_trans_cat;
	
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
	trc_serialize::TransportCatalogue proto_trans_cat_;
	std::vector<Serializable*> tasks_;
};

class TransportCatalogue : public Serializable {
public:
	TransportCatalogue(trans_cat::TransportCatalogue* trc) 
		: trc_(trc) { }
	bool Save() const override;
	bool Load() override;
private:
	trans_cat::TransportCatalogue* trc_;
};

class RenderSettings : public Serializable {
public:
	RenderSettings(trans_cat::RenderSettings* settings)
		: settings_(settings) { }
	bool Save() const override;
	bool Load() override;
private:
	trans_cat::RenderSettings* settings_;
};

class Router : public Serializable {
public:
	Router(trans_cat::TransportRouter* trans_router, const trans_cat::TransportCatalogue& trc)
		: trans_router_(trans_router), trc_(trc) { }
	bool Save() const override;
	bool Load() override;
private:
	trans_cat::TransportRouter* trans_router_;
	const trans_cat::TransportCatalogue& trc_;
};

}

