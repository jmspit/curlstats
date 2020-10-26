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

  /**
   * Construct and init defaults.
   */
  QtyStats() :
    min(0.0),
    max(0.0),
    total(0.0),
    squared_total(0.0),
    items(0) {};

  double min;
  double max;
  double total;
  double squared_total;
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
    total += d;
    squared_total += d*d;
    items++;
  }

  /**
   * Return the statistics as a formatted string.
   * @return the formatted string.
   */
  string asString( bool stddev = false ) {
    stringstream ss;
    ss << FIXED3W7 << min << " ";
    ss << FIXED3W7 << max << " ";
    double average = total / (double)items;
    ss << FIXED3W7 << average << " ";
    if ( stddev ) {
      double sdev = 0;
      if ( items ) sdev = sqrt( ( squared_total / (double)items - average*average ) );
      ss << FIXED3W7 << sdev << " ";
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
    return total / (double)items;
  }

  double getSigma() const {
    double sdev = 0;
    if ( items ) sdev = sqrt( ( squared_total / (double)items ) - getAverage()*getAverage() );
    return sdev;
  }

};

#endif