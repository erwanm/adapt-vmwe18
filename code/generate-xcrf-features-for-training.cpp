#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <unordered_map>
#include <iostream>
#include <algorithm>
#include <fstream>

#include <dirent.h>

#include "deptree.h"
#include "tsv.h"
#include "xcrf_features.h"
#include "misc.h"

using namespace std;


const string progName = "generate-xcrf-features-for-training";
string labelAttrName = "class";
int cliqueLevel = 3;
int maxAtomicTests = 1;
int minFreq = 1;
string defaultClass="O";
string selectionMethod = "max_freq";
int selectN = 0;
bool weightCliques = false;

void usage(ostream &s) {
	s << "Usage: "<<progName << " [options] <xml input dir> <xpath file> <output xcrf xml>" << endl;
	s << endl;
	s << "  <xml input dir>: directory containing the annotated xml trees (input corpus)."<< endl;
	s << "  <xpath file>: file containing the list of xpath 'neighbourhoods' to consider," << endl;
	s << "    one by line. A 'neighbourhood' is an xpath expression describing a list" << endl;
	s << "    of nodes (relative to the current one) on which X features are based." <<endl;
	s << "  <output xcrf xml>: the generated xml file in the format required by xcrf," << endl;
	s << "    which contains the list of generated xcrf features." << endl;
	s << "  Remarks:"<<endl;
	s << "   - classes are collected from the data." << endl;
	s << "   - taking all the nodes into account (internal nodes and leaves)." <<endl;
	s << "   - generating features based only on attributes, not on node names. " << endl;
	s << endl;
	s << "Options" <<endl;
	s << endl;
	s << "  -c [1|2|3] max level of cliques of xcrf features (Y): 1 = only the current node" <<endl;
	s << "     2 = current node and parent node, 3 = current, parent and sibling;" <<endl;
	s << "     The levels below the one selected are always considered as well. Default: "<< cliqueLevel<<"." <<endl;
	s << "  -l <label attribute> name of the attribute used as class; default: '"<<labelAttrName<<"'" <<endl;
	s << "  -u <default class> label to use for the nodes which contain no class" <<endl;
	s << "     attribute (also for XCRF file); default: "<<defaultClass<<"." << endl;
	s << "  -a <attributes file> file containing the list of attributes names to consider"<< endl;
	s << "     for generating features (one by line); by default all the attributes are "<<endl;
	s << "     considered (except the label)." << endl;
	s << "  -x <N> max line number to take into account in <xpath file>: allows to use only"<< endl;
	s << "     a subset of the 'neighbourhoods'; by default all the lines are read."<<endl;
	s << "  -t <N> max number of atomic tests by xcrf feature (conjunction); default: "<<maxAtomicTests<<"."<<endl;
	s << "  -m <N> minimum frequency of a test; default: "<<minFreq<<"."<<endl;
	s << "  -d check xpath expressions on the data; needed only for testing purposes."<<endl;
	s << "  -i <filename> prints the big list of counts by cliques/tests to this file."<<endl;
	s << "  -s <N> select the N most relevant features according to the selection method." << endl;
	s << "  -S <selection method> TODO; used only if -s supplied. default: "<<selectionMethod<<endl;
	s << "  -w use clique weights for feature selection; TODO default: don't use."<<endl;
	s << "  -h print this help message." <<endl;
	s << endl;
}



vector<string> readFileContent(string filename) {
	tsv data(filename);
	return data.valuesAsFlatList();
}



int main (int argc, char **argv) {
	int index;
	int c;



	unordered_map<char, string> opts;
	opterr = 0;


	while ((c = getopt (argc, argv, "dc:l:u:a:x:t:m:i:S:s:wh")) != -1)
		switch (c)
		{
		case 'd':
		case 'w':
			opts[c] = "1"; // any string
			break;
		case 'h':
			usage(cout);
			exit (0);
			break;
		case 's':
		case 'S':
		case 'c':
		case 'l':
		case 'u':
		case 'a':
		case 'x':
		case 't':
		case 'm':
		case 'i':
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
	string corpusDir, xpathFilename, outputFilename;
	if (optind+3==argc) {
		corpusDir = string(argv[optind]);
		xpathFilename =  string(argv[optind+1]);
		outputFilename = string(argv[optind+2]);
	} else {
		cerr << "Error: wrong number of arguments, expecting 3 args"<<endl;
		usage(cerr);
		exit(3);

	}


	vector<string> xpathNeighbours = readFileContent(xpathFilename);

	// optional arguments
	if (!opts['c'].empty()) {
		cliqueLevel = stoi(opts['c']);
	}
	if (!opts['l'].empty()) {
		labelAttrName = opts['l'];
	}
	if (!opts['u'].empty()) {
		defaultClass = opts['u'];
	}
	vector<string> attrs;
	if (!opts['a'].empty()) {
		attrs = readFileContent(opts['a']);
	}
	if (!opts['x'].empty()) {
		int n = stoi(opts['x']);
		xpathNeighbours.erase(xpathNeighbours.begin()+n,xpathNeighbours.end());
	}
	if (!opts['t'].empty()) {
		maxAtomicTests =  stoi(opts['t']);
	}
	if (!opts['m'].empty()) {
		minFreq =  stoi(opts['m']);
	}
	if (!opts['s'].empty()) {
		selectN = stoi(opts['s']);
		if (!opts['S'].empty()) {
			selectionMethod =opts['S'];
		}
		weightCliques = (!opts['w'].empty());
	}



	cout << "CliqueLevel = " << cliqueLevel << " ; class attr name = '"<<labelAttrName<<"' ; default class = '"<<defaultClass<< "' ; max atomic tests = "<<maxAtomicTests << endl;
	cout << "XPath neighbourhoods: ";
	printVector(&xpathNeighbours);
	cout << endl;
	cout << "Attributes: ";
	printVector(&attrs);
	cout << endl;



	cout << "Reading directory '" << corpusDir<<"'"<<endl;
	vector<string> inputFiles = listFilesDirLinux(corpusDir);



	xcrf_features xcrf(cliqueLevel, xpathNeighbours, defaultClass, labelAttrName, attrs, maxAtomicTests);


	// 1. read the trees, store and analyse them
	//  for (string xmlFilename : inputFiles) {
	for (int i=0; i<inputFiles.size(); i++) {
		cout << "\r"<<i+1 <<" / "<< inputFiles.size();
		if (inputFiles[i].substr(inputFiles[i].length()-4,4) == ".xml") {
		  //		  cerr << inputFiles[i]<<endl;
			xcrf.addTree(inputFiles[i]);
		} else {
			cerr << "Warning: "<<inputFiles[i]<<" is not an xml file, skipping." <<endl;
		}
	}
	cout<<endl;

	cout<<endl;
	xcrf.printInfo(cout);

	// 2. filter tests by min frequency
	if (minFreq>1) {
		cout <<endl<< "Filtering min frequency = " << minFreq<<""<<endl;
		xcrf.filterMinFrequency(minFreq);
		cout<<endl;
		xcrf.printInfo(cout);
	}

	if (!opts['i'].empty()) {
		ofstream out(opts['i']);
		if (out) {
			cout << "Writing detailed info to '"<<opts['i']<<"'" << endl<<endl;
			xcrf.print(out,false);
			out.close();
		} else {
			cerr << "Error: cannot open file '"<<opts['i']<<"' for writing detailed info"<<endl;
			exit(10);
		}
	}

	// optional: check the generated xpath expressions
	if (!opts['d'].empty()) {
		cout << "Checking the generated xpath expressions"<<endl;
		xcrf.checkXPathTests(true,cout);
	}

	// optional: feature selection
	if (selectN>0) {
		cout << endl << "Selecting features" <<endl;
		xcrf.selectFeatures(selectionMethod, selectN, weightCliques);
	}


	cout << endl << "Generating and saving to xml CRF document." <<endl;
	int nbFeats = xcrf.generateXCRFFeatures(outputFilename);
	cout << nbFeats << " features have been written to '"<<outputFilename<<"'."<<endl;

	cout << endl;
	//xcrf.print(cout, false);

	return 0;
}
