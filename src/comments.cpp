#include "comments.h"
#include "util.h"

#include <cstring>
#include <iostream>
#include <vector>

using namespace std;

string getIndexFromSplit( const string& src, size_t index, char delimiter = ';' ) {
  vector<string> splitted = split( src, delimiter );
  if ( index < splitted.size() ) return splitted[index]; else return "";
}

void Comments::addComment( const string& line ) {
  vector<string> lpair = split( line, '=' );
  if ( lpair.size() == 2 ) {
    lpair[0].erase( 0, lpair[0].find_first_not_of("#") );
    trim( lpair[0] );
    trim( lpair[1] );

    const auto &existing = comments.equal_range( lpair[0] );
    bool duplicate = false;
    for (  multimap<string,string>::iterator r = existing.first ; r != existing.second; r++ ) {
      if ( (*r).second == lpair[1] ) {
        duplicate = true;
        break;
      }
    }

    if ( ! duplicate ) comments.insert( pair<string,string>( lpair[0], lpair[1] ) );
    if ( strncmp( lpair[0].c_str(), "client FQDN", 11 ) == 0 ) client_fqdn = lpair[1];
    else if ( strncmp( lpair[0].c_str(), "client IP", 9 ) == 0 ) client_ip = lpair[1];
    else if ( strncmp( lpair[0].c_str(), "curl config", 11 ) == 0 ) {
      if (  strncmp( lpair[1].c_str(), "url", 3 ) == 0   ) url = getIndexFromSplit( lpair[1], 1, ' ' );
      else if (  strncmp( lpair[1].c_str(), "request", 7 ) == 0   ) request = getIndexFromSplit( lpair[1], 1, ' ' );
    }
  }
  //if ( line.length() > 2 && line[2] != 'Y' ) comments[string("s")] = line;
}