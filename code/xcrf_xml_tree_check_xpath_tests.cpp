/*
 * xcrf_xml_tree_check_xpath_tests.cpp
 *
 *  Created on: 27 Apr 2018
 *      Author: moreaue
 */


#include "xcrf_xml_tree_check_xpath_tests.h"



xcrf_xml_tree_check_xpath_tests::xcrf_xml_tree_check_xpath_tests(bool printProgress, unordered_map<ATOMICTESTID, unsigned long> &originalTests_,  unordered_map<ATOMICTESTID, char *> &atomicTestsXPath_, unordered_map<ATOMICTESTID, unsigned long> &atomicTestsXPathCheck_) : originalTests(originalTests_), atomicTestsXPath(atomicTestsXPath_), atomicTestsXPathCheck(atomicTestsXPathCheck_)  {
this->printProgress = printProgress;
}




void xcrf_xml_tree_check_xpath_tests::processNode(xmlNodePtr node) {
	checkXPathNode(node, xpathCtx);
}





void xcrf_xml_tree_check_xpath_tests::checkXPathNode(xmlNodePtr node, xmlXPathContextPtr xpathCtx) {
	for (auto itTest : originalTests) {
		char *xpathTest = atomicTestsXPath[itTest.first];
		if (xpathTest == NULL) {
			cerr << "Bug, xpath atomic tests supposed to have been computed"<<endl;
			exit(10);
		}
		// teporary tests
		//    string tmp = "child::TOKEN[@upos='N']";
		//    string tmp = "@deprel";
		//        string tmp = "self::TOKEN[@deprel='root']";
		//xmlXPathObjectPtr xpathObj = xmlXPathNodeEval(node, BAD_CAST tmp.c_str(), xpathCtx);
#ifdef DEBUG
		cerr << "checking xpath = " << xpathTest << endl;
#endif
		xmlXPathObjectPtr xpathObj = xmlXPathNodeEval(node, BAD_CAST xpathTest, xpathCtx);
		if(xpathObj == NULL) {
			cerr << "Error, cannot evaluate xpath expression <"<< xpathTest <<">"<<endl;
			exit(10);
		}
		xmlNodeSetPtr nodes = xpathObj->nodesetval;
#ifdef DEBUG
		cerr << "found "<<nodes->nodeNr<<" nodes for this xpath"<<endl;
#endif
		if (nodes != NULL) { // don't know what it means to have NULL here, nut it's different than having zero nodes!
			if (nodes->nodeNr > 0) {
				atomicTestsXPathCheck[itTest.first]++;
			}
		}
		xmlXPathFreeObject(xpathObj);

	}


}


