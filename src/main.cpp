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

#include "curlstats.h"

using namespace std;

/**
 * Parsed command line options.
 */
Options options;

/**
 * Global (all probes) statistics.
 */
GlobalStats globalstats;

/**
 * Map slow probe statistics to a WaitClass.
 */
map<WaitClass,QtyStats> slow_map;

/**
 * Map slow probe statistics to day-of-week.
 */
map<int,WaitClassStats> slow_dow_map;

/**
 * Map total probe statistics to day-of-week.
 */
map<int,size_t> total_dow_map;

/**
 * Map slow probe statistics to time-of-day.
 */
map<TimeKey,WaitClassStats> slow_day_map;

/**
 * Map total probe statistics to time-of-day.
 */
map<TimeKey,size_t> total_day_map;

/**
 * Map probe stats to date (year,month,day)
 */
map<DateKey,WaitClassStats> total_date_map;

/**
 * Map slow probe stats to date (year,month,day)
 */
map<DateKey,WaitClassStats> slow_date_map;

/**
 * Map slow probes to duration buckets.
 */
map<double,size_t> slow_buckets;

/**
 * Map all probes to duration buckets.
 */
map<double,size_t> all_buckets;

/**
 * Map all DNS to duration buckets.
 */
map<double,size_t> dns_buckets;

/**
 * Map all TCP to duration buckets.
 */
map<double,size_t> tcp_buckets;

/**
 * Map all TLS to duration buckets.
 */
map<double,size_t> tls_buckets;

/**
 * Map all REQ to duration buckets.
 */
map<double,size_t> req_buckets;

/**
 * Map all RSP to duration buckets.
 */
map<double,size_t> rsp_buckets;

/**
 * Map all DAT to duration buckets.
 */
map<double,size_t> dat_buckets;

/**
 * Map probe count to curl error code
 */
map<uint16_t,size_t> curl_error_map;

/**
 * Map probe count to http error code
 */
map<uint16_t,size_t> http_code_map;

/**
 * Map slow probe count to WaitClass
 */
map<WaitClass,size_t> wait_class_map;

/**
 * All probes with curl errors
 */
list<CURL> curl_error_list;

/**
 * All probes with http errors
 */
list<CURL> http_error_list;

/**
 * All slow probes
 */
list<CURL> slow_repsonse_list;

/**
 * Read and parse data.
 */
void read( std::istream& in ) {
  bool has_comment_heading = false;
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
          DateKey dkey = DateKey( curl.datetime.tm_year, curl.datetime.tm_mon, curl.datetime.tm_mday );
          if ( curl.total_time >= options.min_duration ) {
            slow_map[curl.getDominantWaitClass()].addValue( curl.getWaitClassDuration( curl.getDominantWaitClass() ) );
            wait_class_map[curl.getDominantWaitClass()]++;
            if ( options.timing_detail ) slow_repsonse_list.push_back( curl );
            globalstats.items_slow++;
            globalstats.total_slow_time += curl.total_time;
            slow_dow_map[curl.datetime.tm_wday].addValue( wcDNS, curl.getWaitClassDuration( wcDNS ) );
            slow_dow_map[curl.datetime.tm_wday].addValue( wcTCPHandshake, curl.getWaitClassDuration( wcTCPHandshake ) );
            slow_dow_map[curl.datetime.tm_wday].addValue( wcSSLHandshake, curl.getWaitClassDuration( wcSSLHandshake ) );
            slow_dow_map[curl.datetime.tm_wday].addValue( wcSendStart, curl.getWaitClassDuration( wcSendStart ) );
            slow_dow_map[curl.datetime.tm_wday].addValue( wcWaitEnd, curl.getWaitClassDuration( wcWaitEnd ) );
            slow_dow_map[curl.datetime.tm_wday].addValue( wcReceiveEnd, curl.getWaitClassDuration( wcReceiveEnd ) );

            slow_day_map[bucket(tkey,options.day_bucket)].addValue( wcDNS, curl.getWaitClassDuration( wcDNS ) );
            slow_day_map[bucket(tkey,options.day_bucket)].addValue( wcTCPHandshake, curl.getWaitClassDuration( wcTCPHandshake ) );
            slow_day_map[bucket(tkey,options.day_bucket)].addValue( wcSSLHandshake, curl.getWaitClassDuration( wcSSLHandshake ) );
            slow_day_map[bucket(tkey,options.day_bucket)].addValue( wcSendStart, curl.getWaitClassDuration( wcSendStart ) );
            slow_day_map[bucket(tkey,options.day_bucket)].addValue( wcWaitEnd, curl.getWaitClassDuration( wcWaitEnd ) );
            slow_day_map[bucket(tkey,options.day_bucket)].addValue( wcReceiveEnd, curl.getWaitClassDuration( wcReceiveEnd ) );

            slow_date_map[dkey].addValue( wcDNS, curl.getWaitClassDuration( wcDNS ) );
            slow_date_map[dkey].addValue( wcTCPHandshake, curl.getWaitClassDuration( wcTCPHandshake ) );
            slow_date_map[dkey].addValue( wcSSLHandshake, curl.getWaitClassDuration( wcSSLHandshake ) );
            slow_date_map[dkey].addValue( wcSendStart, curl.getWaitClassDuration( wcSendStart ) );
            slow_date_map[dkey].addValue( wcWaitEnd, curl.getWaitClassDuration( wcWaitEnd ) );
            slow_date_map[dkey].addValue( wcReceiveEnd, curl.getWaitClassDuration( wcReceiveEnd ) );

            slow_buckets[ bucket( curl.total_time, options.time_bucket ) ]++;
          }
          globalstats.total_time += curl.total_time;

          dns_buckets[ bucket( curl.getWaitClassDuration( wcDNS ), options.time_bucket ) ]++;
          tcp_buckets[ bucket( curl.getWaitClassDuration( wcTCPHandshake ), options.time_bucket ) ]++;
          tls_buckets[ bucket( curl.getWaitClassDuration( wcSSLHandshake ), options.time_bucket ) ]++;
          req_buckets[ bucket( curl.getWaitClassDuration( wcSendStart ), options.time_bucket ) ]++;
          rsp_buckets[ bucket( curl.getWaitClassDuration( wcWaitEnd ), options.time_bucket ) ]++;
          dat_buckets[ bucket( curl.getWaitClassDuration( wcReceiveEnd ), options.time_bucket ) ]++;

          globalstats.wait_class_stats.namelookup.addValue( curl.getWaitClassDuration( wcDNS ) );
          globalstats.wait_class_stats.connect.addValue( curl.getWaitClassDuration( wcTCPHandshake ) );
          globalstats.wait_class_stats.appconnect.addValue( curl.getWaitClassDuration( wcSSLHandshake ) );
          globalstats.wait_class_stats.pretransfer.addValue( curl.getWaitClassDuration( wcSendStart ) );
          globalstats.wait_class_stats.starttransfer.addValue( curl.getWaitClassDuration( wcWaitEnd ) );
          globalstats.wait_class_stats.endtransfer.addValue( curl.getWaitClassDuration( wcReceiveEnd ) );

          total_date_map[dkey].addValue( wcDNS, curl.getWaitClassDuration( wcDNS ) );
          total_date_map[dkey].addValue( wcTCPHandshake, curl.getWaitClassDuration( wcTCPHandshake ) );
          total_date_map[dkey].addValue( wcSSLHandshake, curl.getWaitClassDuration( wcSSLHandshake ) );
          total_date_map[dkey].addValue( wcSendStart, curl.getWaitClassDuration( wcSendStart ) );
          total_date_map[dkey].addValue( wcWaitEnd, curl.getWaitClassDuration( wcWaitEnd ) );
          total_date_map[dkey].addValue( wcReceiveEnd, curl.getWaitClassDuration( wcReceiveEnd ) );

          if ( globalstats.first_time == 0 || timelocal(&curl.datetime ) < globalstats.first_time )
            globalstats.first_time = timelocal(&curl.datetime );
          if ( globalstats.last_time == 0 || timelocal(&curl.datetime ) > globalstats.last_time )
            globalstats.last_time = timelocal(&curl.datetime );
          all_buckets[ bucket( curl.total_time, options.time_bucket ) ]++;
          globalstats.items++;
          total_day_map[bucket(tkey,options.day_bucket)]++;
          total_dow_map[curl.datetime.tm_wday]++;
        } else {
          curl_error_list.push_back( curl );
        }
      } else {
        cerr << "error on line " << globalstats.items << endl;
      }
    } else {
      if ( !has_comment_heading ) {
        heading( "Comments from input" );
        has_comment_heading = true;
      }
      if ( line.length() > 2 && line[2] != 'Y' ) cout << line << endl;
    }
    getline( in, line );
  }
}

void summary_options() {
  heading( "Options in effect" );
  cout << "Slowness threshold             : " << FIXED3W7 << options.min_duration << " seconds" << endl;
  cout << "Response time histogram bucket : " << FIXED3W7 << options.time_bucket << " seconds" << endl;
  cout << "repeating 24h histogram bucket : " << FIXED3W7 << options.day_bucket << " minutes" << endl;
  cout << "histogram minimum display pct  : " << FIXED3W7 << options.histo_min_pct << " %" << endl;
  cout << "Show trail of slow probes      : " << FIXED3W7 << options.timing_detail << endl;

  if ( options.timing_detail ) {
    heading( "List of slow probes" );
    for ( auto l : slow_repsonse_list ) {
      cout << l.asString() << endl;
    }
  }
}

void show_histogram( const map<double,size_t> &histo ) {
  cout << setw(9) << "bucket" << " " << setw(9) << "count" << setw(7) << "%probe" << endl;
  for ( auto b : histo ) {
    double pct = b.second/(double)globalstats.items*100.0;
    if ( pct >= options.histo_min_pct ) {
      cout << "<" << FIXED3W7 << b.first;
      cout << "s " << FIXEDINT << b.second;
      cout << " " << FIXEDPCT << pct << endl;
    }
  }
}

void summary_qos() {
  heading( "QoS" );
  cout << FIXED3 << 100.0 - (double)globalstats.items_slow / (double)globalstats.items * 100.0 << "% ";
  cout << "of probes return within " << options.min_duration << "s" << endl;

  cout << endl << "probe count to response time distribution, bucket size " << options.time_bucket << "s" << endl;
  show_histogram( all_buckets );

  cout << endl << "probe count to DNS wait time distribution, bucket size " << options.time_bucket << "s" << endl;
  show_histogram( dns_buckets );

  cout << endl << "probe count to TCP wait time distribution, bucket size " << options.time_bucket << "s" << endl;
  show_histogram( tcp_buckets );

  cout << endl << "probe count to TLS wait time distribution, bucket size " << options.time_bucket << "s" << endl;
  show_histogram( tls_buckets );

  cout << endl << "probe count to REQ wait time distribution, bucket size " << options.time_bucket << "s" << endl;
  show_histogram( req_buckets );

  cout << endl << "probe count to RSP wait time distribution, bucket size " << options.time_bucket << "s" << endl;
  show_histogram( rsp_buckets );

  cout << endl << "probe count to DAT wait time distribution, bucket size " << options.time_bucket << "s" << endl;
  show_histogram( dat_buckets );
}

void summary_wait_class() {
  heading( "Slow probe to wait-class distribution" );
  cout << setw(5) << "class";
  cout << setw(10) << "#probes";
  cout << setw(8) << "%blame";
  cout << setw(8) << "min";
  cout << setw(8) << "max";
  cout << setw(8) << "avg";
  cout << setw(8) << "stdev";
  cout << endl;
  for ( auto w : wait_class_map ) {
    cout << "  " << waitClass2String( w.first ) << " " << FIXEDINT << w.second << " ";
    cout << FIXEDPCT << slow_map[w.first].total / globalstats.total_slow_time * 100.0 << "% ";
    cout << slow_map[w.first].asString(true);
    cout << endl;
  }
}

void summary_slow_probes_to_dow() {
  heading( "Slow probes to day-of-week distribution" );
  cout << setw(9) << "day";
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
  for ( auto d : slow_dow_map ) {
    cout << setw(9) << dowStr(d.first) << " ";
    cout << FIXEDPCT << (double)d.second.getNumItems() / (double)total_dow_map[d.first] * 100.0 << " ";
    cout << FIXED3W7 << slow_dow_map[d.first].avgResponse() << " ";
    cout << setw(5) << waitClass2String( slow_dow_map[d.first].blame() ) << " ";
    cout << slow_dow_map[d.first].namelookup.asString();
    cout << slow_dow_map[d.first].connect.asString();
    cout << slow_dow_map[d.first].appconnect.asString();
    cout << slow_dow_map[d.first].pretransfer.asString();
    cout << slow_dow_map[d.first].starttransfer.asString();
    cout << slow_dow_map[d.first].endtransfer.asString();
    cout << endl;
  }
}

void summary_slow_probes_to_daily() {
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
    cout << FIXEDPCT << (double)d.second.getNumItems() / (double)total_day_map[d.first] * 100.0 << " ";
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

void summary_curl_errors() {
  heading( "Curl return codes" );
  cout << setw(9) << "#probes" << " " << "code" << endl;
  for ( auto c : curl_error_map ) {
    cout << FIXEDINT << c.second << " " << curlError2String( c.first ) << endl;
  }
  if ( curl_error_list.size() ) {
    cout << endl << "list of curl errors" << endl;
    for ( auto c : curl_error_list ) {
      cout << c.asString() << endl;
    }
  }
}

void summary_http_errors() {
  heading( "HTTP return codes" );
  cout << setw(9) << "#probes" << " " << "code" << endl;
  for ( auto h : http_code_map ) {
    cout << FIXEDINT << h.second << " " << HTTPError2String( h.first ) << endl;
  }
  if ( http_error_list.size() ) {
    cout << endl << "list of HTTP errors" << endl;
    for ( auto h : http_error_list ) {
      cout << h.asString() << endl;
    }
  }
}

void summary_daily_history() {
  heading( "Daily history - all probes" );
  cout << setw(10) << "date";
  cout << setw(8) << "%slow";
  cout << setw(8) << "avg";
  cout << setw(6) << "most";
  cout << " ----------DNS----------";
  cout << " ----------TCP----------";
  cout << " ----------TLS----------";
  cout << " ----------REQ----------";
  cout << " ----------RSP----------";
  cout << " ----------DAT----------";
  cout << endl;
  for ( auto d : total_date_map ) {
    cout << setw(4) << setfill('0') << d.first.tm_year+1900;
    cout << '-' << setw(2) << setfill('0') << d.first.tm_mon;
    cout << '-' << setw(2) << setfill('0') << d.first.tm_mday;
    cout << " ";
    cout << FIXED3W7 << (double)slow_date_map[d.first].getNumItems() / (double)d.second.getNumItems() * 100.0;
    cout << " ";
    cout << FIXED3W7 << d.second.avgResponse();
    cout << " ";
    cout << setw(5) << waitClass2String( d.second.blame() );
    cout << " ";
    cout << d.second.namelookup.asString();
    cout << d.second.connect.asString();
    cout << d.second.appconnect.asString();
    cout << d.second.pretransfer.asString();
    cout << d.second.starttransfer.asString();
    cout << d.second.endtransfer.asString();
    cout << endl;
  }
}

void summary_global_stats() {
  heading( "Global stats" );
  cout << "first probe          : " << time_t2String( globalstats.first_time ) << endl;
  cout << "last  probe          : " << time_t2String( globalstats.last_time )  << endl;
  cout << "#probes              : " << globalstats.items << endl;
  cout << "#slow probes         : " << globalstats.items_slow << endl;
  cout << "%slow probes         : " << FIXED3 << (double)globalstats.items_slow/(double)globalstats.items*100.0 << endl;
  double global_avg_response = globalstats.total_time / globalstats.items;
  cout << "average response time: " << FIXED3 << global_avg_response << "s" << endl;
  cout << "ideal response time  : " << FIXED3 << globalstats.wait_class_stats.getOptimalResponse() << "s" << endl;
  cout << "estimate network RTT : " << FIXED3 << globalstats.wait_class_stats.getNetworkRoundtrip()*1000.0 << "ms" << endl;
  cout << setw(4) << "class";
  cout << setw(8) << "%slow";
  cout << setw(8) << "min";
  cout << setw(8) << "max";
  cout << setw(8) << "avg";
  cout << setw(8) << "stddev";
  cout << setw(8) << "%rtrip";
  cout << setw(22) << "monotonicity";
  cout << endl;
  cout << setw(5) << waitClass2String( wcDNS ) << " "
       << FIXED3W7 << (double)slow_map[wcDNS].items / (double)globalstats.items * 100.0 << " "
       << globalstats.wait_class_stats.namelookup.asString(true) << " "
       << FIXEDPCT << globalstats.wait_class_stats.namelookup.total / globalstats.total_time * 100.0
       << setw(22) << globalstats.wait_class_stats.namelookup.monotonicity(global_avg_response)
       <<  endl;
  cout << setw(5) <<  waitClass2String( wcTCPHandshake ) << " "
       << FIXED3W7 << (double)slow_map[wcTCPHandshake].items / (double)globalstats.items * 100.0 << " "
       << globalstats.wait_class_stats.connect.asString(true) << " "
       << FIXEDPCT << globalstats.wait_class_stats.connect.total / globalstats.total_time * 100.0
       << setw(22) << globalstats.wait_class_stats.connect.monotonicity(global_avg_response)
       <<  endl;
  cout << setw(5) <<  waitClass2String( wcSSLHandshake ) << " "
       << FIXED3W7 << (double)slow_map[wcSSLHandshake].items / (double)globalstats.items * 100.0 << " "
       << globalstats.wait_class_stats.appconnect.asString(true) << " "
       << FIXEDPCT << globalstats.wait_class_stats.appconnect.total / globalstats.total_time * 100.0
       << setw(22) << globalstats.wait_class_stats.appconnect.monotonicity(global_avg_response)
       <<  endl;
  cout << setw(5) <<  waitClass2String( wcSendStart ) << " "
       << FIXED3W7 << (double)slow_map[wcSendStart].items / (double)globalstats.items * 100.0 << " "
       << globalstats.wait_class_stats.pretransfer.asString(true) << " "
       << FIXEDPCT<< globalstats.wait_class_stats.pretransfer.total / globalstats.total_time * 100.0
       << setw(22) << globalstats.wait_class_stats.pretransfer.monotonicity(global_avg_response)
       <<  endl;
  cout << setw(5) <<  waitClass2String( wcWaitEnd ) << " "
       << FIXED3W7 << (double)slow_map[wcWaitEnd].items / (double)globalstats.items * 100.0 << " "
       << globalstats.wait_class_stats.starttransfer.asString(true) << " "
       << FIXEDPCT << globalstats.wait_class_stats.starttransfer.total / globalstats.total_time * 100.0
       << setw(22) << globalstats.wait_class_stats.starttransfer.monotonicity(global_avg_response)
       <<  endl;
  cout << setw(5) <<  waitClass2String( wcReceiveEnd ) << " "
       << FIXED3W7 << (double)slow_map[wcReceiveEnd].items / (double)globalstats.items * 100.0 << " "
       << globalstats.wait_class_stats.endtransfer.asString(true) << " "
       << FIXEDPCT << globalstats.wait_class_stats.endtransfer.total / globalstats.total_time * 100.0
       << setw(22) << globalstats.wait_class_stats.endtransfer.monotonicity(global_avg_response)
       <<  endl;
}

/**
 * Write summary.
 */
void summary() {
  summary_options();
  summary_qos();
  if ( globalstats.items_slow > 0 ) {
    summary_wait_class();
    summary_slow_probes_to_dow();
    summary_slow_probes_to_daily();
  }
  summary_curl_errors();
  summary_http_errors();
  summary_daily_history();
  summary_global_stats();
}

/**
 * Program entry.
 */
int main( int argc, char* argv[] ) {
  if ( parseArgs( argc, argv, options ) ) {
    // to prevent timezone translations
    setenv( "TZ", "UTC", 1 );
    read( cin );
    summary();
    return 0;
  } else return 1;
}
