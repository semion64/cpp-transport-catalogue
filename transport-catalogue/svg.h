#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <optional>
#include <variant>

namespace svg {

struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
};

namespace details {
template <typename Value>
void attr_out(std::ostream& out, std::string_view name, Value value) {
	using namespace std::literals; 
	out << name << "=\""sv << value << "\" "sv;
}

void replace_all(std::string& str, const std::string& from, const std::string& to);
}// namespace detail

struct Rgb { 
	Rgb() = default;
	Rgb(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue (b) {}
	uint8_t red = 0, green = 0, blue = 0; 
};

struct Rgba { 
	Rgba() = default;
	Rgba(uint8_t r, uint8_t g, uint8_t b, double a) : red(r), green(g), blue (b), opacity(a) {}
	uint8_t red = 0, green = 0, blue = 0; 
	double opacity = 1.0; 
};

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
// Объявив в заголовочном файле константу со спецификатором inline,
// мы сделаем так, что она будет одной на все единицы трансляции,
// которые подключают этот заголовок.
// В противном случае каждая единица трансляции будет использовать свою копию этой константы
inline const Color NoneColor;

struct ColorPrinter {
public:
	ColorPrinter(std::ostream& os) : os_(os) {}
    void operator()([[maybe_unused]]const std::monostate& none) const {
		using namespace std::literals; 
		os_ << "none"sv; 
	} 
	void operator()(const std::string& value) const {
		using namespace std::literals; 
		os_ << /*(value.empty() ? "none"sv :*/ value/*)*/; 
	}
    void operator()(const Rgb& value) const { 
		using namespace std::literals; 
		os_ << "rgb("sv << static_cast<int>(value.red) << ","sv << static_cast<int>(value.green) << ","sv << static_cast<int>(value.blue) << ")"sv; 
	}
    void operator()(const Rgba& value) const { 
		using namespace std::literals; 
		os_ << "rgba("sv << static_cast<int>(value.red) << ","sv << static_cast<int>(value.green) << ","sv << static_cast<int>(value.blue) << ","sv << value.opacity << ")"sv; 
	}
private:
	std::ostream& os_;
};

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

std::ostream& operator<<(std::ostream& os, const StrokeLineCap& line_cap);

std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& line_join);

std::ostream& operator<<(std::ostream& os, const Color& clr);
/*
class Attr {
public: 
    virtual void Out(ostream& os) = 0;
private:
    std::string name;
}
class Color : public Attr {
public:
    
}*/
    
    
template <typename Owner>
class PathProps {
public:
    Owner& SetFillColor (Color color) {
        fill_color_ = std::move(color);
        return AsOwner();
    }
    Owner& SetStrokeColor(Color color) {
        stroke_color_ = std::move(color);
        return AsOwner();
    }
	Owner& SetStrokeWidth(double width) {
        stroke_width_ = width;
        return AsOwner();
    }
	Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
        line_cap_ = line_cap;
        return AsOwner();
    }
	Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
        line_join_ = line_join;
        return AsOwner();
    }
protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const {
		using namespace std::literals;

		if (fill_color_/* && !std::holds_alternative<std::monostate>(*fill_color_)*/) {
			out << "fill=\""sv << *fill_color_ << "\" "sv;
		}
		if (stroke_color_/* && !std::holds_alternative<std::monostate>(*stroke_color_)*/) {
			out << "stroke=\""sv << *stroke_color_ << "\" "sv;
		}
		if (stroke_width_) {
			out << "stroke-width=\""sv << *stroke_width_ << "\" "sv;
		}
		if (line_cap_) {
			out << "stroke-linecap=\""sv << *line_cap_ << "\" "sv;
		}
		if (line_join_) {
			out << "stroke-linejoin=\""sv << *line_join_ << "\" "sv;
		}
	}

private:
    Owner& AsOwner() {
        // static_cast безопасно преобразует *this к Owner&,
        // если класс Owner — наследник PathProps
        return static_cast<Owner&>(*this);
    }

    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
	std::optional<StrokeLineCap> line_cap_;
	std::optional<StrokeLineJoin> line_join_;
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

class ObjectContainer {
public:
	template <typename Obj>
	void Add(Obj obj);
	virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
	virtual ~ObjectContainer() = default;
protected: 
	std::vector<std::unique_ptr<Object>> objects_;
};

template <typename Obj>
void ObjectContainer::Add(Obj obj) {
	objects_.emplace_back(std::make_unique<Obj>(std::move(obj)));
} 

class Drawable {
public:
	virtual void Draw(ObjectContainer& obj_cont) const = 0;
	virtual ~Drawable() = default;
};

/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> {
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);
	
private:
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final : public Object, public PathProps<Polyline>{
public:
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);

private:
	void RenderObject(const RenderContext& context) const override;
	
	std::vector<Point> points_;
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final : public Object, public PathProps<Text> {
public:
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

private:
	void RenderObject(const RenderContext& context) const override;
	
	Point pos_;
	Point offset_;
	uint32_t size_ = 1;
	std::string font_family_;
	std::string font_weight_;
	std::string data_;
};

class Document : public ObjectContainer {
public:
    /*
     Метод Add добавляет в svg-документ любой объект-наследник svg::Object.
     Пример использования:
     Document doc;
     doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));
    */
   

    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;
};	
}  // namespace svg
