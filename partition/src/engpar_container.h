#ifndef ENGPAR_CONTAINER_H
#define ENGPAR_CONTAINER_H

#include <sstream>
#include <unordered_map>
namespace engpar {

  template <class T> class Container {
    typedef std::unordered_map<int,T> Data;

  public:
    typedef typename Data::iterator iterator;
    Container() {}
    iterator begin() {
      return d.begin();
    }
    typedef std::pair<const int, T> Item;
    const Item* iterate(iterator& itr) {
      if( itr == d.end() ) 
	return NULL;
      else
	return &(*itr++);
    }
    iterator end() {
      return d.end();
    }
    T& operator[](int key) {
      return d[key];
    }
    T get(int key) {
      return d[key];
    }
    void set(int key, T value) {
      d[key] = value;
    }
    bool has(int key) {
      return (d.count(key) != 0);
    }
    size_t size() {
      return d.size();
    }
    std::string print(const char* name) {
      std::stringstream s;
      s << name << "\n";
      const Item* i;
      iterator itr = begin();
      while( (i = iterate(itr)) ) 
	s << "  "<<i->first << " -> " << i->second << "\n";
      return s.str();
    }
  protected:
    Data d;
    
  };
}
#endif
