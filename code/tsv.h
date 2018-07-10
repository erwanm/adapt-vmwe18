/*
 *
 * simple TSV file manipulation as 2 dimension vector
 *Just a data structure to store the content from CONLL, it can give us some of the features.
 *
 */


#ifndef TSV_H
#define TSV_H

#include<string>
#include<vector>
#include <locale>
#include<unordered_map>
#include<iostream>

#define DEFAULT_SEPARATOR '\t'
#define DEFAULT_COMMENT_MARK "#"

using namespace std;

class tsv {

 public:
  tsv(); // why?
  tsv(string filename);
  tsv(string filename, string ignoreLinesStartingWithStr);
  tsv(string filename, char separator, string ignoreLinesStartingWithStr);

  void print() const;
  int length() const;
  // remark: returns a vector of tsv; every tsv contains pointers to the lines in the current object (for space/time efficiency reasons)
  vector<tsv> splitOnEmptyLines(const string &ignoreLinesStartingWithStr) const;
  vector<tsv> splitOnEmptyLines() const;
  void addLine(vector<string>* line);
  // returns true if correct number of columns in every line, false otherwise
  bool checkNbCols(int nbColsExpected) const;
  const vector<vector<string> *> &getVectorLines() const; /// at the start the value const cant be modified.
  vector<string> *getLine(int lineNo) const;
  vector<string> valuesAsFlatList();
  void pasteColumns(tsv &tsvColumnsToAdd);
  static bool ignoreLine(const string &line, const string &ignoreLinesStartingWithStr);
  void removeColumn(int colNo);
  void writeToFile(string filename);
  void writeToFile(string filename, char separator);
  vector<string> getColumn(int col);
  
 private:
  vector<vector<string> *> lines;

};

#endif 
