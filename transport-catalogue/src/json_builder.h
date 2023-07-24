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
	std::optional<json::Node> root_;
	std::vector<Node*> nodes_stack_;
	void CheckComplete();
	Node* Back();
	void Push(Node* node);
	void Pop();
	bool Empty();
	template <typename Container>
	void StartItem();

	template <typename CheckStack>
	Builder& EndItem(CheckStack check_stack);
};

template <typename Container>
void Builder::StartItem() {
	CheckComplete();
	Container cont;
	if(!root_) {
		root_ = Node(cont);
		Push(&(*root_));
	} 
	else {
		if(Back()->IsArray()) {
			Back()->AsArray().push_back(Node(cont));
			Push(&Back()->AsArray().back());
		}
		else if(Back()->IsString()) {
			auto key = Back()->AsString();
			delete Back();
			Pop();
			
			Back()->AsDict()[key] = Node(cont);
			Push(&Back()->AsDict()[key]);
		}
		else {
			throw std::logic_error("there are must be a key before value");
		}
	}
}

template <typename CheckStack>
Builder& Builder::EndItem(CheckStack check_stack) {
    if(Empty()) {
		std::logic_error("try call construct method after builder complete");
	}
	CheckComplete();
	if(!check_stack()) {
		throw std::logic_error("not StartItem() for calling EndItem()");
	}
	
	Pop();
	return *this;
}

namespace detail {
class ItemContext {
public:
	ItemContext(Builder& b) : builder_(b) { }
	DictItemContext StartDict();
	ArrayItemContext StartArray();
protected:
	Builder& builder_;
};

class DictItemContext {
public:
	DictItemContext(Builder& b);
	KeyItemContext Key(std::string key);
	Builder& EndDict();
private:
	Builder& builder_;
};

class KeyItemContext : public ItemContext {
public:
	KeyItemContext(Builder& b);
	DictItemContext Value(Node::Value value);
};

class ArrayItemContext : public ItemContext {
public:
	ArrayItemContext(Builder& b);
	ArrayItemContext Value(Node::Value value);
	Builder& EndArray();
};
} // end ::detail
} // end ::json
