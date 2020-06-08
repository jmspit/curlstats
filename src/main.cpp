#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <ctime>
#include <unistd.h>
#include <vector>
#include <set>
#include <list>
#include <string.h>
#include <cmath>

using namespace std;

#define FIXED3 fixed << setfill(' ') << setprecision(3)
#define FIXED3W7 fixed << setfill(' ') << setprecision(3) << setw(7)
#define FIXEDINT fixed << setfill(' ') << setprecision(0) << setw(9)
#define FIXEDPCT fixed << setfill(' ') << setprecision(2) << setw(6)

#define DEFAULT_DAY_BUCKET 30
#define DEFAULT_MIN_DURATION 1.0
#define DEFAULT_TIME_BUCKET 1.0
#define DEFAULT_TIMING_DETAIL false

struct Options {
  Options() : timing_detail(DEFAULT_TIMING_DETAIL),
              time_bucket(DEFAULT_TIME_BUCKET),
              day_bucket(DEFAULT_DAY_BUCKET) {};
  bool timing_detail;
  double time_bucket;
  int day_bucket;
};
Options options;

struct QtyStats {
  QtyStats() :
    min(0.0),
    max(0.0),
    total(0.0),
    items(0) {};

  double min;
  double max;
  double total;
  size_t items;

  void addValue( double d ) {
    if ( min == 0.0 || d < min ) min = d;
    if ( max == 0.0 || d > max ) max = d;
    total += d;
    items++;
  }

  string asString() {
    stringstream ss;
    ss << FIXED3W7 << min << " ";
    ss << FIXED3W7 << max << " ";
    ss << FIXED3W7 << total / (double)items << " ";
    return ss.str();
  }
};

enum WaitClass {
  wcDNS,
  wcTCPHandshake,
  wcSSLHandshake,
  wcSendStart,
  wcWaitEnd,
  wcReceiveEnd
};

struct Order {
  WaitClass wc;
  double value;
};

bool operator<( const Order& o1, const Order& o2 ) {
  return o1.value < o2.value;
}

struct WaitClassStats {
  QtyStats namelookup;
  QtyStats connect;
  QtyStats appconnect;
  QtyStats pretransfer;
  QtyStats starttransfer;
  QtyStats endtransfer;
  size_t slow = 0;

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
        break;
    }
  }

  void bumpSlow() { slow++; };

  double avgResponse() const {
    return
      ( namelookup.total +
        connect.total +
        appconnect.total +
        pretransfer.total +
        starttransfer.total +
        endtransfer.total ) / (double)slow;
  }

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

};

struct GlobalStats {
  GlobalStats() :
    items(0),
    items_slow(0),
    total_time(0),
    total_slow_time(0),
    first_time(0),
    last_time(0) {};

  WaitClassStats wait_class_stats;
  size_t items;
  size_t items_slow;
  double total_time;
  double total_slow_time;

  time_t first_time;
  time_t last_time;
};
GlobalStats globalstats;

vector<string> split( const string& src, char delimiter = ';' ) {
  vector<string> result;
  istringstream is(src);
  string s;
  while ( getline( is, s, delimiter ) ) {
    result.push_back( s );
  }
  return result;
}



struct TimeKey {
  TimeKey() : tm_hour(0),tm_min(0) {};
  TimeKey( int hour, int min ) : tm_hour(hour),tm_min(min) {};
  int tm_hour;
  int tm_min;
};

bool operator==( const TimeKey& k1, const TimeKey &k2 ) {
  return k1.tm_hour == k2.tm_hour && k1.tm_min == k2.tm_min;
}

bool operator<( const TimeKey& k1, const TimeKey &k2 ) {
  return k1.tm_hour < k2.tm_hour || ( k1.tm_hour == k2.tm_hour && k1.tm_min < k2.tm_min );
}

TimeKey bucket( const TimeKey &k ) {
  return TimeKey( k.tm_hour, k.tm_min / options.day_bucket * options.day_bucket );
}

double bucket( double v ) {
  return ceil( v / options.time_bucket ) * options.time_bucket;
}

map<WaitClass,QtyStats> slow_map;
map<int,size_t> slow_dow_map;
map<int,size_t> total_dow_map;
map<TimeKey,WaitClassStats> slow_day_map;
map<TimeKey,size_t> total_day_map;
map<double,size_t> slow_buckets;
map<double,size_t> all_buckets;

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

  double getWaitClassDuration( WaitClass wc ) {
    switch ( wc ) {
      case wcDNS :
        return time_namelookup;
      case wcTCPHandshake :
        return (time_connect-time_namelookup);
      case wcSSLHandshake :
        return (time_appconnect-time_connect);
      case wcSendStart :
        return (time_pretransfer-time_appconnect);
      case wcWaitEnd :
        return (time_starttransfer-time_pretransfer);
      case wcReceiveEnd :
        return (total_time-time_starttransfer);
    }
    return 0.0;
  };

  double getWaitClassPct( WaitClass wc ) {
    switch ( wc ) {
      case wcDNS :
        return time_namelookup/total_time*100.0;
      case wcTCPHandshake :
        return (time_connect-time_namelookup)/total_time*100.0;
      case wcSSLHandshake :
        return (time_appconnect-time_connect)/total_time*100.0;
      case wcSendStart :
        return (time_pretransfer-time_appconnect)/total_time*100.0;
      case wcWaitEnd :
        return (time_starttransfer-time_pretransfer)/total_time*100.0;
      case wcReceiveEnd :
        return (total_time-time_starttransfer)/total_time*100.0;
    }
    return 0.0;
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

struct Filter {
  Filter() : min_duration(DEFAULT_MIN_DURATION) {}
  double min_duration;
};
Filter filter;

bool parseArgs( int argc, char* argv[] ) {
  for(;;)
  {
    switch( getopt(argc, argv, "d:tb:T:") ) // note the colon (:) to indicate that 'b' has a parameter and is not a switch
    {
      case 'b':
        options.time_bucket = stod( optarg );
        continue;
      case 'd':
        filter.min_duration = stod( optarg );
        if ( filter.min_duration == 0 ) filter.min_duration = DEFAULT_MIN_DURATION;
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
        cout << "     (uint) 24 hour time bucket ( 0 < x <= 60 )" << endl;
        return false;
      case -1:
        break;
    }
    break;
  }
  return true;
}

bool isCommment( const string& s ) {
  return s.length() == 0 || s[0] == '#';
}


map<uint16_t,size_t> curl_error_map;
map<uint16_t,size_t> http_code_map;
map<WaitClass,size_t> wait_class_map;

list<CURL> curl_error_list;
list<CURL> http_error_list;
list<CURL> slow_repsonse_list;

void read( std::istream& in ) {
  string line;
  getline( in, line );
  while ( in.good()  ) {
    if ( !isCommment( line ) ) {
      CURL curl;
      if ( curl.parse( line ) ) {
        curl_error_map[curl.curl_error]++;
        if ( curl.curl_error == 0 ) {
          http_code_map[curl.http_code]++;
          if ( curl.http_code >= 400 ) http_error_list.push_back( curl );
          TimeKey tkey = TimeKey( curl.datetime.tm_hour, curl.datetime.tm_min );
          if ( curl.total_time >= filter.min_duration ) {
            slow_map[curl.getDominantWaitClass()].addValue( curl.getWaitClassDuration( curl.getDominantWaitClass() ) );
            wait_class_map[curl.getDominantWaitClass()]++;
            if ( options.timing_detail ) slow_repsonse_list.push_back( curl );
            globalstats.items_slow++;
            globalstats.total_slow_time += curl.total_time;
            slow_dow_map[curl.datetime.tm_wday]++;
            slow_day_map[bucket(tkey)].addValue( wcDNS, curl.time_namelookup );
            slow_day_map[bucket(tkey)].addValue( wcTCPHandshake, curl.time_connect - curl.time_namelookup );
            slow_day_map[bucket(tkey)].addValue( wcSSLHandshake, curl.time_appconnect - curl.time_connect );
            slow_day_map[bucket(tkey)].addValue( wcSendStart, curl.time_pretransfer - curl.time_appconnect );
            slow_day_map[bucket(tkey)].addValue( wcWaitEnd, curl.time_starttransfer - curl.time_pretransfer );
            slow_day_map[bucket(tkey)].addValue( wcReceiveEnd, curl.total_time - curl.time_starttransfer );
            slow_day_map[bucket(tkey)].bumpSlow();
            slow_buckets[ bucket( curl.total_time ) ]++;
          }
          globalstats.total_time += curl.total_time;
          globalstats.wait_class_stats.namelookup.addValue( curl.time_namelookup );
          globalstats.wait_class_stats.connect.addValue( curl.time_connect - curl.time_namelookup );
          globalstats.wait_class_stats.appconnect.addValue( curl.time_appconnect - curl.time_connect );
          globalstats.wait_class_stats.pretransfer.addValue( curl.time_pretransfer - curl.time_appconnect );
          globalstats.wait_class_stats.starttransfer.addValue( curl.time_starttransfer - curl.time_pretransfer );
          globalstats.wait_class_stats.endtransfer.addValue( curl.total_time - curl.time_starttransfer );
          if ( globalstats.first_time == 0 || timelocal(&curl.datetime ) < globalstats.first_time )
            globalstats.first_time = timelocal(&curl.datetime );
          if ( globalstats.last_time == 0 || timelocal(&curl.datetime ) > globalstats.last_time )
            globalstats.last_time = timelocal(&curl.datetime );
          all_buckets[ bucket( curl.total_time ) ]++;
          globalstats.items++;
          total_day_map[bucket(tkey)]++;
          total_dow_map[curl.datetime.tm_wday]++;
        } else {
          curl_error_list.push_back( curl );
        }
      } else {
        cerr << "error on line " << globalstats.items << endl;
      }
    } else {
      if ( line.length() > 1 && line[1] != 'Y' ) cout << line << endl;
    }
    getline( in, line );
  }
}

string time_t2String( time_t t ) {
  tm tm = *std::localtime(&t);
  std::stringstream ss;
  ss << put_time( &tm, "%F %T");
  return ss.str();
}

void heading( const string &h ) {
  cout << endl << h << endl;
  cout << string( 170, '-' ) << endl;
}

string HTTPError2String( uint16_t code ) {
  stringstream ss;
  switch ( code ) {
    case 200 :
      ss << "Ok ";
      break;
    case 302:
      ss << "Found / moved temporarily ";
      break;
    case 404:
      ss << "Not found ";
      break;
    case 503:
      ss << "Service unavailable ";
      break;
    case 504:
      ss << "Gateway timeout ";
      break;
  }
  ss << "(" << code << ")";
  return ss.str();
};

string curlError2String( uint16_t code ) {
  stringstream ss;
  switch ( code ) {
    case 0:
      ss << "Ok ";
      break;
    case 35:
      ss << "SSL connect error ";
      break;
  }
  ss << "(" << code << ")";
  return ss.str();
}

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

void summary() {
  heading( "Wait class acronyms" );
  cout << waitClass2String( wcDNS, true )  << endl;
  cout << waitClass2String( wcTCPHandshake, true )  << endl;
  cout << waitClass2String( wcSSLHandshake, true )  << endl;
  cout << waitClass2String( wcSendStart, true )  << endl;
  cout << waitClass2String( wcWaitEnd, true )  << endl;
  cout << waitClass2String( wcReceiveEnd, true )  << endl;

  heading( "Options in effect" );
  cout << "Response time distribution bucket : " << FIXED3W7 << options.time_bucket << " seconds" << endl;
  cout << "repeating 24h bucket              : " << FIXED3W7 << options.day_bucket << " minutes" << endl;
  cout << "Show trail of slow probes         : " << FIXED3W7 << options.timing_detail << endl;

  if ( options.timing_detail ) {
    heading( "List of slow probes" );
    for ( auto l : slow_repsonse_list ) {
      cout << l.asString() << endl;
    }
  }

  heading( "QoS" );
  cout << FIXED3 << 100.0 - (double)globalstats.items_slow / (double)globalstats.items * 100.0 << "% ";
  cout << "of probes return within " << filter.min_duration << "s" << endl;

  cout << endl << "probe count to response time distribution, bucket size " << options.time_bucket << "s" << endl;
  cout << setw(9) << "bucket" << " " << setw(8) << "count" << setw(8) << "%probe" << endl;
  for ( auto b : all_buckets ) {
    cout << "<" << FIXED3W7 << b.first;
    cout << "s " << FIXEDINT << b.second;
    cout << " " << FIXEDPCT << b.second/(double)globalstats.items*100 << endl;
  }

  if ( globalstats.items_slow > 0 ) {

    heading( "Slow probe to wait-class distribution" );
    cout << setw(5) << "class";
    cout << setw(10) << "#probes";
    cout << setw(8) << "%blame";
    cout << setw(8) << "min";
    cout << setw(8) << "max";
    cout << setw(8) << "avg";
    cout << endl;
    for ( auto w : wait_class_map ) {
      cout << "  " << waitClass2String( w.first ) << " " << FIXEDINT << w.second << " ";
      cout << FIXEDPCT << slow_map[w.first].total / globalstats.total_slow_time * 100.0 << "% ";
      cout << slow_map[w.first].asString();
      cout << endl;
    }

    heading( "Slow probes to day-of-week distribution" );
    cout << setw(10) << "day";
    cout << setw(7) << "%slow";
    cout << endl;
    for ( auto d : slow_dow_map ) {
      cout << setw(10) << dowStr(d.first) << " ";
      cout << FIXEDPCT << (double)d.second / (double)total_dow_map[d.first] * 100.0 << endl;
    }

    heading( "Slow probe to daily time bucket distribution" );
    cout << "truncated to " << options.day_bucket << " minute buckets, per waitclass min max avg" << endl;
    cout << setw(4) << "hh:mm";
    cout << setw(7) << "%slow";
    cout << setw(8) << "avg";
    cout << setw(6) << "blame";
    cout << " ----------DNS----------";
    cout << " ----------TCP----------";
    cout << " ----------TLS----------";
    cout << " ----------REQ----------";
    cout << " ----------RSP----------";
    cout << " ----------DAT----------";
    cout << endl;
    for ( auto d : slow_day_map ) {
      cout << fixed << setw(2) << setfill('0') << d.first.tm_hour << ":"
           << fixed << setw(2) << setfill('0') << d.first.tm_min << " ";
      cout << FIXEDPCT << (double)d.second.slow / (double)total_day_map[d.first] * 100.0 << " ";
      cout << FIXED3W7 << slow_day_map[d.first].avgResponse() << " ";
      cout << setw(5) << waitClass2String( slow_day_map[d.first].blame() ) << " ";
      cout << slow_day_map[d.first].namelookup.asString();
      cout << slow_day_map[d.first].connect.asString();
      cout << slow_day_map[d.first].appconnect.asString();
      cout << slow_day_map[d.first].pretransfer.asString();
      cout << slow_day_map[d.first].starttransfer.asString();
      cout << slow_day_map[d.first].endtransfer.asString();
      cout << endl;
    }
  }

  heading( "Curl return codes" );
  cout << setw(9) << "count" << " " << "code" << endl;
  for ( auto c : curl_error_map ) {
    cout << FIXEDINT << c.second << " " << curlError2String( c.first ) << endl;
  }
  if ( curl_error_list.size() ) {
    cout << endl << "list of curl errors" << endl;
    for ( auto c : curl_error_list ) {
      cout << c.asString() << endl;
    }
  }

  heading( "HTTP return codes" );
  cout << setw(9) << "count" << " " << "code" << endl;
  for ( auto h : http_code_map ) {
    cout << FIXEDINT << h.second << " " << HTTPError2String( h.first ) << endl;
  }
  if ( http_error_list.size() ) {
    cout << endl << "list of HTTP errors" << endl;
    for ( auto h : http_error_list ) {
      cout << h.asString() << endl;
    }
  }

  heading( "Global stats" );
  cout << "first data point     : " << time_t2String( globalstats.first_time ) << endl;
  cout << "last  data point     : " << time_t2String( globalstats.last_time )  << endl;
  cout << "#probes              : " << globalstats.items << endl;
  cout << "#slow probes         : " << globalstats.items_slow << endl;
  cout << "average response time: " << FIXED3 << globalstats.total_time / globalstats.items << "s" << endl;
  cout << setw(4) << "class";
  cout << setw(8) << "%slow";
  cout << setw(8) << "min";
  cout << setw(8) << "max";
  cout << setw(8) << "avg";
  cout << setw(8) << "%rtrip";
  cout << endl;
  cout << setw(5) << waitClass2String( wcDNS ) << " "
       << FIXED3W7 << (double)slow_map[wcDNS].items / (double)globalstats.items * 100.0 << " "
       << globalstats.wait_class_stats.namelookup.asString() << " "
       << FIXEDPCT << globalstats.wait_class_stats.namelookup.total / globalstats.total_time * 100.0
       <<  endl;
  cout << setw(5) <<  waitClass2String( wcTCPHandshake ) << " "
       << FIXED3W7 << (double)slow_map[wcTCPHandshake].items / (double)globalstats.items * 100.0 << " "
       << globalstats.wait_class_stats.connect.asString() << " "
       << FIXEDPCT << globalstats.wait_class_stats.connect.total / globalstats.total_time * 100.0
       <<  endl;
  cout << setw(5) <<  waitClass2String( wcSSLHandshake ) << " "
       << FIXED3W7 << (double)slow_map[wcSSLHandshake].items / (double)globalstats.items * 100.0 << " "
       << globalstats.wait_class_stats.appconnect.asString() << " "
       << FIXEDPCT << globalstats.wait_class_stats.appconnect.total / globalstats.total_time * 100.0
       <<  endl;
  cout << setw(5) <<  waitClass2String( wcSendStart ) << " "
       << FIXED3W7 << (double)slow_map[wcSendStart].items / (double)globalstats.items * 100.0 << " "
       << globalstats.wait_class_stats.pretransfer.asString() << " "
       << FIXEDPCT<< globalstats.wait_class_stats.pretransfer.total / globalstats.total_time * 100.0
       <<  endl;
  cout << setw(5) <<  waitClass2String( wcWaitEnd ) << " "
       << FIXED3W7 << (double)slow_map[wcWaitEnd].items / (double)globalstats.items * 100.0 << " "
       << globalstats.wait_class_stats.starttransfer.asString() << " "
       << FIXEDPCT << globalstats.wait_class_stats.starttransfer.total / globalstats.total_time * 100.0
       <<  endl;
  cout << setw(5) <<  waitClass2String( wcReceiveEnd ) << " "
       << FIXED3W7 << (double)slow_map[wcReceiveEnd].items / (double)globalstats.items * 100.0 << " "
       << globalstats.wait_class_stats.endtransfer.asString() << " "
       << FIXEDPCT << globalstats.wait_class_stats.endtransfer.total / globalstats.total_time * 100.0
       <<  endl;
}

int main( int argc, char* argv[] ) {
  if ( parseArgs( argc, argv ) ) {
    read( cin );
    summary();
    return 0;
  } else return 1;
}
