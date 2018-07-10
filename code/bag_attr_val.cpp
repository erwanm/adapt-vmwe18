
// nope, see .h for explanation
//#include "bag_attr_val.h"

template<class T>
bag_attr_val<T>::bag_attr_val(bidirectional_string_index<ATTRID> &attrIndex, unordered_map<ATTRID, bidirectional_string_index<VALID>> &valIndexMap) : attrIndex(attrIndex), valIndexMap(valIndexMap) {
}



template<class T>
void bag_attr_val<T>::add(const string attr, const string val, T *o) {
  ATTRID attrI = attrIndex.getCreate(attr);
  bidirectional_string_index<VALID>& valIndex = getCreateAttrValIndexMap(attrI);
  VALID valI = valIndex.getCreate(val);
  auto it = map.find(attrI);
  if (it == map.end()) {
    unordered_map<VALID, T*> m;
    m[valI] = o;
    map[attrI] = m;
  } else {
    it->second[valI] = o;
  }
}



template<class T>
bool bag_attr_val<T>::exists(const string attr, const string val) {
  return (getCheck(attr, val) != NULL);
}

template <class T>
T *bag_attr_val<T>::getCheck(const string attr, const string val) {
  ATTRID attrI = attrIndex.get(attr);
  if (attrI == 0) {
    return NULL;
  } else {
    auto it = valIndexMap.find(attrI);
    if (it == valIndexMap.end()) {
      return NULL;
    } else {
      VALID valI = it->second.get(val);
      if (valI == 0) {
	return NULL;
      } else {
	auto it2 = map.find(attrI);
	if (it2 == map.end()) {
	  return NULL;
	} else {
	  auto it3 = it2->second.find(valI);
	  if (it3 == it2->second.end()) {
	    return NULL;
	  } else {
	    return it3->second;
	  }
	}
      }
    }
  }
}
  

template<class T>
T *bag_attr_val<T>::get(const string attr, const string val) {
  ATTRID attrI = attrIndex.get(attr);
  return map[attrI][valIndexMap[attrI].get(val)];
}


template<class T>
void bag_attr_val<T>::print() {
  cout << "printing..."<<endl;
  for (auto it1 : map) {
    ATTRID attrI = it1.first;
    cout <<"ATTR "<<attrIndex.get(attrI) <<"("<<attrI<<"):\t";
    for (auto it2 : it1.second) {
      VALID valI = it2.first;
      T *o = it2.second;
      cout <<valIndexMap[attrI].get(valI) <<"("<<valI<<"):"<<*o<<"\t";
    }
    cout << endl;
  }
}



template<class T>
bidirectional_string_index<VALID>& bag_attr_val<T>::getCreateAttrValIndexMap(ATTRID attrIndex) {
  auto it = valIndexMap.find(attrIndex);
  if (it == valIndexMap.end()) {
    valIndexMap[attrIndex] = bidirectional_string_index<VALID>();
    return valIndexMap[attrIndex];
  } else {
    return it->second;
  }
}
