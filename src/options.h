#ifndef options_h
#define options_h

#include "waitclass.h"
#include "output.h"

#include <unistd.h>

#define DEFAULT_DAY_BUCKET 60
#define DEFAULT_MIN_DURATION 1.0
#define DEFAULT_MAX_BUCKETS 20
#define DEFAULT_TIMING_DETAIL false
#define DEFAULT_TIMING_DETAIL false
#define DEFAULT_HISTO_MIN_PCT 0.0

//#define MINIMUM_PROBES 10
#define MINIMUM_PROBES 1

/**
 * Options passed through command line.
 */
struct Options {
  Options() : histo_max_buckets(DEFAULT_MAX_BUCKETS),
              day_bucket(DEFAULT_DAY_BUCKET),
              min_duration(DEFAULT_MIN_DURATION),
              histo_min_pct(DEFAULT_HISTO_MIN_PCT),
              output_mode(omNone) {};
  unsigned   histo_max_buckets;
  int        day_bucket;
  double     min_duration;
  double     histo_min_pct;
  OutputMode output_mode;

  bool hasMode( OutputMode mode ) {
    unsigned long t = static_cast<unsigned long>(output_mode);
    unsigned long m = static_cast<unsigned long>(mode);
    return ((t & m) || (t & omAll));
  }

  string slowString() const {
    stringstream ss;
    ss << ">" << FIXED3 << min_duration << "s";
    return ss.str();
  }
};

/**
 * Parsed command line options.
 */
extern Options options;

void printHelp();

/**
 * Parse command line arguments.
 */
bool parseArgs( int argc, char* argv[], Options &options );

#endif