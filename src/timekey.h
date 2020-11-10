#ifndef timekey_h
#define timekey_h

#include <cassert>

/**
 * Time with minute precision as an ordered key.
 */
struct TimeKey {
  /**
   * Default constructor.
   */
  TimeKey() : hour(0),minute(0) {};

  /**
   * Construct a specified TimeKey.
   * @param hour The hour to set.
   * @param minute The minute to set.
   */
  TimeKey( int hour, int minute ) : hour(hour),minute(minute) {};

  /**
   * The hour (0..23)
   */
  int hour;

  /**
   * The minute (0..59)
   */
  int minute;

  string asString() const {
    ostringstream oss;
    oss << fixed << setw(2) << setfill('0') << hour;
    oss << ":";
    oss << fixed << setw(2) << setfill('0') << minute;
    return oss.str();
  }

};

/**
 * Equality on TimeKey.
 * @param k1 The left key.
 * @param k2 The right key.
 */
inline bool operator==( const TimeKey& k1, const TimeKey &k2 ) {
  return k1.hour == k2.hour && k1.minute == k2.minute;
}

/**
 * Ordering on TimeKey.
 * @param k1 The left key.
 * @param k2 The right key.
 */
inline bool operator<( const TimeKey& k1, const TimeKey &k2 ) {
  return k1.hour < k2.hour || ( k1.hour == k2.hour && k1.minute < k2.minute );
}

/**
 * Bucket a TimeKey to a minute-based interval. The bucket parameter must be > 0 and <= 60.
 */
inline TimeKey bucket( const TimeKey &k, int bucket ) {
  assert( bucket > 0 && bucket <= 60 );
  return TimeKey( k.hour, k.minute / bucket * bucket );
}


#endif