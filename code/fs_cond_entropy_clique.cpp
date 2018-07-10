
#include "fs_cond_entropy_clique.h"
#include <math.h>


template<typename TESTID>
fs_cond_entropy_clique<TESTID>::fs_cond_entropy_clique(unordered_map<CLASSID, unsigned long> &totalNodesByCliqueLevel,
									unordered_map<CLIQUEID, unsigned long> &cliqueFreq,
									unordered_map<CLASSID, unordered_map<TESTID, unsigned long>> &testFreqCliqueLevel,
									unordered_map<CLIQUEID, unordered_map<TESTID, unsigned long>> &testsFreq,
									bool weightCliqueProb,
									CLASSID defaultClass) : feature_selection_cliques<TESTID>(totalNodesByCliqueLevel, cliqueFreq, testFreqCliqueLevel,
											                testsFreq, weightCliqueProb, -1, defaultClass) {

}


template<typename TESTID>
double fs_cond_entropy_clique<TESTID>::computeRelevanceTestClique(unordered_map<unsigned char, unordered_map<unsigned char,unsigned long>> &confusionMatrix, unsigned long totalCliqueLevel, unsigned long freqThisTestCliqueLevel) {

  unordered_map<unsigned char, unsigned long> marginalTestTF;
  marginalTestTF[0] = totalCliqueLevel - freqThisTestCliqueLevel;
  marginalTestTF[1] = freqThisTestCliqueLevel;
  
  double condEntropy = 0;
  for (unsigned char testTF=0; testTF<=1; testTF++) {
    double sumOverCliqueTF = 0;
    for (unsigned char cliqueTF=0; cliqueTF<=1; cliqueTF++) {
      unsigned long jointCliqueTest = confusionMatrix[cliqueTF][testTF];
      unsigned long marginalTest = marginalTestTF[testTF];
      double condCliqueGivenTest = (double)jointCliqueTest/(double) marginalTest;
      if (condCliqueGivenTest != 0) {
	sumOverCliqueTF += condCliqueGivenTest * log2(1.0/condCliqueGivenTest);
      }
    }
    condEntropy += sumOverCliqueTF * ((double) marginalTestTF[testTF] / (double) totalCliqueLevel);
  }
  if (isnan(condEntropy)) {
    cerr << "Warning: cond entropy (clique) is NaN" << endl;
    condEntropy = 0;
  }
  
  return condEntropy;
}





template class fs_cond_entropy_clique<ATOMICTESTID>;
template class fs_cond_entropy_clique<MULTITESTID>;

