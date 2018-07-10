
#include "xcrf_features.h"

#include <string.h>


string const xcrf_features::xcrfFeatTypes[] = { "CurrentNode", "Current-Parent", "Current-Parent-Sibling" };
string const xcrf_features::xcrfFeatY[] = {"Ycur", "Ypar", "Ysib" };



xcrf_features::xcrf_features(int cliqueLevel_, vector<string> &xpathNeighbourhoods_, string &defaultClass_, string &classAttrName_, vector<string> &attributes_, int nbMaxTests_)  {
	cliqueLevel = cliqueLevel_;

	// initalized as xmlChar * in constructor because it's used only and repeatedly to access xml node attr
	classAttrName = xmlCharStrdup(classAttrName_.c_str());

	// default class
	if (!defaultClass_.empty()) {
		defaultClassId = classes.getCreate(defaultClass_);
	} else {
		defaultClassId = 0;
	}

	// attributes
	if (attributes_.empty()) {
		closedAttributes = false;
		// map for attributes values cannot be initialized
	} else {
		closedAttributes = true;
		for (string attr : attributes_) {
			ATTRID attrId = attributes.getCreate(attr);
			// map for attributes values can be initialized
			valuesByAttr[attrId] = bidirectional_string_index<VALID>();
		}
	}

	// xpath neighbourhoods
	if (xpathNeighbourhoods_.empty()) {
		cerr << "Error: no xpath neighbourhoods provided." << endl;
		exit(10);
	} else {
		for (string xpath : xpathNeighbourhoods_) {
			xpathNeighbourhoods.getCreate(xpath);
		}
	}

	// nb tests
	nbMaxTests = nbMaxTests_;
	if (nbMaxTests > HARDCODEDMAXTESTS) {
		cerr << "Error: nb max tests cannot be higher than "<<HARDCODEDMAXTESTS<<" (or you need to change the harcoded value in 'xcrf_features.h'' and recompile)." <<endl;
		exit(10);
	}


}

void xcrf_features::addTree(string filename) {
	dataFilenames.push_back(filename);
	exploreXmlTree(filename);
}


void xcrf_features::processNode(xmlNodePtr node) {
	analyseNode(node, xpathCtx);
}





void xcrf_features::filterMinFrequency(int minFreq) {

  // filter on the map by clique level, because otherwise there would be inconsistencies
  // in the frequency when a test is removed for one clique but not the other
  for (auto &itCliqueLevel : atomicTestFreqCliqueLevel) {
    unordered_map<ATOMICTESTID, unsigned long> &m = itCliqueLevel.second;
    auto itTest = m.begin();
    while (itTest != m.end()) {
      ATOMICTESTID testId = itTest->first;
      if (itTest->second < minFreq) { // from http://thispointer.com/c11-unordered_map-erase-elements-while-iterating-in-a-loop/
	itTest = m.erase(itTest);
      } else {
	itTest++;
      }
    }
    
  }

  // also remove from map by clique following map by clique level
  for (auto &itClique : atomicTests) {
    CLASSID level;
    CLASSID *classIds = decodeClique(itClique.first, &level);
    unordered_map<ATOMICTESTID, unsigned long> &m = itClique.second;
    auto itTest = m.begin();
    while (itTest != m.end()) {
      ATOMICTESTID testId = itTest->first;
      if (atomicTestFreqCliqueLevel[level].find(testId) == atomicTestFreqCliqueLevel[level].end()) { // test has been removed
	itTest = m.erase(itTest);
      } else {
	itTest++;
      }
    }
    free(classIds);
  }


  // same for multitest
  for (auto &itNbTests : multiTestFreqCliqueLevel) {
    
    int nbTests = itNbTests.first;
    for (auto &itCliqueLevel : itNbTests.second) {
      unordered_map<MULTITESTID, unsigned long> &m = itCliqueLevel.second;
      auto itTest = m.begin();
      while (itTest != m.end()) {
	MULTITESTID testId = itTest->first;
	if (itTest->second < minFreq) { // from http://thispointer.com/c11-unordered_map-erase-elements-while-iterating-in-a-loop/
	  itTest = m.erase(itTest);
	} else {
	  itTest++;
	}
      }
    }

  // also remove from map by clique following map by clique level
    for (auto &itClique : multiTests[nbTests]) {
      CLASSID level;
      CLASSID *classIds = decodeClique(itClique.first, &level);
      unordered_map<MULTITESTID, unsigned long> &m = itClique.second;
      auto itTest = m.begin();
      while (itTest != m.end()) {
	MULTITESTID testId = itTest->first;
	if (multiTestFreqCliqueLevel[nbTests][level].find(testId) == multiTestFreqCliqueLevel[nbTests][level].end()) { // test has been removed
	  itTest = m.erase(itTest);
	} else {
	  itTest++;
	}
      }
    free(classIds);
  }

  }

}


// redo the whole counting with xpath exprs and check that we find the same numbers as the first time.
// only for clique level 1, since it contains all the possible tests
void xcrf_features::checkXPathTests(bool printProgress, ostream &s) {

	  unordered_map<ATOMICTESTID, unsigned long> atomicTestsXPathCheck;
	  xcrf_xml_tree_check_xpath_tests xpathChecking(printProgress, atomicTestFreqCliqueLevel[1], atomicTestsXPath, atomicTestsXPathCheck);

	if (printProgress) {
		s << endl;
		s << "Checking numbers found by xpath exprs against values from first pass..." <<endl;
	}
	for (int i=0; i<dataFilenames.size(); i++) {
		if (printProgress) {
			s << "\r"<<i+1 <<" / "<< dataFilenames.size();
		}
		xpathChecking.exploreXmlTree(dataFilenames[i]);
	}
	if (printProgress) {
		s << endl;
	}
	int nbDiffs = 0;
	unordered_map<ATOMICTESTID, unsigned long> &testsMap = atomicTestFreqCliqueLevel[1];
	for (auto itTest : testsMap) {
		if (itTest.second != atomicTestsXPathCheck[itTest.first]) {
			cerr << "Warning: different values for test <"<< atomicTestsXPath[itTest.first] <<">: "<< itTest.second <<", "<< atomicTestsXPathCheck[itTest.first] <<endl;
			nbDiffs++;
		}
	}
	if (nbDiffs == 0) {
		if (printProgress) {
			s << testsMap.size() << "atomic tests have been checked, no differences found. OK." << endl;
		}
	} else {
		cerr << "WARNING: " << nbDiffs << "differences found." << endl;
	}



}



int xcrf_features::generateXCRFFeatures(string xmlOutputFilename) {

if (selectedAtomicTests.empty()) {
	cerr << "Warning: no feature selection selected, using all the features."<<endl;
	selectAllFeatures();
}

  xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
  xmlNodePtr rootNode = xmlNewNode(NULL, BAD_CAST "Crf");
  xmlNewProp(rootNode, BAD_CAST "xmlns:xsi", BAD_CAST "http://www.w3.org/2001/XMLSchema-instance");
  xmlNewProp(rootNode, BAD_CAST "xsi:noNamespaceSchemaLocation", BAD_CAST "crf.xsd");
  xmlNewProp(rootNode, BAD_CAST "type", BAD_CAST "tree");
  xmlDocSetRootElement(doc, rootNode);

  // labels
  xmlNodePtr labelsNode = xmlNewChild(rootNode, NULL, BAD_CAST "Labels", NULL);
  string defaultClassStr = "label_" + classes.get(defaultClassId);
  xmlNewProp(labelsNode, BAD_CAST "emptyAnnotation", BAD_CAST defaultClassStr.c_str());
  for (int i=1; i<= classes.size(); i++) {// size of 2: I,O
    string label = classes.get(i);
    xmlNodePtr labelNode = xmlNewChild(labelsNode, NULL, BAD_CAST "Label", NULL);
    xmlNewProp(labelNode, BAD_CAST "value", BAD_CAST label.c_str());
    label = "label_"+label;
    xmlNewProp(labelNode, BAD_CAST "name", BAD_CAST label.c_str());
  }

  // Features
  xmlNodePtr featsNode = xmlNewChild(rootNode, NULL, BAD_CAST "Features", NULL);

  int featNo = 1;
  for (CLASSID level=1; level<=cliqueLevel; level++) {
	  for (TESTRELEVANCE<ATOMICTESTID> &tr : selectedAtomicTests[level]) {// here its sorted by level
		    CLASSID level0;
		    CLASSID *classIds = decodeClique(tr.c, &level0);
		    if (level0 != level) {
		    	cerr << "Bug, different clique level not supposed to happen..." <<endl;
		    }
		    vector<string> classLabels;
		    for (int i=1; i<=level; i++) {
		      classLabels.push_back("label_"+classes.get(classIds[i-1]));
		    }

		    ATOMICTESTID testId = tr.t;
		    generateXCRFFeatureNode(featsNode, level, classLabels, 1, &testId, featNo);
		     featNo++;
		    free(classIds);
	  }
	  for (int nbTests =2; nbTests <=nbMaxTests; nbTests++) {
		  for (TESTRELEVANCE<MULTITESTID> &tr : selectedMultiTests[nbTests][level]) {
				CLASSID level0;
				CLASSID *classIds = decodeClique(tr.c, &level0);
				if (level0 != level) {
					cerr << "Bug, different clique level not supposed to happen..." <<endl;
				}
				vector<string> classLabels;
				for (int i=1; i<=level; i++) {
				  classLabels.push_back("label_"+classes.get(classIds[i-1]));
				}

				MULTITESTID testId = tr.t;
				generateXCRFFeatureNode(featsNode, level, classLabels, testId.nbTests, testId.tests, featNo);
				 featNo++;
				free(classIds);
		  }
	  }
  }


  xmlSaveFormatFileEnc(xmlOutputFilename.c_str(), doc, "UTF-8", 1);
  xmlFreeDoc(doc);
  return featNo-1;

}



void xcrf_features::generateXCRFFeatureNode(xmlNodePtr featsNode, CLASSID level, vector<string> &classLabels, int nbTests, ATOMICTESTID *tests, int featNo) {

	char featName[256];

	// feature element
	xmlNodePtr featNode = xmlNewChild(featsNode, NULL, BAD_CAST "Feature", NULL);
	xmlNewProp(featNode, BAD_CAST "xsi:type", BAD_CAST xcrfFeatTypes[level-1].c_str());
	sprintf(featName,"%06d", featNo);
	xmlNewProp(featNode, BAD_CAST "name", BAD_CAST featName);

	// clique elements
	for (int l=1; l<=level; l++) {
		xmlNodePtr yNode = xmlNewChild(featNode, NULL, BAD_CAST xcrfFeatY[l-1].c_str(), NULL);
		xmlNewProp(yNode, BAD_CAST "xsi:type", BAD_CAST "Label");
		xmlNewProp(yNode, BAD_CAST "value", BAD_CAST classLabels[l-1].c_str());
	}

	// test elements
	for (int i=0; i< nbTests; i++) {
		xmlNodePtr testNode = xmlNewChild(featNode, NULL, BAD_CAST "TestX", NULL);
		char *xpathExpr = atomicTestsXPath[tests[i]];
		xmlNewProp(testNode, BAD_CAST "value", BAD_CAST xpathExpr);
	}
}




void xcrf_features::printInfo(ostream &s) {
	s << totalNodesByCliqueLevel[1] <<" nodes have been analyzed."<<endl;
	s << "Number of cliques (all sizes): "<<atomicTests.size()-1 <<endl;
	s << "Number of distinct atomic tests (clique level 1): " << atomicTestFreqCliqueLevel[1].size()<<endl;
	s << "Number of distinct multi-tests (clique level 1): ";
	for (int mts=2; mts <= nbMaxTests; mts++) {
		s << " size "<<mts<<" = "<< multiTestFreqCliqueLevel[mts][1].size()<<";";
	}
	s<<endl;
	print(s, true);
}




void xcrf_features::analyseNode(xmlNodePtr node, xmlXPathContextPtr xpathCtx) {

	// remark: at first we don't care about which clique each tests will be assigned to, we just collect all the tests

	// this set will contain all the atomic tests found only for the node being explored
	unordered_set<ATOMICTESTID> atomicTestsThisNode;


	// 1. find all possible atomic tests from this node
	for (XPATHID xpathId = 1; xpathId <= xpathNeighbourhoods.size(); xpathId++) {
		string &xpathStr = xpathNeighbourhoods.get(xpathId);
#ifdef DEBUG
		cerr << "xpath = "<<xpathStr<<endl;
#endif
		xmlXPathObjectPtr xpathObj = xmlXPathNodeEval(node, BAD_CAST xpathStr.c_str(), xpathCtx);
		if(xpathObj == NULL) {
			cerr << "Error, cannot evaluate xpath expression '"<< xpathStr <<"'"<<endl;
			exit(10);
		}
		xmlNodeSetPtr nodes = xpathObj->nodesetval;
		if (nodes != NULL) {
#ifdef DEBUG
			cerr << "found "<<nodes->nodeNr<<" nodes"<<endl;
#endif
			for (int i=0; i< nodes->nodeNr ; i++) {
				xmlNodePtr resultNode = nodes->nodeTab[i];
				if (resultNode == NULL) {
					cerr << "Error xpath query returned a NULL node, for some reason." << endl;
					exit(10);
				}
				if (resultNode->type == XML_ELEMENT_NODE) {
					if (strcmp((char *) resultNode->name, "TOKEN") == 0) { // because xpath expr might reach outside token nodes, e.g. sentence one
						analyseAttributes(xpathId, resultNode, atomicTestsThisNode);
					}
				} else {
					cerr << "Warning: xpath query '"<<xpathStr<<"' returned a non-element node, I don't think that's supposed to happen (?)" << endl;
				}
			}
		} else {
		  cerr << "Bug: xpath query returned NULL (not empty list), not sure what this means." <<endl;
		    exit(10);
		}


		xmlXPathFreeObject(xpathObj);
	}

	// 2. generate the combinations (multi-tests) which work for this node

	unordered_map<int,unordered_set<MULTITESTID>> multiTestsThisNode; // this map will contain combinations of tests only for the node being explored, based on the list of atomic tests
	generateMultiTests(atomicTestsThisNode, multiTestsThisNode);
	// CAUTION: must free the allocated space for multi-tests, but only if the multi-test already exists in the global map!!
#ifdef DEBUG
	cerr << "found "<<multiTestsThisNode.size()<<" multitests for this node "<<endl;
#endif

	// 3. update the global map of tests by incrementing the ones satisfied for this node

	CLASSID nbLevels;
	CLIQUEID *cliques = getCliqueClasses(node, &nbLevels); // returns an array of size cliqueLevel, to be freed later
	for (CLASSID level=0; level<nbLevels; level++) {
		totalNodesByCliqueLevel[level+1]++; // this work based on level
		cliqueFreq[cliques[level]]++;// this works based on id
	}

	// TODO review the nested loop: accessing 2 or 3 levels of map every time in the inner-most loop
	// maybe rearrange the maps themselves??
	//	unordered_map<ATOMICTESTID, int> &atomicTestsAll = atomicTests[ALLCLIQUESID];
	for (ATOMICTESTID t : atomicTestsThisNode) {
	  //		atomicTestsAll[t]++;
		if (atomicTestsXPath.find(t) == atomicTestsXPath.end()) {
			string expr = atomicTestString(t, true);
			if (!expr.empty()) {
			  atomicTestsXPath[t] = strdup(expr.c_str());
			}
		}
		for (CLASSID cliqueNo=0; cliqueNo< nbLevels; cliqueNo++) { // adding also for cliques at lower levels (i.e. if level = 3, adding for level 1 and level 2 as well)
			CLIQUEID cliqueId = cliques[cliqueNo];
			atomicTests[cliqueId][t]++; // the freq of the test for this id
			atomicTestFreqCliqueLevel[cliqueNo+1][t]++;// the freq of the test for this clique level
		}
	}
	for (int i=2; i<= nbMaxTests; i++) {
	  //		unordered_map<MULTITESTID, int> &multiTestsAll = multiTests[ALLCLIQUESID][i];
		for (MULTITESTID t: multiTestsThisNode[i]) {
		  //			multiTestsAll[t]++;
			for (CLASSID cliqueNo=0; cliqueNo< nbLevels; cliqueNo++) { // adding also for cliques at lower levels (i.e. if level = 3, adding for level 1 and level 2 as well)
				CLIQUEID cliqueId = cliques[cliqueNo];
				multiTests[i][cliqueId][t]++;
				multiTestFreqCliqueLevel[i][cliqueNo+1][t]++;

			}
		}
	}

	//cerr << "hello?"<<endl;
	free(cliques);

#ifdef DEBUG
	cerr << "analyseNode: done" <<endl;
#endif
}





void xcrf_features::analyseAttributes(int xpathId, xmlNodePtr node, unordered_set<ATOMICTESTID> &thisNodesTests) {


	xmlAttrPtr attrNode = node->properties;
	if (attrNode == NULL) {
		// should this be considered normal???
		cerr << "warning: no properties at all in TOKEN node"<<endl;
	} else {
		while (attrNode != NULL) {
			string attrName((char *) attrNode->name);
			// the attribute is taken into account if either closedAttributes is false or the attribute is a member of attributes
			ATTRID attrId;
			if (closedAttributes) {
				attrId = attributes.get(attrName); // returns 0 if attrName is not a member
			} else {
				attrId = attributes.getCreate(attrName); // creates a new attribute if needed, never returns 0
			}
			if (attrId != 0) {
				bidirectional_string_index<VALID> &valMap = valuesByAttr[attrId]; // this creates the bidirectional_string_index object if it didn't exist before in the map
				// get the value
				xmlChar* value = xmlNodeListGetString(node->doc, attrNode->children, 1);
				string attrValue((char *) value);
				xmlFree(value);
#ifdef DEBUG
				cerr <<"attr: " << attrName << "; val = " << attrValue<<endl;
#endif
				VALID valId = valMap.getCreate(attrValue);
				ATOMICTESTID testId;
				testId.xpathId = xpathId;
				testId.attrId = attrId;
				testId.valId = valId;
				thisNodesTests.insert(testId);
			}
			attrNode = attrNode->next;
		}
	}
}






void xcrf_features::generateMultiTests(unordered_set<ATOMICTESTID> &atomicTestsThisNode, unordered_map<int,unordered_set<MULTITESTID>> &multiTestsThisNode) {

	// 2 tests is a special case because we combine both atomic tests
	unordered_set<MULTITESTID> &multi2 = multiTestsThisNode[2];
	for (const auto &test1 : atomicTestsThisNode) {
		for (const auto &test2 : atomicTestsThisNode) {
			if (test1 < test2) { // ordered to avoid duplicates
				MULTITESTID mt;
				mt.tests[0] = test1;
				mt.tests[1] = test2;
				mt.nbTests = 2;
				multi2.insert(mt);
			}
		}
	}

	for (int nbTests = 3; nbTests <= nbMaxTests; nbTests++) {
		unordered_set<MULTITESTID> &multiN = multiTestsThisNode[nbTests];
		unordered_set<MULTITESTID> &multiNMinus1 = multiTestsThisNode[nbTests-1];
		for (const auto &atomTest : atomicTestsThisNode) {
			for (const auto &multiTest : multiNMinus1) {
				// returns 0 in nbTests if this test should be discarded (because ordered to avoid duplicates)
				MULTITESTID mt = combineMultiTest(atomTest, multiTest);
				if (mt.nbTests >0) {
					multiN.insert(mt);
				}
			}
		}

	}

}



// returns 0 in nbTests if case to be discarded
MULTITESTID xcrf_features::combineMultiTest(ATOMICTESTID atomic, MULTITESTID multi) {
	MULTITESTID res;
	res.nbTests = 0;
	vector<ATOMICTESTID> v;
	for (int i = 0; i< multi.nbTests; i++) {
		if (multi.tests[i] == atomic) { // to avoid having twice the same test in a multitest
			return res; // with nbTests = 0
		}
		v.push_back(multi.tests[i]);
	}
	v.push_back(atomic);
	sort(v.begin(), v.end()); // sorting for checking duplicates
	res.nbTests = multi.nbTests +1;
	for (int i = 0; i< multi.nbTests+1; i++) {
		res.tests[i] = v[i];
	}
	return res;
}




CLASSID xcrf_features::getNodeClass(xmlNodePtr node) {

	CLASSID res;

	xmlChar *classVal = xmlGetProp(node, classAttrName);

	if (classVal == NULL) {
		if (defaultClassId == 0) {
			cerr << "Error: no class attribute '"<< (char *) classAttrName<<"' found, and default class not provided." << endl;
			exit(10);
		} else {
			res = defaultClassId;
		}
	} else {
		res =  classes.getCreate(string((char *) classVal));
		xmlFree(classVal);
	}


	return res;

}





// returns an dynamic array of length nbLevel (to be freed by caller)
// remark: the size is not necessarily cliqueLevel because the node might not have parent and/or sibling
CLIQUEID *xcrf_features::getCliqueClasses(xmlNodePtr node, CLASSID *nbLevels) {
	CLIQUEID *res = (CLIQUEID *) malloc(sizeof(CLIQUEID) * cliqueLevel);
	CLASSID *current = (CLASSID *) malloc(sizeof(CLASSID) * cliqueLevel);

	current[0] = getNodeClass(node);
	res[0] = encodeClique(current, 1);
	*nbLevels = 1;
#ifdef DEBUG
	cerr << "getCliqueClass: level 1 clique key = "<<res[0]<<endl;
#endif
	if (cliqueLevel>=2) {
		xmlNodePtr parentNode = node->parent;
		if ((parentNode != NULL)  && (parentNode->type == XML_ELEMENT_NODE) && (strcmp((char *) parentNode->name, "TOKEN") == 0)) {
			current[1] = getNodeClass(parentNode);
			res[1] = encodeClique(current, 2);
			*nbLevels = 2;
#ifdef DEBUG
			cerr << "getCliqueClass: level 2 clique key = "<<res[1]<<endl;
#endif
			if (cliqueLevel >=3) {
				xmlNodePtr siblingNode = node->next;
				while ((siblingNode != NULL) && (siblingNode->type != XML_ELEMENT_NODE)) {
					siblingNode = siblingNode->next;
				}
				if ((siblingNode != NULL)  && (siblingNode->type == XML_ELEMENT_NODE)) {
					current[2] = getNodeClass(siblingNode);
					res[2] = encodeClique(current, 3);
					*nbLevels = 3;
#ifdef DEBUG
					cerr << "getCliqueClass: level 3 clique key = "<<res[2]<<endl;
#endif
				}
			}
		}
	}

	free(current);
	return res;
}



template<typename X>
string xcrf_features::formatStringInt(X v, bidirectional_string_index<X> index) {
	return index.get(v)+ "[" +to_string(v)+"]";
}


string xcrf_features::cliqueString(CLIQUEID cliqueKey) {
	CLASSID level;
	CLASSID * cliqueInt = decodeClique(cliqueKey, &level);
	string res = cliqueString(cliqueInt, level);
	free(cliqueInt);
	return res;

}


string xcrf_features::cliqueString(CLASSID *cliqueKey, int level) {
	//  	cerr << "level="<<level<<endl;
	//  	cerr << "c0"<<endl;
	//  	cerr << (int) cliqueKey[0]<<endl;
	string res = formatStringInt(cliqueKey[0], classes);
	for (int i=1; i<level; i++) {
		//		cerr << "c"<<i<<endl;
		//	cerr << cliqueKey[i]<<endl;
		res += ";"+formatStringInt(cliqueKey[i], classes);
	}
	return res;
}


string xcrf_features::atomicTestString(ATOMICTESTID test, bool xpathFormat) {
#ifdef DEBUG
	cerr << "atomicTestString: xpath = "<<(int) test.xpathId<<"; attr = "<<(int)test.attrId << "; val = "<<(int) test.valId<< endl;
#endif
	if (xpathFormat) {
		string valStr = valuesByAttr[test.attrId].get(test.valId);
		if (valStr.find("'") == string::npos) {
			valStr = "'"+valStr+"'";
		} else {
			if (valStr.find('"') == string::npos) {
			  //			  cerr << "Warning: found apostrophe in value '"<<valStr<<"', possible problems with xcrf later."<<endl;
				valStr = '"'+valStr+'"';
			} else {
				cerr << "Warning: cannot deal with value containing both kinds of quotes: **" << valuesByAttr[test.attrId].get(test.valId) << "**"<<endl;
				return "";
				//exit(10);
			}
		}
		return xpathNeighbourhoods.get(test.xpathId) + XPATH_ATTRIBUTE_PREFIX + attributes.get(test.attrId) + XPATH_EQUAL +  valStr + XPATH_ATTRIBUTE_SUFFIX;
	} else {
		return formatStringInt(test.xpathId, xpathNeighbourhoods)+":"+formatStringInt(test.attrId,attributes)+"="+formatStringInt(test.valId,valuesByAttr[test.attrId]);
	}
}



string xcrf_features::multiTestString(MULTITESTID test) {
	string res = atomicTestString(test.tests[0], false);
	for (int i=1; i< test.nbTests; i++) {
		res = res + " | " + atomicTestString(test.tests[i], false);
	}
	return res;
}



void xcrf_features::print(ostream &s, bool shortVersion) {

  //	cerr << "Warning: printing collected data, not selected features"<<endl;
  s << "##### ATOMIC TESTS #####"<<endl;
  for (auto &itClique : atomicTests) {
    s << " *** CLIQUE "<< cliqueString(itClique.first);
    s << " : " << itClique.second.size()<<endl;
    if (!shortVersion) {
      for (auto &itTest : itClique.second) {
	ATOMICTESTID testId = itTest.first;
	int freq = itTest.second;
	s << "    "<<atomicTestString(itTest.first,false)<<" : "<<freq<<endl;
      }
      s <<endl;
    }
  }
  for (auto &itNbTests : multiTests){
    s << "##### MULTI TESTS: "<< itNbTests.first<<" tests #####"<<endl;
    for (auto &itClique : itNbTests.second) {
      s << " *** CLIQUE "<< cliqueString(itClique.first);
      s << " : " << itClique.second.size()<<endl;
      if (!shortVersion) {
	for (auto &itTest : itClique.second) {
	  MULTITESTID testId = itTest.first;
	  int freq = itTest.second;
	  s << "    "<<multiTestString(itTest.first)<<" : "<<freq<<endl;
	}
	s<<endl;
      }
    }
  }
}


// fills selectedXXX with collected tests
void xcrf_features::selectAllFeatures() {
	for (auto &itClique : atomicTests) {
		CLIQUEID clique= itClique.first;
		CLASSID level;
		CLASSID *classIds = decodeClique(clique, &level);
		for (auto &itTest : itClique.second) {
			ATOMICTESTID testId = itTest.first;
			int freq = itTest.second;
			TESTRELEVANCE<ATOMICTESTID> x;
			x.c = clique;
			x.t = testId;
			x.value = freq;
			selectedAtomicTests[level].push_back(x);
		}
		free(classIds);
	}
	for (auto &itNbTests : multiTests){
		int nbTests = itNbTests.first;
		for (auto &itClique : itNbTests.second) {
			CLIQUEID clique= itClique.first;
			CLASSID level;
			CLASSID *classIds = decodeClique(clique, &level);
			for (auto &itTest : itClique.second) {
				MULTITESTID testId = itTest.first;
				int freq = itTest.second;
				TESTRELEVANCE<MULTITESTID> x;
				x.c = clique;
				x.t = testId;
				x.value = freq;
				selectedMultiTests[nbTests][level].push_back(x);
			}
			free(classIds);
		}
	}

}





void xcrf_features::selectFeatures(string method, int n, bool weightClique) {

	feature_selection<ATOMICTESTID> *atomicFS;
	unordered_map<int, feature_selection<MULTITESTID>*> multiFS;
	if (method == "max_freq") {
		// atomic tests
		atomicFS = new fs_max_freq<ATOMICTESTID>(totalNodesByCliqueLevel, cliqueFreq, atomicTestFreqCliqueLevel, atomicTests, weightClique,defaultClassId);
		for (int nbTests=2; nbTests<=nbMaxTests; nbTests++) {
//			cerr <<"selecting multi tests nbTests="<<nbTests<<endl;
			multiFS[nbTests] = new fs_max_freq<MULTITESTID>(totalNodesByCliqueLevel, cliqueFreq, multiTestFreqCliqueLevel[nbTests], multiTests[nbTests], weightClique, defaultClassId);
		}
	} else {
		if (method == "binary_split") {
			//		cerr <<"selecting atomic tests"<<endl;
					atomicFS = new fs_binary_split<ATOMICTESTID>(totalNodesByCliqueLevel, cliqueFreq, atomicTestFreqCliqueLevel, atomicTests, weightClique,defaultClassId);
					for (int nbTests=2; nbTests<=nbMaxTests; nbTests++) {
			//			cerr <<"selecting multi tests nbTests="<<nbTests<<endl;
						multiFS[nbTests] = new fs_binary_split<MULTITESTID>(totalNodesByCliqueLevel, cliqueFreq, multiTestFreqCliqueLevel[nbTests], multiTests[nbTests], weightClique, defaultClassId);
					}
		} else {
			if (method == "cond_entropy") {
				//		cerr <<"selecting atomic tests"<<endl;
						atomicFS = new fs_cond_entropy<ATOMICTESTID>(totalNodesByCliqueLevel, cliqueFreq, atomicTestFreqCliqueLevel, atomicTests, weightClique,defaultClassId);
						for (int nbTests=2; nbTests<=nbMaxTests; nbTests++) {
				//			cerr <<"selecting multi tests nbTests="<<nbTests<<endl;
							multiFS[nbTests] = new fs_cond_entropy<MULTITESTID>(totalNodesByCliqueLevel, cliqueFreq, multiTestFreqCliqueLevel[nbTests], multiTests[nbTests], weightClique, defaultClassId);
						}
			} else {
			if (method == "cond_entropy_clique") {
				//		cerr <<"selecting atomic tests"<<endl;
						atomicFS = new fs_cond_entropy_clique<ATOMICTESTID>(totalNodesByCliqueLevel, cliqueFreq, atomicTestFreqCliqueLevel, atomicTests, weightClique,defaultClassId);
						for (int nbTests=2; nbTests<=nbMaxTests; nbTests++) {
				//			cerr <<"selecting multi tests nbTests="<<nbTests<<endl;
							multiFS[nbTests] = new fs_cond_entropy_clique<MULTITESTID>(totalNodesByCliqueLevel, cliqueFreq, multiTestFreqCliqueLevel[nbTests], multiTests[nbTests], weightClique, defaultClassId);
						}
			} else {
			cerr << "Error: invalid feature selection '"<<method<<"'"<<endl;
			exit(10);
			}
			}
		}
	}
	int nbSelected;
	//cerr <<"selecting atomic tests"<<endl; /// here in get most relevence we loop by levels because we give as many features to each level.
	selectedAtomicTests = atomicFS->getMostRelevantTests(n/nbMaxTests, &nbSelected);// we give have of the features to one test and half for the other!
	int nbLeft = n - nbSelected;
	//cerr << "nb left before multi = "<<nbLeft<<endl;
	for (int nbTests=2; nbTests<=nbMaxTests; nbTests++) {
	  //cerr <<"selecting multi tests nbTests="<<nbTests<<endl;
		selectedMultiTests[nbTests] =  multiFS[nbTests]->getMostRelevantTests(nbLeft, &nbSelected);
		nbLeft -= nbSelected;
		//cerr << "nb left inside multi = "<<nbLeft<<endl;
	}
}


