/*
  PrefixSpan: An efficient algorithm for sequential pattern mining

  $Id: prefixspan.cpp 2015/01/03 19:42:42 masayu-a Exp $;

  Copyright (C) 2002 Taku Kudo  All rights reserved.
  This is free software with ABSOLUTELY NO WARRANTY.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
  02111-1307, USA
*/

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <unistd.h>
#include <set>
#include <cstdlib>
using namespace std;

template <class T> class PrefixSpan {
private:
  vector < vector <T> >             transaction;
  vector < pair <T, unsigned int> > pattern;
  set <T> stopwordset;   // Stop Word の集合
  set <T> redupset;   // reduplication「々」などの定義
  map <T, T> redupmap; // reduplication の連濁用
  unsigned int minsup;   // 最小サポート
  unsigned int minpat;   // 最小パターン長
  unsigned int maxpat;   // 最大パターン長
  unsigned int mingaplen;   // 最小ギャップ長
  unsigned int maxgaplen;   // 最大ギャップ長
  unsigned int mingapnum;   // 最小ギャップ数
  unsigned int maxgapnum;   // 最大ギャップ数
  bool all;              // 全パターン枚挙するか否かのフラグ
  bool redup;            // 畳語のみを出力するか否かのフラグ
  bool redupfull;        // 畳語の全パターンを出力するか否かのフラグ
  string redupfile;      // 連濁のパターンを指定するファイル
  bool where;            // 場所を出力するか否かのフラグ
  string delimiter;      // デリミタとして用いる文字列
  string stopwordfile;   // ストップワードを指定するファイル
  bool stopword;         // ストップワードが指定されているか否かのフラグ
  string type_or_token;  // 出力が type か token か
  bool zero;             // 長さ 0 のギャップを考えるか否かのフラグ
  bool excluded;         // 「a // b」 のパターンの // に a も b も入らない
  bool verbose;          // こうるさいモードか否かのフラグ
  ostream *os;           // 出力ストリーム
  unsigned int gapcount; // gap の数
  bool gapped;           // gap があったか否かのフラグ

  void report (vector <pair <unsigned int, int> > &projected) 
  {
    if (minpat > pattern.size() - gapcount) return;
 
    if (redup) {
      bool is_redup = true;
      if (pattern.size() % 2 == 0) {
	unsigned int half = pattern.size() / 2;
	for (unsigned int i = half ; i < pattern.size(); i++) {
	  if (pattern[i].first != pattern[i - half].first) {
	    is_redup = false;
	  } 
	}
      } else {
	is_redup = false;
      }
      if (! is_redup) return;
    }
    
    if (redupfull) {
      bool is_redup = true;
      if (pattern.size() % 2 == 0) {
	unsigned int half = pattern.size() / 2;
	for (unsigned int i = half ; i < pattern.size(); i++) {
	  if (redupset.find(pattern[i].first) != redupset.end()) {
	    ; // 「々」チェック
	  } else if (pattern[i].first != pattern[i - half].first) {
	    if (redupmap[pattern[i - half].first] != pattern[i].first) {
	      is_redup = false;
	    }
	  } 
	}
      } else {
	is_redup = false;
      }
      if (! is_redup) return;
    }
    
    // print where & pattern
    if (where) { 
      *os << "<pattern>" << endl;
      
      // what:
      if (all) {
	*os << "<freq>" << pattern[pattern.size()-1].second << "</freq>" << endl;
	*os << "<what>";
	for (unsigned int i = 0; i < pattern.size(); i++) {
	  if (pattern[i].second == 0) {
	    *os << (i ? " " : "") << delimiter << delimiter;
	  } else {
	    *os << (i ? " " : "") << pattern[i].first;
	  }
	}

      } else {
	*os << "<what>";
	gapped = false;
	for (unsigned int i = 0; i < pattern.size(); i++) {
	  if (! gapped &&  pattern[i].second == 0 ) {
	    *os << (i ? " " : "") << delimiter << delimiter;
	    gapped = true;
	    
	  } else {
	    *os << (i ? " " : "") << pattern[i].first
		<< delimiter << pattern[i].second;
	    gapped = false;
	  }
	}
      }

      *os << "</what>" << endl;
      
      // where
      *os << "<where>";
      for (unsigned int i = 0; i < projected.size(); i++) 
	*os << (i ? " " : "") << projected[i].first;
      *os << "</where>" << endl;

      *os << "</pattern>" << endl;

    } else {
      if (mingapnum <= gapcount)  {
	// print found pattern only
	if (all) {
	  *os << pattern[pattern.size()-1].second;
	  for (unsigned int i = 0; i < pattern.size(); i++) {
	    if (pattern[i].second == 0) {
	      *os << " " << delimiter << delimiter;
	    } else {
	      *os << " " << pattern[i].first;
	    }
	  }
	} else {
	  gapped = false;
	  for (unsigned int i = 0; i < pattern.size(); i++) {
	    if (! gapped &&  pattern[i].second == 0 ) {
	      *os << (i ? " " : "") << delimiter << delimiter;
	      gapped = true;
	      
	    } else {
	      *os << (i ? " " : "") << pattern[i].first
		  << delimiter << pattern[i].second;
	      gapped = false;
	    }
	  }
	}
	*os << endl;
      }
    }
  }

  void project_rel_token (vector <pair <unsigned int, int> > &projected, bool init = false)
  {
    if (all) report(projected);
    map <T, vector <pair <unsigned int, int> > > ncounter;  // neighbor
    map <T, vector <pair <unsigned int, int> > > scounter;  // gap or root
  
    for (unsigned int i = 0; i < projected.size(); i++) {
      int pos = projected[i].second;
      unsigned int id  = projected[i].first;
      unsigned int size = transaction[id].size();
      map <T, int> ntmp; // neighbor
      multimap <T, int> stmp; // for gap or root
      if (init) { // root :任意の位置から数える
	for (unsigned int j = pos + 1; j < size; j++) {
	  T item = transaction[id][j];
	  if ((stopwordset.find(item) == stopwordset.end())) {
	      stmp.insert(std::pair<T, int>(item, j)) ;
	  }
	}
      } else {
	// neighbor :隣接要素を数える 
	unsigned int j = pos + 1;
	if (j < size) {
	  T item = transaction[id][j];	
	  if ((stopwordset.find(item) == stopwordset.end()) && (ntmp.find(item) == ntmp.end())) {
	    ntmp[item] = j ;
	  }
	  // (zero) gap :とびとびの要素を数える際に隣接要素を含める
	  if ((zero) && (stopwordset.find(item) == stopwordset.end()) && (stmp.find(item) == stmp.end())) {
	    stmp.insert(std::pair<T, int>(item, j)) ;
	  }
	}
	// gap :とびとびの要素を数える
	for (unsigned int j = pos + 2 ; j < size ; j++) {
	  if ((j >= pos + 1 + mingaplen) && (j <= pos + 1 + maxgaplen)){ 
	    T item = transaction[id][j];
	    if ((excluded) && (stopwordset.find(item) == stopwordset.end()) && (stmp.find (item) == stmp.end())) {
	      // -x オプション
	      bool is_excluded = true;
	      for (unsigned int k = pos + 1; k < j; k++) {
		if (transaction[id][pos] == transaction[id][k]) {
		  is_excluded = false;
		} else if (transaction[id][j] == transaction[id][k]) {
		  is_excluded = false;
		}
	      }
	      if (is_excluded) {
		stmp.insert(std::pair<T, int>(item, j));
	      }
	    } else if ((! excluded) && stopwordset.find(item) == stopwordset.end()) {
	      stmp.insert(std::pair<T, int>(item, j));
	    }
	  }
	}
      }

      // ntmp の情報を ncounter に移す 
      for (typename map <T, int>::iterator k = ntmp.begin(); k != ntmp.end(); ++k) 
	ncounter[k->first].push_back (make_pair <unsigned int, int> (id, k->second));
      // stmp の情報を scounter に移す 
      for (typename map <T, int>::iterator k = stmp.begin(); k != stmp.end(); ++k) 
	scounter[k->first].push_back (make_pair <unsigned int, int> (id, k->second));

    }

    // ncounter の枝刈り
    for (typename map <T, vector <pair <unsigned int, int> > >::iterator l = ncounter.begin();  l != ncounter.end(); ) {
      if (l->second.size() < minsup) {
	typename map <T, vector <pair <unsigned int, int> > >::iterator tmp = l;
	tmp = l; ++tmp;	ncounter.erase (l); l = tmp;
      } else {
	++l;
      }
    }

    // scounter の枝刈り
    for (typename map <T, vector <pair <unsigned int, int> > >::iterator l = scounter.begin(); l != scounter.end(); ) {
      if (l->second.size() < minsup) {
	typename map <T, vector <pair <unsigned int, int> > >::iterator tmp = l;
	tmp = l; ++tmp;	scounter.erase(l); l = tmp;
      } else {
	++l;
      }
    }

    bool isprint = true;  // 再帰にはいるかどうかのフラグ：再帰にはいると false/再帰にはいらないと true -> print

    // ncounter の再帰
    for (typename map <T, vector <pair <unsigned int, int> > >::iterator l = ncounter.begin();  l != ncounter.end(); ++l) {
      if (pattern.size () - gapcount < maxpat ){
	pattern.push_back (make_pair <T, unsigned int> (l->first, l->second.size()));
	project_rel_token (l->second);
	pattern.erase (pattern.end());
	isprint = false;
      } 
    }
    
    // scounter の再帰
    for (typename map <T, vector <pair <unsigned int, int> > >::iterator l = scounter.begin(); l != scounter.end(); ++l) {
      if (init) {
	if (pattern.size() - gapcount < maxpat ){
	  pattern.push_back(make_pair <T, unsigned int> (l->first, l->second.size()));
	  project_rel_token(l->second);
	  pattern.erase(pattern.end());
	  isprint = false;
	}
      } else {
	if (pattern.size() - gapcount < maxpat ){
	  gapcount += 1;
	  if (gapcount <= maxgapnum) {
	    pattern.push_back(make_pair <T, unsigned int> (l->first, 0));  // dummy for gap
	    pattern.push_back(make_pair <T, unsigned int> (l->first, l->second.size()));
	    project_rel_token(l->second);
	    pattern.erase(pattern.end());
	    pattern.erase(pattern.end());
	    isprint = false;
	  }
	  gapcount -= 1;
	}
      }      
    }

    // print 
    if (!all && isprint) {
      report (projected);
      return;
    }
  }

  void project_rel_type (vector <pair <unsigned int, int> > &projected, bool init = false)
  {
    if (all) report(projected);  // 全表示の場合はそのつど出力
    
    map <T, vector <pair <unsigned int, int> > > ncounter;  // neighbor
    map <T, vector <pair <unsigned int, int> > > scounter;  // gap or root

    for (unsigned int i = 0; i < projected.size(); i++) {
      int pos = projected[i].second;
      unsigned int id  = projected[i].first;
      unsigned int size = transaction[id].size();
      map <T, int> ntmp; // for neighbor
      map <T, int> stmp; // for gap or root 
      if (init) { // root :任意の位置から数える
	for (unsigned int j = pos + 1; j < size; j++) {
	  T item = transaction[id][j];
	  if (stopwordset.find(item) == stopwordset.end()) {
	    stmp.insert(std::pair<T, int>(item, j));
	  }
	}
      } else {
	// neighbor :隣接要素を数える 
	unsigned int j = pos + 1;
	if (j < size) {
	  T item = transaction[id][j];	
	  if ((stopwordset.find(item) == stopwordset.end()) && (ntmp.find (item) == ntmp.end())) {
	    ntmp[item] = j ;
	  }
	  // (zero) gap :とびとびの要素を数える際に隣接要素を含める
	  if ((zero) && (stopwordset.find(item) == stopwordset.end()) && (stmp.find (item) == stmp.end())) {
	    stmp.insert(std::pair<T, int>(item, j));
	  }
	}
	// gap :とびとびの要素を数える
	for (unsigned int j = pos + 2 ; j < size ; j++) {
	  if ((j >= pos + 1 + mingaplen) && (j <= pos + 1 + maxgaplen)){ 
	    T item = transaction[id][j];
	    if (stopwordset.find(item) == stopwordset.end() &&  (stmp.find (item) == stmp.end())) {
	      stmp.insert(std::pair<T, int>(item, j));
	    }
	  }
	}
      }

      // ntmp の情報を ncounter に移す 
      for (typename map <T, int>::iterator k = ntmp.begin(); k != ntmp.end(); ++k) 
	ncounter[k->first].push_back (make_pair <unsigned int, int> (id, k->second));
      // stmp の情報を scounter に移す 
      for (typename map <T, int>::iterator k = stmp.begin(); k != stmp.end(); ++k) 
	scounter[k->first].push_back (make_pair <unsigned int, int> (id, k->second));
      
    }

    // ncounter の枝刈り
    for (typename map <T, vector <pair <unsigned int, int> > >::iterator l = ncounter.begin(); l != ncounter.end(); ) {
      if (l->second.size() < minsup) {
	typename map <T, vector <pair <unsigned int, int> > >::iterator tmp = l;
	tmp = l; ++tmp; ncounter.erase (l); l = tmp;
      } else {
	++l;
      }
    }

    // scounter の枝刈り
    for (typename map <T, vector <pair <unsigned int, int> > >::iterator l = scounter.begin(); l != scounter.end(); ) {
      if (l->second.size() < minsup) {
	typename map <T, vector <pair <unsigned int, int> > >::iterator tmp = l;
	tmp = l; ++tmp;	scounter.erase (l); l = tmp;
      } else {
	++l;
      }
    }

    bool isprint = true; // 再帰にはいるかどうかのフラグ：再帰にはいると false/再帰にはいらないと true -> print

    // ncounter の再帰
    for (typename map <T, vector <pair <unsigned int, int> > >::iterator l = ncounter.begin(); l != ncounter.end(); ++l) {
      if (pattern.size() - gapcount < maxpat){
	pattern.push_back(make_pair <T, unsigned int> (l->first, l->second.size()));
	project_rel_type(l->second);
	pattern.erase(pattern.end());
	isprint = false;
      }
    }
    
    // scounter の再帰
    for (typename map <T, vector <pair <unsigned int, int> > >::iterator l = scounter.begin(); l != scounter.end(); ++l) {
      if (init) {
	if (pattern.size() - gapcount < maxpat ){
	  pattern.push_back(make_pair <T, unsigned int> (l->first, l->second.size()));
	  project_rel_type(l->second);
	  pattern.erase(pattern.end());
	  isprint = false;
	}
      } else {
	if (pattern.size() - gapcount < maxpat){
	  gapcount += 1;
	  if (gapcount <= maxgapnum) {
	    pattern.push_back(make_pair <T, unsigned int> (l->first, 0));  // dummy for gap
	    pattern.push_back(make_pair <T, unsigned int> (l->first, l->second.size()));
	    project_rel_type(l->second);
	    pattern.erase (pattern.end());
	    pattern.erase (pattern.end());
	    isprint = false;
	  }
	  gapcount -= 1;
	}
      }      
    }

    // print 
    if (!all && isprint) {
      report(projected);
      return;
    }
  }


public:
  PrefixSpan (unsigned int _minsup = 1,
	      unsigned int _minpat = 1, 	      
	      unsigned int _maxpat = 0x0fffffff,
	      unsigned int _mingaplen = 1,
	      unsigned int _maxgaplen = 0x0fffffff,
	      unsigned int _mingapnum = 0,
	      unsigned int _maxgapnum = 0x0fffffff,
	      bool _all = false,
	      bool _redup = false,
	      bool _redupfull = false,
	      string _redupfile = "",
	      bool _where = false,
	      string _delimiter = "/",
	      string _stopwordfile = "",
	      bool _stopword = false,
	      string _type_or_token = "type",
	      bool _zero = false,
	      bool _excluded = false,
	      bool _verbose = false):
    minsup(_minsup), minpat (_minpat), maxpat (_maxpat), mingaplen (_mingaplen), maxgaplen (_maxgaplen), mingapnum (_mingapnum), maxgapnum (_maxgapnum), all(_all), redup(_redup), redupfull(_redupfull), redupfile(_redupfile),
    where(_where), delimiter (_delimiter),  stopwordfile (_stopwordfile), stopword (_stopword), type_or_token(_type_or_token), zero(_zero), excluded(_excluded), verbose (_verbose) {};

  ~PrefixSpan () {};

  istream& read (istream &is) 
  {
    unsigned int linenum;   // 読み込み文数
    string line;
    vector <T> tmp;
    T item;
    ifstream ifs;
    T pitem;
    T sitem;

    if (stopword) {
      ifs.open(stopwordfile.c_str(), ios::in);
   
      while (ifs >> item) {
	stopwordset.insert(item);
      }
      ifs.close();
    }

    if (redupfull) {
      ifs.open(redupfile.c_str(), ios::in);
   
      while (ifs >> pitem) {
	ifs >> sitem;
	if (pitem == sitem) {
	  redupset.insert(pitem);
	} else {
	  redupmap[pitem] = sitem; 
	}
      }
      ifs.close();
    }

    linenum = 0;
    while (getline (is, line)) {
      tmp.clear ();
      istringstream istrs ((string) line);
      while (istrs >> item) tmp.push_back (item);
      transaction.push_back (tmp);
      linenum += 1;
      if (linenum % 1000000 == 0) {
	cerr << " " << linenum << endl;
      } else if (linenum % 100000 == 0) {
	cerr << ".";
      }
    }
    
    return is;
  }

  ostream& run (ostream &_os)
  {
    os = &_os;
    if (verbose) *os << transaction.size() << endl;
    vector <pair <unsigned int, int> > root;
    for (unsigned int i = 0; i < transaction.size(); i++) 
      root.push_back (make_pair (i, -1));
    gapcount = 0; 
    if (type_or_token == "token") {
      project_rel_token (root, true); 
    } else if (type_or_token == "type") {
      project_rel_type (root, true); 
    } else {
      // error 処理 
    }
    return *os;
  }

  void clear ()
  {
    transaction.clear ();
    pattern.clear ();
  }
};

int main (int argc, char **argv)
{
  extern char *optarg;
  unsigned int minsup = 1;
  unsigned int minpat = 1;
  unsigned int maxpat = 0x0fffffff;
  unsigned int mingaplen = 1;
  unsigned int maxgaplen = 0x0fffffff;
  unsigned int mingapnum = 0;
  unsigned int maxgapnum = 0x0fffffff;
  bool all = false;
  bool redup = false;
  bool redupfull = false;
  string redupfile = "";
  bool where = false;
  string delimiter = "/";
  string stopwordfile = "";
  bool stopword = false;
  bool verbose = false;
  string type = "string";
  string type_or_token = "type";
  bool zero = false;
  bool excluded = false;

  int opt;
  while ((opt = getopt(argc, argv, "arwvR:t:M:m:L:d:s:S:g:G:k:c:0x")) != -1) {
    switch(opt) {
    case 'a':
      all = true;
      break;
    case 'r':
      redup = true;
      break;
    case 'R':
      redupfull = true;
      redupfile = string (optarg);
      break;
    case 'w':
      where = true;
      break;
    case 'v':
      verbose = true;
      break;
    case 'm':
      minsup = atoi (optarg);
      break;
    case 'M':
      minpat = atoi (optarg);
      break;
    case 'L':
      maxpat = atoi (optarg);
      break;
    case 's':
      mingapnum = atoi (optarg);
      break;
    case 'S':
      maxgapnum = atoi (optarg);
      break;
    case 'g':
      mingaplen = atoi (optarg);
      break;
    case 'G':
      maxgaplen = atoi (optarg);
      break;
    case 't':
      type = string (optarg); 
      break;
    case 'd':
      delimiter = string (optarg);
      break;
    case 'k':
      stopword = true;
      stopwordfile = string (optarg);
      break;
    case 'c': // type or token
      type_or_token = string (optarg); 
      break;
    case '0': // with 0 length gap
      zero = true;
      break;
    case 'x': // exclude 'a' and 'b' between 'a' and 'b'
      excluded = true;
      break;
    default:
      cout << "Usage: " << argv[0] 
	   << " [-m minsup] [-M minpat] [-L maxpat] [-s mingapnum] [-S maxgapnum] [-g mingaplen] [-G maxgaplen] [-a] [-r] [-R] [-w] [-0] [-x] [-v] [-t type] [-d delimiter] [-k stopwordfile] < data .." << endl;
      return -1;
    }
  }
   
  if (type == "int") { 
    PrefixSpan<unsigned int> prefixspan (minsup, minpat, maxpat, mingaplen, maxgaplen, mingapnum, maxgapnum, all, redup, redupfull, redupfile, where, delimiter, stopwordfile, stopword, type_or_token, zero, excluded, verbose);
    prefixspan.read (cin);
    prefixspan.run  (cout);
  } else if (type == "short") {
    PrefixSpan<unsigned short> prefixspan (minsup, minpat, maxpat, mingaplen, maxgaplen, mingapnum, maxgapnum, all, redup, redupfull, redupfile, where, delimiter, stopwordfile, stopword, type_or_token, zero, excluded, verbose);
    prefixspan.read (cin);
    prefixspan.run  (cout);
  } else if (type == "char") {
    PrefixSpan<unsigned char> prefixspan (minsup, minpat, maxpat, mingaplen, maxgaplen, mingapnum, maxgapnum, all, redup, redupfull, redupfile, where, delimiter, stopwordfile, stopword, type_or_token, zero, excluded, verbose);
    prefixspan.read (cin);
    prefixspan.run  (cout);
  } else if (type == "string") {
    PrefixSpan<string> prefixspan (minsup, minpat, maxpat, mingaplen, maxgaplen, mingapnum, maxgapnum, all, redup, redupfull, redupfile, where, delimiter, stopwordfile, stopword, type_or_token, zero, excluded, verbose);
    prefixspan.read (cin);
    prefixspan.run  (cout);
  } else { 
    cerr << "Unknown Item Type: " << type << " : choose from [string|int|short|char]" << endl;
    return -1;
  }

  return 0;
}
