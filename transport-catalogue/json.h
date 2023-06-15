#pragma once

#include <cassert>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

namespace json {
	
class Node;
class Document;

class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

struct Bool { 
	bool value; 
	std::string ToString() const {
		return value ? "true" : "false";
	}
};

using Number = std::variant<int64_t, double>;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
using Value = std::variant<std::nullptr_t, Array, Dict, Bool, int64_t, double, std::string>;

bool operator==(const Node& r, const Node& l);
bool operator!=(const Node& r, const Node& l);
bool operator==(const Document& r, const Document& l);
bool operator!=(const Document& r, const Document& l);
bool operator==(const Bool& r, const Bool& l);
bool operator!=(const Bool& r, const Bool& l);

class Node {
public:
	Node() = default;
    Node(Array array);
    Node(Dict map);
	Node(bool value);
    Node(int64_t value);
    Node(double value);
    Node(std::nullptr_t);
    Node(std::string value);
	
	bool IsArray() const;
	bool IsMap() const;
	bool IsInt() const;
	bool IsDouble() const;
	bool IsPureDouble() const;
	bool IsBool() const;
	bool IsString() const;
	bool IsNull() const;
	
	const Array& AsArray() const;
	const Dict& AsMap() const;
	int64_t AsInt() const;
	double AsDouble() const;
	bool AsBool() const;
	const std::string& AsString() const;
	
	const Value& GetValue() const;

private:
    Value value_;
	
	template <typename T>	
	bool CheckType() const;
};

template <typename T>	
bool Node::CheckType() const {
	return std::holds_alternative<T>(value_);
}

class Document {
public:
    explicit Document(Node root);
    const Node& GetRoot() const;
private:
    Node root_;
};

Document Load(std::istream& input);
void Print(const Document& doc, std::ostream& output);

// Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;
	bool is_key_value = false;
    void PrintIndent(bool is_container = false) const {
		if(!is_key_value || is_container) {
			for (int i = 0; i < indent; ++i) {
				out.put(' ');
			}
		} 
    }

    // Возвращает новый контекст вывода с увеличенным смещением
    PrintContext Indented(bool is_key_value = false) const {
        return {out, indent_step, indent_step + indent, is_key_value};
    }
};

}  // namespace json
