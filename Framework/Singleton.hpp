// Singleton.hpp

#ifndef SINGLETON_HPP
#define SINGLETON_HPP

#include <string.h> // NULL

template <typename T>
class Singleton {
	static T *instance;
public:
	static T *getInstance() {
		if(!instance)
			instance = new T();
		return instance;
	}

	virtual ~Singleton() {
		instance = NULL;
	}
protected:
	Singleton() {}
};

template <typename T>
T *Singleton<T>::instance = NULL;

#endif
