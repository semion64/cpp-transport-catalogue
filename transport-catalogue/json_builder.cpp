#include "json_builder.h"
#include <sstream>

namespace json{
	
detail::KeyItemContext Builder::Key(std::string key) {
	CheckComplete();
	if(Back()->IsString()) {
		throw std::logic_error("key after key");
	}
	
	if(Empty() || !Back()->IsDict()) {
		throw std::logic_error("key (" + key + ") locate out of any dictionary");
	}
	
	Push(new Node(key));
	return detail::KeyItemContext(*this);
}

Builder& Builder::Value(Node::Value value) {
	CheckComplete();
	Node node;
	std::visit([&node](const auto& v) { node = Node(v);}, value);

	if(!root_) {
		*root_ = std::move(node);
	} 
	else if(Empty()) {
		throw std::logic_error("try add value after build completed");
    }
    else {
        if(Back()->IsArray()){
            Back()->AsArray().push_back(std::move(node));
        }
        else if(Back()->IsString()) {
            auto key = Back()->AsString();
            delete Back();
            Pop();
            Back()->AsDict()[key] = std::move(node);
        }
        else {
            throw std::logic_error("dictionary key must be a string");
		}
    }
	
	return *this;
}

detail::DictItemContext Builder::StartDict() {
	StartItem<Dict>();
	return detail::DictItemContext(*this);
}

Builder& Builder::EndDict() {
	return EndItem([this](){ return Back()->IsDict(); });	
}

detail::ArrayItemContext Builder::StartArray() {
	StartItem<Array>();
	return detail::ArrayItemContext(*this);
}

Builder& Builder::EndArray() {
    return EndItem([this](){ return Back()->IsArray(); });	
}

json::Node Builder::Build() {
	if(!root_) {
		throw std::logic_error("builder is empty");
	}
	
	if(!Empty()) {
		throw std::logic_error("not all structers ended");
	}
	
	return *root_;
}

void Builder::CheckComplete() {
	if(Empty() && root_) {
		throw std::logic_error("try call construct method after builder complete");
	}
}

Node* Builder::Back() {
	return nodes_stack_.back();
}

void Builder::Push(Node* node) {
	nodes_stack_.push_back(node);
}

void Builder::Pop() {
	nodes_stack_.pop_back();
}

bool Builder::Empty() {
	return nodes_stack_.size() == 0;
}

namespace detail {
	
KeyItemContext::KeyItemContext(Builder& b) : ItemContext(b){}
DictItemContext KeyItemContext::Value(Node::Value value) {
	builder_.Value(std::move(value));
	return DictItemContext(builder_);
}

DictItemContext::DictItemContext(Builder& b) : builder_(b) {}
KeyItemContext DictItemContext::Key(std::string key) {
	builder_.Key(std::move(key));
	return KeyItemContext(builder_);
}

Builder& DictItemContext::EndDict() {
	return builder_.EndDict();
}

ArrayItemContext::ArrayItemContext(Builder& b) : ItemContext(b){}
ArrayItemContext ArrayItemContext::Value(Node::Value value) {
	builder_.Value(std::move(value));
	return ArrayItemContext(builder_);
}

DictItemContext ItemContext::StartDict() {
	builder_.StartDict();
	return DictItemContext(builder_);
}

ArrayItemContext ItemContext::StartArray() {
	builder_.StartArray();
	return ArrayItemContext(builder_);
}

Builder& ArrayItemContext::EndArray() {
	return builder_.EndArray();
}

} // end ::detail
} // end ::json
