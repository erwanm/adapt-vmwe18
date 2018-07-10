#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <unordered_map>
#include <iostream>
#include <algorithm>


#include "tsv.h"
#include "misc.h"

#define DEBUG 1

using namespace std;

const string progName = "bio-to-cupt-labels";
int colNo=11;
string defaultCuptLabel = "*";
bool singleCol=false;
//string charsAsO ="";
string dummyCateg="IAV";

bool debug=false;

#define XCRF_FORBIDDEN_CHAR '.'
#define XCRF_REPLACEMENT_CHAR '_'


void usage(ostream &s) {
	s << "Usage: "<<progName << " [options] <bio labels file> <output file>" << endl;
	s << endl;
	s << "  Expects IO/BIO/BILOU labels as: <L>_<category>  where L=[BILOU]"<<endl;
	s << endl;
	s << "  <bio labels file> TODO"<< endl;
	s << "  <label scheme> = IO or BIO or BILOU"<< endl;
	s << endl;
	s << "Options" <<endl;
	s << endl;
	s << "  -1 single column as output; default = replace last column" <<endl;
	s << "  -l <label col no> column of the cupt label. default: "<<colNo<<endl;
	s << "  -d <default cupt label>; default: '"<<defaultCuptLabel<<"'"<<endl;
	//	s << "  -o <characters> characters accepted and considered as 'O';"<<endl;
	//	s << "     this is a workaround for contractions with '*'."<<endl;
	s << "  -c <majority category> category to use if BIO has been "<<endl;
	s << "     made with method 'none', i.e. 'x' as category name." << endl;
	s << "     default: "<<dummyCateg<<"."<<endl;
	s << "  -D debug mode."<<endl;
	//  s << "  -u <default class>; default: '"<<defaultClass<<"'"<<endl;
	s << "  -h print this help message." <<endl;
	s << endl;
}






string makeLabel(int id, string cat, bool first) {
  if (id == 0) {
    return defaultCuptLabel;
  } else {
    // must follow cupt format with label, doesn't matter if dummy label
    if (!first) {
      return ""+to_string(id);
    } else {
      if  (cat=="x") {
	cat = dummyCateg;
      }
      return ""+to_string(id)+":"+replaceChar(cat, XCRF_REPLACEMENT_CHAR, XCRF_FORBIDDEN_CHAR);
    }
  }

}





void assignLabel(vector<string> *line, string label) {
  if (singleCol) {
    line->clear();
    line->push_back(label);
  } else {
    line->back() = label;
  }
}






void convertLabels(tsv &sentence) {
	vector<string> originalLabels; // used only in debug mode
	vector<string> newLabels;
	if (debug) {
		for (int i=0; i<sentence.length(); i++) {
			originalLabels.push_back(sentence.getLine(i)->back());
		}
	}

	// pass 1: collect labels
	int currentId = 0;
	
	for (int i=0; i<sentence.length(); i++) {
		vector<string> *line = sentence.getLine(i);
		if (line->size() <= colNo-1) {
		  cerr << "Bug, line size = "<<line->size()<<endl;
		  exit(10);
		}
		string bioLabel = line->at(colNo-1);
		//		cerr <<  "debug: '"<<bioLabel<<"'"<<endl;
		string cat="";
		if (bioLabel.size() != 1) {
			if ((bioLabel.size() >= 3) && (bioLabel.at(1) == '_')) {
				cat = bioLabel.substr(2);
			} else {
				cerr << "Error: BIO label format invalid for label '"<<bioLabel<<"'"<<endl;
				exit(10);
			}
		}
		char bilou = bioLabel.at(0);
		// WRONG
		//		if (charsAsO.find(bilou) != string::npos) {
		//		  bilou = 'O';
		//		}
		switch (bilou) {
		case 'B':
			currentId++;
			newLabels.push_back(makeLabel(currentId,cat, true));
			break;
		case 'U':
			currentId++;
			newLabels.push_back(makeLabel(currentId,cat, true));
			break;
		case 'I':
		case 'L':
			if (currentId==0) {
				//     		cerr << "Warning: inconsistent label '"<<bilou<<"' found, correcting to 'B'"<<endl; // warning not correct in the case of IO
				currentId++;
				newLabels.push_back(makeLabel(currentId,cat, true));
			} else {
				newLabels.push_back(makeLabel(currentId,cat, false));
			}
			break;
		case 'O':
			newLabels.push_back(makeLabel(0,cat, false));
			break;
		default:
			cerr << "Error, invalid first character in BIO label '"<<bioLabel<<"' (expecting BILOU)."<<endl;
			exit(10);
		}
	}
	
	if (debug) {
		cerr << "final comparison original vs result:"<<endl;
	}
	for (int i=0; i<sentence.length(); i++) {
		assignLabel(sentence.getLine(i), newLabels[i]);
		if (debug) {
			cerr << originalLabels[i] << "\t"<<sentence.getLine(i)->back()<<endl;
		}
	}
	if (debug) {
		cerr<<endl;
	}
	

}





int main (int argc, char **argv) {
	int c;

	unordered_map<char, string> opts;
	opterr = 0;


	while ((c = getopt (argc, argv, "D1l:d:c:h")) != -1)
		switch (c)
		{
		case '1':
		case 'D':
			opts[c] = "1"; // any string
			break;
		case 'h':
			usage(cout);
			exit (0);
			break;
		case 'l':
		case 'd':
		case 'c':
		  //		case 'o':
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
	string inputFile;
	string outputFile;
	if (optind+2==argc) {
		inputFile = string(argv[optind]);
		outputFile = string(argv[optind+1]);
	} else {
		cerr << "Error: wrong number of arguments, expecting 2"<<endl;
		usage(cerr);
		exit(3);
	}


	// optional arguments

	if (!opts['l'].empty()) {
		colNo=stoi(opts['l']);
	}
	//	if (!opts['o'].empty()) {
	//	  charsAsO = opts['o'];
	//	}
	if (!opts['c'].empty()) {
	  dummyCateg = opts['c'];
	}
	singleCol = (!opts['1'].empty());
	debug=(!opts['D'].empty());

	if (!opts['d'].empty()) {
		defaultCuptLabel = opts['d'];
	}



	// read as tsv from conll file
	tsv content(inputFile, "");

	// split sentences (separated by empty lines)
	vector<tsv> sentences = content.splitOnEmptyLines("#");


	for (int i=0; i< sentences.size(); i++) {
		cout << "\r"<<i+1<<" / "<<sentences.size();
		if (debug) {
			cerr << "SENTENCE "<<i+1<<endl;
		}
		convertLabels(sentences[i]);
	}

	content.writeToFile(outputFile);

	cout <<endl<<"Done."<<endl;


	return 0;
}


