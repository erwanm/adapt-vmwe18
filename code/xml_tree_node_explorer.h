/*
 * xml_tree_node_explorer.h
 *
 *  Created on: 27 Apr 2018
 *      Author: moreaue
 */

#ifndef XML_TREE_NODE_EXPLORER_H_
#define XML_TREE_NODE_EXPLORER_H_

#include<string>
#include<vector>
#include <locale>
#include<iostream>

#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

#include "tsv.h"

using namespace std;

class xml_tree_node_explorer {

public:

	xmlXPathContextPtr xpathCtx;

	void exploreXmlTree(string filename);
	void exploreSubtree(xmlNodePtr node);

	// default: do nothing
	virtual void processNode(xmlNodePtr node);

};


#endif /* XML_TREE_NODE_EXPLORER_H_ */
