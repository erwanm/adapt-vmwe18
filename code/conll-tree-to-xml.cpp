#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <unordered_map>
#include <iostream>
#include <algorithm>

#include <libxml/tree.h>
#include <libxml/parser.h>


#include "tsv.h"
#include "deptree.h"

using namespace std;

const string progName = "conll-tree-to-xml";

string idColName="id";
string headColName="head";
string labelAttrName = "class";


int idColIndex = 0;
int headColIndex = 6;

void usage(ostream &s) {
  s << "Usage: "<<progName << " [options] <features tsv>" << endl;
  s << endl;
  s << "  <features tsv> contains tab-separated values in the conllu format (or similar),"<< endl;
  s << "  i.e. one token by line and sentences separated by an empty line." <<endl;
  s << "  <>" << endl;
  s << endl;
  s << "Options" <<endl;
  s << endl;
  s << "  -n <features names file> names of the features, in a tsv file with a single line."<< endl;
  s << "     Caution: if not provided, the number of columns is not checked and default" << endl;
  s << "     values are used for the id ("<< idColIndex<<") and head ("<<headColIndex<<") column indexes." <<endl;
  s << "  -t <labels file> labels for training mode, in a file containing a single column aligned"<<endl;
  s << "     with the features file."<<endl;
  s << "  -o <output prefix> prefix of the xml files as output. Can include a path; empty by default."  << endl;
  s << "  -l <label attribute> name of the attribute used as class; default: '"<<labelAttrName<<"'" <<endl;
  s << "  -k <attributes file> filen containing a list of attributes to keep, all others will be "<<endl;
  s << "     removed from the output xml trees (except id and class). attribute names should be"<<endl;
  s << "     used if -n is supplied, column numbers otherwise. By default all the attributes are kept."<<endl;
  s << "  -h print this help message." <<endl;
  s << endl;
}


int main (int argc, char **argv) {
  int aflag = 0;
  int bflag = 0;
  char *cvalue = NULL;
  int index;
  int c;
  
  unordered_map<char, string> opts;
  opterr = 0;


  while ((c = getopt (argc, argv, "n:t:o:l:k:h")) != -1)
    switch (c)
      {
      case 'a':
      case 'b':
	opts[c] = "1"; // any string
        break;
      case 'h':
	usage(cout);
	exit (0);
	break;
      case 'n':
      case 't':
      case 'o':
      case 'l':
      case 'k':
	opts[c] = string(optarg);
        break;
      case '?':
        if (optopt == 'c')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        return 1;
      default:
        abort ();
      }

  // mandatory arguments
  string featuresFilename;
  if (optind+1==argc) {
    featuresFilename = string(argv[optind]);
  } else {
    cerr << "Error: wrong number of arguments, expecting 1"<<endl;
    usage(cerr);
    exit(3);
  }


  // optional arguments
  string colNamesFilename = opts['n'];
  string outputPrefix = opts['o'];
  string trainingLabelsFilename = opts['t'];
  if (!opts['l'].empty()) {
    labelAttrName = opts['l'];
  }

  
  // columns names and number
  tsv *colNames = NULL;
  int nbColumns;
  if (!colNamesFilename.empty()) {
    colNames = new tsv(colNamesFilename);
    //colNames->print();
    const vector<string> *v=colNames->getLine(0);
    nbColumns = v->size();
    auto foundId = find(v->begin(), v->end(), idColName);
    auto foundHead = find(v->begin(), v->end(), headColName);
    if ((foundId == v->end()) || (foundHead == v->end())) {
      cerr << "Error: cannot find 'id' and/or 'head' in the columns names in '"<<colNamesFilename<<"'.";
      exit(5);
    } else {
      idColIndex = foundId - v->begin();
      headColIndex = foundHead - v->begin();
    }
  } else {
    cerr << "Warning no columns names provided, cannot check number of columns."<< endl;
  } 


  vector<int> keepAttrs;
  if (!opts['k'].empty()) {
    tsv t(opts['k']);
    for (string &a : t.valuesAsFlatList()) {
      int colNo;
      if (colNames != NULL) { // string names
	const vector<string> *v=colNames->getLine(0);
	auto found = find(v->begin(), v->end(), a);
	if ((found == v->end()) || (found == v->end())) {
	  cerr << "Error: cannot find '"<<a<<"' in the columns names.";
	  exit(5);
	} else {
	  colNo = found - v->begin();
	}
      }  else { // column numbers;
	colNo = stoi(a)-1;
      }
      keepAttrs.push_back(colNo);
    }
    // add id
    keepAttrs.push_back(idColIndex);
  }


  // read features as tsv from conll file
  tsv content(featuresFilename, "");

  
  if (!trainingLabelsFilename.empty()) {
    tsv classesColumn(trainingLabelsFilename, "");
	  content.pasteColumns(classesColumn);
	  if (colNames != NULL) { // adding column name for class to column names
		  vector<string> *tmpLabelNameVector = new vector<string>();
		  tmpLabelNameVector->push_back(labelAttrName);
		  tsv tmpLabelName;
		  tmpLabelName.addLine(tmpLabelNameVector);
		  colNames->pasteColumns(tmpLabelName);
		  nbColumns++;
	  }
	  if (!keepAttrs.empty()) {
	    // add class attr to attributes to keep
	    keepAttrs.push_back(nbColumns-1);
	  }
  }


  // split sentences (separated by empty lines)
  vector<tsv> sentences = content.splitOnEmptyLines("#");

  cout <<"Read " <<sentences.size() <<" sentences (containing approximately "<<content.length()<<" tokens) from '"<<featuresFilename <<"'"<<endl;
  cout << "Converting to xml..."<<endl;
  
  //  vector<deptree> deptrees;
  for (int i=0; i< sentences.size(); i++) {
    cout << "\r"<<i+1<<" / "<<sentences.size();
    // 	  cout << "******************************************************************"<<endl;
    //	  s.print();
    if ((colNames != NULL) && (!sentences[i].checkNbCols(nbColumns))) {
      cerr << "Error: some lines don't contain exactly "<< nbColumns <<" columns." <<endl;
      exit(2);
    }

    // build the dependency tree from the tsv file
    deptree t(sentences[i], idColIndex, headColIndex);
    //deptrees.push_back(t);
    if (t.isBadTree()) {
      cerr << "Warning: sentence "<< i+1<<"has an invalid tree, skipping"<<endl;
    } else {

      xmlDocPtr doc = t.generateXMLDoc(colNames, keepAttrs);

      char idStr[10];
      sprintf(idStr, "%06d",i);
      string outputFilename = outputPrefix + idStr+ ".xml";
      saveXMLToFile(outputFilename, doc);
    }	  
    //	  t.print();
  }
  cout <<endl<<"Done."<<endl;

  

  return 0;
}
