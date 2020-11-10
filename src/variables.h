#ifndef variables_h
#define variables_h

#include "globalstats.h"

#include "comments.h"
#include "curlprobe.h"
#include "datekey.h"
#include "timekey.h"

#include <list>

/**
 * Global (all probes) statistics.
 */
extern GlobalStats globalstats;

/**
 * Map slow probe statistics to a WaitClass.
 */
extern map<WaitClass,QtyStats> slow_map;

/**
 * Map slow probe statistics to day-of-week.
 */
extern map<int,ProbeStats> slow_dow_map;

/**
 * Map total probe statistics to day-of-week.
 */
extern map<int,ProbeStats> total_dow_map;

/**
 * Map slow probe statistics to time-of-day.
 */
extern map<TimeKey,ProbeStats> slow_day_map;

/**
 * Map total probe statistics to time-of-day.
 */
extern map<TimeKey,ProbeStats> total_day_map;

/**
 * Map probe stats to date (year,month,day)
 */
extern map<DateKey,ProbeStats> total_date_map;

/**
 * Map slow probe stats to date (year,month,day)
 */
extern map<DateKey,ProbeStats> slow_date_map;

/**
 * Map probe count to curl error code
 */
extern map<uint16_t,size_t> curl_error_map;

/**
 * Map probe count to http error code
 */
extern map<uint16_t,size_t> http_code_map;

/**
 * Maps day of week to a map of time of day the QtyStats.
 */
extern map<int,map<TimeKey,QtyStats>> weekmap_qtystats;

/**
 * To track Qos for weekmaps.
 */
struct QoS {
  size_t total = 0;
  size_t slow = 0;
  size_t curl_errors = 0;
  size_t http_errors = 0;
  double getQoS() const { return (1.0 - ( (double)slow + (double)curl_errors + (double)http_errors ) / (double)total) * 100.0; }
  double getSlowPct() const { return (double)slow / (double)total * 100.0; }
  double getHTTPErrorPct() const { return (double)http_errors / (double)total * 100.0; }
  double getProbeErrorPct() const { return (double)curl_errors / (double)total * 100.0; }
};

/**
 * Map http code count to date
 */
extern map<DateKey,QoS> qos_by_date;

/**
 * Track Qos for weekmap entries;
 */
extern map<int,map<TimeKey,QoS>> weekmap_probestats;

/**
 * Map slow probe count to WaitClass
 */
extern map<WaitClass,size_t> wait_class_map;

/**
 * All probes with curl errors
 */
extern list<CURLProbe> curl_error_list;

/**
 * All probes with http errors
 */
extern list<CURLProbe> http_error_list;

/**
 * All slow probes
 */
extern list<CURLProbe> slow_repsonse_list;

/**
 * Comments
 */
extern Comments comments;

/**
 * Recent trail
 */
extern list<CURLProbe> recent_probes;

#endif