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
	std::optional<json::Node> root_;
	std::vector<WaitValue> wait_value_;
	std::vector<Node*> nodes_stack_;
	void CheckComplete();
	
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
		nodes_stack_.push_back(&(*root_));
	} 
	else {
		if(nodes_stack_.back()->IsArray()) {
			nodes_stack_.back()->AsArray().push_back(Node(cont));
			nodes_stack_.push_back(&nodes_stack_.back()->AsArray().back());
		}
		else if(nodes_stack_.back()->IsDict()) {
			if(wait_value_.size() == 0) { 
				throw std::logic_error("there are must be a key before value");
			}
			
			nodes_stack_.back()->AsDict()[wait_value_.back().key] = Node(cont);
			nodes_stack_.push_back(&nodes_stack_.back()->AsDict()[wait_value_.back().key]);
			wait_value_.pop_back();
		}
	}
}

template <typename CheckStack>
Builder& Builder::EndItem(CheckStack check_stack) {
    if(nodes_stack_.size() == 0) {
		std::logic_error("try call construct method after builder complete");
	}
	CheckComplete();
	if(!check_stack()) {
		throw std::logic_error("not StartItem() for calling EndItem()");
	}
	nodes_stack_.pop_back();
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
