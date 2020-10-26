#ifndef globalstats_h
#define globalstats_h

#include "datetime.h"
#include "qtystats.h"
#include "waitclass.h"

/**
 * Global statistics.
 */
struct GlobalStats {

  /**
   * Construct with defaults.
   */
  GlobalStats() :
    items(0),
    items_slow(0),
    total_time(0),
    total_slow_time(0),
    first_time(),
    last_time() {};

  /**
   * Global WaitClassStats
   */
  WaitClassStats wait_class_stats;

  /**
   * total response statistics (full request-response cycle)
   */
  QtyStats response_stats;

  /**
   * The number of items (probes).
   */
  size_t items;

  /**
   * The number of slow items (probes).
   */
  size_t items_slow;

  /**
   * The total (summed) round trip time.
   */
  double total_time;

  /**
   * The total (summed) slow round trip time.
   */
  double total_slow_time;

  /**
   * The time of the first probe seen.
   */
  DateTime first_time;

  /**
   * The time of the last probe seen.
   */
  DateTime last_time;

  /**
   * A list of findings.
   */
  list<string> findings;

  /**
   * Aggregate upload size
   */
  size_t size_upload;

  /**
   * Aggregate download size
   */
  size_t size_download;
};


#endif