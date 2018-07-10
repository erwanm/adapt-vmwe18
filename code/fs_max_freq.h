/*
 * fs_max_freq.h
 *
 *  Created on: 3 May 2018
 *      Author: moreaue
 */

#ifndef FS_MAX_FREQ_H_
#define FS_MAX_FREQ_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <algorithm>



#include "feature_selection.h"



using namespace std;

template<typename TESTID>
class fs_max_freq : public feature_selection<TESTID> {
 public:
	fs_max_freq(unordered_map<CLASSID, unsigned long> &totalNodesByCliqueLevel,
			     unordered_map<CLIQUEID, unsigned long> &cliqueFreq,
				 unordered_map<CLASSID, unordered_map<TESTID, unsigned long>> &testFreqCliqueLevel,
				 unordered_map<CLIQUEID, unordered_map<TESTID, unsigned long>> &testsFreq,
				 bool weightCliqueProb,
				 CLASSID defaultClass);


	// adds to relevance vector for one test, all cliques (unless a clique is not needed)
	virtual void computeRelevanceTest(vector<TESTRELEVANCE<TESTID>> &recipient, TESTID t, CLASSID level, unsigned long freqThisTestCliqueLevel, unsigned long  totalCliqueLevel);
};



#endif
