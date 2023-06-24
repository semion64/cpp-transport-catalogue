#pragma once
#include "json.h"
#include <optional>

namespace json{
	
class Builder;

namespace detail {	
class ItemContext;
class DictItemContext;
class ArrayItemContext;
class KeyItemContext;
}

class Builder {
	struct WaitValue {
		Node* dict_node;
		std::string key;
	};
public:
	Builder() { };
	
	detail::KeyItemContext Key(std::string key);
	detail::DictItemContext StartDict();
	detail::ArrayItemContext StartArray();
	
	Builder& Value(Node::Value value);
	Builder& EndDict();
	Builder& EndArray();
	
	json::Node Build();
private:	
	json::Node root_;
	bool empty_builder = true;
	std::vector<WaitValue> wait_value_;
	std::vector<Node*> nodes_stack_;
	void CheckComplete();
};

namespace detail {
class ItemContext {
public:
	ItemContext(Builder& b) : builder_(b) { }
protected:
	Builder& builder_;
};

class DictItemContext : public ItemContext {
public:
	DictItemContext(Builder& b);
	KeyItemContext Key(std::string key);
	Builder& EndDict();
};

class KeyItemContext : public ItemContext {
public:
	KeyItemContext(Builder& b);
	DictItemContext Value(Node::Value value);
	DictItemContext StartDict();
	ArrayItemContext StartArray();
};

class ArrayItemContext : public ItemContext {
public:
	ArrayItemContext(Builder& b);
	ArrayItemContext Value(Node::Value value);
	DictItemContext StartDict();
	ArrayItemContext StartArray();
	Builder& EndArray();
};
} // end ::detail
} // end ::json
