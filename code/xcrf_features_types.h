/*
 * xcrf_features_types.h
 *
 *  Created on: 23 Apr 2018
 *      Author: moreaue
 */

#ifndef XCRF_FEATURES_TYPES_H_
#define XCRF_FEATURES_TYPES_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

#define HARDCODEDMAXTESTS 3


typedef unsigned char CLASSID;
typedef unsigned char ATTRID;
typedef unsigned long VALID;
typedef unsigned char XPATHID;
typedef unsigned long CLIQUEID;
typedef unsigned char NBTESTS;

using namespace std;


// no need to typedef structs in C++ -> https://stackoverflow.com/questions/2448242/struct-with-template-variables-in-c

// following https://stackoverflow.com/questions/17016175/c-unordered-map-using-a-custom-class-type-as-the-key
// for operator equal and hash function
// see also https://stackoverflow.com/questions/30149697/defining-hash-function-as-part-of-a-struct
struct ATOMICTESTID {
  
	XPATHID xpathId;
	ATTRID attrId;
	VALID valId;
  
  bool operator==(const ATOMICTESTID &other) const {
    return (xpathId == other.xpathId
            && attrId == other.attrId
            && valId == other.valId);
  }
  bool operator<(const ATOMICTESTID &other) const {
	  if (xpathId < other.xpathId) {
		  return true;
	  } else {
		  if (xpathId > other.xpathId) {
			  return false;
		  }
	  }
	  // xpath ==
	  if (attrId < other.attrId) {
		  return true;
	  } else {
		  if (attrId > other.attrId) {
			  return false;
		  }
	  } // attrId ==
	  if  (valId < other.valId) {
		  return true;
	  }
	  return false;
  }
  
};


/*
// TODO possibly better version of MULTITESTID using static array, size defined with template?
// problem: how to assign the size in the map?? maybe just casting?
template<unsigned char NBTESTS>
struct MULTITESTID {
  ATOMICTESTID tests[NBTESTS];
};
*/


// remark: equal and hash on the array of pointers would be ok,
// since two sequences of ATOMICTESTID pointers with the same content
// must point to the same addresses in the same order, so they get the
// same hash code
// relying on the equal operator defined for ATOMICTESTID
struct MULTITESTID {
  
  NBTESTS nbTests;
  ATOMICTESTID tests[HARDCODEDMAXTESTS];
  
  bool operator==(const MULTITESTID &other) const {
    if (nbTests == other.nbTests) {
      for (NBTESTS i=0; i< nbTests; i++) {
	if (!(tests[i] == other.tests[i])) {
	  return false;
	}
      }
      return true;
    } else {
      return false;
    }
  }

  bool operator<(const MULTITESTID &other) const {
	  if (nbTests < other.nbTests) {
		  return true;
	  } else {
		  if (nbTests > other.nbTests) {
			  return false;
		  }
	  }
	  // nbTests ==
      for (NBTESTS i=0; i< nbTests; i++) {
    		if (tests[i] < other.tests[i]) {
    			return true;
    		} else {
    			if (!(tests[i] == other.tests[i])) {
        			return false;
        		}
    		}
    		// here they are == so continue
      }
      // still == after all
      return false;
  }
};


namespace std
{
    template <>
    struct hash<ATOMICTESTID>
    {
        size_t operator()( const ATOMICTESTID& k ) const
        {
            // Compute individual hash values for first, second and third
            // http://stackoverflow.com/a/1646913/126995
            size_t res = 17;
            res = res * 31 + hash<XPATHID>()( k.xpathId );
            res = res * 31 + hash<ATTRID>()( k.attrId );
            res = res * 31 + hash<VALID>()( k.valId );
            return res;
        }
    };

    template <>
    struct hash<MULTITESTID>
    {
        size_t operator()( const MULTITESTID& k ) const
        {
            // Compute individual hash values for first, second and third
            // http://stackoverflow.com/a/1646913/126995
            size_t res = 17;
            res = res * 31 + hash<NBTESTS>()( k.nbTests );
	    for (int i=0; i <k.nbTests; i++) {
	      res = res * 31 + hash<ATOMICTESTID>()(k.tests[i]);
	    }
            return res;
        }
    };


}




//ATOMICTESTID encodeAtomicTest(XPATHID xpathId, ATTRID attrId, VALID valId);
//MULTITESTID encodeMultiTest(NBTESTS nbTests, );
CLIQUEID encodeClique(CLASSID *cliques, CLASSID level);
CLASSID *decodeClique(CLIQUEID key, CLASSID *level);




#endif /* XCRF_FEATURES_TYPES_H_ */
