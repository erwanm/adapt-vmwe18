/*
 * fs_binary_split.cpp
 *
 *  Created on: 3 May 2018
 *      Author: moreaue
 */


#include "fs_binary_split.h"

template<typename TESTID>
fs_binary_split<TESTID>::fs_binary_split(unordered_map<CLASSID, unsigned long> &totalNodesByCliqueLevel,
									unordered_map<CLIQUEID, unsigned long> &cliqueFreq,
									unordered_map<CLASSID, unordered_map<TESTID, unsigned long>> &testFreqCliqueLevel,
									unordered_map<CLIQUEID, unordered_map<TESTID, unsigned long>> &testsFreq,
									bool weightCliqueProb,
									CLASSID defaultClass) : feature_selection<TESTID>(totalNodesByCliqueLevel, cliqueFreq, testFreqCliqueLevel, testsFreq, weightCliqueProb, 1, defaultClass) {

}


// just split data
template<typename TESTID>
void fs_binary_split<TESTID>::computeRelevanceTest(vector<TESTRELEVANCE<TESTID>> &recipient, TESTID t, CLASSID level, unsigned long freqThisTestCliqueLevel, unsigned long  totalCliqueLevel) {

	CLIQUEID defaultCliq = this->defaultClique[level];
	unsigned long freqDefaultT = this->cliqueFreq[defaultCliq];
	unsigned long freqDefaultF =  totalCliqueLevel - freqDefaultT;
	unsigned long freqDefaultTThisTestT = this->testsFreq[defaultCliq][t];
	unsigned long freqDefaultFThisTestT = freqThisTestCliqueLevel - freqDefaultTThisTestT;

	double testTGivenDefaultT = (double) freqDefaultTThisTestT / (double) freqDefaultT;
	double testTGivenDefaultF = (double) freqDefaultFThisTestT / (double) freqDefaultF;
	double absDiff = testTGivenDefaultT -testTGivenDefaultF;
	if (absDiff<0) {
		absDiff = -absDiff;
	}
#ifdef DEBUG
	cerr <<" binary_split: freqDefaultT = "<<freqDefaultT<<"; freqDefaultF="<<freqDefaultF<<"; freqDefaultTThisTestT="<<freqDefaultTThisTestT<<"; freqDefaultFThisTestT="<<freqDefaultFThisTestT<<"; testTGivenDefaultT="<<testTGivenDefaultT<<"; testTGivenDefaultF="<<testTGivenDefaultF<<"; absDiff="<<absDiff<<endl;
#endif
	double v= absDiff;
	for (auto &itClique : this->freqCNotC[level]) {
		CLIQUEID thisClique = itClique.first;

		double v_thisClique = 0.0;

		if (this->weightCliqueProb) {
			v_thisClique = v * (double) itClique.second[true] / (double)  totalCliqueLevel;
		}else{

		 v_thisClique =  v ;

		}



		TESTRELEVANCE<TESTID> x;
		x.c = thisClique;
		x.t = t;
		x.value =  v_thisClique;
		recipient.push_back(x);


	}

}


template class fs_binary_split<ATOMICTESTID>;
template class fs_binary_split<MULTITESTID>;
