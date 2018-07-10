#ifndef MISC_H
#define MISC_H

#include<string>
#include<vector>
#include <locale>
#include<unordered_map>
#include<iostream>
#include <algorithm>

#include <string.h>
#include <dirent.h>

using namespace std;


template<typename T>
void printVector(vector<T> *v);
vector<string> listFilesDirLinux(string dirName);
vector<string> split(string s, char sep) ;
string replaceChar(string s, char replaced, char replaceWith);
string join(vector<string> v, string sep);
  



template<typename T>
void printVector(vector<T> *v) {
  cout << "[";
  if (v->size()>0) {
    cout << v->at(0);
    for (int i=1; i< v->size(); i++) {
      cout << ";" << v->at(i);
    }
  }
  cout << "]";

}



#endif
