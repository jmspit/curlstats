#include "qtystats.h"

#include <cmath>
#include <sstream>
#include "output.h"
#include "options.h"
#include "util.h"

QtyStats::QtyStats() :
  min(std::numeric_limits<double>::max() ),
  max(std::numeric_limits<double>::min() ),
  total(0.0),
  _M(0.0),
  _C(0.0),
  current_bucket(initial_bucket),
  items(0) {

  };

void QtyStats::addValue( double d ) {
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
  while ( buckets.size() > options.histo_max_buckets ) reBucket();
}

void QtyStats::reBucket() {
  current_bucket *= 10.0;
  map<double,size_t> tmp;
  for ( const auto& old : buckets ) {
    tmp[ bucket( old.first, current_bucket ) ] += old.second;
  }
  buckets = tmp;
}

string QtyStats::asString( bool stddev ) {
  stringstream ss;
  ss << FIXED3W7 << min << " ";
  ss << FIXED3W7 << max << " ";
  ss << FIXED3W7 << getAverage() << " ";
  if ( stddev ) {
    ss << FIXED3W7 << getSigma() << " ";
  }
  return ss.str();
}

string QtyStats::consistency() {
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
  if ( score > 112.0 ) ss << "abysmal";
  else if ( score > 56.0 ) ss << "awful";
  else if ( score > 28.0 ) ss << "bad";
  else if ( score > 16.0 ) ss << "poor";
  else if ( score > 6.0 ) ss << "mediocre";
  else if ( score > 2.0 ) ss << "fair";
  else if ( score > 0.5 ) ss << "good";
  else if ( score > 0.1 ) ss << "excellent";
  else ss << "phenomenal";
  ss << fixed << setprecision(2);
  ss << " (" << above_ideal_ratio * sdev_spread_ratio << ")";

  return ss.str();
}  

double QtyStats::getAverage() const {
  return _M;
}

double QtyStats::getSigma() const {
  double sdev = 0;
  if ( items > 1 ) sdev = sqrt( _C / ( items - 1) );
  return sdev;
}