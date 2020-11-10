#ifndef datekey_h
#define datekey_h

#include <iomanip>
#include <sstream>
using namespace std;

/**
 * Date as an ordered key.
 */
struct DateKey {
  /**
   * Construct a default DateKey.
   */
  DateKey() : year(0),month(1),day(1) {}

  /**
   * Construct a specified DateKey.
   * @param year The year to set.
   * @param month The month to set.
   * @param day The day of the month to set.
   */
  DateKey( int year, int month, int day ) : year(year),month(month),day(day) {};

  string asString() const { 
    ostringstream oss;
    oss << setfill('0') << setw(4) << year << "-";
    oss << setfill('0') << setw(2) << month << "-";
    oss << setfill('0') << setw(2) << day;
    return oss.str();
  }


  /**
   * The year of the date.
   */
  int year;

  /**
   * The month of the date (January=1).
   */  
  int month;

  /**
   * The day within the month.
   */  
  int day;
};

/**
 * Equality on DateKey.
 * @param k1 Left key.
 * @param k2 Right key.
 */
inline bool operator==( const DateKey& k1, const DateKey &k2 ) {
  return k1.year == k2.year && k1.month == k2.month && k1.day == k2.day;
}

/**
 * Ordering on DateKey.
 * @param k1 Left key.
 * @param k2 Right key.
 */
inline bool operator<( const DateKey& k1, const DateKey &k2 ) {
  return k1.year < k2.year || ( k1.year == k2.year && k1.month < k2.month ) ||
          ( k1.year == k2.year && k1.month == k2.month && k1.day < k2.day );
}

#endif