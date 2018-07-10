#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <unordered_map>
#include <map>
#include <set>
#include <iostream>
#include <algorithm>
#include <fstream>


#include "tsv.h"
#include "misc.h"

#define DEBUG 1

using namespace std;

const string progName = "cupt-list-exprs";
int labelColNo=11;
int idColNo=1;
int lemmaColNo=3;
string defaultCuptLabel = "*";
vector<string> filesIds;
int contiCol=0;
map<string, string> lemmaExprs;
map<string, int> contiExprs;
string summaryFile="";
string summaryPercentages="";
map<string, int> totalExprsByFile;
map<int, map<string, int>> totalExprsByFileConti;

void usage(ostream &s) {
  s << "Usage: "<<progName << " [options] <cupt file 1> [cupt file 2] [cupt file 3] ..." << endl;
  s << endl;
  s << "  prints the list of expressions as labeled in the cupt input file(s) in"<< endl;
  s << "  the following way:" << endl;
  s << "    <expr id> <lemmas> <files ids>"<<endl;
  s << endl;
  s << "    - <expr id> identifies the expr as: <sentence no>:<token id1>[:<token id2>...]"<<endl;
  s << "    - <files ids> contains the ids of the files in which the expression is"<< endl;
  s << "      annotated, e.g. '1:3' (always 1 if a single file is provided)."<<endl;
  s << endl;
  s << "  The input files must contain the same sentences (only different labels)." <<endl;
  s << endl;
  s << "Options" <<endl;
  s << endl;
  s << "  -h print this help message." <<endl;
  s << "  -l <label col no> column of the cupt label. default: "<<labelColNo<<endl;
  s << "  -d <default cupt label>; default: '"<<defaultCuptLabel<<"'"<<endl;
  s << "  -n <name1>[:name2...] names to use as ids for the files." <<endl;
  s << "  -c add a column for the continuity status of the expression."<<endl;
  s << "  -s <filename> write summary to this file" << endl;
  s << "  -p <file id> print counts in summary as percentage of the count in this file." << endl;
  s << "     the id is either the number (1,2,..) or the name given with -n."<< endl;
  s << endl;
}





unordered_map<int, vector<int>> *getExprs(tsv &sentence) {
  unordered_map<int, vector<int>> *tokensById = new unordered_map<int, vector<int>>(); // tokensById[id] = vector of tokens index

  return tokensById;
}




map<string, set<int>> *populate(vector<string> filenames) {
  map<string, set<int>> *res = new map<string, set<int>>();
  int nbSent;
  for (int f=0;f<filenames.size(); f++) {

    //    cout << "Reading file "<<(f+1)<<"/"<<filenames.size()<<": "<<filenames[f]<<endl;
    // read as tsv from conll file
    tsv content(filenames[f], "");

    // split sentences (separated by empty lines)
    vector<tsv> sentences = content.splitOnEmptyLines("#");
    if (f==0) { // done only for the first file
      nbSent = sentences.size();
    } else {
      if (sentences.size() != nbSent) {
	cerr << "Error: different number of sentences in file 1 and file "<<f+1<<endl;
	exit(4);
      }
    }
	  
    vector<string> exprs;
    for (int i=0; i< sentences.size(); i++) {
      char sentId[10];
      sprintf(sentId, "%06d",i);
      unordered_map<int, vector<int>> tokensById;
      unordered_map<int, vector<string>> tokenIdsById; // not great but well...
      unordered_map<int, vector<string>> lemmasById;
      for (int j=0; j<sentences[i].length(); j++) {
	vector<string> *line = sentences[i].getLine(j);
	string cuptLabel = line->at(labelColNo-1);
	string tokenId = line->at(idColNo-1);
	string lemma = line->at(lemmaColNo-1);
	if (cuptLabel != defaultCuptLabel) {
	  vector<string> idCategPairs = split(cuptLabel, ';');
	  for (string idCategPair : idCategPairs) {
	    int id;
	    int posColon= idCategPair.find(':');
	    if (posColon != string::npos) { // first id with categ, e.g. "1:LVC"
	      id = stoi(idCategPair.substr(0,posColon));
	    } else { //only id, e.g. "1"
	      id = stoi(idCategPair);
	    }
	    tokenIdsById[id].push_back(tokenId); 
	    tokensById[id].push_back(stoi(tokenId));
	    lemmasById[id].push_back(lemma);
	  }
	}
      }
      for (auto &it : tokenIdsById) {
	vector<string> &tokens = it.second;
	string s = string(sentId)+":"+join(tokens,",");
	(*res)[s].insert(f);
	lemmaExprs[s] = join(lemmasById[it.first], ",");
	totalExprsByFile[filesIds[f]]++;
	if (contiCol) {
	  vector<int> tokensInt = tokensById[it.first];
	  if (tokensInt.size() < tokensInt.back()-tokensInt[0]+1) {
	    contiExprs[s] = 0;
	    totalExprsByFileConti[0][filesIds[f]]++;
	  } else {
	    contiExprs[s] = 1;
	    totalExprsByFileConti[1][filesIds[f]]++;
	  }
	}
      }
    }
  }
  return res;
}


string stringDouble(const char *pattern, double v) {
  char buffer[100];
  sprintf(buffer, pattern, v);
  return string(buffer);
}


int main (int argc, char **argv) {
  int c;
  
  unordered_map<char, string> opts;
  opterr = 0;
  while ((c = getopt (argc, argv, "hl:d:n:cs:p:")) != -1)
    switch (c)
      {
      case 'c':
	opts[c] = "1"; // any string
        break;
      case 'h':
	usage(cout);
	exit (0);
	break;
      case 'n':
      case 'l':
      case 'd':
      case 's':
      case 'p':
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

  if (optind==argc) { // 0 args
    cerr << "Error: wrong number of arguments, expecting at least one"<<endl;
    usage(cerr);
    exit(3);
  }    
  vector<string> filenames;
  for (int i=0; i<argc-optind; i++) {
    filenames.push_back(string(argv[optind+i]));
    filesIds.push_back(to_string(i+1)); // default, overwritten if -n
  }
  if (!opts['n'].empty()) {
    filesIds = split(opts['n'], ':');
  }
  if (!opts['l'].empty()) {
    labelColNo=stoi(opts['l']);
  }
  if (!opts['d'].empty()) {
    defaultCuptLabel = opts['d'];
  }
  if (!opts['s'].empty()) {
    summaryFile = opts['s'];
  }
  if (!opts['c'].empty()) {
    contiCol=1;
  }
  summaryPercentages = opts['p'];
  map<string, set<int>> *exprs= populate(filenames);

  map<string, int> summary;
  map<int, map<string, int>> summaryConti;
  for (auto &it : *exprs) {
    string e = it.first;
    set<int> &filesWithExpr = it.second;
    cout << e << "\t" << lemmaExprs[e]<<"\t";
    int first=1;
    string files;
    for (int fileNo : filesWithExpr) {
      if (first) {
	first =0;
	files = filesIds[fileNo];
      } else {
	files +=  "," + filesIds[fileNo];
      }
    }
    cout << files;
    summary[files]++;
    //    summary["TOTAL"]++;
    if (contiCol) {
      cout << "\t" << ((contiExprs[e] ) ? "conti" : "discon" );
      summaryConti[contiExprs[e]][files]++;
      //      summaryConti[contiExprs[e]]["TOTAL"]++;
    }
    cout << endl;
    
    /*
      if (printContainsAsColumns) {
      for (int i=0;i<filenames.size(); i++) {
      if (filesWithExpr.find(i) == filesWithExpr.end()) {
      cout <<"";
      } else {
      }
      }
    */
  }
  

  if (!summaryFile.empty()) {
    ofstream o;
    o.open(summaryFile);
    o << "containedIn\ttotal" << ((contiCol)?"\tconti\tdiscon":"")<<endl;
    for (auto it : summary) {
      string files = it.first;
      //      if (files != "TOTAL") {
      int total = it.second;
      if (summaryPercentages.empty()) {
	o << files << "\t" << total ;
	if (contiCol) {
	  o << "\t"<< summaryConti[1][files]  << "\t"<< summaryConti[0][files] << endl;
	} else {
	  o<< endl;
	}
      } else {
	//	  cout << "debug "<<files<<" : "<< total << " / " << summary[summaryPercentages] <<endl;
	string all = stringDouble("%6.2f", (double) total / (double) totalExprsByFile[summaryPercentages] * 100);
	o << files << "\t" << all ;
	if (contiCol) {
	  cout << "debug conti "<<files<<" : "<< summaryConti[1][files] << " / " << totalExprsByFileConti[1][summaryPercentages] <<endl;
	  cout << "debug discon "<<files<<" : "<< summaryConti[0][files] << " / " << totalExprsByFileConti[0][summaryPercentages] <<endl;
	  string conti = stringDouble("%6.2f", (double) summaryConti[1][files] / (double) totalExprsByFileConti[1][summaryPercentages] *100);
	  string discon = stringDouble("%6.2f", (double) summaryConti[0][files] / (double) totalExprsByFileConti[0][summaryPercentages] * 100);
	  o << "\t"<< conti  << "\t"<< discon << endl;
	} else {
	  o<< endl;
	}
      }
      //      }
    }
    o.close();
    return 0;
  }
  
  return 0;
}


