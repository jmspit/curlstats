#ifndef datetime_h
#define datetime_h

#include <string>

using namespace std;

struct DateTime {
  int year = 0;
  int month = 0;
  int day = 0;
  int hour = 0;
  int minute = 0;
  int second = 0;
  int wday = 0;

  bool parse( const std::string& src );

  std::string asString() const;

  bool operator<( const DateTime& other ) const;

  bool operator>( const DateTime& other ) const;

  bool operator==( const DateTime& other ) const;

  DateTime& operator=( const DateTime& other );

};

#endif