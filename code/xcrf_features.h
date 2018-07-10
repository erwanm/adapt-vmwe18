/*
 * TODO
 * 
 *
 */


#ifndef XCRF_FEATURES_H
#define XCRF_FEATURES_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <algorithm>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>


#include "xcrf_features_types.h"
#include "bidirectional_string_index.h"
#include "xml_tree_node_explorer.h"
#include "xcrf_xml_tree_check_xpath_tests.h"
#include "feature_selection.h"
#include "fs_max_freq.h"
#include "fs_binary_split.h"
#include "fs_cond_entropy.h"
#include "fs_cond_entropy_clique.h"

//#include "bag_attr_val.h"

//#define DEBUG 1



#define XPATH_ATTRIBUTE_PREFIX "[@"
#define XPATH_ATTRIBUTE_SUFFIX "]"
#define XPATH_EQUAL "="

using namespace std;


// inherits xml_tree_node_explorer so that processNode can access all the data structures directly
class xcrf_features : public xml_tree_node_explorer {
 public:

static string const xcrfFeatTypes[];
static string const xcrfFeatY[];


  xcrf_features(int cliqueLevel_, vector<string> &xpathNeighbourhoods, string &defaultClass, string &classAttrName, vector<string> &attributes, int nbMaxTests);
  void addTree(string filename);
  void processNode(xmlNodePtr node);

  void filterMinFrequency(int minFreq);
  void checkXPathTests(bool printProgress, ostream &s);
  void selectFeatures(string method, int n, bool weightClique);
  int generateXCRFFeatures(string filename);
  void printInfo(ostream &s);
  void print(ostream &s, bool shortVersion);
  void selectAllFeatures();


  string cliqueString(CLIQUEID cliqueKey);
  string cliqueString(CLASSID *cliqueKey, int level);
  string atomicTestString(ATOMICTESTID test, bool xpathFormat);
  string multiTestString(MULTITESTID test);

  
 private:

  // parameters
  unsigned char cliqueLevel;
  CLASSID defaultClassId;
  xmlChar *classAttrName;
  bidirectional_string_index<ATTRID> attributes;
  bool closedAttributes;
  bidirectional_string_index<XPATHID> xpathNeighbourhoods;
  int nbMaxTests;

  // internal data
  bidirectional_string_index<CLASSID> classes;
  unordered_map<CLASSID, unsigned long> totalNodesByCliqueLevel;
  unordered_map<CLIQUEID, unsigned long> cliqueFreq;
  // remark: any possible test exists as a key in atomicTestFreqCliqueLevel[1], since
  // tests are always added at least at clique level 1
  unordered_map<CLASSID, unordered_map<ATOMICTESTID, unsigned long>> atomicTestFreqCliqueLevel;
  unordered_map<int, unordered_map<CLASSID, unordered_map<MULTITESTID, unsigned long>>> multiTestFreqCliqueLevel;
  unordered_map<ATTRID, bidirectional_string_index<VALID>> valuesByAttr;
  unordered_map<CLIQUEID, unordered_map<ATOMICTESTID, unsigned long>> atomicTests;
  unordered_map<int, unordered_map<CLIQUEID,unordered_map<MULTITESTID, unsigned long>>> multiTests;
  unordered_map<ATOMICTESTID, char *> atomicTestsXPath;
  vector<string> dataFilenames;

  unordered_map<CLASSID,vector<TESTRELEVANCE<ATOMICTESTID>>> selectedAtomicTests;
  unordered_map<int, unordered_map<CLASSID,vector<TESTRELEVANCE<MULTITESTID>>>> selectedMultiTests;

  // analyse a node: run xpath queries to find the nodes in the different "neighbourhoods", then analyze the attributes of these nodes with analyseAttributes
  void analyseNode(xmlNodePtr node, xmlXPathContextPtr xpathCtx);
  void analyseAttributes(int xpathId, xmlNodePtr node, unordered_set<ATOMICTESTID> &thisNodesTests);
//  void addAtomicTestsByClique(CLIQUEID * cliques, XPATHID xpathId, ATTRID attrId, VALID valId, unordered_map<ATOMICTESTID, int> &thisNodesTests);

  void generateMultiTests(unordered_set<ATOMICTESTID> &atomicTestsThisNode, unordered_map<int,unordered_set<MULTITESTID>> &multiTestsThisNode);
  MULTITESTID combineMultiTest(ATOMICTESTID atomic, MULTITESTID multi);
  void checkXPathNode(xmlNodePtr node, xmlXPathContextPtr xpathCtx);

  
  CLASSID getNodeClass(xmlNodePtr node);
  // returns an dynamic array of length cliqueLevel (to be freed by caller)
  CLIQUEID *getCliqueClasses(xmlNodePtr node, CLASSID *nbLevels);


  void generateXCRFFeatureNode(xmlNodePtr featsNode, CLASSID level, vector<string> &classLabels, int nbTests, ATOMICTESTID *tests, int featNo);

  template<typename X> string formatStringInt(X v, bidirectional_string_index<X> index);

  // remove
  //   void debugCliqueKeys();
};


#endif
