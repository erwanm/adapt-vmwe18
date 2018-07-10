#include <iostream>

#include "bidirectional_string_index.h"
#include "bag_attr_val.h"

using namespace std;

int main() {
  bidirectional_string_index<ATTRID> attrs;
  unordered_map<ATTRID, bidirectional_string_index<VALID>> vals;


  cout << "info: size_t size in bytes = "<<sizeof(size_t)<<endl;

  string xx = "Ash";
  string &s = xx;
  cout << "&xx="<<&xx<<endl;
  cout << "xx="<<xx<<endl;
  cout << "&s="<<&s<<endl;
  cout << "s="<<s<<endl;

  /*
  int i = attrs.getCreate("hello");
  int i2 = attrs.get("hello");
  string s = attrs.get(1);
  attrs.getCreate("bye");
  int i3 = attrs.getCreate("hello");
  cout << "testing: i=" <<i <<"; i2="<<i2<<"; s="<<s<<"; i3="<<i3<<endl;
  */

  bag_attr_val<int> bag(attrs, vals);

  int a= 1;
  int b=10;
  int c=100;
  int d=1000;
  bag.add("attr1", "nameA", &a);
  bag.add("attr2", "nameA", &b);
  bag.add("attr1", "nameB", &c);
  bag.add("attr1", "nameC", &d);
  
  if (bag.exists("attr1", "nameB")) cout << "test 1 ok"<<endl; else cout << "test 1 failed" <<endl;
  if (!bag.exists("attr2", "nameB")) cout << "test 2 ok"<<endl; else cout << "test 2 failed" <<endl;
  if (bag.exists("attr2", "nameA")) cout << "test 3 ok"<<endl; else cout << "test 3 failed" <<endl;
  if (bag.getCheck("attr1", "nameD")==NULL)  cout << "test 4 ok"<<endl; else cout << "test 4 failed" <<endl;
  int *x = bag.getCheck("attr1", "nameA");
  //  cerr <<"before: addr x = "<< x << "; addr a = "<<&a<<"; *x="<<*x<<endl;
  if (*x == 1)   cout << "test 5 ok"<<endl; else cout << "test 5 failed" <<endl;
  (*x)++;
  //  cerr <<"after: addr x = "<< x << "; addr a = "<<&a<<"; *x="<<*x<<endl;
  if (a == 2)  cout << "test 6 ok"<<endl; else cout << "test 6 failed" <<endl;

  bag.print();

}
