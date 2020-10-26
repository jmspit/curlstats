#ifndef datekey_h
#define datekey_h

struct DateKey {
  DateKey() : year(0),month(1),day(1) {}
  DateKey( int year, int month, int day ) : year(year),month(month),day(day) {};
  int year;
  int month;
  int day;
};

/**
 * Equality on DateKey.
 */
bool operator==( const DateKey& k1, const DateKey &k2 ) {
  return k1.year == k2.year && k1.month == k2.month && k1.day == k2.day;
}

/**
 * Ordering on DateKey.
 */
bool operator<( const DateKey& k1, const DateKey &k2 ) {
  return k1.year < k2.year || ( k1.year == k2.year && k1.month < k2.month ) ||
          ( k1.year == k2.year && k1.month == k2.month && k1.day < k2.day );
}

#endif