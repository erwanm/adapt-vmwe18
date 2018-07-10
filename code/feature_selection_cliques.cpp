/*
 * feature_selection_cliques.cpp
 *
 *  Created on: 15 May 2018
 *      Author: moreaue
 */

#include "feature_selection_cliques.h"

template<typename TESTID>
feature_selection_cliques<TESTID>::feature_selection_cliques(unordered_map<CLASSID, unsigned long> &totalNodesByCliqueLevel,
		unordered_map<CLIQUEID, unsigned long> &cliqueFreq,
		unordered_map<CLASSID, unordered_map<TESTID, unsigned long>> &testFreqCliqueLevel,
		unordered_map<CLIQUEID, unordered_map<TESTID, unsigned long>> &testsFreq,
		bool weightCliqueProb,
		int selectHighestValues,
		CLASSID defaultClass) : feature_selection<TESTID>(totalNodesByCliqueLevel, cliqueFreq, testFreqCliqueLevel, testsFreq, weightCliqueProb, selectHighestValues, defaultClass) {

}


// adds to relevance vector for one test, all cliques (unless a clique is not needed)
template<typename TESTID>
void feature_selection_cliques<TESTID>::computeRelevanceTest(vector<TESTRELEVANCE<TESTID>> &recipient, TESTID t, CLASSID level, unsigned long freqThisTestCliqueLevel, unsigned long  totalCliqueLevel) {


	// cliqueConfusionMatrix[clique][clique=0|1][test=0|1] = freq
//	unordered_map<CLIQUEID, unordered_map<unsigned char, unordered_map<unsigned char,unsigned long>>> cliqueConfusionMatrix;
	for (auto &itClique : this->freqCNotC[level]) {
		CLIQUEID thisClique = itClique.first;
		// confusionMatrixThisClique[clique=0|1][test=0|1] = freq
		unordered_map<unsigned char, unordered_map<unsigned char,unsigned long>> confusionMatrixThisClique; // = cliqueConfusionMatrix[thisClique];
		unsigned long int freqThisCliqueT = itClique.second[true];
		unsigned long int freqThisCliqueF = itClique.second[false];
		unsigned long freqThisCliqueTThisTestT = this->testsFreq[thisClique][t];
		confusionMatrixThisClique[1][1] = freqThisCliqueTThisTestT;
		confusionMatrixThisClique[1][0] = freqThisCliqueT-freqThisCliqueTThisTestT;
		confusionMatrixThisClique[0][1] = freqThisTestCliqueLevel - freqThisCliqueTThisTestT;
		confusionMatrixThisClique[0][0] = totalCliqueLevel - freqThisCliqueTThisTestT - confusionMatrixThisClique[1][0] - confusionMatrixThisClique[0][1];
		TESTRELEVANCE<TESTID> x;
		x.c = thisClique;
		x.t = t;
		x.value =  computeRelevanceTestClique(confusionMatrixThisClique, totalCliqueLevel, freqThisTestCliqueLevel);
		//		cerr << "v="<<x.value<<endl;
		recipient.push_back(x);
	}
}





template class feature_selection_cliques<ATOMICTESTID>;
template class feature_selection_cliques<MULTITESTID>;





