/*
 * fs_max_freq.cpp
 *
 *  Created on: 3 May 2018
 *      Author: moreaue
 */


#include "fs_max_freq.h"

template<typename TESTID>
fs_max_freq<TESTID>::fs_max_freq(unordered_map<CLASSID, unsigned long> &totalNodesByCliqueLevel,
									unordered_map<CLIQUEID, unsigned long> &cliqueFreq,
									unordered_map<CLASSID, unordered_map<TESTID, unsigned long>> &testFreqCliqueLevel,
									unordered_map<CLIQUEID, unordered_map<TESTID, unsigned long>> &testsFreq,
									bool weightCliqueProb,
									CLASSID defaultClass) : feature_selection<TESTID>(totalNodesByCliqueLevel, cliqueFreq, testFreqCliqueLevel, testsFreq, weightCliqueProb, 1, defaultClass) {

}


// just split data
template<typename TESTID>
// this is done only for one test
void fs_max_freq<TESTID>::computeRelevanceTest(vector<TESTRELEVANCE<TESTID>> &recipient, TESTID t, CLASSID level, unsigned long freqThisTestCliqueLevel, unsigned long  totalCliqueLevel) {
	// get the min between test = T and test = F
	double v = freqThisTestCliqueLevel; // test = T selected
//	cerr << "start v = "<<v<<"; total = "<<totalCliqueLevel<<endl;
	if (totalCliqueLevel- freqThisTestCliqueLevel < v) {
	  //		cerr << "found a test more often true than false: v = "<<v<<"; total = "<<totalCliqueLevel<<endl;
		v = totalCliqueLevel- freqThisTestCliqueLevel; // test = F selected
	}
	//cerr << "end v = "<<v<<endl;
	v /= totalCliqueLevel; // normalize

	for (auto &itClique : this->freqCNotC[level]) {// this is done only for one test// we did it just to get the clique id at this level.
		CLIQUEID thisClique = itClique.first;

		double v_thisClique = 0.0;
		if (this->weightCliqueProb) {
			v_thisClique = v * (double) itClique.second[true] /  (double) totalCliqueLevel;
		}else {

			v_thisClique = v;
		}
		TESTRELEVANCE<TESTID> x;
		x.c = thisClique;
		x.t = t;
		x.value = v_thisClique;
		recipient.push_back(x);


	}

}


template class fs_max_freq<ATOMICTESTID>;
template class fs_max_freq<MULTITESTID>;
