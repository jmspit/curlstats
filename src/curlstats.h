#ifndef curlstats_h
#define curlstats_h

#include <string>

using namespace std;

#define FIXED3 fixed << setfill(' ') << setprecision(3)
#define FIXED3W7 fixed << setfill(' ') << setprecision(3) << setw(7)
#define FIXEDINT fixed << setfill(' ') << setprecision(0) << setw(9)
#define FIXEDPCT fixed << setfill(' ') << setprecision(2) << setw(6)

#define DEFAULT_DAY_BUCKET 30
#define DEFAULT_MIN_DURATION 1.0
#define DEFAULT_TIME_BUCKET 1.0
#define DEFAULT_TIMING_DETAIL false

/**
 * Options passed through command line.
 */
struct Options {
  Options() : timing_detail(DEFAULT_TIMING_DETAIL),
              time_bucket(DEFAULT_TIME_BUCKET),
              day_bucket(DEFAULT_DAY_BUCKET),
              min_duration(DEFAULT_MIN_DURATION) {};
  bool   timing_detail;
  double time_bucket;
  int    day_bucket;
  double min_duration;
};

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

  string stability( double relate ) {
    double ratio = 1000;
    //if ( getSigma() > 0.0 ) ratio = getAverage() / getSigma();
    if ( getSigma() > 0.0 ) ratio = min / getSigma();
    stringstream ss;
    if ( getAverage() / relate  >= 0.01 ) {
      if ( ratio < 0.005 ) ss << "abysmal";
      else if ( ratio < 0.05 ) ss << "awful";
      else if ( ratio < 0.1 ) ss << "bad";
      else if ( ratio < 0.3 ) ss << "poor";
      else if ( ratio < 0.7 ) ss << "mediocre";
      else if ( ratio < 1.5 ) ss << "fair";
      else if ( ratio < 3.0 ) ss << "good";
      else if ( ratio < 6.0 ) ss << "excellent";
      else ss << "phenomenal";
    } else {
      ss << "n/a";
    }
    ss << " (";
    ss << FIXED3W7 << ratio;
    ss << ")";
    return ss.str();
  }

  double getAverage() const {
    return total / (double)items;
  }

  double getSigma() const {
    double sdev = 0;
    if ( items ) sdev = sqrt( ( squared_total / (double)items - getAverage()*getAverage() ) );
    return sdev;
  }

};

/**
 * Enumerate the roundtrip wait classes (steps).
 */
enum WaitClass {
  wcDNS,            /**< The target name needs be resolved to an IP address. */
  wcTCPHandshake,   /**< The TCP layer needs to handshake. */
  wcSSLHandshake,   /**< The TLS layer needs to handshake. */
  wcSendStart,      /**< The client may need time to prepare the data before sending. */
  wcWaitEnd,        /**< The client waited on the first response packet from the peer. */
  wcReceiveEnd      /**< The client spend time retrieving additional network packets. */
};

/**
 * Structure to order say std::set<Order> WaitClasses so WiatClass with highest
 * value can easily be retrieved.
 */
struct Order {
  WaitClass wc;
  double value;
};

/**
 * Ordering for the Order struct.
 */
bool operator<( const Order& o1, const Order& o2 ) {
  return o1.value < o2.value;
}

/**
 * To aggregate timeing data for each WaitClass.
 * The caller should take care to call addValue  for each wait class, for each statistics round.
 * If addValue is called with wcReceiveEnd, and after that the number of items for the different
 * WaitClasses are not equal,an exception is thrown.
 */
struct WaitClassStats {
  QtyStats namelookup;
  QtyStats connect;
  QtyStats appconnect;
  QtyStats pretransfer;
  QtyStats starttransfer;
  QtyStats endtransfer;

  /**
   * Add a value for the WaitClass.
   */
  void addValue( WaitClass wc, double value ) {
    switch ( wc ) {
      case wcDNS:
        namelookup.addValue( value );
        break;
      case wcTCPHandshake:
        connect.addValue( value );
        break;
      case wcSSLHandshake:
        appconnect.addValue( value );
        break;
      case wcSendStart:
        pretransfer.addValue( value );
        break;
      case wcWaitEnd:
        starttransfer.addValue( value );
        break;
      case wcReceiveEnd:
        endtransfer.addValue( value );
        size_t compare = endtransfer.items;
        if ( namelookup.items  != compare ||
             connect.items     != compare ||
             appconnect.items  != compare ||
             pretransfer.items != compare ||
             starttransfer.items != compare ) throw std::runtime_error( "WaitClassStats - item count inconsistency" );
        break;
    }
  }

  double avgResponse() const {
    if ( endtransfer.items > 0 ) return ( namelookup.total +
                                          connect.total +
                                          appconnect.total +
                                          pretransfer.total +
                                          starttransfer.total +
                                          endtransfer.total ) / (double)endtransfer.items;
    else return 0.0;
  }

  size_t getNumItems() const { return endtransfer.items; }

  WaitClass blame() const {
    set<Order> ordered;
    ordered.insert( { wcDNS , namelookup.total } );
    ordered.insert( { wcTCPHandshake , connect.total } );
    ordered.insert( { wcSSLHandshake , appconnect.total } );
    ordered.insert( { wcSendStart , pretransfer.total } );
    ordered.insert( { wcWaitEnd , starttransfer.total } );
    ordered.insert( { wcReceiveEnd , endtransfer.total } );
    return (*ordered.rbegin()).wc;
  }

  double getNetworkRoundtrip() const {
    return connect.min / 1.5;
  }

  int getTLSRoundTrips() const {
    return static_cast<int>( floor( appconnect.min / getNetworkRoundtrip() ) );
  }

};

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
    first_time(0),
    last_time(0) {};

  /**
   * Global WaitClassStats
   */
  WaitClassStats wait_class_stats;

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
  time_t first_time;

  /**
   * The time of the last probe seen.
   */
  time_t last_time;
};

/**
 * Key to bucket repeating daily time-od-day ranges.
 */
struct TimeKey {
  TimeKey() : tm_hour(0),tm_min(0) {};
  TimeKey( int hour, int min ) : tm_hour(hour),tm_min(min) {};
  int tm_hour;
  int tm_min;
};

/**
 * Equality on TimeKey.
 */
bool operator==( const TimeKey& k1, const TimeKey &k2 ) {
  return k1.tm_hour == k2.tm_hour && k1.tm_min == k2.tm_min;
}

/**
 * Ordering on TimeKey.
 */
bool operator<( const TimeKey& k1, const TimeKey &k2 ) {
  return k1.tm_hour < k2.tm_hour || ( k1.tm_hour == k2.tm_hour && k1.tm_min < k2.tm_min );
}

struct DateKey {
  DateKey() : tm_year(1970),tm_mon(0),tm_mday(1) {}
  DateKey( int year, int month, int day ) : tm_year(year),tm_mon(month),tm_mday(day) {};
  int tm_year;
  int tm_mon;
  int tm_mday;
};

/**
 * Equality on DateKey.
 */
bool operator==( const DateKey& k1, const DateKey &k2 ) {
  return k1.tm_year == k2.tm_year && k1.tm_mon == k2.tm_mon && k1.tm_mday == k2.tm_mday;
}

/**
 * Ordering on DateKey.
 */
bool operator<( const DateKey& k1, const DateKey &k2 ) {
  return k1.tm_year < k2.tm_year || ( k1.tm_year == k2.tm_year && k1.tm_mon < k2.tm_mon ) ||
          ( k1.tm_year == k2.tm_year && k1.tm_mon == k2.tm_mon && k1.tm_mday < k2.tm_mday );
}

/**
 * Bucket a TimeKey.
 */
TimeKey bucket( const TimeKey &k, int bucket ) {
  return TimeKey( k.tm_hour, k.tm_min / bucket * bucket );
}

/**
 * Split a string.
 */
vector<string> split( const string& src, char delimiter = ';' ) {
  vector<string> result;
  istringstream is(src);
  string s;
  while ( getline( is, s, delimiter ) ) {
    result.push_back( s );
  }
  return result;
}

/**
 * Bucket a real value to its ceil (largest integer above).
 */
double bucket( double v, double bucket ) {
  return ceil( v / bucket ) * bucket;
}

/**
 * Convert a WaitClass to a acronym (describe==false) or a full description (describe==true).
 */
string waitClass2String( WaitClass wc, bool describe = false ) {
  if ( describe ) {
    switch ( wc ) {
      case wcDNS :          return waitClass2String(wc) + " = DNS name resolution";
      case wcTCPHandshake : return waitClass2String(wc) + " = TCP handshake";
      case wcSSLHandshake : return waitClass2String(wc) + " = TLS ('SSL') handshake";
      case wcSendStart :    return waitClass2String(wc) + " = Request send lead time";
      case wcWaitEnd :      return waitClass2String(wc) + " = Waiting for response";
      case wcReceiveEnd :   return waitClass2String(wc) + " = Waiting for response more data";
      default:              return "ERR";
    };
  } else {
    switch ( wc ) {
      case wcDNS :          return "DNS";
      case wcTCPHandshake : return "TCP";
      case wcSSLHandshake : return "TLS";
      case wcSendStart :    return "REQ";
      case wcWaitEnd :      return "RSP";
      case wcReceiveEnd :   return "DAT";
      default:              return "ERR";
    };
  }
}

/**
 * Curl statistics line.
 */
struct CURL {
  struct tm datetime;
  uint16_t  curl_error;
  uint16_t  http_code;
  double    total_time;
  double    time_namelookup;
  double    time_connect;
  double    time_appconnect;
  double    time_pretransfer;
  double    time_starttransfer;

  /**
   * Calculate the duration of a WaitClass.
   */
  double getWaitClassDuration( WaitClass wc ) {
    switch ( wc ) {
      case wcDNS :
        return time_namelookup;
      case wcTCPHandshake :
        return (time_connect-time_namelookup);
      case wcSSLHandshake :
        if ( time_appconnect > 0 )
          return (time_appconnect-time_connect);
        else return 0.0;
      case wcSendStart :
        if ( time_appconnect > 0 )
          return (time_pretransfer-time_appconnect);
        else
          return (time_pretransfer-time_connect);
      case wcWaitEnd :
        return (time_starttransfer-time_pretransfer);
      case wcReceiveEnd :
        return (total_time-time_starttransfer);
    }
    return 0.0;
  };

  double getWaitClassPct( WaitClass wc ) {
    return getWaitClassDuration( wc ) / total_time*100.0;
  };


  WaitClass getDominantWaitClass() {
    set<Order> ordered;
    ordered.insert( { wcDNS , getWaitClassPct( wcDNS ) } );
    ordered.insert( { wcTCPHandshake , getWaitClassPct( wcTCPHandshake ) } );
    ordered.insert( { wcSSLHandshake , getWaitClassPct( wcSSLHandshake ) } );
    ordered.insert( { wcSendStart , getWaitClassPct( wcSendStart ) } );
    ordered.insert( { wcWaitEnd , getWaitClassPct( wcWaitEnd ) } );
    ordered.insert( { wcReceiveEnd , getWaitClassPct( wcReceiveEnd ) } );
    return (*ordered.rbegin()).wc;
  };

  string asString() {
    stringstream ss;
    ss << put_time( &datetime, "%F %T") << " ";
    ss << "curl=" << curl_error << " ";
    if ( curl_error == 0 ) {
      ss << "http=" << http_code << " ";
      ss << "roundtrip : " << FIXED3 << total_time << "s blame " << waitClass2String( getDominantWaitClass() ) << " ";
      ss << fixed << FIXEDPCT << getWaitClassPct( getDominantWaitClass() ) << "% | ";
      ss << waitClass2String( wcDNS ) << "=" << FIXED3 << time_namelookup << "s, ";
      ss << waitClass2String( wcTCPHandshake ) << "=" << FIXED3 << time_connect - time_namelookup << "s, ";
      ss << waitClass2String( wcSSLHandshake ) << "=" << FIXED3 << time_appconnect - time_connect << "s, ";
      ss << waitClass2String( wcSendStart ) << "=" << FIXED3 << time_pretransfer - time_appconnect << "s, ";
      ss << waitClass2String( wcWaitEnd )  << "=" << FIXED3 << time_starttransfer - time_pretransfer << "s, ";
      ss << waitClass2String( wcReceiveEnd ) << "=" << FIXED3 << total_time - time_starttransfer << "s";
    }
    return ss.str();
  };

  bool parse( const string &s ) {
    vector<string> tokens = split( s, ';' );
    if ( tokens.size() == 12 ) {
      string stm = tokens[0] + " +00:00";
      memset( &datetime, 0, sizeof(datetime) );
      strptime( stm.c_str(), "%Y-%m-%d %H:%M:%S %z", &datetime );

      curl_error         = stoi(tokens[1]);
      http_code          = stoi(tokens[3]);
      total_time         = stod(tokens[5]);
      time_namelookup    = stod(tokens[6]);
      time_connect       = stod(tokens[7]);
      time_appconnect    = stod(tokens[8]);
      time_pretransfer   = stod(tokens[9]);
      time_starttransfer = stod(tokens[11]);
    } else return false;
    return true;
  }
};

/**
 * Parse command line arguments.
 */
bool parseArgs( int argc, char* argv[], Options &options ) {
  for(;;)
  {
    switch( getopt(argc, argv, "d:tb:T:h") ) // note the colon (:) to indicate that 'b' has a parameter and is not a switch
    {
      case 'b':
        options.time_bucket = stod( optarg );
        continue;
      case 'd':
        options.min_duration = stod( optarg );
        if ( options.min_duration == 0 ) options.min_duration = DEFAULT_MIN_DURATION;
        continue;
      case 't':
        options.timing_detail = true;
        continue;
      case 'T':
        options.day_bucket = stoi( optarg );
        if ( options.day_bucket <= 0 ) options.day_bucket = DEFAULT_DAY_BUCKET;
        if ( options.day_bucket > 60 ) options.day_bucket = DEFAULT_DAY_BUCKET;
        continue;
      case '?':
      case 'h':
      default :
        cout << "usage: " << endl;
        cout << "  -b seconds" << endl;
        cout << "     (real) 24h time distribution bucket" << endl;
        cout << "  -d minimum" << endl;
        cout << "     (real) specify a slow threshold filter in seconds" << endl;
        cout << "  -t" << endl;
        cout << "     include a full list of slow probes" << endl;
        cout << "  -T minutes" << endl;
        cout << "     (uint) 24 hour time bucket in minutes ( 0 < x <= 60 )" << endl;
        return false;
      case -1:
        break;
    }
    break;
  }
  return true;
}

/**
 * return true if the line is a comment.
 */
bool isCommment( const string& s ) {
  return s.length() == 0 || s[0] == '#';
}

/**
 * Convert a time_t to a string.
 */
string time_t2String( time_t t ) {
  tm tm = *std::localtime(&t);
  std::stringstream ss;
  ss << put_time( &tm, "%F %T");
  return ss.str();
}

/**
 * Write a heading.
 */
void heading( const string &h, char c = '=', int width = 176 ) {
  cout << endl << string( width, c ) << endl;
  cout << h << endl;
  cout << string( width, c ) << endl;
}

/**
 * convert a HTTP error to a string description.
 */
string HTTPError2String( uint16_t code ) {
  stringstream ss;
  switch ( code ) {
    case 200 :
      ss << "Ok";
      break;
    case 301:
      ss << "Moved Permanently";
      break;
    case 302:
      ss << "Found / moved temporarily";
      break;
    case 400:
      ss << "Bad request";
      break;
    case 401:
      ss << "Unauthorized";
      break;
    case 402:
      ss << "Payment Required";
      break;
    case 403:
      ss << "Forbidden";
      break;
    case 404:
      ss << "Not found";
      break;
    case 405:
      ss << "Method Not Allowed";
      break;
    case 503:
      ss << "Service unavailable";
      break;
    case 504:
      ss << "Gateway timeout";
      break;
  }
  if ( ss.str().size() ) ss << " ";
  ss << "(" << code << ")";
  return ss.str();
};

/**
 * convert a curl error to a string description.
 */
string curlError2String( uint16_t code ) {
  stringstream ss;
  switch ( code ) {
    case 0:
      ss << "CURLE_OK";
      break;
    case 5:
      ss << "CURLE_COULDNT_RESOLVE_PROXY";
      break;
    case 6:
      ss << "CURLE_COULDNT_RESOLVE_HOST";
      break;
    case 7:
      ss << "CURLE_COULDNT_CONNECT";
      break;
    case 8:
      ss << "CURLE_WEIRD_SERVER_REPLY";
      break;
    case 28:
      ss << "CURLE_OPERATION_TIMEDOUT";
      break;
    case 35:
      ss << "CURLE_SSL_CONNECT_ERROR";
      break;
  }
  if ( ss.str().length() > 0 ) ss << " ";
  ss << "(" << code << ")";
  return ss.str();
}

/**
 * convert a dow-of-week to a string description.
 */
string dowStr( int dow ) {
  switch( dow ) {
    case 0 : return "Sunday";
    case 1 : return "Monday";
    case 2 : return "Tuesday";
    case 3 : return "Wednesday";
    case 4 : return "Thursday";
    case 5 : return "Friday";
    case 6 : return "Saturday";
    default : return "ErrorDay";
  }
};

#endif
