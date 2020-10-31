#ifndef curlprobe_h
#define curlprobe_h

#include "datetime.h"
#include "waitclass.h"

/**
 * Curl probe line.
 */
struct CURLProbe {
  DateTime  datetime;
  uint16_t  curl_error;
  uint16_t  http_code;
  double    total_time;
  double    time_namelookup;
  double    time_connect;
  double    time_appconnect;
  double    time_pretransfer;
  double    time_starttransfer;
  size_t    size_upload;
  size_t    size_download;

  /**
   * Calculate the duration of a WaitClass.
   */
  double getWaitClassDuration( WaitClass wc ) const;

  double getWaitClassPct( WaitClass wc ) const;

  WaitClass getDominantWaitClass() const;

  string asString() const;

  bool parse( const string &s );

};

#endif