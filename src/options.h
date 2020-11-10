#ifndef options_h
#define options_h

#include "waitclass.h"
#include "output.h"

#include <unistd.h>

/** The default time of day bucket. */
#define DEFAULT_DAY_BUCKET 60

/** The default slowness threshold. */
#define DEFAULT_SLOW_DURATION 1.0

/** The default maximum number of buckets in a histogram. */
#define DEFAULT_MAX_BUCKETS 20

/** Default threshold for displaying low contributing buckets. */
#define DEFAULT_HISTO_MIN_PCT 0.0

/** Do not attempt calculating output until this amount of probes are available. */
#define MINIMUM_PROBES 1

/** Default weekmap_bucket in minutes .*/
#define DEFAULT_WEEKMAP_BUCKET 30

/**
 * Options passed through command line.
 */
struct Options {

  enum class OutputFormat {
    Text,     /**< Output as plain text. */
    HTML      /**< Output as HTML. */
  };

  /**
   * Default constructor.
   */
  Options() : histo_max_buckets(DEFAULT_MAX_BUCKETS),
              day_bucket(DEFAULT_DAY_BUCKET),
              slow_threshold(DEFAULT_SLOW_DURATION),
              histo_min_pct(DEFAULT_HISTO_MIN_PCT),
              output_format(OutputFormat::Text),
              weekmap_bucket(DEFAULT_WEEKMAP_BUCKET),
              output_mode(omNone) {};

  /** Maximum number of buckets in a histogram. */
  unsigned   histo_max_buckets;

  /** The minute-bucket for to time-of-day histograms. */
  int        day_bucket;

  /** The threshold for slow responses. */
  double     slow_threshold;

  /** The threshold for displaying buckets with few probes. */
  double     histo_min_pct;

  /**< The output format. */
  OutputFormat output_format;

  /**< Time bucket for weekmap in minutes. */
  int weekmap_bucket;

  /** A bitmask of output modes. */
  OutputMode output_mode;

  /** 
   * Return true if the mode was turned on.
   * @param mode The OutputMode to check.
   * @return True if the mode was turned on.
   */
  bool hasMode( OutputMode mode ) const;

  /**
   * Return a description of the slow limit.
   * @return a description of the slow limit.
   */
  string slowString() const {
    stringstream ss;
    ss << ">" << FIXED3 << slow_threshold << "s";
    return ss.str();
  }

  static string formatAsString( const OutputFormat &format ) {
    switch ( format ) {
      case OutputFormat::Text:
        return "text";
      case OutputFormat::HTML:
        return "html";        
      default:
        return "invalid";
    }
  }

};

/**
 * Parsed command line options.
 */
extern Options options;

/**
 * Print command line help to standard out.
 */
void printHelp();

/**
 * Parse command line arguments.
 * @param argc The argument count.
 * @param argv The argument array.
 * @param options The options to fill.
 * @return False if the fommand line options are invalid.
 */
bool parseArgs( int argc, char* argv[], Options &options );

#endif