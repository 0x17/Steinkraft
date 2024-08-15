// Observable.hpp

#ifndef OBSERVABLE_HPP
#define OBSERVABLE_HPP

#include <list>

namespace as {

template <class T>
class Observer {
public:
	virtual ~Observer() {}
	virtual void update(T *changed) = 0;
};

template <class T>
class Observable {
public:
	virtual ~Observable() {}

	void addObserver(Observer<T> *o);
	// views should be allowed to cache if !immediate
	void notifyObservers(T *changed);

private:
	std::list<Observer<T> *> observers;
};

template <class T>
void Observable<T>::addObserver(Observer<T> *o) {
	observers.push_back(o);
}

template <class T>
void Observable<T>::notifyObservers(T *changed) {
	typename std::list<Observer<T> *>::iterator it;
	for (it = observers.begin(); it != observers.end(); ++it) {
		(*it)->update(changed);
	}
}

}

#endif
