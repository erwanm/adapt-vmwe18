/*
 * feature_selection_cliques.h
 *
 *  Created on: 3 May 2018
 *      Author: moreaue
 */

#ifndef FEATURE_SELECTION_CLIQUES_H_
#define FEATURE_SELECTION_CLIQUES_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <algorithm>



#include "feature_selection.h"

//#define DEBUG 1

using namespace std;

template<typename TESTID>
class feature_selection_cliques : public feature_selection<TESTID> {
 public:
	feature_selection_cliques(unordered_map<CLASSID, unsigned long> &totalNodesByCliqueLevel,
						unordered_map<CLIQUEID, unsigned long> &cliqueFreq,
						unordered_map<CLASSID, unordered_map<TESTID, unsigned long>> &testFreqCliqueLevel,
						unordered_map<CLIQUEID, unordered_map<TESTID, unsigned long>> &testsFreq,
						bool weightCliqueProb,
						int selectHighestValues,
						CLASSID defaultClass);
	// adds to relevance vector for one test, all cliques (unless a clique is not needed)
	// IMPLEMENTED BY THIS CLASS, calls computeRelevanceTestClique
	virtual void computeRelevanceTest(vector<TESTRELEVANCE<TESTID>> &recipient, TESTID t, CLASSID level, unsigned long freqThisTestCliqueLevel, unsigned long  totalCliqueLevel);

	virtual double computeRelevanceTestClique(unordered_map<unsigned char, unordered_map<unsigned char,unsigned long>> &confusionMatrix, unsigned long totalCliqueLevel, unsigned long freqThisTestCliqueLevel) = 0;
};


#endif /* FEATURE_SELECTION_CLIQUES_H_ */
