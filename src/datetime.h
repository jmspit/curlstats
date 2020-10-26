#ifndef datetime_h
#define datetime_h

#include <sstream>

using namespace std;

struct DateTime {
  int year = 0;
  int month = 0;
  int day = 0;
  int hour = 0;
  int minute = 0;
  int second = 0;
  int wday = 0;

  bool parse( const std::string& src ) {
    std::stringstream ss;
    char c;
    ss << src;
    ss >> year;
    ss >> c;
    if ( c != '-' ) return false;
    ss >> month;
    ss >> c;
    if ( c != '-' ) return false;
    ss >> day;
    ss >> hour;
    ss >> c;
    if ( c != ':' ) return false;
    ss >> minute;
    ss >> c;
    if ( c != ':' ) return false;
    ss >> second;
    std::tm tm_in = { 0, 0, 0, day, month-1, year - 1900 };
    time_t tmp = mktime( &tm_in );
    const std::tm* tm_out = std::localtime( &tmp );
    wday = tm_out->tm_wday;
    return true;
  }

  std::string asString() {
    stringstream ss;
    ss << setfill('0') << setw(4) << year;
    ss << '-';
    ss << setfill('0') << setw(2) << month;
    ss << '-';
    ss << setfill('0') << setw(2) << day;
    ss << " ";
    ss << setfill('0') << setw(2) << hour;
    ss << ":";
    ss << setfill('0') << setw(2) << minute;
    ss << ":";
    ss << setfill('0') << setw(2) << second;
    return ss.str();
  }

  bool operator<( const DateTime& other ) {
    return year < other.year ||
           (year == other.year && month < other.month ) ||
           (year == other.year && month == other.month && day < other.day ) ||
           (year == other.year && month == other.month && day == other.day && hour < other.hour ) ||
           (year == other.year && month == other.month && day == other.day && hour == other.hour && minute < other.minute) ||
           (year == other.year && month == other.month && day == other.day && hour == other.hour && minute == other.minute && second < other.second );
  }

  bool operator>( const DateTime& other ) {
    return year > other.year ||
           (year == other.year && month > other.month ) ||
           (year == other.year && month == other.month && day > other.day ) ||
           (year == other.year && month == other.month && day == other.day && hour > other.hour ) ||
           (year == other.year && month == other.month && day == other.day && hour == other.hour && minute > other.minute) ||
           (year == other.year && month == other.month && day == other.day && hour == other.hour && minute == other.minute && second > other.second );
  }

  bool operator==( const DateTime& other ) {
    return (year == other.year && month == other.month && day == other.day && hour == other.hour && minute == other.minute && second == other.second );
  }

  DateTime& operator=( const DateTime& other ) {
    year = other.year;
    month = other.month;
    day = other.day;
    hour = other.hour;
    minute = other.minute;
    second = other.second;
    wday = other.wday;
    return *this;
  }
};

#endif