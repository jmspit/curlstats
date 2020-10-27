#ifndef qtytats_h
#define qtytats_h

#include <cmath>
#include <sstream>
#include "output.h"

using namespace std;

/**
 * To assemble statistics
 */
struct QtyStats {

  const size_t max_buckets = 30;
  const double initial_bucket = 1.0E-6;

  /**
   * Construct and init defaults.
   */
  QtyStats() :
    min(0.0),
    max(0.0),
    total(0.0),
    _M(0.0),
    _C(0.0),
    current_bucket(initial_bucket),
    items(0) {};

  double min;
  double max;
  double total;
  double _M = 0;
  double _C = 0;

  double current_bucket;
  map<double,size_t> buckets;
  size_t items;

  /**
   * Add a new value to the statistics,
   * updating min,max,total and items.
   * @param d The value to add.
   */
  void addValue( double d ) {
    //if ( ( items == 0 || d < min ) && d > 0.0 ) min = d;
    if ( ( items == 0 || d < min ) ) min = d;
    if ( items == 0 || d > max ) max = d;
    if ( items == 0 ) {
      _M = d;
      _C = 0.0;
      items++;      
    } else {
      items++;
      double delta = d - _M;
      _M += delta / items;
      _C += delta * (d - _M);
    }
    total += d;
    buckets[ bucket( d, current_bucket ) ]++;
    if ( buckets.size() > max_buckets ) reBucket();
  }

  void reBucket() {
    current_bucket *= 10.0;
    map<double,size_t> tmp;
    for ( const auto& old : buckets ) {
      tmp[ bucket( old.first, current_bucket ) ] += old.second;
    }
    buckets = tmp;
  }

  /**
   * Return the statistics as a formatted string.
   * @return the formatted string.
   */
  string asString( bool stddev = false ) {
    stringstream ss;
    ss << FIXED3W7 << min << " ";
    ss << FIXED3W7 << max << " ";
    ss << FIXED3W7 << getAverage() << " ";
    if ( stddev ) {
      ss << FIXED3W7 << getSigma() << " ";
    }
    return ss.str();
  }


  string consistency() {
    const double avg = getAverage();
    const double sdev = getSigma();
    // what is the relation between minimum=ideal and the average?
    double above_ideal_ratio = 1.0;
    if ( min > 0.0 ) above_ideal_ratio = (avg - min) / min;

    // what is the relation between the spread (max-min) and the standard deviation?
    double sdev_spread_ratio = 1.0;
    if ( (avg - min)  > 0.0 ) sdev_spread_ratio = sdev / (avg - min);

    double score = above_ideal_ratio * sdev_spread_ratio;

    stringstream ss;
    if ( score > 35.0 ) ss << "abysmal";
    else if ( score > 20.0 ) ss << "awful";
    else if ( score > 14.0 ) ss << "bad";
    else if ( score > 8.0 ) ss << "poor";
    else if ( score > 3.0 ) ss << "mediocre";
    else if ( score > 1.0 ) ss << "fair";
    else if ( score > 0.2 ) ss << "good";
    else if ( score > 0.05 ) ss << "excellent";
    else ss << "phenomenal";
    ss << fixed << setprecision(2);
    ss << " (" << above_ideal_ratio * sdev_spread_ratio << ")";

    return ss.str();
  }  

  double getAverage() const {
    return _M;
  }

  double getSigma() const {
    double sdev = 0;
    if ( items > 1 ) sdev = sqrt( _C / ( items - 1) );
    return sdev;
  }

};

#endif