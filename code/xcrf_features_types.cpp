

#include "xcrf_features_types.h"


/*
// not really useful?
ATOMICTESTID encodeAtomicTest(XPATHID xpathId, ATTRID attrId, VALID valId) {
  ATOMICTESTID res;
//	res->nbTests = 1;
	res.xpathId = xpathId;
	res.attrId = attrId;
	res.valId = valId;
	return res;

}
*/




CLIQUEID encodeClique(CLASSID *cliques, CLASSID level) {
	CLIQUEID res = 0;
	for (CLASSID ft=1; ft<=level; ft++) {
		res = res | cliques[ft-1];
		res = res << 8;
		//		cerr << "encoding clique key, ft ="<<(int) ft<< " ; cliques[ft-1] = "<<(int)cliques[ft-1]<< "; res = "<<(int)res<<endl;
	}
	res = res | level;
	//	cerr << "final res = "<<res<<endl;
	return res;
}



// returns an dynamic array of length cliqueLevel (to be freed by caller)
CLASSID *decodeClique(CLIQUEID key, CLASSID *level) {
	*level = key & 255;
//	cerr <<"decode key = "<<key<<"; level = "<< (int)*level<<endl;
	if ((*level == 0) || (*level>3)) {
	  cerr << "Bug: level = "<<(int) *level<<" when decoding clique key"<<endl;
		exit(10);
	}
	CLASSID *res = (CLASSID *) malloc(sizeof(CLASSID) * (*level));
	key= key >> 8;
	for (CLASSID ft=*level; ft>=1; ft--) {
		res[ft-1] = key & 255;
		key= key >> 8;
	}
	return res;
}

