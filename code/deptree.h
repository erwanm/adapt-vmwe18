/*
 *
 * Data structure for dependency tree
 *
 *
 *
 */

#ifndef DEP_TREE_H
#define DEP_TREE_H

#include<string>
#include<vector>
#include <locale>
#include<unordered_map>
#include<iostream>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "tsv.h"
#include "misc.h"

using namespace std;





class deptree {

 public:
  /* constructor from conll data. Arguments:
   * conllSentence: tsv object for one sentence (use function splitOnEmptyLines to separate the sentences)
   * noIndex: zero-indexed column number for integer id (normally first column, i.e. index 0)
   * parentIndex: zero-index column number for parent id (normally column 7 so index 6)
   *
   * remark: this function ignores the contracted forms, e.g. "20-21 du", which are not part of the tree (the expanded form is used)
   *
   */
  deptree(const tsv &conllSentence,  int noIndex, int parentIndex);
  void addChild(deptree* subtree);
  void print();
  bool isBadTree();

  // functions for exporting tree as xml
  // colunmsNames is a tsv which contains only one line representing the columns names (used as attributes names in the xml), it also recieves all the trees of the dataset.
  xmlDocPtr generateXMLDoc(tsv *columnsNames, vector<int> &keepColumnsNos);
  

 private:
  // variables
  deptree *parent;  // remark: bidirectional parent-child relation (i.e. the parent can access its children and the child can access its parent)
  vector<string> *attributes;
  vector<deptree *> children;
  bool badtree;

  // functions
  // this is an internal constructor
  deptree(vector<string> *attrs);
  void printDepth(int depth);
  void generateXMLTree(xmlNodePtr parent, tsv *columnsNames, vector<int> &keepColumnsNos);


};


// non-class functions


void saveXMLToFile(string filename, xmlDocPtr xmldoc);
void freeXMLDoc(xmlDocPtr xmldoc);
// replaces special characters with their XML escaped version: <, &, >, ', " (remark: there is probably a better way with libxml??)


/*************************
 * NOT USED AND NOT TESTED PROPERLY
 * currently doesn't seem useful anyway
char *xmlString(string s);
*/


#endif 
