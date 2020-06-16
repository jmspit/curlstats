#ifndef curlstats_h
#define curlstats_h

#include <string>
#include <bitset>

using namespace std;

#define FIXED3 fixed << setfill(' ') << setprecision(3)
#define FIXED3W7 fixed << setfill(' ') << setprecision(3) << setw(7)
#define FIXEDINT fixed << setfill(' ') << setprecision(0) << setw(9)
#define FIXEDPCT fixed << setfill(' ') << setprecision(2) << setw(6)

#define DEFAULT_DAY_BUCKET 30
#define DEFAULT_MIN_DURATION 1.0
#define DEFAULT_TIME_BUCKET 0.2
#define DEFAULT_TIMING_DETAIL false
#define DEFAULT_TIMING_DETAIL false
#define DEFAULT_HISTO_MIN_PCT 0.0

//#define MINIMUM_PROBES 10
#define MINIMUM_PROBES 1

enum OutputMode : unsigned long {
  omNone                 = 0b0000000000000000, /**< Output nothing */
  omAll                  = 0b0000000000000001, /**< Output everything 'all'*/
  omSlowTrail            = 0b0000000000000010, /**< Output a trail of slow probes 'slowtrail' */
  omHistograms           = 0b0000000000000100, /**< Output wait class histograms 'histo' */
  omDailyTrail           = 0b0000000000001000, /**< Output the daily trail 'daytrail'*/
  om24hMap               = 0b0000000000010000, /**< Output the 24h map '24hmap' */
  omWeekdayMap           = 0b0000000000100000, /**< Output the Weekday map 'wdmap'  */
  om24hSlowMap           = 0b0000000001000000, /**< Output the 24h slow map '24hslowmap' */
  omWeekdaySlowMap       = 0b0000000010000000, /**< Output the Weekday slow map 'wdslowmap' */
  omErrors               = 0b0000000100000000, /**< Output trail of errors 'errortrail' */
  omGlobal               = 0b0000001000000000, /**< Output global stats 'global' */
  omOptions              = 0b0000010000000000, /**< Output options 'options' */
  omComments             = 0b0000100000000000, /**< Output comments 'comments' */
  omSlowWaitClass        = 0b0001000000000000, /**< Output comments 'slowwait' */
};

inline OutputMode operator|( OutputMode a, OutputMode b ) {
  return static_cast<OutputMode>(static_cast<unsigned long>(a) | static_cast<unsigned long>(b));
}

inline OutputMode& operator|=( OutputMode &a, OutputMode b ) {
  a = static_cast<OutputMode>(static_cast<unsigned long>(a) | static_cast<unsigned long>(b));
  return a;
}

/**
 * Options passed through command line.
 */
struct Options {
  Options() : time_bucket(DEFAULT_TIME_BUCKET),
              day_bucket(DEFAULT_DAY_BUCKET),
              min_duration(DEFAULT_MIN_DURATION),
              histo_min_pct(DEFAULT_HISTO_MIN_PCT),
              output_mode(omNone) {};
  double     time_bucket;
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


string consistencyVerdict( double avg, double sdev, double relate ) {
  double ratio = 1000;
  if ( sdev > 0.0 ) ratio = 2.5 * avg / sdev;
  stringstream ss;
  if ( avg / relate  >= 0.01 ) {
    if ( ratio < 0.04 ) ss << "abysmal";
    else if ( ratio < 0.08 ) ss << "awful";
    else if ( ratio < 0.1 ) ss << "bad";
    else if ( ratio < 0.2 ) ss << "poor";
    else if ( ratio < 0.3 ) ss << "mediocre";
    else if ( ratio < 1.2 ) ss << "fair";
    else if ( ratio < 2.5 ) ss << "good";
    else if ( ratio < 10.0 ) ss << "excellent";
    else ss << "phenomenal";
  } else {
    ss << "n/a";
  }
  //ss << " (";
  //ss << FIXED3W7 << ratio;
  //ss << ")";
  return ss.str();
}

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

  string consistency( double relate ) {
    return consistencyVerdict( min, getSigma(), relate );
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
    return connect.getAverage() / 1.5;
  }

  int getTLSRoundTrips() const {
    return static_cast<int>( floor( appconnect.min / getNetworkRoundtrip() ) );
  }

  double getOptimalResponse() const {
    return namelookup.min +
           connect.min +
           appconnect.min +
           pretransfer.min +
           starttransfer.min +
           endtransfer.min;
  };

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

  /**
   * A list of findings.
   */
  list<string> findings;
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
      ss << waitClass2String( wcDNS ) << "=" << FIXED3 << getWaitClassDuration( wcDNS ) << "s, ";
      ss << waitClass2String( wcTCPHandshake ) << "=" << FIXED3 << getWaitClassDuration( wcTCPHandshake ) << "s, ";
      ss << waitClass2String( wcSSLHandshake ) << "=" << FIXED3 << getWaitClassDuration( wcSSLHandshake ) << "s, ";
      ss << waitClass2String( wcSendStart ) << "=" << FIXED3 << getWaitClassDuration( wcSendStart ) << "s, ";
      ss << waitClass2String( wcWaitEnd )  << "=" << FIXED3 << getWaitClassDuration( wcWaitEnd ) << "s, ";
      ss << waitClass2String( wcReceiveEnd ) << "=" << FIXED3 << getWaitClassDuration( wcReceiveEnd ) << "s";
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
  string mode = "";
  for(;;)
  {
    switch( getopt(argc, argv, "d:tb:T:p:o:h") ) // note the colon (:) to indicate that 'b' has a parameter and is not a switch
    {
      case 'b':
        options.time_bucket = stod( optarg );
        continue;
      case 'd':
        options.min_duration = stod( optarg );
        if ( options.min_duration == 0 ) options.min_duration = DEFAULT_MIN_DURATION;
        continue;
      case 'o':
        mode = optarg;
        if ( mode == "all" ) options.output_mode |= omAll;
        else if ( mode == "24hmap" ) options.output_mode |= om24hMap;
        else if ( mode == "24hslowmap" ) options.output_mode |= om24hSlowMap;
        else if ( mode == "comments" ) options.output_mode |= omComments;
        else if ( mode == "daytrail" ) options.output_mode |= omDailyTrail;
        else if ( mode == "errors" ) options.output_mode |= omErrors;
        else if ( mode == "global" ) options.output_mode |= omGlobal;
        else if ( mode == "histo" ) options.output_mode |= omHistograms;
        else if ( mode == "options" ) options.output_mode |= omOptions;
        else if ( mode == "slowtrail" ) options.output_mode |= omSlowTrail;
        else if ( mode == "slowwait" ) options.output_mode |= omSlowWaitClass;
        else if ( mode == "wdmap" ) options.output_mode |= omWeekdayMap;
        else if ( mode == "wdslowmap" ) options.output_mode |= omWeekdaySlowMap;
        else {
          cerr << "unknown mode '" << mode << "'" << endl;
          return false;
        }
        continue;
      case 'p':
        options.histo_min_pct = stod( optarg );
        if ( options.histo_min_pct > 10.0 ) options.histo_min_pct = DEFAULT_HISTO_MIN_PCT;
        continue;
      case 'T':
        options.day_bucket = stoi( optarg );
        if ( options.day_bucket <= 0 ) options.day_bucket = DEFAULT_DAY_BUCKET;
        if ( options.day_bucket > 60 ) options.day_bucket = DEFAULT_DAY_BUCKET;
        continue;
      case '?':
      case 'h':
      default :
        cout << "reads from standard input" << endl;
        cout << "see https://github.com/jmspit/curlstats" << endl;
        cout << endl;
        cout << "usage: " << endl;
        cout << "  -b seconds" << endl;
        cout << "     (real) response time histogram bucket in seconds (-o histo)" << endl;
        cout << "     default: " << DEFAULT_TIME_BUCKET << " seconds" << endl;
        cout << "  -d threshold" << endl;
        cout << "     (real) specify a slow threshold in seconds" << endl;
        cout << "     default: " << DEFAULT_MIN_DURATION << " seconds" << endl;
        cout << "  -o option" << endl;
        cout << "     limit the output, multiple options can be given by repeating -o" << endl;
        cout << "       24hmap     : show 24h map of all probes" << endl;
        cout << "       24hslowmap : show 24h map of slow probes" << endl;
        cout << "       comments   : show comments from input" << endl;
        cout << "       daytrail   : show daily history of all probes" << endl;
        cout << "       errors     : show errors" << endl;
        cout << "       global     : show global stats" << endl;
        cout << "       histo      : show wait class histograms" << endl;
        cout << "       options    : show options in effect" << endl;
        cout << "       slowtrail  : trail of slow probes" << endl;
        cout << "       slowwait   : show waits class distribution of slow probes" << endl;
        cout << "       wdmap      : show weekday map of all probes" << endl;
        cout << "       wdslowmap  : show weekday map of slow probes" << endl;
        cout << "     default: 'all'"<< endl;
        cout << "  -p threshold" << endl;
        cout << "     only show histogram buckets with % total probes larger than this value (-o histo)" << endl;
        cout << "     default: " << DEFAULT_HISTO_MIN_PCT << " %" <<endl;
        cout << "  -T minutes" << endl;
        cout << "     (uint) 24 hour time bucket in minutes ( 0 < x <= 60 ) (-o 24hmap, 24hslowmap)" << endl;
        cout << "     default: " << DEFAULT_DAY_BUCKET << " minutes" << endl;
        cout << endl;
        cout << waitClass2String( wcDNS, true )  << endl;
        cout << waitClass2String( wcTCPHandshake, true )  << endl;
        cout << waitClass2String( wcSSLHandshake, true )  << endl;
        cout << waitClass2String( wcSendStart, true )  << endl;
        cout << waitClass2String( wcWaitEnd, true )  << endl;
        cout << waitClass2String( wcReceiveEnd, true )  << endl;
        return false;
      case -1:
        break;
    }
    break;
  }
  if ( options.output_mode == omNone ) options.output_mode = omAll;
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
      ss << "OK";
      break;
    case 201 :
      ss << "Created";
      break;
    case 202 :
      ss << "Accepted";
      break;
    case 204 :
      ss << "No Content";
      break;

    case 301:
      ss << "Moved Permanently";
      break;
    case 302:
      ss << "Found / moved temporarily";
      break;
    case 304:
      ss << "Not modified";
      break;
    case 307:
      ss << "Temporary Redirect";
      break;
    case 308:
      ss << "Permanent Redirect";
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
    case 406:
      ss << "Not Acceptable";
      break;
    case 407:
      ss << "Proxy Authentication Required";
      break;
    case 408:
      ss << "Request Timeout";
      break;
    case 409:
      ss << "Conflict";
      break;
    case 410:
      ss << "Gone";
      break;

    case 500:
      ss << "Internal Server Error";
      break;
    case 501:
      ss << "Not Implemented";
      break;
    case 502:
      ss << "Bad Gateway";
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
    case 52:
      ss << "CURLE_GOT_NOTHING";
      break;
    case 55:
      ss << "CURLE_SEND_ERROR";
      break;
    case 56:
      ss << "CURLE_RECV_ERROR";
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
