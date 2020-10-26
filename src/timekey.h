#ifndef timekey_h
#define timekey_h

/**
 * Key to bucket repeating daily time-od-day ranges.
 */
struct TimeKey {
  TimeKey() : hour(0),minute(0) {};
  TimeKey( int hour, int minute ) : hour(hour),minute(minute) {};
  int hour;
  int minute;
};

/**
 * Equality on TimeKey.
 */
bool operator==( const TimeKey& k1, const TimeKey &k2 ) {
  return k1.hour == k2.hour && k1.minute == k2.minute;
}

/**
 * Ordering on TimeKey.
 */
bool operator<( const TimeKey& k1, const TimeKey &k2 ) {
  return k1.hour < k2.hour || ( k1.hour == k2.hour && k1.minute < k2.minute );
}

/**
 * Bucket a TimeKey.
 */
TimeKey bucket( const TimeKey &k, int bucket ) {
  return TimeKey( k.hour, k.minute / bucket * bucket );
}


#endif