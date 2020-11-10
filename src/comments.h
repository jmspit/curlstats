#ifndef comments_h
#define comments_h

#include <map>

using namespace std;

struct Comments {
  multimap<string,string> comments;

  void addComment( const string& line );

  string client_fqdn = "";
  string client_ip = "";
  string request = "";
  string url = "";
};

#endif