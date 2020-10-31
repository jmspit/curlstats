#include "util.h"
#include <stdio.h>
#include <sstream>

vector<string> split( const string& src, char delimiter ) {
  vector<string> result;
  istringstream is(src);
  string s;
  while ( getline( is, s, delimiter ) ) {
    result.push_back( s );
  }
  return result;
}

double bucket( double v, double bucket ) {
  return ceil( v / bucket ) * bucket;
}

bool isCommment( const string& s ) {
  return s.length() == 0 || s[0] == '#';
}