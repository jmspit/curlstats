#ifndef util_h
#define util_h

#include <cmath>
#include <string>
#include <vector>

using namespace std;

/**
 * Split a string.
 */
vector<string> split( const string& src, char delimiter = ';' ) {
  vector<string> result;
  istringstream is(src);
  string s;
  while ( getline( is, s, delimiter ) ) {
    result.push_back( s );
  }
  return result;
}

/**
 * Bucket a real value to its ceil (largest integer above).
 */
double bucket( double v, double bucket ) {
  return ceil( v / bucket ) * bucket;
}

/**
 * return true if the line is a comment.
 */
bool isCommment( const string& s ) {
  return s.length() == 0 || s[0] == '#';
}

#endif