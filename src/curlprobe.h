#ifndef curlprobe_h
#define curlprobe_h

#include "datetime.h"
#include "waitclass.h"

/**
 * Curl probe line.
 */
struct CURLProbe {
  
  /** DateTime of the probe */
  DateTime  datetime;

  /** CURL error code of the probe */
  uint16_t  curl_error;

  /** HTTP return code of the probe */
  uint16_t  http_code;

  /** Total time */
  double    total_time;

  /** DNS resolving time */
  double    time_namelookup;

  /** TCP handshake time */
  double    time_connect;

  /** TLS handshake time */
  double    time_appconnect;

  /** local pre-send time */
  double    time_pretransfer;

  /** first server response */
  double    time_starttransfer;

  /** bytes sent */
  size_t    size_upload;

  /** bytes received */
  size_t    size_download;

  /**
   * Calculate the duration of a WaitClass.
   * @param wc The WaitClass to get the duration for.
   * @return The duration.
   */
  double getWaitClassDuration( WaitClass wc ) const;

  /**
   * Calculate the contribution of this WaitClass to total_time.
   * @param wc The WaitClass to get the percentage for.
   * @return The duration.
   */
  double getWaitClassPct( WaitClass wc ) const;

  /**
   * Return the WaitClass that contributes most to total_time.
   * @return The dominant WaitCLass.
   */
  WaitClass getDominantWaitClass() const;

  /**
   * Return the probe line as a human-readable string.
   * @return The string representation.
   */
  string asString() const;

  /**
   * Parse a curl probe line.
   * @param line The line to parse.
   * @return True if the parse succeeeded.
   */
  bool parse( const string &line );

};

#endif