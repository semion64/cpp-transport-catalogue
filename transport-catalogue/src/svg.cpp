#include "svg.h"

namespace svg {

using namespace std::literals;

void details::replace_all(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
}

std::ostream& operator<<(std::ostream& os, const Color& clr) {
	std::visit(ColorPrinter(os), clr);
	return os;
}

std::ostream& operator<<(std::ostream& os, const StrokeLineCap& line_cap) {
	switch(line_cap) {
		case StrokeLineCap::BUTT: 
			os << "butt"sv;
		break;
		case StrokeLineCap::ROUND: 
			os << "round"sv;
		break;
		case StrokeLineCap::SQUARE: 
			os << "square"sv;
		break;
	}
	
	return os;
}

std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& line_join) {
	
	switch(line_join) {
		case StrokeLineJoin::ARCS:
			os << "arcs"sv;
		break;
		case StrokeLineJoin::BEVEL:
			os << "bevel"sv;
		break;
		case StrokeLineJoin::MITER:
			os << "miter"sv;
		break;
		case StrokeLineJoin::MITER_CLIP:
			os << "miter-clip"sv;
		break;
		case StrokeLineJoin::ROUND:
			os << "round"sv;
		break;
	}
	
	return os;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();
    // Делегируем вывод тега своим подклассам
    RenderObject(context);
    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\""sv;
    out << " r=\""sv << radius_ << "\""sv;
	RenderAttrs(out);
    out << "/>"sv;
}

// -------- Polyline------------------

Polyline& Polyline::AddPoint(Point point) {
	points_.emplace_back(point);
	return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    bool is_first = true;
    for(const auto& pt : points_) {
		if(is_first) {
			is_first = false;
		}
		else {
			out << " "sv;
		}
		out << pt.x << ","sv << pt.y;
	}
    out << "\""sv;
	RenderAttrs(out);
	out << "/>"sv;
}

// --------Text-----------------------

Text& Text::SetPosition(Point pos) {
	pos_ = pos;
	return *this;
}

Text& Text::SetOffset(Point offset) {
	offset_ = offset;
	return *this;
}

Text& Text::SetFontSize(uint32_t size) {
	size_ = size;
	return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
	font_family_ = font_family;
	return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
	font_weight_ = font_weight;
	return *this;
}

Text& Text::SetData(std::string data) {
	details::replace_all(data, "&"s, "&amp;"s);
	details::replace_all(data, "\""s, "&quot;"s);
	details::replace_all(data, "'"s, "&apos;"s);
	details::replace_all(data, "<"s, "&lt;"s);
	details::replace_all(data, ">"s, "&gt;"s);
	data_ = data;
	return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text"sv;
    
	RenderAttrs(out);
	
    details::attr_out(out, "x"sv, pos_.x);
    details::attr_out(out, "y"sv, pos_.y);
    details::attr_out(out, "dx"sv, offset_.x);
    details::attr_out(out, "dy"sv, offset_.y);

    details::attr_out(out, "font-size"sv, size_);
    if(!font_family_.empty()) {
		details::attr_out(out, "font-family"sv, font_family_);
	}
	
	if(!font_weight_.empty()) {
		details::attr_out(out, "font-weight"sv, font_weight_);
	}
	
    out << ">"sv << data_ << "</text>"sv;
}


//------------- Documents------------------
// Добавляет в svg-документ объект-наследник svg::Object
void Document::AddPtr(std::unique_ptr<Object>&& obj) {
	objects_.emplace_back(std::move(obj));
}

// Выводит в ostream svg-представление документа
void Document::Render(std::ostream& out) const {
    RenderContext ctx(out, 2, 2);
	out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
	std::for_each(objects_.begin(), objects_.end(), [&ctx] (const auto& obj) {
			obj->Render(ctx);
		});
	out << "</svg>"sv;
}
}  // namespace svg
