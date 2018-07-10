/*
 * xcrf_xml_tree_check_xpath_tests.h
 *
 *  Created on: 27 Apr 2018
 *      Author: moreaue
 */

#ifndef XCRF_XML_TREE_CHECK_XPATH_TESTS_H_
#define XCRF_XML_TREE_CHECK_XPATH_TESTS_H_

#include "xml_tree_node_explorer.h"
#include "xcrf_features_types.h"

class xcrf_xml_tree_check_xpath_tests : public xml_tree_node_explorer {

public:
	xcrf_xml_tree_check_xpath_tests(bool printProgress, unordered_map<ATOMICTESTID, unsigned long> &originalTests_, unordered_map<ATOMICTESTID, char *> &atomicTestsXPath_, unordered_map<ATOMICTESTID, unsigned long> &atomicTestsXPathCheck_);

	virtual void processNode(xmlNodePtr node);
	void checkXPathNode(xmlNodePtr node, xmlXPathContextPtr xpathCtx);

	bool printProgress;
	unordered_map<ATOMICTESTID, unsigned long> &atomicTestsXPathCheck;
	unordered_map<ATOMICTESTID, unsigned long> &originalTests;
	unordered_map<ATOMICTESTID, char *> &atomicTestsXPath;

};



#endif /* XCRF_XML_TREE_CHECK_XPATH_TESTS_H_ */
