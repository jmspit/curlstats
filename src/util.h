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

inline std::string& ltrim(std::string& s, const char* t = " \t\n\r\f\v")
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}

inline std::string& rtrim(std::string& s, const char* t = " \t\n\r\f\v")
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

inline std::string& trim(std::string& s, const char* t = " \t\n\r\f\v")
{
    return ltrim(rtrim(s, t), t);
}

#endif