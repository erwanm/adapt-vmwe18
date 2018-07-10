
#include "misc.h"


// from https://stackoverflow.com/questions/306533/how-do-i-get-a-list-of-files-in-a-directory-in-c
// would probably not work with other OS than linux
vector<string> listFilesDirLinux(string dirName) {
	DIR *dpdf;
	struct dirent *epdf;
	vector<string> res;

	dpdf = opendir(dirName.c_str());
	if (dpdf != NULL){
		while (epdf = readdir(dpdf)){
			if ((strcmp(epdf->d_name, ".")!=0) && (strcmp(epdf->d_name, "..")!=0)) {
				res.push_back(dirName+"/"+string(epdf->d_name));
			}
		}
	}
	closedir(dpdf);
	sort(res.begin(), res.end());
	return res;
}



vector<string> split(string s, char sep) {
  vector<string> res;
  int prevPos=0;
  int pos = s.find(sep, prevPos-prevPos);
  while (pos != string::npos) {
    //    cerr << "A split: s='"<<s<<"'; prev="<<prevPos<<"; pos="<<pos<<"; substr="<<s.substr(prevPos, pos-prevPos)<<endl;
    res.push_back(s.substr(prevPos, pos-prevPos));
    prevPos=pos+1;
    pos = s.find(sep,prevPos);
  }
  //  cerr << "B split: s='"<<s<<"'; prev="<<prevPos<<"; pos="<<pos<<"; substr="<<s.substr(prevPos)<<endl;
  res.push_back(s.substr(prevPos));
  return res;
}


string replaceChar(string s, char replaced, char replaceWith) {
  string res = s;
  replace(res.begin(), res.end(), replaced, replaceWith);
  return res;
}



string join(vector<string> v, string sep) {
  string res="";
  if (v.size()>0) {
    res=v[0];
    for (int i=1; i<v.size(); i++) {
      res += sep+v[i];
    }
  }
  return res;
}
