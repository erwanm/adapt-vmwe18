/*
 * xml_tree_node_explorer.cpp
 *
 *  Created on: 27 Apr 2018
 *      Author: moreaue
 */

#include "xml_tree_node_explorer.h"


void xml_tree_node_explorer::exploreXmlTree(string filename) {
  xmlDocPtr doc = xmlReadFile(filename.c_str(), NULL, 0);

  if (doc == NULL) {
    cerr << "Error: cannot parse xml file '"<<filename<<"'"<<endl;
    exit(10);
  }
  //cerr << "DEBUG " << attributes.size() << "; " << classes.size()<<endl;

  //  sentences.push_back(doc);
  xmlNodePtr root_element = xmlDocGetRootElement(doc);
  if ((root_element != NULL) && (strcmp((char *) root_element->name, "SENTENCE") == 0)) {

    // creating the XPath context here to avoid creating it many times
    xpathCtx =  xmlXPathNewContext(doc);
    if (xpathCtx == NULL) {
      cerr << "Error when creating the XPath context (no idea how this happened!)" << endl;
      exit(10);
    }

    xmlNodePtr child = root_element->children;
    bool foundChild=false;
    while (child != NULL) { // finding the node(s) child(ren) which is (are) the TOKEN root(s)
	if (child->type == XML_ELEMENT_NODE) {
	  foundChild=true;
	  exploreSubtree(child);
	}
      child = child->next;
    }
    if (!foundChild) {
      cerr << "Error: cannot find root token node under sentence node." << endl;
      exit(10);
    }
  } else {
    cerr << "Error: root element is NULL or is not a SENTENCE element." <<endl;
    exit(10);
  }

  // CAREFUL if not freeing the xml doc, the FR training data requires more than 1 GB memory, and that slows down the thing a lot...
  // a workaround might be to remove attributes not needed and useless text nodes??? unsure
  xmlXPathFreeContext(xpathCtx);
  xmlFreeDoc(doc);
  xmlCleanupParser();
}



void xml_tree_node_explorer::exploreSubtree(xmlNodePtr node) {
  if (node == NULL) {
    cerr << "Bug: xml_tree_node_explorer::exploreSubtree(xmlNodePtr node) called with NULL" << endl;
    exit(10);
  }
  if (node->type != XML_ELEMENT_NODE) {
    cerr << "Bug: xml_tree_node_explorer::exploreSubtree(xmlNodePtr node) called with a node which is not an XML element node" << endl;
    exit(10);
  }
  if (strcmp((char *) node->name, "TOKEN") != 0) {
    cerr << "Bug: xml_tree_node_explorer::exploreSubtree(xmlNodePtr node) called with a node which is not a '"<< "TOKEN" <<"' node" << endl;
    exit(10);
  }

  processNode(node);

  // recursively process children nodes
  xmlNodePtr child = node->children;
  while (child != NULL) {
    if ((child->type == XML_ELEMENT_NODE) && (strcmp((char *) node->name, "TOKEN") == 0)){
      exploreSubtree(child);
    }
    child = child->next;
  }


}



void xml_tree_node_explorer::processNode(xmlNodePtr node) {
  // default: do nothing
  cerr << "Warning: probable bug, calling xml_tree_node_explorer::processNode(xmlNodePtr node) which does nothing."<<endl;
}


