/*
 * fs_cond_entropy.h
 *
 *  Created on: 3 May 2018
 *      Author: moreaue
 */

#ifndef FS_COND_ENTROPY_CLIQUE_H_
#define FS_COND_ENTROPY_CLIQUE_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <algorithm>



#include "feature_selection_cliques.h"



using namespace std;

template<typename TESTID>
class fs_cond_entropy_clique : public feature_selection_cliques<TESTID> {
 public:
	fs_cond_entropy_clique(unordered_map<CLASSID, unsigned long> &totalNodesByCliqueLevel,
			     unordered_map<CLIQUEID, unsigned long> &cliqueFreq,
				 unordered_map<CLASSID, unordered_map<TESTID, unsigned long>> &testFreqCliqueLevel,
				 unordered_map<CLIQUEID, unordered_map<TESTID, unsigned long>> &testsFreq,
				 bool weightCliqueProb,
				 CLASSID defaultClass);


        virtual double computeRelevanceTestClique(unordered_map<unsigned char, unordered_map<unsigned char,unsigned long>> &confusionMatrix, unsigned long totalCliqueLevel, unsigned long freqThisTestCliqueLevel);
};




#endif /* FS_COND_ENTROPY_CLIQUE_H_ */
