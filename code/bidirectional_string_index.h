/*
 *
 * Bidirectional string index:
 * assigns a unique int id to each string, can be queried from the string
 * or from the int id.
 * index 0 is not used, reserved for "not found";
 * 
 *
 */


#ifndef BIDIRECTIONAL_STRING_INDEX_H
#define BIDIRECTIONAL_STRING_INDEX_H

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

using namespace std;


template<typename UINT>
class bidirectional_string_index {

 public:
  bidirectional_string_index();
  
  // returns the index of s in the map, possibly creating a new entry
  UINT getCreate(const string &s);
  // returns its index if s in the map, or 0 if s is not a member
  UINT get(const string &s);
  // returns the string corresponding to index, or dies if i is out of bound.
  string &get(UINT i);
  // returns the private hash map as const, typically for iterating over all elements
  const unordered_map<string, UINT> &getMap() const;
  // returns the number of strings in the map
  UINT size();

  void print(ostream &s);

  
 private:
  unordered_map<string, UINT> map;
  vector<string> index;
  
  
};




#endif 
