#include "json.h"

using namespace std;

namespace json {

bool operator==(const Node& r, const Node& l) {
	return r.GetValue().index() == l.GetValue().index() && r.GetValue() == l.GetValue();
}
bool operator!=(const Node& r, const Node& l) {
	return !(r == l);
}
bool operator==(const Document& r, const Document& l) {
	return r.GetRoot() == l.GetRoot();
}
bool operator!=(const Document& r, const Document& l) {
	return !(r == l);
}
bool operator==(const Bool& r, const Bool& l) {
	return r.value == l.value;
}
bool operator!=(const Bool& r, const Bool& l) {
	return !(r == l);
}

// ----------------------------------------------------------- Node declaration--------------------------------------------------------------------
Node::Node(Array array) : value_(move(array)) {}
Node::Node(Dict map) : value_(move(map)) {}
Node::Node(bool value) : value_(Bool{value}) {}
Node::Node(int value) : value_(value) {}
Node::Node(double value) : value_(value) {}
Node::Node(string value) : value_(move(value)) {}
Node::Node(std::nullptr_t) : Node() {} 

const Value& Node::GetValue() const { return value_; }

bool Node::IsArray() const {return CheckType<Array>(); }
bool Node::IsMap() const {return CheckType<Dict>(); }
bool Node::IsInt() const {return CheckType<int>(); }
bool Node::IsDouble() const {return CheckType<int>() || CheckType<double>(); }
bool Node::IsPureDouble() const {return CheckType<double>(); }
bool Node::IsBool() const {return CheckType<Bool>(); }
bool Node::IsString() const {return CheckType<std::string>(); }
bool Node::IsNull() const {return CheckType<std::nullptr_t>(); }

const Array& Node::AsArray() const {
	if(!IsArray()){ throw std::logic_error("value is not array"); }
	return std::get<Array>(value_);
}
const Dict& Node::AsMap() const {
	if(!IsMap()){ throw std::logic_error("value is not map"); }
	//PrintValue(value_, cerr);
	return std::get<Dict>(value_);
}
int Node::AsInt() const {
	if(IsInt()){  
		return get<int>(value_);
	}
	throw std::logic_error("value is not int");
}
double Node::AsDouble() const  {
	if(IsInt()){ 
		return std::get<int>(value_);
	}
	
	if(IsDouble()){ 
		return std::get<double>(value_);
	}	
	
	throw std::logic_error("value is not double");
}

bool Node::AsBool() const {
	if(!IsBool()){ throw std::logic_error("value is not bool"); }
	return std::get<Bool>(value_).value;
}
const std::string& Node::AsString() const {
	if(!IsString()){ throw std::logic_error("value is not string"); }
	return std::get<std::string>(value_);
}

// ----------------------------------------------------------------------Parsing extract functions-------------------------------------------------------------
namespace detail {
namespace parser {
	
std::string ExtractKeyWord(std::istream& input);
Number ExtractNumber(std::istream& input);
std::string ExtractString(std::istream& input);

std::string ExtractKeyWord(std::istream& input) {
    using namespace std::literals;

    std::string parsed_word;
	while (!std::isspace(input.peek()) && input.peek() > 0 && input.peek() != ',' && input.peek() != ']' && input.peek()!='}') {
        parsed_word += static_cast<char>(input.get());
    }
	
	return parsed_word;
}

Number ExtractNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
		
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
	
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } 
	else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
		
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return std::stoi(parsed_num);
            } 
			catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
		
        return std::stod(parsed_num);
    } 
	catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
std::string ExtractString(std::istream& input) {
    using namespace std::literals;
    
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
		
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } 
		else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } 
		else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } 
		else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
		
        ++it;
    }

    return s;
}
} // namespace parser

// ------------------------------------------------------------Load node functions-----------------------------------------------------------
namespace load {
	
Node LoadNode(istream& input);
Node LoadArray(istream& input);
Node LoadNumber(istream& input);
Node LoadString(istream& input);
Node LoadDict(istream& input);

Node LoadNode(istream& input) {
	// Пропускаем пробельные символы
    while (std::isspace(input.peek())) {
		input.get();
	}
	
	char c;
	input >> c;
    if (c == '[') {
        return LoadArray(input);
    } 
	else if (c == '{') {
        return LoadDict(input);
    } 
	else if (c == '"') {
        return LoadString(input);
	} 
	else if(std::isdigit(c) || c == '+' || c == '-') { 
		input.putback(c);
        return LoadNumber(input);
    } 
	else {
		input.putback(c);
		std::string key = move(parser::ExtractKeyWord(input));
		
		if(key == "null") {
			return Node();
		} 
		else if(key == "true") {
			return Node(true);
		}
		else if(key == "false") {
			return Node(false);
		}
		
        throw ParsingError("Key word (" + key + ") not supported");
	}
}

Node LoadNumber(istream& input) {
	Number n = parser::ExtractNumber(input);
	if(std::holds_alternative<int>(n)) {
		return Node(get<int>(n));
	}
	
	if(std::holds_alternative<double>(n)) {
		return Node(get<double>(n));
	}
	
    throw ParsingError("Load number exception");
}

Node LoadString(istream& input) {
    return Node(move(parser::ExtractString(input)));
}

Node LoadArray(istream& input) {
    Array result;
	char c;
    while(input >> c && c != ']') {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
	
	if(c != ']') {
		throw ParsingError("expected ']' int the end of Array");
	}
	
    return Node(move(result));
}

Node LoadDict(istream& input) {
    Dict result;
	char c; 
    while (input >> c && c != '}') {
        if (c == ',') {
            input >> c;
        }
		
        string key = parser::ExtractString(input);
        input >> c;
        result.insert({move(key), LoadNode(input)});
    }
	
	if(c != '}') {
		throw ParsingError("expected ']' int the end of Array");
	}

    return Node(move(result));
}
}  // namespace load


// ------------------------------------------------Print functions----------------------------------------------------------------------
namespace print {
void replace_all(std::string& str, const std::string& from, const std::string& to);

void PrintNode(const Node& node, std::ostream& out, const PrintContext& ctx);
void PrintValue(const std::nullptr_t&, std::ostream& out, const PrintContext& ctx);
void PrintValue(const Array& arr, std::ostream& out, const PrintContext& ctx);
void PrintValue(const Dict& Dict, std::ostream& out, const PrintContext& ctx);
void PrintValue(const std::string& s, std::ostream& out, const PrintContext& ctx);
void PrintValue(const Bool& value, std::ostream& out, const PrintContext& ctx);

void replace_all(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
}

// Шаблон, подходящий для вывода double и int
template <typename V>
void PrintValue(const V& value, std::ostream& out, const PrintContext& ctx) {
	ctx.PrintIndent();
    out << value;
}

void PrintNode(const Node& node, std::ostream& out, const PrintContext& ctx) {
    std::visit(
        [&out, &ctx](const auto& value){ PrintValue(value, out, ctx); },
        node.GetValue());
} 

// Перегрузка функции PrintValue для вывода значений null
void PrintValue(const std::nullptr_t&, std::ostream& out, const PrintContext& ctx) {
	ctx.PrintIndent();
    out << "null"sv;
}

// Перегрузка функции PrintValue для вывода значений null
void PrintValue(const Bool& value, std::ostream& out, const PrintContext& ctx) {
	ctx.PrintIndent();
    out << value.ToString();
}

// Перегрузка функции PrintValue для вывода значений null
void PrintValue(const std::string& str, std::ostream& out, const PrintContext& ctx) {
	ctx.PrintIndent();
	std::string s = str;
	
	replace_all(s, "\\"s, "\\\\"s);
	replace_all(s, "\""s, "\\\""s);
	replace_all(s, "\n"s, "\\n"s);
	replace_all(s, "\r"s, "\\r"s);
	
    out << "\""sv << s << "\""sv;
}

void PrintValue(const Array& arr, std::ostream& out, const PrintContext& ctx) {
	out << std::endl;
	ctx.PrintIndent(true);
	out << "["sv << std::endl;
	bool is_first = true;
	for(const auto& a : arr) {
		if(is_first) {
			is_first = false;
		}
		else {
			out << ", "sv << std::endl;
		}
		
		PrintNode(a, out, ctx.Indented());
	}
	
	out << std::endl;
	ctx.PrintIndent(true);
	out << "]"sv;
}

void PrintValue(const Dict& Dict, std::ostream& out, const PrintContext& ctx) {
	PrintContext curr_ctx = ctx.Indented();
	out << std::endl;
	ctx.PrintIndent(true);
    out << "{"sv << std::endl;
	bool is_first = true;
	for(const auto& [key, value] : Dict) {
		if(is_first) {
			is_first = false;
		}
		else {
			out << ", "sv << std::endl;
		}
		
		curr_ctx.PrintIndent();
		out << "\"" << key << "\"" << ": ";
		PrintNode(value, out, curr_ctx.Indented(true));
	}
	
	out << std::endl;
	
	ctx.PrintIndent(true);
	out << "}"sv;
}
} // namespace print
} // namespace detail

// -----------------------------------------Document declaration------------------------------------------------------------------
Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{detail::load::LoadNode(input)};
}

void Print(const Document& doc, std::ostream& output) {
    detail::print::PrintNode(doc.GetRoot(), output, {output, 4, 0});
}

}  // namespace json
