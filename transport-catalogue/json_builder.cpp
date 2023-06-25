#include "json_builder.h"
#include <sstream>

namespace json{
	
void Builder::CheckComplete() {
	if(nodes_stack_.size() == 0 && root_) {
		throw std::logic_error("try call construct method after builder complete");
	}
}
    
detail::KeyItemContext Builder::Key(std::string key) {
	CheckComplete();
	if(wait_value_.size() > 0) {
		throw std::logic_error("key after key");
	}
	
	if(nodes_stack_.size() == 0 || !nodes_stack_.back()->IsDict()) {
		throw std::logic_error("key (" + key + ") locate out of any dictionary");
	}
	
	wait_value_.push_back(WaitValue{nodes_stack_.back(), key});
	return detail::KeyItemContext(*this);
}

Builder& Builder::Value(Node::Value value) {
	CheckComplete();
	Node node;
	std::visit([&node](const auto& v) { node = Node(v);}, value);

	if(!root_) {
		*root_ = std::move(node);
	} 
	else if(nodes_stack_.size() == 0) {
		throw std::logic_error("try add value after build completed");
    }
    else {
        if(nodes_stack_.back()->IsArray()){
            nodes_stack_.back()->AsArray().push_back(std::move(node));
        }
        else if(nodes_stack_.back()->IsDict()) {
            if(wait_value_.size() == 0) {
                throw std::logic_error("dictionary key must be a string");
            }
            nodes_stack_.back()->AsDict()[wait_value_.back().key] = std::move(node);
            wait_value_.pop_back();
        }
    }
	
	return *this;
}

detail::DictItemContext Builder::StartDict() {
	StartItem<Dict>();
	return detail::DictItemContext(*this);
}

Builder& Builder::EndDict() {
	return EndItem([this](){ return nodes_stack_.back()->IsDict(); });	
}

detail::ArrayItemContext Builder::StartArray() {
	StartItem<Array>();
	return detail::ArrayItemContext(*this);
}

Builder& Builder::EndArray() {
    return EndItem([this](){ return nodes_stack_.back()->IsArray(); });	
}

json::Node Builder::Build() {
	if(!root_) {
		throw std::logic_error("builder is empty");
	}
	
	if(nodes_stack_.size() > 0 || wait_value_.size() > 0) {
		throw std::logic_error("not all structers ended");
	}
	
	return *root_;
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
