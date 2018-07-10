#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <unordered_map>
#include <iostream>
#include <algorithm>


#include "tsv.h"
#include "misc.h"


using namespace std;

const string progName = "merge-independent-labels";
int colNo=11;
string defaultCuptLabel = "*";
bool singleCol=false;

bool debug=false;



void usage(ostream &s) {
  s << "Usage: "<<progName << " [options] <cupt output file> <cupt input file 1> [...]" << endl;
  s << endl;
  s << "  TODO"<< endl;
  s << endl;
  s << "Options" <<endl;
  s << endl;
  s << "  -1 single column as output; default = replace last column" <<endl;
  s << "  -l <label col no> column of the cupt label. default: "<<colNo<<endl;
  s << "  -d <default cupt label>; default: '"<<defaultCuptLabel<<"'"<<endl;
  s << "  -D debug mode."<<endl;
  //  s << "  -u <default class>; default: '"<<defaultClass<<"'"<<endl;
  s << "  -h print this help message." <<endl;
  s << endl;
}





string makeLabel(int id, string cat, bool first) {
  if (id == 0) {
    return defaultCuptLabel;
  } else {
    if (!first || (cat=="x")) {
      return ""+to_string(id);
    } else {
      return ""+to_string(id)+":"+cat;
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
    string bioLabel = line->at(colNo-1);
    string cat="";
    if (bioLabel.size() != 1) {
      if ((bioLabel.size() >= 3) || (bioLabel.at(1) == '_')) {
	cat = bioLabel.substr(2);
      } else {
	cerr << "Error: BIO label format invalid for label '"<<bioLabel<<"'"<<endl;
	exit(10);
      }
    }
    char bilou = bioLabel.at(0);
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

}



void mergeLabels(tsv &sentence, vector<vector<string>> &categLabels) {

  vector<string> newLabels;
  // categId[categ][originalId] = newId
  unordered_map <int, unordered_map<int, int>>  categId;
  int current = 0;
  for (int i=0; i<sentence.length(); i++) {
    bool noLabel=true;
    string tokenLabels;
    for (int c=0; c<categLabels.size(); c++) {
      string &fullLabel = categLabels[c][i];
      if (fullLabel != defaultCuptLabel) {
	int pos = fullLabel.find(':');
	string newLabel;
	if (pos == string::npos) { // not the first token for this expr: lookup map
	  int newid = categId[c][stoi(fullLabel)];
	  newLabel = to_string(newid);
	} else {  // first token for this expr: add to map
	  string id =  fullLabel.substr(0, pos);
	  string cat = fullLabel.substr(pos+1);
	  current++;
	  int newid = current;
	  categId[c][stoi(id)] = newid;
	  newLabel = to_string(newid) + ":" + cat;
	}
	if (noLabel) { // first label found for this token
	  tokenLabels = newLabel;
	} else {
	  tokenLabels += ";" + newLabel;
	  if (debug) {
	    cerr << "info: overlap found = "<<tokenLabels<<endl;
	  }
	}
	noLabel = false;
      }
      
    }
    if (noLabel) {
      newLabels.push_back(defaultCuptLabel);
    } else {
      newLabels.push_back(tokenLabels);
    }
  }

  
  if (debug) {
    cerr << "final comparison original vs result:"<<endl;
  }
  for (int i=0; i<sentence.length(); i++) {
    assignLabel(sentence.getLine(i), newLabels[i]);
    if (debug) {
      for (int c=0; c<categLabels.size(); c++) {
	cerr << "\t"<< categLabels[c][i];
      }
      cerr << "\t"<<sentence.getLine(i)->back()<<endl;
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


  while ((c = getopt (argc, argv, "D1l:d:h")) != -1)
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
  vector<string> inputFiles;
  string outputFile;
  if (optind+2<=argc) {
    outputFile = string(argv[optind]);
    for (int i=optind+1;i<argc;i++) {
      inputFiles.push_back(string(argv[i]));
    }
  } else {
    cerr << "Error: wrong number of arguments, expecting at least 2"<<endl;
    usage(cerr);
    exit(3);
  }


  // optional arguments

  if (!opts['l'].empty()) {
    colNo=stoi(opts['l']);
  }
  singleCol = (!opts['1'].empty());
  debug=(!opts['D'].empty());

  if (!opts['d'].empty()) {
    defaultCuptLabel = opts['d'];
  }

  vector<tsv> content; // must store all the files otherwise pointers to line get lost!
  // categSentences[categ][sentNo]
  vector<vector<tsv>> categSentences;
  for (int i=0; i<inputFiles.size(); i++) {
    cout << "\rLoading file "<<i+1<<" / "<<inputFiles.size();
    // read as tsv from conll file
    content.push_back(tsv(inputFiles[i], ""));
    if (content[0].length() != content.back().length()) {
      cerr << "Error: different number of lines found between "<<inputFiles[0]<<" and "<<inputFiles[i]<<endl;
      exit(10);
    }

    // split sentences (separated by empty lines)
    categSentences.push_back(content.back().splitOnEmptyLines("#"));

    if (categSentences.back().size() != categSentences[0].size()) {
      cerr << "Error: different number of sentences found between "<<inputFiles[0]<<" and "<<inputFiles[i]<<endl;
      exit(10);
    }
  }
  cout << endl;

  
  for (int i=0; i< categSentences[0].size(); i++) {
    cout << "\rProcessing sentence "<<i+1<<" / "<<categSentences[0].size();

    // categLabels[categ][token] = label
    vector<vector<string>> categLabels;
    for (int c=0; c<categSentences.size(); c++) {
      tsv &catSent = categSentences[c][i];
      categLabels.push_back(catSent.getColumn(colNo-1));
    }
    mergeLabels(categSentences[0][i], categLabels);
  }
  
  content[0].writeToFile(outputFile);

  cout <<endl<<"Done."<<endl;


  return 0;
}



