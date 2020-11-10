#ifndef qtytats_h
#define qtytats_h

#include <cmath>
#include <map>

using namespace std;

/**
 * Statistics automaton that tracks min, max, average, stddev, an auto-sclaing histogram
 * and the number of samples (values) added.
 */
struct QtyStats {

  /** The initial (smallest) size of a bucket. */
  const double initial_bucket = 1.0E-6;

  /**
   * Construct and init defaults.
   */
  QtyStats();

  /** The minimum value added. */
  double min;

  /** The maximum value added. */
  double max;

  /** The sum of values added. */
  double total;

  /** To track average and stddev. */
  double _M;

  /** To track average and stddev. */
  double _C;

  /** The current bucket size of auto-scaloing buckets. */
  double current_bucket;

  /** The histogram. */
  map<double,size_t> buckets;

  /** The number of values added. */
  size_t items;

  /**
   * Add a new value to the statistics,
   * updating min,max,total and items.
   * @param d The value to add.
   */
  void addValue( double d );

  /**
   * Rescale the histogram.
   */
  void reBucket();

  /**
   * fill bucket gaps.
   */
  void fillBuckets();

  /**
   * Return the statistics as a formatted string.
   * @param stddev If true, include the standard deviation.
   * @return the formatted string.
   */
  string asString( bool stddev = false );


  /**
   * Return a word describing the consistency of the timings. To convince managers who think numbers are also incomprehensible.
   * Please do not take this too seriously - look at the numbers instead.
   * @return A juding word.
   */
  string consistency();

  /**
   * Return the average (mean) of the added values.
   * @return The average.
   */
  double getMean() const;

  /**
   * Return the standard deviation in the added values.
   * @return The standard deviation.
   */
  double getSigma() const;

};

#endif