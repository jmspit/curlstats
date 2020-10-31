#ifndef util_h
#define util_h

#include <cmath>
#include <string>
#include <vector>

using namespace std;

/**
 * Split a string.
 */
vector<string> split( const string& src, char delimiter = ';' );

/**
 * Bucket a real value to its ceil (largest integer above).
 */
double bucket( double v, double bucket );

/**
 * return true if the line is a comment.
 */
bool isCommment( const string& s );

#endif