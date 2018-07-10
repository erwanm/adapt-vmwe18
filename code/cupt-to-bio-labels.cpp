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

const string progName = "cupt-to-bio-labels";
int colNo=11;
string category="";
bool ignoreCat=false;
string defaultCuptLabel = "*";
bool singleCol=false;
bool killSmallest=false;

bool debug=false;

#define XCRF_FORBIDDEN_CHAR '.'
#define XCRF_REPLACEMENT_CHAR '_'



void usage(ostream &s) {
  s << "Usage: "<<progName << " [options] <label scheme> <cupt labels file> <output file>" << endl;
  s << endl;
  s << "  <cupt labels file> TODO"<< endl;
  s << "  <label scheme> = IO or BIO or BILOU"<< endl;
  s << endl;
  s << "Options" <<endl;
  s << endl;
  s << "  -1 single column as output; default = replace last column" <<endl;
  s << "  -i ignore categories, i.e. treat all categories together as a single category."<< endl;
  s << "  -c <category> treat only this ccategory and ignore any other."<<endl;
  s << "  -l <label col no> column of the cupt label. default: "<<colNo<<endl;
  s << "  -d <default cupt label>; default: '"<<defaultCuptLabel<<"'"<<endl;
  s << "  -k 'kill' smallest expression in case of overlap; default:"<<endl;
  s << "     add the tokens of the smallest to the largest."<<endl;
  s << "  -D debug mode."<<endl;
  //  s << "  -u <default class>; default: '"<<defaultClass<<"'"<<endl;
  s << "  -h print this help message." <<endl;
  s << endl;
}






// returns 1 = keep first, 2 = keep second; 0 = no overlap, keep both
int overlapExprs(vector<int> &tokens1, vector<int> &tokens2) {
  if ((tokens1.back() < tokens2.front()) || (tokens2.back() < tokens1.front())) {
    return 0;
  }
  if (debug) {
    cerr<< "overlap: start1="<<tokens1.front()<<"; start2="<<tokens2.front()<<"; end1="<<tokens1.back()<<"; end2="<<tokens2.back()<<"; lenght1="<<tokens1.size()<<"; length2="<<tokens2.size()<<endl;
  }
  if (tokens1.size()>tokens2.size()) {
    return 1;
  }
  if (tokens1.size()<tokens2.size()) {
    return 2;
  }
  if (tokens1.front()<tokens2.front()) {
    return 1;
  }
  if (tokens1.front()>tokens2.front()) {
    return 2;
  }
  if (tokens1.back()>tokens2.back()) { // spans further
    return 1;
  } else { // if equal, they are exactly the same expr
    return 2;
  }
    
  
}




bool findOverlapPair(unordered_map<int, vector<int>> &tokensById, int &replaceThis, int &replaceWithThat) {
  unordered_map<int, vector<int>>::iterator exprIt1 = tokensById.begin();
  while (exprIt1 != tokensById.end()) { 
    unordered_map<int, vector<int>>::iterator exprIt2 = tokensById.begin();
    while (exprIt2 != tokensById.end()) {
      if (exprIt1->first != exprIt2->first) {
	int o = overlapExprs(exprIt1->second, exprIt2->second);
	if (o!=0) {
	  if (o == 1) {
	    replaceThis = exprIt2->first;
	    replaceWithThat = exprIt1->first;
	  } else {
	    replaceThis = exprIt1->first;
	    replaceWithThat = exprIt2->first;
	  }
	  return true;
	}
      }
      exprIt2++;
    }
    exprIt1++;
  }
  
  return false;
}


void insertSorted(int e, vector<int> &v) {
  vector<int>::iterator it = v.begin();
  while ((it != v.end()) && ((*it)<e)) {
    it++;
  }
  v.insert(it, e);
}



int flatten(unordered_map<int, string> &categoriesByExpr, unordered_map<int, vector<int>> &tokensById) {

  int exprReplaceThis, exprReplaceWithThat;
  int nb = 0;
  bool o = findOverlapPair(tokensById, exprReplaceThis, exprReplaceWithThat);
  while (o) {
    nb++;
    // solve overlap
    if (killSmallest) {
      //      cerr << "Warning: overlaping expressions, deleting smallest"<<endl;
      if (debug) {
	cerr << "## overlap found, deleting "<<exprReplaceThis<<" and keeping " << exprReplaceWithThat<<endl;
      }
    } else {
      // cerr << "Warning: overlaping expressions, replacing"<<endl;
      if (debug) {
	cerr << "## overlap found, replacing "<<exprReplaceThis<<" with " << exprReplaceWithThat<<endl;
      }
      vector<int> &tokensReplaceWithThat = tokensById[exprReplaceWithThat];
      for (int token : tokensById[exprReplaceThis]) {
	if (find(tokensReplaceWithThat.begin(), tokensReplaceWithThat.end(), token) == tokensReplaceWithThat.end()) { // token not already in expr to keep
	  insertSorted(token, tokensReplaceWithThat);
	  if (debug) {
	    cerr << "Token "<<token<<"added to expr "<<exprReplaceWithThat<<endl;
	  }
	} else {
	  if (debug) {
	    cerr << "Token "<<token<<" already in expr "<<exprReplaceWithThat<<endl;
	  }
	}
      }
    }
    categoriesByExpr.erase(exprReplaceThis);
    tokensById.erase(exprReplaceThis);
    o = findOverlapPair(tokensById, exprReplaceThis, exprReplaceWithThat);
  }
  return nb;

}



void applyOptions(unordered_map<int, string> &categoriesByExpr, unordered_map<int, vector<int>> &tokensById) {
  unordered_map<int, string>::iterator exprIt = categoriesByExpr.begin();
  while (exprIt != categoriesByExpr.end()) {
    //      for (auto exprIt: categoriesByExpr) {
    int id = exprIt->first;
    string cat = exprIt->second;
    vector<int> &tokens = tokensById[id];
    bool remove=false;
    if (!category.empty()) {
      if (category == cat) {
	exprIt->second = category;
      } else {
	// remove expr
	tokensById.erase(id);
	remove = true;
      }
    }
    if (ignoreCat) {
      exprIt->second="x";
    }
    if (remove) {
      exprIt = categoriesByExpr.erase(exprIt);
    } else {
      exprIt++;
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





int convertLabels(tsv &sentence, unordered_map<char, string> &bilouLabels) {
  unordered_map<int, string> categoriesByExpr;
  unordered_map<int, vector<int>> tokensById; // tokensById[id] = vector of tokens index

  vector<string> originalLabels; // used only in debug mode
  vector<string> newLabels;
  if (debug) {
    for (int i=0; i<sentence.length(); i++) {
      originalLabels.push_back(sentence.getLine(i)->back());
    }
  }
  
  // pass 1: collect labels in variables above
  for (int i=0; i<sentence.length(); i++) {
    vector<string> *line = sentence.getLine(i);
    string cuptLabel = line->at(colNo-1);
    if (cuptLabel != defaultCuptLabel) {
      vector<string> idCategPairs = split(cuptLabel, ';');
      if (debug) {
	cerr << "token i="<<i<< "; splitting cupt label = '"<<cuptLabel<<"'"<<endl;
      }
      for (string idCategPair : idCategPairs) {
	if (debug) {
	  cerr << "  part = '"<<idCategPair<<"'"<<endl;
	}
	int id;
	string categ;
	int posColon= idCategPair.find(':');
	if (posColon != string::npos) { // first id with categ, e.g. "1:LVC"
	  id = stoi(idCategPair.substr(0,posColon));
	  categ = idCategPair.substr(posColon+1);
	  if (debug) {
	    cerr << "  id=  "<<id<<" ; categ = "<<categ<<endl;
	  }
	  categoriesByExpr[id] = categ;
	} else { //only id, e.g. "1"
	  id = stoi(idCategPair);
	  categ = categoriesByExpr[id];
	}
	tokensById[id].push_back(i);
      }
    }
    newLabels.push_back(""+bilouLabels['O']); // fill new labels with default case = O
    
  }

  // simplify exprs to make it BIO-compatible
  applyOptions(categoriesByExpr, tokensById);
  int nbOverlaps = flatten(categoriesByExpr, tokensById);

    
  // pass 2: replace labels depending on options
  for (auto exprIt: categoriesByExpr) {
    int id = exprIt.first;
    string cat = replaceChar(exprIt.second, XCRF_FORBIDDEN_CHAR, XCRF_REPLACEMENT_CHAR); // XCRF does not want . in labels...
    vector<int> &v = tokensById[id];
    if (v.size() == 1) {
      newLabels[v.front()] = ""+bilouLabels['U']+"_"+cat;
    } else {
      for (int i : v) {
      newLabels[i] = ""+bilouLabels['I']+"_"+cat;
      }
      // overwrite first and last
      newLabels[v.front()] = ""+bilouLabels['B']+"_"+cat;
      newLabels[v.back()] = ""+bilouLabels['L']+"_"+cat;
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

  return nbOverlaps;
}





int main (int argc, char **argv) {
  int c;
  
  unordered_map<char, string> opts;
  opterr = 0;


  while ((c = getopt (argc, argv, "Dki1c:l:d:h")) != -1)
    switch (c)
      {
      case 'i':
      case '1':
      case 'k':
      case 'D':
	opts[c] = "1"; // any string
        break;
      case 'h':
	usage(cout);
	exit (0);
	break;
      case 'c':
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
  string labelScheme;
  string inputFile;
  string outputFile;
  if (optind+3==argc) {
    labelScheme = string(argv[optind]);
    inputFile = string(argv[optind+1]);
    outputFile = string(argv[optind+2]);
  } else {
    cerr << "Error: wrong number of arguments, expecting 3"<<endl;
    usage(cerr);
    exit(3);
  }


  // optional arguments

  if (!opts['l'].empty()) {
    colNo=stoi(opts['l']);
  }
  category=opts['c'];
  singleCol = (!opts['1'].empty());
  ignoreCat=(!opts['i'].empty());
  if (ignoreCat && !category.empty()) {
    cerr << "Cannot have both -c and -i, choose only one"<<endl;
    exit(10);
  }
  killSmallest=(!opts['k'].empty());
  debug=(!opts['D'].empty());

  if (!opts['d'].empty()) {
    defaultCuptLabel = opts['d'];
  }
  
  unordered_map<char, string> bilouLabels;
  if (labelScheme == "IO") {
    bilouLabels['B'] = "I";
    bilouLabels['I'] = "I";
    bilouLabels['L'] = "I";
    bilouLabels['O'] = "O";
    bilouLabels['U'] = "I";
  } else {
    if (labelScheme == "BIO") {
      bilouLabels['B'] = "B";
      bilouLabels['I'] = "I";
      bilouLabels['L'] = "I";
      bilouLabels['O'] = "O";
      bilouLabels['U'] = "B";
    } else {
      if (labelScheme == "BILOU") {
	bilouLabels['B'] = "B";
	bilouLabels['I'] = "I";
	bilouLabels['L'] = "L";
	bilouLabels['O'] = "O";
	bilouLabels['U'] = "U";
      } else {
	cerr << "Error: unknown label scheme '"<<labelScheme<<"', should be IO or BIO or BILOU"<<endl;
	exit(10);
      }
    }
  }
  
  
  // read as tsv from conll file
  tsv content(inputFile, "");

  // split sentences (separated by empty lines)
  vector<tsv> sentences = content.splitOnEmptyLines("#");

  int nbOverlaps=0;
  for (int i=0; i< sentences.size(); i++) {
    cout << "\r"<<i+1<<" / "<<sentences.size();
    if (debug) {
      cerr << "SENTENCE "<<i+1<<endl;
    }
    nbOverlaps += convertLabels(sentences[i], bilouLabels);
  }
  cout <<endl;
  if (nbOverlaps>0) {
    cerr << endl << "Warning: "<<nbOverlaps<<" overlaps found."<<endl;
  }

  content.writeToFile(outputFile);
  
  cout <<endl<<"Done."<<endl;

  

  return 0;
}


