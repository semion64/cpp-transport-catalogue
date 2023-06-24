#include "json_builder.h"
#include <sstream>

namespace json{
	
void Builder::CheckComplete() {
	if(nodes_stack_.size() == 0 && !empty_builder) {
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
	if(std::holds_alternative<std::string>(value)) {
		node = Node(std::move(std::get<std::string>(value)));
	}
	else if(std::holds_alternative<bool>(value)) {
		node = Node(std::get<bool>(value));
	}
	else if(std::holds_alternative<int>(value)) {
		node = Node(std::get<int>(value));
	}
	else if(std::holds_alternative<double>(value)) {
		node = Node(std::get<double>(value));
	}
	else if(std::holds_alternative<std::nullptr_t>(value)) {
		node = Node{nullptr};
	}
	else if(std::holds_alternative<Dict>(value)) {
		node = Node(std::move(std::get<Dict>(value)));
	}
	if(std::holds_alternative<Array>(value)) {
		node = Node(std::move(std::get<Array>(value)));
	}
	
	if(empty_builder) {
		empty_builder = false;
		root_ = node;
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
	CheckComplete();
	Dict dict;
	if(empty_builder) {
		empty_builder = false;
		root_ = Node(dict);
		nodes_stack_.push_back(&root_);
	} 
	else {
		if(nodes_stack_.back()->IsArray()) {
			nodes_stack_.back()->AsArray().push_back(Node(dict));
			nodes_stack_.push_back(&nodes_stack_.back()->AsArray().back());
		}
		else if(nodes_stack_.back()->IsDict()) {
			if(wait_value_.size() == 0) { 
				throw std::logic_error("there are must be a key before value");
			}
			
			nodes_stack_.back()->AsDict()[wait_value_.back().key] = Node(dict);
			nodes_stack_.push_back(&nodes_stack_.back()->AsDict()[wait_value_.back().key]);
			wait_value_.pop_back();
		}
	}
	
	return detail::DictItemContext(*this);
}

Builder& Builder::EndDict() {
    if(nodes_stack_.size() == 0) {
		std::logic_error("try call construct method after builder complete");
	}
	CheckComplete();
	if(!nodes_stack_.back()->IsDict()) {
		throw std::logic_error("not StartDict() for calling EndDict()");
	}
	nodes_stack_.pop_back();
	return *this;
}

detail::ArrayItemContext Builder::StartArray() {
	CheckComplete();
	Array arr;
	if(empty_builder) {
		empty_builder = false;
		root_ = Node(arr);
		nodes_stack_.push_back(&root_);
	} else {
		if(nodes_stack_.back()->IsArray()) {
			nodes_stack_.back()->AsArray().push_back(Node(arr));
			nodes_stack_.push_back(&nodes_stack_.back()->AsArray().back());
		}
		else if(nodes_stack_.back()->IsDict()) {
			if(wait_value_.size() == 0) { 
				throw std::logic_error("there are must be a key before value");
			}
			
			nodes_stack_.back()->AsDict()[wait_value_.back().key] = Node(arr);
			nodes_stack_.push_back(&nodes_stack_.back()->AsDict()[wait_value_.back().key]);
			wait_value_.pop_back();
		}
	}
	
	return detail::ArrayItemContext(*this);
}

Builder& Builder::EndArray() {
    if(nodes_stack_.size() == 0) {
		std::logic_error("try call construct method after builder complete");
	}
    
	CheckComplete();
	if(!nodes_stack_.back()->IsArray()) {
		throw std::logic_error("not StartArray() for calling EndArray()");
	}
	
	nodes_stack_.pop_back();
	
	return *this;
}

json::Node Builder::Build() {
	if(empty_builder) {
		throw std::logic_error("builder is empty");
	}
	
	if(nodes_stack_.size() > 0 || wait_value_.size() > 0) {
		throw std::logic_error("not all structers ended");
	}
	
	return root_;
}

namespace detail {
	
KeyItemContext::KeyItemContext(Builder& b) : ItemContext(b){}
DictItemContext KeyItemContext::Value(Node::Value value) {
	builder_.Value(std::move(value));
	return DictItemContext(builder_);
}

DictItemContext KeyItemContext::StartDict() {
	builder_.StartDict();
	return DictItemContext(builder_);
}

ArrayItemContext KeyItemContext::StartArray() {
	builder_.StartArray();
	return ArrayItemContext(builder_);
}


DictItemContext::DictItemContext(Builder& b) : ItemContext(b) {}
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

DictItemContext ArrayItemContext::StartDict() {
	builder_.StartDict();
	return DictItemContext(builder_);
}

ArrayItemContext ArrayItemContext::StartArray() {
	builder_.StartArray();
	return ArrayItemContext(builder_);
}

Builder& ArrayItemContext::EndArray() {
	return builder_.EndArray();
}

} // end ::detail
} // end ::json
