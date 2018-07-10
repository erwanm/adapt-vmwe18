
#include "bidirectional_string_index.h"


template<typename UINT>
bidirectional_string_index<UINT>::bidirectional_string_index() {
  // init index vector with empty string at position 0 which must not be used.
  index.push_back("");
}




// returns the index of s in the map, possibly creating a new entry
template<typename UINT>
UINT bidirectional_string_index<UINT>::getCreate(const string &s) {
  auto it = map.find(s);
  if (it ==map.end()) {
    UINT n=index.size();
    map[s] = n;
    index.push_back(s);
    return n;
  } else {
    return it->second;
  }
}



// returns its index if s in the map, or 0 if s is not a member
template<typename UINT>
UINT bidirectional_string_index<UINT>::get(const string &s) {
  auto it = map.find(s);
  if (it ==map.end()) {
    return 0;
  } else {
    return it->second;
  }
}



// returns the string corresponding to index, or dies if i is out of bound.
template<typename UINT>
string & bidirectional_string_index<UINT>::get(UINT i) {
  if ((i>0) && (i < index.size())) {
    return index[i];
  } else {
    cerr << "Error: invalid index ("<<to_string(i)<<") in bidirectional_string_index::get(UINT)" << endl;
    print(cerr);
    cerr << endl;
    exit(10);
  }
}


template<typename UINT>
void bidirectional_string_index<UINT>::print(ostream &s) {
	s << "{";
	for (UINT i=1; i<index.size(); i++) {
		s <<" "<<to_string(i) <<":"<<index[i];
	}
	s<<" }";
}



template<typename UINT>
const unordered_map<string, UINT> &bidirectional_string_index<UINT>::getMap() const {
  return map;
}


template<typename UINT>
UINT bidirectional_string_index<UINT>::size() {
  return index.size()-1;
}

template class bidirectional_string_index<unsigned char>;
template class bidirectional_string_index<unsigned int>;
template class bidirectional_string_index<unsigned short>;
template class bidirectional_string_index<unsigned long>;
