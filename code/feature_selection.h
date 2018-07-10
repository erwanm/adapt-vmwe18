/*
 * feature_selection.h
 *
 *  Created on: 2 May 2018
 *      Author: moreaue
 */

#ifndef FEATURE_SELECTION_H_
#define FEATURE_SELECTION_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <algorithm>



#include "xcrf_features_types.h"


//#define DEBUG 1

using namespace std;

template<typename TESTID>
struct TESTRELEVANCE {
	CLIQUEID c;
	TESTID t;
	double value;
	  bool operator<(const TESTRELEVANCE &other) const {
		  return (value < other.value);
	  }
};


template<typename TESTID>
class feature_selection {
 public:
	feature_selection(unordered_map<CLASSID, unsigned long> &totalNodesByCliqueLevel,
						unordered_map<CLIQUEID, unsigned long> &cliqueFreq,
						unordered_map<CLASSID, unordered_map<TESTID, unsigned long>> &testFreqCliqueLevel,
						unordered_map<CLIQUEID, unordered_map<TESTID, unsigned long>> &testsFreq,
						bool weightCliqueProb,
						int selectHighestValues,
						CLASSID defaultClass);
	// initializes full relevance vector
	virtual void computeRelevance();
	// adds to relevance vector for one test, all cliques (unless a clique is not needed)
	virtual void computeRelevanceTest(vector<TESTRELEVANCE<TESTID>> &recipient, TESTID t, CLASSID level, unsigned long freqThisTestCliqueLevel, unsigned long  totalCliqueLevel) = 0;



	//not here
	// possible additional args: TESTID, CLIQUEID
//	virtual void computeRelevanceCliqueTest(int freqThisTestThisClique, int freqTotalCliqueLevel, int freqThisClique, int totalTestInThisCliqueLevel) = 0;

	unordered_map<CLASSID,vector<TESTRELEVANCE<TESTID>>> getMostRelevantTests(int n, int *nbSelected);

 protected:
  CLASSID cliqueLevel;
  unordered_map<CLASSID, unsigned long> &totalNodesByCliqueLevel;
  unordered_map<CLIQUEID, unsigned long> &cliqueFreq;
  unordered_map<CLASSID, unordered_map<TESTID, unsigned long>> &testFreqCliqueLevel;
  unordered_map<CLIQUEID, unordered_map<TESTID, unsigned long>> &testsFreq;
  // freqCNotC[cliqueLevel][cliqueId][T|F] = freq clique is T|F
  unordered_map<CLASSID, unordered_map<CLIQUEID, unordered_map<bool, unsigned long>>> freqCNotC;

  //relevance[cliqueLevel] = vector for this level
 unordered_map<CLASSID, vector<TESTRELEVANCE<TESTID>>> relevance;

 bool weightCliqueProb;
  int selectHighestValues;
  // just store the default clique for each clique level
  unordered_map<CLASSID, CLIQUEID> defaultClique;
};


#endif /* FEATURE_SELECTION_H_ */
