
#pragma once

#include <vector>
#include <unordered_map>
#include <iostream>
#include <typeindex>
#include <algorithm>
#include <stdexcept>
#include <functional>

//! A lightweight observer pattern implementation, partially based on http://codereview.stackexchange.com/a/92202

namespace lapkt { namespace events {

class Subject;
class Event {
public:
	//! We force at least one virtual method, so that derived classes are polimorphic and
	//! typeid works well on them (otherwise the typeid will be determined statically)
	virtual ~Event() = default;
};


class Observer {
public:
	virtual ~Observer() {}
	virtual void onNotify(Subject& subject, const Event& event) = 0;
};

class Subject {
public:
	void subscribe(Observer& observer) {
		observers.push_back(&observer);
	}
	
	void unsubscribe(Observer& observer) {
		observers.erase(std::remove(std::begin(observers), std::end(observers), &observer), std::end(observers));
	}
	
	void notify(const Event& event) {
		for(Observer* observer: observers) {
			observer->onNotify(*this, event);
		}
	}

protected:
	//! The list of all observers that observe events that happen on this subject
	std::vector<Observer*> observers;
};

//!
class EventHandler: public Observer {
public:
	void onNotify(Subject& subject, const Event& event) override {
		auto it = handlers.find(std::type_index(typeid(event)));
		if (it != handlers.end()) {
			it->second(subject, event);
		}
	}
	
	template<typename T>
	void registerEventHandler(std::function<void(Subject&, const Event&)> handler) {
		handlers[std::type_index(typeid(T))] = handler;
	}

protected:
	//! A map from (type_index's of) events to the particular function that this handler
	//! invokes upon the occurrence of each event
	std::unordered_map<std::type_index, std::function<void(Subject&, const Event&)>> handlers;
};

//! A helper to register vectors of observer pointers.
template <typename ObserverT>
void subscribe(Subject& subject, const std::vector<ObserverT>& observers) {
	for (auto& observer:observers) {
		subject.subscribe(*observer);
	}
}


//!
//! Some standard search events
//!
template <typename NodeT>
class NodeEvent : public Event {
public:
	NodeEvent(NodeT& node_) : node(node_) {}
	NodeT& node;
};

template <typename NodeT>
class NodeOpenEvent : public NodeEvent<NodeT> {
public:
	using BaseClass = NodeEvent<NodeT>;
	NodeOpenEvent(NodeT& node_) : BaseClass(node_) {}
};

template <typename NodeT>
class GoalFoundEvent : public NodeEvent<NodeT> {
public:
	using BaseClass = NodeEvent<NodeT>;
	GoalFoundEvent(NodeT& node_) : BaseClass(node_) {}
};

template <typename NodeT>
class NodeExpansionEvent : public NodeEvent<NodeT> {
public:
	using BaseClass = NodeEvent<NodeT>;
	NodeExpansionEvent(NodeT& node_) : BaseClass(node_) {}
};

template <typename NodeT>
class NodeCreationEvent : public NodeEvent<NodeT> {
public:
	using BaseClass = NodeEvent<NodeT>;
	NodeCreationEvent(NodeT& node_) : BaseClass(node_) {}
};

} } // namespaces