#ifndef qtytats_h
#define qtytats_h

#include <cmath>
#include <map>

using namespace std;

/**
 * To assemble statistics
 */
struct QtyStats {

  const double initial_bucket = 1.0E-6;

  /**
   * Construct and init defaults.
   */
  QtyStats();

  double min;
  double max;
  double total;
  double _M;
  double _C;

  double current_bucket;
  map<double,size_t> buckets;
  size_t items;

  /**
   * Add a new value to the statistics,
   * updating min,max,total and items.
   * @param d The value to add.
   */
  void addValue( double d );

  void reBucket();

  /**
   * Return the statistics as a formatted string.
   * @return the formatted string.
   */
  string asString( bool stddev = false );


  string consistency();

  double getAverage() const;

  double getSigma() const;

};

#endif