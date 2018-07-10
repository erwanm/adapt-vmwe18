/*
 * fs_cond_entropy.cpp
 *
 *  Created on: 3 May 2018
 *      Author: moreaue
 */

#include "fs_cond_entropy.h"
#include <math.h>


template<typename TESTID>
fs_cond_entropy<TESTID>::fs_cond_entropy(unordered_map<CLASSID, unsigned long> &totalNodesByCliqueLevel,
									unordered_map<CLIQUEID, unsigned long> &cliqueFreq,
									unordered_map<CLASSID, unordered_map<TESTID, unsigned long>> &testFreqCliqueLevel,
									unordered_map<CLIQUEID, unordered_map<TESTID, unsigned long>> &testsFreq,
									bool weightCliqueProb,
									CLASSID defaultClass) : feature_selection<TESTID>(totalNodesByCliqueLevel, cliqueFreq, testFreqCliqueLevel,
											                testsFreq, weightCliqueProb, -1, defaultClass) {

}


// just split data
template<typename TESTID>
void fs_cond_entropy<TESTID>::computeRelevanceTest(vector<TESTRELEVANCE<TESTID>> &recipient, TESTID t, CLASSID level, unsigned long freqThisTestCliqueLevel, unsigned long  totalCliqueLevel) {



   //when the test is ture
	double sumoverCliques_TestTRUE = 0;
	for (auto &itClique : this->freqCNotC[level]) {
		CLIQUEID thisClique = itClique.first;

		unsigned long jointCliqueTest = this->testsFreq[thisClique][t];
		unsigned long marginalTest    = this->testFreqCliqueLevel[level][t];
		double condCliquegivenTest    = (double)jointCliqueTest/(double) marginalTest;

		if (condCliquegivenTest != 0) {
		  sumoverCliques_TestTRUE += condCliquegivenTest * log2(1.0/condCliquegivenTest);
		}

//		cerr << "thisClique: "<<thisClique<<"; jointCliqueTest="<<jointCliqueTest<<"; marginalTest="<<marginalTest<<"; condCliquegivenTest="<<condCliquegivenTest<<"; sumoverCliques_TestTRUE="<<sumoverCliques_TestTRUE<<endl;

	}
	if (isnan(sumoverCliques_TestTRUE)) {
	  cerr << "Warning: cond entropy NaN"<<endl;
	  sumoverCliques_TestTRUE = 0;
	}


	// when the test is false
	double sumoverCliques_TestFALSE = 0;
	for (auto &itClique : this->freqCNotC[level]) {
		CLIQUEID thisClique = itClique.first;

		unsigned long jointCliqueTest = this->cliqueFreq[thisClique] - this->testsFreq[thisClique][t];
		unsigned long marginalTest    = totalCliqueLevel - this->testFreqCliqueLevel[level][t];
		double condCliquegivenTest    = (double)jointCliqueTest/(double) marginalTest;

		if (condCliquegivenTest != 0) {
		  sumoverCliques_TestFALSE += condCliquegivenTest * log2(1.0/condCliquegivenTest);
		}
	//	cerr << "thisClique: "<<thisClique<<"; jointCliqueTest="<<jointCliqueTest<<"; marginalTest="<<marginalTest<<"; condCliquegivenTest="<<condCliquegivenTest<<"; sumoverCliques_TestFALSE="<<sumoverCliques_TestFALSE<<endl;

	}
	if (isnan(sumoverCliques_TestFALSE)) {
	  cerr << "Warning: cond entropy NaN"<<endl;
	  sumoverCliques_TestFALSE = 0;
	}



	double prob_TestTRUE = (double)this->testFreqCliqueLevel[level][t] / (double) totalCliqueLevel;

	double prob_TestFALSE =  (double) 1.0 - prob_TestTRUE;

    double cond_entropyvalue =  sumoverCliques_TestTRUE * prob_TestTRUE + sumoverCliques_TestFALSE * prob_TestFALSE ;

//    cerr << "prob_TestTRUE="<<prob_TestTRUE<<"; prob_TestFALSE="<<prob_TestFALSE<<"; cond_entropyvalue="<<cond_entropyvalue<<endl;



	for (auto &itClique : this->freqCNotC[level]) {

		CLIQUEID thisClique = itClique.first;

		double  cond_entropyvalue_thisClique = 0.0;

		if (this->weightCliqueProb) {
			cond_entropyvalue_thisClique  = cond_entropyvalue * (double) itClique.second[true] / (double) totalCliqueLevel;
		}else{

			cond_entropyvalue_thisClique = cond_entropyvalue;

		}

		TESTRELEVANCE<TESTID> x;
		x.c = thisClique;
		x.t = t;
		x.value = cond_entropyvalue_thisClique ;
		recipient.push_back(x);
		//		cerr <<"level="<<(int)level<<"; val = "<<x.value<<endl;


	}

}


template class fs_cond_entropy<ATOMICTESTID>;
template class fs_cond_entropy<MULTITESTID>;
