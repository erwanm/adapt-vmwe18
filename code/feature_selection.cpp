/*
 * feature_selection.cpp
 *
 *  Created on: 2 May 2018
 *      Author: moreaue
 */

#include "feature_selection.h"

template<typename TESTID>
feature_selection<TESTID>::feature_selection(unordered_map<CLASSID, unsigned long> &totalNodesByCliqueLevel_,
					     unordered_map<CLIQUEID, unsigned long> &cliqueFreq_,
					     unordered_map<CLASSID, unordered_map<TESTID, unsigned long>> &testFreqCliqueLevel_,
					     unordered_map<CLIQUEID, unordered_map<TESTID, unsigned long>> &testsFreq_,
					     bool weightCliqueProb_,
					     int selectHighestValues_,
					     CLASSID defaultClass) :
  totalNodesByCliqueLevel(totalNodesByCliqueLevel_),
  cliqueFreq(cliqueFreq_),
  testFreqCliqueLevel(testFreqCliqueLevel_),
  testsFreq(testsFreq_) {
  cliqueLevel = totalNodesByCliqueLevel_.size();
  weightCliqueProb = weightCliqueProb_;
  selectHighestValues = selectHighestValues_;

  // prepare map with clique/not clique frequencies for convenience
  for (auto &itClique : cliqueFreq) {// its used to loop for the clique id for a certain level... we are dividing the clique id by levels here in the whole data
    CLIQUEID thisClique = itClique.first;
    unsigned long thisCliqueFreq = itClique.second;
    CLASSID levelThisClique;
    CLASSID *classIds = decodeClique(thisClique, &levelThisClique);// we decode to get the level because cliqueFreq is not orgnised by level.
    //	cerr << "DEBUG "<<thisClique<<", level = "<<(int) levelThisClique<<"; freq = "<<thisCliqueFreq<<endl;
    freqCNotC[levelThisClique][thisClique][true] = thisCliqueFreq;// the number of nodes that belong to this clique
    freqCNotC[levelThisClique][thisClique][false] = totalNodesByCliqueLevel[levelThisClique] - thisCliqueFreq;// the number of nodes that doesnt belong to this clique
    free(classIds);
  }

  // default clique for every level
  //	cerr << "default class = "<<(int) defaultClass<<endl;
  CLASSID *classes= (CLASSID *) malloc(sizeof(CLASSID) * 4);
  for (CLASSID level = 1; level <= cliqueLevel; level++) {
    classes[level-1] = defaultClass;
    CLIQUEID cliqueId = encodeClique(classes, level);
    //		cerr << "DEBUG default for level "<<(int) level<<": "<<cliqueId<<endl;
    defaultClique[level] = cliqueId;
  }
  free(classes);

  }



template<typename TESTID>
void feature_selection<TESTID>::computeRelevance() {

  for (CLASSID level = 1; level <= cliqueLevel; level++) {
    //    cerr << "computing relevance vector level "<<level<<endl;
    vector<TESTRELEVANCE<TESTID>> &v = relevance[level];// this map by level
    unsigned long totalCliqueLevel = totalNodesByCliqueLevel[level];
    for (auto &itTest : testFreqCliqueLevel[level]) {// to collect relevent info
      TESTID t = itTest.first;
      unsigned long freqThisTestCliqueLevel = itTest.second;
      computeRelevanceTest(v, t, level, freqThisTestCliqueLevel, totalCliqueLevel);

    }
    sort(v.begin(), v.end());
#ifdef DEBUG
    cerr << "end computing relevance vector level "<<level<<": size="<<v.size()<<endl;//", start="<<v.front().value<<"; end="<<v.back().value()<<endl;
#endif

  }
}









template<typename TESTID>
unordered_map<CLASSID,vector<TESTRELEVANCE<TESTID>>> feature_selection<TESTID>::getMostRelevantTests(int n, int *nbSelected) {

  unordered_map<CLASSID,vector<TESTRELEVANCE<TESTID>>> res;

  if (selectHighestValues == 0) {
    cerr << "Bug: selectHighestValues has not been set to 1 or -1"<<endl;
    exit(10);
  }

  *nbSelected = 0;
  int nbLeftToSelect = n;//100
  for (CLASSID level=1; level <= cliqueLevel; level++) {

    if (relevance[level].empty()) {// we have one array for every clique level.
      computeRelevance();
    }
    int nbLevelsLeft = cliqueLevel-level+1;//
    int nbToSelectForThisLevel = nbLeftToSelect/nbLevelsLeft; //50 50
    #ifdef DEBUG
    cerr << "getMostRelevantTetsts: level "<<(int) level<<", nb left = "<<nbLeftToSelect<<"; nbLevelsLeft="<<nbLevelsLeft<<"; nbToSelectForThisLevel="<<nbToSelectForThisLevel<<endl;
    #endif
    vector<TESTRELEVANCE<TESTID>> &thisLevelVector =relevance[level];
    if (thisLevelVector.size()<= nbToSelectForThisLevel) { // copy the whole vector
      //cerr << "lower or equal: "<<thisLevelVector.size() <<endl;
      #ifdef DEBUG
      cerr << "level="<<(int) level<<"; value at the start = "<<thisLevelVector.front().value<<"; at the end = "<<thisLevelVector.back().value<<endl;
      #endif
      res[level] = thisLevelVector;
      nbLeftToSelect -= thisLevelVector.size();
      (*nbSelected) += thisLevelVector.size();
    } else {
      nbLeftToSelect -= nbToSelectForThisLevel;//100-50 = 50
      (*nbSelected) += nbToSelectForThisLevel;
      #ifdef DEBUG
      cerr << "level="<<(int) level<<"; value at the start = "<<thisLevelVector.front().value<<"; at the end = "<<thisLevelVector.back().value<<endl;
      #endif
      if (selectHighestValues > 0) {
	// retrieving the end of the vector
#ifdef DEBUG
	cerr << "Selecting "<<nbToSelectForThisLevel<<" highest values at the end of the vecor"<<endl;
#endif
	res[level] =  vector<TESTRELEVANCE<TESTID>>(thisLevelVector.end()-nbToSelectForThisLevel, thisLevelVector.end());
      } else {
#ifdef DEBUG
	cerr << "Selecting "<<nbToSelectForThisLevel<<" lowest values at the start of the vecor"<<endl;
#endif
	// retrieving the beginning of the vector
	res[level] = vector<TESTRELEVANCE<TESTID>>(thisLevelVector.begin(), thisLevelVector.begin()+nbToSelectForThisLevel);
      }
    }
  }
  return res;

}



template class feature_selection<ATOMICTESTID>;
template class feature_selection<MULTITESTID>;
