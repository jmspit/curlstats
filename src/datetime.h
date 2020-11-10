#ifndef datetime_h
#define datetime_h

#include <string>

using namespace std;

/**
 * A date-time value with second precision. Note that there is not attempt to do any timezone conversion,
 * DateTime values are read as-is from the source data.
 */
struct DateTime {
  /** The year. */
  int year = 0;

  /** The month. */
  int month = 1;

  /** The day within the month. */
  int day = 0;

  /** The 24 hour of day. */
  int hour = 0;

  /** The minute (0..59). */
  int minute = 0;

  /** The second (0..59). */
  int second = 0;

  /** The day of the week (Sunday=0). */
  int wday = 0;

  /**
   * Parse a string value, format '2020-10-29 22:54:04'
   * @param src The string value to parse.
   * @return True if the string value parsed ok.
   */
  bool parse( const std::string& src );

  /**
   * Return the dateTiem as a string, format '2020-10-29 22:54:04'.
   * @return The string representation.
   */
  std::string asString() const;

  /**
   * Return true if this DateTime < other.
   * @param other The other DateTime.
   * @return True if this DateTime < other.
   */
  bool operator<( const DateTime& other ) const;

  /**
   * Return true if this DateTime > other.
   * @param other The other DateTime.
   * @return True if this DateTime > other.
   */
  bool operator>( const DateTime& other ) const;

  /**
   * Return true if this DateTime == other (matching to the second).
   * @param other The other DateTime.
   * @return True if this DateTime == other.
   */
  bool operator==( const DateTime& other ) const;

  /**
   * Assign the value of another DateTime.
   * @param other The other DateTime.
   * @return A reference to this.
   */
  DateTime& operator=( const DateTime& other );

};

#endif