// -*- C++ -*-


#ifndef MAP_H_
#define MAP_H_

#include "list.h"
typedef List Container;

class map {
  Container c;
public:

  typedef void* address;
  typedef List::iter iter;

  struct Pair {
    Pair(address f, address s) : first(f), second(s) {}
    Pair() : first(0), second(0) { }
    address first;
    address second;
  };

  map() { }

  ~map() {
    for (iter i = c.end(); i != c.end(); i++) {
      delete (Pair*)(*i);
    }
  }
  
  Pair* begin();
  Pair* end();
  
  address& operator[](address key)
  {
    address* found = (address*)0;
    for (iter i = c.begin(); i != c.end(); i++) {
      if (((Pair*)*i)->first == key)
	found = &((Pair*)*i)->second;
    }
    if (! found) {
      iter tmp = c.insert(c.begin(), new Pair(key,0));
      found = &((Pair*)*tmp)->second;
    }
    return *found;
  }

  void erase(address key)
  {
    for (iter i = c.begin(); i != c.end(); i++) {
      if (((Pair*)*i)->first == key) {
	delete (Pair*)*i;
	c.erase(i); break;
      }
    }
  }
};

#endif




