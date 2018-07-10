
#include "tsv.h"
#include <string>
#include <iostream>
#include <fstream>

using namespace std;



tsv::tsv() {
}


//a constructor inside a constructor.

tsv::tsv(string filename) : tsv(filename, DEFAULT_SEPARATOR, DEFAULT_COMMENT_MARK) { // must call constructor in initialization list, otherwise members variables are not initialized!!
}


tsv::tsv(string filename, string ignoreLinesStartingWithStr) : tsv(filename, DEFAULT_SEPARATOR, ignoreLinesStartingWithStr) {
}



tsv::tsv(string filename, char separator, string ignoreLinesStartingWithStr)  {

	//  lines.clear();
	ifstream file(filename);
	if (file) {
		string str;

		//iterate line by line over all the lines in the data
		while (std::getline(file, str))  { // getline (streamtoGetdata, stringtoPutdatainto, thedelimiter charcter)
			vector<string> *line = new vector<string>();
			//cerr << "LINE : " << str<<endl;

			bool ignore=ignoreLine(str, ignoreLinesStartingWithStr);

			if (!ignore) {
				size_t prevPos=0;
				size_t pos = str.find(separator, prevPos);
				while (pos != string::npos) {// iterate over thye coloums in one line to tokensie by coloums.
					string v = str.substr(prevPos,pos-prevPos);
					//cerr << "pos="<< pos<<" ; prev="<<prevPos<< "; token="<<v<<endl;
					if (!v.empty()) {
						line->push_back(v);
					} else {
						cerr << "Warning: ignoring empty value (multiple consecutive separators? separator at the end of the line?)" <<endl;
					}
					prevPos = pos+1;
					pos = str.find(separator, prevPos);
				}
				// last column
				string v = str.substr(prevPos);
				//cerr << "pos="<< pos<<" ; prev="<<prevPos<< "; token="<<v<<endl;
				if (!v.empty()) { // this is for the case where the line was completely empty -> empty vector
					line->push_back(v);
				}
				lines.push_back(line);
			}
		}
	} else {
		cerr << "Error: cannot open file '"<<filename<<"'"<<endl;
	}
	//  cerr <<"size="<<lines.size()<<endl;

}

int tsv::length() const {
	return lines.size();
}

void tsv::print() const {
	for (int i=0; i<lines.size(); i++) {
		vector<string> *line = lines[i];
		cout <<"LINE "<<i<<"\t";
		for (int j=0; j<line->size(); j++) {
			cout << "*" << line->at(j) << "*  ";
		}
		cout<<endl;
	}
}


vector<tsv> tsv::splitOnEmptyLines(const string &ignoreLinesStartingWithStr) const {
	vector<tsv> res;
	int i=0;

	while (i<lines.size()) {


		while ((i<lines.size()) && ((lines[i]->empty()) || (ignoreLine(lines[i]->at(0), ignoreLinesStartingWithStr)))) { // skip empty lines
			i++;
		}

		tsv group;
		while ((i<lines.size()) && (!lines[i]->empty())) {
			vector<string> *line=lines[i];
			group.addLine(lines[i]);
			i++;
		}
		if (group.lines.size()>0) {
			res.push_back(group);
		}

	}
	return res;
}



//One tsv object each sentence since they are many sentences, return a vector

vector<tsv> tsv::splitOnEmptyLines() const {
	return splitOnEmptyLines("");
}


void tsv::addLine(vector<string>* line) {
	lines.push_back(line);

}


bool tsv::checkNbCols(int nbColsExpected) const {
	for (auto line : lines) {
		if (line->size() != nbColsExpected) {
		  cerr << "Warning: line contains "<<line->size()<< " columns, expected "<<nbColsExpected <<endl;
		  for (int j=0; j<line->size(); j++) {
			cerr << "*" << line->at(j) << "*  ";
		  }
		  cerr << endl;
		  return false;
		}
	}
	return true;
}


const vector<vector<string> *> &tsv::getVectorLines() const {
	return lines;
}

vector<string> *tsv::getLine(int lineNo) const {
	return lines[lineNo];
}



vector<string> tsv::valuesAsFlatList() {
	vector<string> res;
	for (vector<string> *line : lines) {
		res.insert(res.end(), line->begin(), line->end());
	}
	return res;
}


void tsv::pasteColumns(tsv &tsvColumnsToAdd) {
	if (lines.size() != tsvColumnsToAdd.lines.size()) {
		cerr << "Error: cannot paste columns because the tsv objects have different length: "<<lines.size() <<" ; "<< tsvColumnsToAdd.lines.size() << endl;
		exit(10);
	}
	for (int i=0; i< lines.size(); i++) {
		if (tsvColumnsToAdd.lines[i]->size() >0) {
			lines[i]->insert(lines[i]->end(), tsvColumnsToAdd.lines[i]->begin(), tsvColumnsToAdd.lines[i]->end());
		}
	}
}


bool tsv::ignoreLine(const string &line, const string &ignoreLinesStartingWithStr) {
	if (!ignoreLinesStartingWithStr.empty()) {
		size_t pos = line.find(ignoreLinesStartingWithStr, 0);
		if (pos == 0) {
			return true;
		}
	}
	return false;

}


void tsv::removeColumn(int colNo) {
  for (int lineNo=0; lineNo<lines.size(); lineNo++) {
    vector<string> *line = lines[lineNo];
    if ((colNo >=0) && (colNo<line->size())) {
      line->erase(line->begin()+colNo);
    } else {
      cerr << "Error: no column "<<colNo<<" line "<<lineNo<<" in tsv object"<<endl;
      exit(12);
    }
  }
}


void tsv::writeToFile(string filename) {
  writeToFile(filename, DEFAULT_SEPARATOR);
}
  
void tsv::writeToFile(string filename, char separator) {
  ofstream myfile (filename);
  if (myfile.is_open())  {
    for (vector<string> *line : lines) {
      if (!line->empty()) {
	myfile << line->at(0);// prinitng to a file
	for (int i=1; i< line->size(); i++) {
	  myfile <<separator<< line->at(i);
	}
      }
      myfile << endl;
    }
    myfile.close();
  } else {
    cerr << "Error: cannot open file '"<<filename<<"' for writing."<<endl;
    exit(10);
  }
}


vector<string> tsv::getColumn(int col) {
  vector<string> res;
  for (vector<string> *line : lines) {
    if (col<line->size()) {
      res.push_back(line->at(col));
    }  else {
      cerr << "Error: cannot extract column "<<col<<", line contains only "<<line->size()<<" columns.";
      exit(10);
    }
  }
  return res;
}
