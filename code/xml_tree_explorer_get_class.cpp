/*
 * xml_tree_explorer_get_class.cpp
 *
 *  Created on: 27 Apr 2018
 *      Author: moreaue
 */

#include "xml_tree_explorer_get_class.h"

xml_tree_explorer_get_class::xml_tree_explorer_get_class(string idAttrName_, string classAttrName_, string defaultClass_) {
	idAttrName = idAttrName_;
	classAttrName = classAttrName_;
	maxId = 0;// to check that we have the same number of tokens in the xml and the conll
	defaultClass = defaultClass_;
}

void xml_tree_explorer_get_class::processNode(xmlNodePtr node) {
	xmlChar *idStr = xmlGetProp(node, BAD_CAST idAttrName.c_str());
	if (idStr == NULL) {
		cerr << "Error: node with no attribute '"<<idAttrName<<"'" << endl;
		exit(10);
	}
	int id = stoi((char *) idStr);
	xmlFree(idStr);
	xmlChar *clas = xmlGetProp(node, BAD_CAST classAttrName.c_str());
	if (clas == NULL) {
	  // using default class
	  classesByNodeId[id] = defaultClass;
	  //	  cerr<< "processNode: assigning default class "<<defaultClass<<" to node id " << id<<endl;
	} else {
	  classesByNodeId[id] = string((char *) clas);
	  //	  cerr<< "processNode: assigning class "<<clas<<" to node id " << id<<endl;
	  xmlFree(clas);
	}
	if (id > maxId) {
		maxId = id;
	}


}



vector<string> xml_tree_explorer_get_class::finalizeClasses() {
	if (classesByNodeId.size() != maxId) {
		cerr << "Error: number of nodes collected is "<<classesByNodeId.size()<< " but the max id found is "<<maxId<<endl;
		exit(10);
	}
	//	for (auto it : classesByNodeId) {
	//	  cerr << "key "<<it.first<<" : value = "<<it.second<<endl;
	//	}
	vector<string> res;
	
	for (int i=1; i<=maxId;i++) {
		auto it = classesByNodeId.find(i);
		if (it == classesByNodeId.end()) {
			cerr << "Error: no id "<<i<<" found in the xml tree with max id "<<maxId<<endl;
			exit(10);
		}
		res.push_back(it->second);
		}
	
	return res;
}
