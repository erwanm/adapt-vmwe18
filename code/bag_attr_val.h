/*
 * TODO
 * 
 *
 */


#ifndef BAG_ATTR_VAL_H
#define BAG_ATTR_VAL_H

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

#include "bidirectional_string_index.h"

using namespace std;

typedef unsigned short ATTRID;
typedef unsigned long  VALID;

template <typename T> class bag_attr_val {
 public:

  bag_attr_val(bidirectional_string_index<ATTRID> &attrIndex, unordered_map<ATTRID, bidirectional_string_index<VALID>> &valIndexMap);

  // adds a new entry to the map
  // if the entry alredy existed, it is overwritten
  void add(const string attr, const string val, T *o);

  // returns true if there is an entry for the pair (attr,val), false otherwise
  bool exists(const string attr, const string val);

  // returns the object corresponding to the entry (attr,val) if it exists in the map, or NULL otherwise.
  T *getCheck(const string attr, const string val);

  // returns the object corresponding to the entry (attr,val), which must be present in the map
  // Caution: things might go horribly wrong if the entry does not exist in the map!
  T *get(const string attr, const string val);


  // Caution: assumes that objects can be printed.
  void print();

 private:
  bidirectional_string_index<ATTRID> &attrIndex;  
  unordered_map<ATTRID, bidirectional_string_index<VALID>> &valIndexMap;
  unordered_map<ATTRID, unordered_map<VALID, T*>> map;

  bidirectional_string_index<VALID> &getCreateAttrValIndexMap(ATTRID attrIndex);
};


// horrible c++ style inclusion of the implemented member functions,
// made necessary by the use of templates.
// see https://stackoverflow.com/questions/495021/why-can-templates-only-be-implemented-in-the-header-file
// see https://stackoverflow.com/questions/8752837/undefined-reference-to-template-class-constructor#8752879
#include "bag_attr_val.cpp"

#endif
