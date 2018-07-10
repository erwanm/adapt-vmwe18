/*
 * xml_tree_explorer_get_class.h
 *
 *  Created on: 27 Apr 2018
 *      Author: moreaue
 */

#ifndef XML_TREE_EXPLORER_GET_CLASS_H_
#define XML_TREE_EXPLORER_GET_CLASS_H_

#include <unordered_map>
#include "xml_tree_node_explorer.h"

class xml_tree_explorer_get_class : public xml_tree_node_explorer {

public:
  xml_tree_explorer_get_class(string idAttrName, string classAttrName, string defaultClass);

	virtual void processNode(xmlNodePtr node);

	vector<string> finalizeClasses();

	unordered_map<int,string> classesByNodeId;
	string idAttrName;
	string classAttrName;
	int maxId;
	string defaultClass;
};




#endif /* XML_TREE_EXPLORER_GET_CLASS_H_ */
