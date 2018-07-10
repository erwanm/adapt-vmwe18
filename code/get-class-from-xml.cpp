#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <algorithm>
#include <unordered_set>


#include "misc.h"
#include "tsv.h"
#include "xml_tree_explorer_get_class.h"

using namespace std;


const string progName = "get-class-from-xml";

string classSymbolContraction = "*";
string idAttrName = "id";
string classAttrName = "class";
string defaultClass = "O";
string annotatedPrefixFilename = "annotated_";

int idColNo = 1;

void usage(ostream &s) {
	s << "Usage: "<<progName << " [options] <input tsv> <xml trees directory> <output tsv>" << endl;
	s << endl;
	s << "  extracts the class attribute from every xml tree in the directory, and"<< endl;
	s << "  writes it line by line according to the original order given in " <<endl;
	s << "  <input tsv> (can be the .conllu or .parsemetsv file)." << endl;
	s << "  Comments and empty lines are preserved so as to keep the output aligned"<<  endl;
	s << "  with the original tsv." << endl;
	s << "" << endl;
	s << "Options" <<endl;
	s << endl;
	s << "  -1  print class as single column; default: paste after columns from <input tsv>."<< endl;
	s << "  -i <id column in input tsv>; default: "<<idColNo<<endl;
	s << "  -c <symbol> class symbol to use for contractions; default: '"<<classSymbolContraction<<'"'<<endl;
	s << "  -u <default class> default: '"<<defaultClass<<"'"<<endl;
	s << "  -h print this help message." <<endl;
	s << endl;
}


// adds a class column to each line (or replaces if -1) in the sentence
void matchTokens(vector<string> &xmlClassesByTokenId, tsv &tsvSentence, bool singleCol) {
  for (int i=0; i<tsvSentence.length(); i++) {
    vector<string> *line = tsvSentence.getLine(i);
    string tsvTokenIdStr = line->at(idColNo-1);
    //    cerr << "reading tsv id = "<<tsvTokenIdStr<<endl;
    if (singleCol) {
      line->clear();
    } 
    if (tsvTokenIdStr.find('-') == string::npos) { // normal id
      //      cerr << "normal id "<<endl;
      int tsvTokenId = stoi(tsvTokenIdStr);
      if ((tsvTokenId > 0) && (tsvTokenId <= xmlClassesByTokenId.size())) {
	string clas =  xmlClassesByTokenId[tsvTokenId-1];
	xmlClassesByTokenId[tsvTokenId-1] = ""; // to check later that all classes have been used
	line->push_back(clas);
      } else {
	cerr << "Error: tsvTokenId = "<<tsvTokenId<< " but xmlClasses size = "<< xmlClassesByTokenId.size()<<endl;
	exit(10);
      }
    } else { // contraction id
      //      cerr << "contraction id "<<endl;
	line->push_back(classSymbolContraction);
    }
  }

  // checking that all classes have been used
  for (int i=0; i< xmlClassesByTokenId.size(); i++) {
    if (!xmlClassesByTokenId[i].empty()) {
      cerr << "Error: class for id "<<i+1<<" from xml file was not used"<<endl;
      exit(10);
    }
  }
}


void assignDefaultPred(tsv &tsvSentence, bool singleCol) {
  for (int i=0; i<tsvSentence.length(); i++) {
    vector<string> *line = tsvSentence.getLine(i);
    string tsvTokenIdStr = line->at(idColNo-1);
    if (singleCol) {
      line->clear();
    } 
    if (tsvTokenIdStr.find('-') == string::npos) { // normal id
	line->push_back(defaultClass);
    } else { // contraction id
      //      cerr << "contraction id "<<endl;
	line->push_back(classSymbolContraction);
    }
  }
}




int main (int argc, char **argv) {
	int aflag = 0;
	int bflag = 0;
	char *cvalue = NULL;
	int index;
	int c;

	unordered_map<char, string> opts;
	opterr = 0;


	while ((c = getopt (argc, argv, "1i:c:u:h")) != -1)
		switch (c)
		{
		case '1':
			opts[c] = "1"; // any string
			break;
		case 'h':
			usage(cout);
			exit (0);
			break;
		case 'i':
		case 'c':
		case 'u':
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
	string tsvFilename;
	string xmlDir;
	string outputFilename;
	if (optind+3==argc) {
		tsvFilename = string(argv[optind]);
		xmlDir = string(argv[optind+1]);
		outputFilename = string(argv[optind+2]);
	} else {
		cerr << "Error: wrong number of arguments, expecting 2"<<endl;
		usage(cerr);
		exit(3);
	}


	// optional arguments
	bool outputSingleCol = !opts['1'].empty();
	if (!opts['i'].empty()) {
		idColNo = stoi(opts['i']);
	}
	if (!opts['c'].empty()) {
	  classSymbolContraction = opts['c'];
	}
	if (!opts['u'].empty()) {
	  defaultClass = opts['u'];
	}





	tsv data(tsvFilename, ""); // second arg to avoid removing comments
	// split sentences (separated by empty lines)
	vector<tsv> sentences = data.splitOnEmptyLines("#"); // ignore comments here

	vector<string> inputFiles0 = listFilesDirLinux(xmlDir);
	unordered_set<string> inputFiles;
	for (int i=0; i<inputFiles0.size(); i++) { // removing non-xl files
		if (inputFiles0[i].substr(inputFiles0[i].length()-4,4) == ".xml") {
			inputFiles.insert(inputFiles0[i]);
		} else {
			cerr << "Warning: "<<inputFiles0[i]<<" is not an xml file, skipping." <<endl;
		}

	}

	
	int nbErr = 0;
	for (int i=0; i<sentences.size(); i++) { 
		cout << "\r"<<i+1 <<" / "<< sentences.size();

		char idStr[10];
		sprintf(idStr, "%06d",i);
		string inputFilename = xmlDir+"/" + annotatedPrefixFilename + idStr+ ".xml";
		if (inputFiles.find(inputFilename) == inputFiles.end()) {
		  cerr << "Warning: file '"<<inputFilename<<"' not found in directory '"<<xmlDir<<"', replacing with default predictions ("<<sentences[i].length()<<" tokens in this sentence)."<<endl;
		  assignDefaultPred(sentences[i], outputSingleCol);
		  nbErr++;
		} else {		
		  xml_tree_explorer_get_class extractor(idAttrName, classAttrName, defaultClass);
		  //cerr << "xml file =  "<<inputFiles[i]<<endl;
		  extractor.exploreXmlTree(inputFilename);
		  vector<string> classes = extractor.finalizeClasses();
		  matchTokens(classes, sentences[i], outputSingleCol);
		}
	}
	cout << endl;
	if (nbErr>0) {
	  cerr << "Warning: "<<nbErr<<"/"<<sentences.size()<<" files were not found in the xml dir, replaced with default predictions."<<endl;
	}


	
	data.writeToFile(outputFilename);// we are printing the conll file/cupt "data"
	return 0;
}

