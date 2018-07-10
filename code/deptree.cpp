
#include "deptree.h"
#include <string.h>
#include <algorithm>

using namespace std;

/*************************
 *
 * NOT USED AND NOT TESTED PROPERLY
 * currently doesn't seem useful anyway
 *
 *
 * int xmlNbInvalidChars = 5;
 * char xmlInvalidChars[] = "<&>'\"";
 * const char *xmlEntitiesForInvalidChars[] = {"&lt;", "&amp;", "&gt;", "&apos;", "&quot;" };
 */


bool allDigits(string s) {
  for (int i=0; i<s.size(); i++) {
    if (!isdigit(s.at(i))) {
      return false;
    }
  }
  return true;
}


deptree::deptree(vector<string> *attrs) {
  parent = NULL;
  attributes = attrs;
}


deptree::deptree(const tsv &conllSentence, int noIndex, int parentIndex) {

  badtree=false;
  // get the number of nodes (real nodes, not counting lines for contracted forms (see below)
  string nbNodesStr = conllSentence.getVectorLines().back()->at(noIndex);
  // cerr << "nbNodesStr = "<<nbNodesStr<<"; first token= "<<conllSentence.getLine(0)->at(1)<<endl;
  int nbNodes;
  if (nbNodesStr.find('-') == string::npos) { // the last token should always be a regular one, not a contracted form
    if (allDigits(nbNodesStr)) {
      nbNodes = std::stoi(nbNodesStr);
    } else {
      cerr << "Warning: invalid node no "<< nbNodesStr<<endl;
      badtree=true;
      return;
    }
  } else {
    cerr << "Bug: last line of sentence seems to be a contracted form." <<endl;
    exit(2);
  }
    

  // temporary array for building the tree (contains all the nodes)
  // we allocate nbNodes+1 cells and we don't use cell 0, so that the node id directly corresponds to the cell index (for convenience)
  deptree **allNodes = (deptree **) calloc(nbNodes+1, sizeof(deptree *));

  // first loop: store all the nodes in the array
  for (auto line : conllSentence.getVectorLines()) {
    string nodeStr = line->at(noIndex);
    // if the id does not contain hyphen = regular id
    // otherwise it's the line for a contracted form, e.g. "20-21 du", which is not part of the tree, so we ignore it
    if (nodeStr.find('-') == string::npos) {
      if (allDigits(nodeStr)) {
	int nodeNo = std::stoi(nodeStr);
	if (allNodes[nodeNo] == NULL) {
	  allNodes[nodeNo] = new deptree(line);
	} else {
	  cerr << "Warning: duplicate node no "<<nodeNo<<endl;
	  badtree=true;
	free(allNodes);
	return;
	}
      } else {
	cerr << "Warning: invalid node no "<< nodeStr<<endl;
	badtree=true;
	free(allNodes);
	return;
      }
    }
  }
    
  // second loop: connect the nodes together using their their parent node number
  //              and identify the one with no parent = the root of the tree
  vector<int> rootNo;
  for (int nodeNo=1; nodeNo<=nbNodes; nodeNo++) {
    if (allNodes[nodeNo] != NULL) { // check that it's a regular node, otherwise there was an error
      // get parent no for the current node
      string parentNodeStr = allNodes[nodeNo]->attributes->at(parentIndex);
      int parentNo;
      if (allDigits(parentNodeStr)) {
	parentNo = std::stoi(parentNodeStr);
      } else {
	cerr << "Warning: invalid node no "<< parentNodeStr<<endl;
	badtree=true;
	free(allNodes);
	return;
      }
      if (parentNo > 0) {
	// connect the node with its parent
	if (parentNo<=nbNodes) {
	  allNodes[parentNo]->addChild(allNodes[nodeNo]);
	} else {
	  cerr << "Warning: invalid head no "<< parentNo<< ", only "<<nbNodes<<" nodes in this tree"<<endl;
	  badtree=true;
	  free(allNodes);
	  return;
	}
      } else {
	//	if (!rootNo.empty()) { // we already found a root
	  //	  cerr << "Warning: dependency tree has more than one root!" <<endl;
	//	}
	rootNo.push_back(nodeNo);
      }
    } else {
      cerr << "Bug! A node in the array is NULL." <<endl;
      exit(4);
    }
  }
 
  /* previous version with only one root node allowed: 
  children = allNodes[rootNo]->children;
  for (deptree *child : children) {
    child->parent = this;
  }
  attributes = allNodes[rootNo]->attributes;
  parent = NULL;
  // free root deptree object from heap
  delete(allNodes[rootNo]);
  */

  // current object is set as the 'sentence node' which has the root(s) as children
  parent = NULL;
  for (int rootNodeNo : rootNo) {
    children.push_back(allNodes[rootNodeNo]);
    allNodes[rootNodeNo]->parent = this;
  }
  
  // free temporary array
  free(allNodes);
}


bool deptree::isBadTree() {
  return badtree;
}


void deptree::addChild(deptree* subtree) {
  children.push_back(subtree);
  subtree->parent = this;
}


void deptree::print() {
  printDepth(0);
}


void deptree::printDepth(int depth) {

  for (int i=0; i<depth; i++) {
    cout << "  ";
  }
  printVector<string>(attributes);
  cout << endl;
  for (auto child : children) { // we don't need the index that's why we don't do "for (int i=0; i<....)"
    child->printDepth(depth+1);
  }
}


void deptree::generateXMLTree(xmlNodePtr parent, tsv *columnsNames, vector<int> &keepColumnsNos) {
  char buff[256];

  //  sprintf(buff, "%02d", std::stoi(id)); // node name (???)
  // cerr << "generateXMLTree, token =  "<<id  << endl;
  xmlNodePtr node = xmlNewChild(parent, NULL, BAD_CAST "TOKEN", NULL);
  for (int i=0; i<attributes->size(); i++) {
    // either no list of attrs to keep provided (then keep everything), or the col no must be in the list
    if (keepColumnsNos.empty() || (find(keepColumnsNos.begin(), keepColumnsNos.end(), i) !=  keepColumnsNos.end())) {
      if (columnsNames == NULL) { // using generic attribute name if names are not provided
      sprintf(buff, "attr%02d", i+1);
    } else {
      strcpy(buff, columnsNames->getLine(0)->at(i).c_str());
    }
    //    cerr << "attr "<< i << " : " << attributes->at(i) << endl;

    // IMPORTANT it looks like the double quote char " is the only one replaced automatically (??) by the xml entity
    //           and it doesn't matter whether xmlCharStrdup is used or not (????)
    //    xmlNewProp(node, BAD_CAST buff, xmlCharStrdup(attributes->at(i).c_str()));
    xmlNewProp(node, BAD_CAST buff, BAD_CAST attributes->at(i).c_str());
    //    xmlNewProp(node, BAD_CAST buff, BAD_CAST xmlString(attributes->at(i).c_str()));
    }
  }
  
  //  for (auto child : children) {   // equivalent to the one below:
    for (int i=0; i<children.size(); i++) {
      deptree *child = children[i];
      child->generateXMLTree(node, columnsNames, keepColumnsNos);
  }
}


// called for the "sentence" node
xmlDocPtr deptree::generateXMLDoc(tsv *columnsNames, vector<int> &keepColumnsNos) {

  xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
  xmlNodePtr rootNode = xmlNewNode(NULL, BAD_CAST "SENTENCE");
    xmlDocSetRootElement(doc, rootNode);

    for (deptree *child : children) { // token root(s)
      child->generateXMLTree(rootNode , columnsNames, keepColumnsNos);
    }

    return doc;

}




void saveXMLToFile(string filename, xmlDocPtr xmldoc) {
  xmlSaveFormatFileEnc(filename.c_str(), xmldoc, "UTF-8", 1);
  
}


void freeXMLDoc(xmlDocPtr xmldoc) {
    xmlFreeDoc(xmldoc);
}





/*************************
 *
 * NOT USED AND NOT TESTED PROPERLY
 * currently doesn't seem useful anyway
 *
 *
// replaces special characters with their XML escaped version: <, &, >, ', " (remark: there is probably a better way with libxml??)
char *xmlString(string s) {

  char buffer[1024];
  int l=0;

  for (int i=0; i<s.size(); i++) {
    char *found = strchr(xmlInvalidChars,s[i]);
    if (found == NULL) {
      buffer[l] = s[i];
      l++;
    } else {
      cerr << "ddd"<<endl;
      while (found == NULL) {
	int invalidIndex = found - xmlInvalidChars;
	for (int j=0; j<xmlNbInvalidChars; j++) {
	  cerr << "DEBUG char = '" <<xmlInvalidChars[j] <<"' entity="<<xmlEntitiesForInvalidChars[j] <<endl;
	}
	cerr << "DEBUG string='"<<s<<"'; found = "<<invalidIndex<<endl;
	int le = strlen(xmlEntitiesForInvalidChars[invalidIndex]);
	strncpy(buffer+l, xmlEntitiesForInvalidChars[invalidIndex], le);
	l += le;
	found = strchr(xmlInvalidChars,s[i]);
      }
    }
    
  }
  char *res = (char *) malloc(l*sizeof(char));
  strncpy(res, buffer, l);

  cerr << "debug res='"<<res<<"'"<<endl;
  return res;
}
*/
