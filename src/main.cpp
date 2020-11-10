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

#include "comments.h"
#include "curlprobe.h"
#include "datekey.h"
#include "datetime.h"
#include "globalstats.h"
#include "html.h"
#include "options.h"
#include "output.h"
#include "qtystats.h"
#include "text.h"
#include "timekey.h"
#include "util.h"
#include "waitclass.h"
#include "variables.h"

using namespace std;



/**
 * Read and parse data.
 */
void read( std::istream& in ) {
  string line;
  getline( in, line );
  while ( in.good()  ) {
    if ( !isCommment( line ) ) {
      CURLProbe curl;
      if ( curl.parse( line ) ) {
        DateKey dkey = DateKey( curl.datetime.year, curl.datetime.month, curl.datetime.day );
        TimeKey tkey = TimeKey( curl.datetime.hour, curl.datetime.minute );        
        const auto &qos_ref = qos_by_date.find( dkey );
        if ( qos_ref == qos_by_date.end() ) qos_by_date[dkey] = { 0, 0, 0 };
        qos_by_date[dkey].total++;
        weekmap_probestats[curl.datetime.wday][bucket(tkey,options.weekmap_bucket)].total++;
        curl_error_map[curl.curl_error]++;
        if ( curl.curl_error == 0 ) { 
          http_code_map[curl.http_code]++;
          if ( curl.http_code >= 400 ) {
            weekmap_probestats[curl.datetime.wday][bucket(tkey,options.weekmap_bucket)].http_errors++;
            qos_by_date[dkey].http_errors++;
            http_error_list.push_back( curl );
          } else {
            recent_probes.push_front( curl );
            while ( recent_probes.size() > 700 ) recent_probes.pop_back();
            if ( curl.total_time >= options.slow_threshold ) {
              qos_by_date[dkey].slow++;
              weekmap_probestats[curl.datetime.wday][bucket(tkey,options.weekmap_bucket)].slow++;
              slow_map[curl.getDominantWaitClass()].addValue( curl.getWaitClassDuration( curl.getDominantWaitClass() ) );
              wait_class_map[curl.getDominantWaitClass()]++;
              if ( options.hasMode( omSlowTrail ) ) slow_repsonse_list.push_back( curl );
              globalstats.items_slow++;
              globalstats.total_slow_time += curl.total_time;
              if ( options.hasMode( omWeekdayMap ) || options.hasMode( omWeekdaySlowMap ) ) {
                auto &ref = slow_dow_map[curl.datetime.wday];
                ref.addValues( curl.getWaitClassDuration( wcDNS ), 
                               curl.getWaitClassDuration( wcTCPHandshake ),
                               curl.getWaitClassDuration( wcSSLHandshake ),
                               curl.getWaitClassDuration( wcSendStart ),
                               curl.getWaitClassDuration( wcWaitEnd ),
                               curl.getWaitClassDuration( wcReceiveEnd ) );
              }
              if ( options.hasMode( om24hMap ) || options.hasMode( om24hSlowMap ) ) {
                auto &ref = slow_day_map[bucket(tkey,options.day_bucket)];
                ref.addValues( curl.getWaitClassDuration( wcDNS ), 
                               curl.getWaitClassDuration( wcTCPHandshake ),
                               curl.getWaitClassDuration( wcSSLHandshake ),
                               curl.getWaitClassDuration( wcSendStart ),
                               curl.getWaitClassDuration( wcWaitEnd ),
                               curl.getWaitClassDuration( wcReceiveEnd ) );
              }

              if ( options.hasMode( omDailyTrail ) ) {
                auto &ref = slow_date_map[dkey];
                ref.addValues( curl.getWaitClassDuration( wcDNS ), 
                               curl.getWaitClassDuration( wcTCPHandshake ),
                               curl.getWaitClassDuration( wcSSLHandshake ),
                               curl.getWaitClassDuration( wcSendStart ),
                               curl.getWaitClassDuration( wcWaitEnd ),
                               curl.getWaitClassDuration( wcReceiveEnd ) );
              }

            }
            globalstats.total_time += curl.total_time;
            globalstats.response_stats.addValue( curl.total_time );

            globalstats.wait_class_stats.namelookup.addValue( curl.getWaitClassDuration( wcDNS ) );
            globalstats.wait_class_stats.connect.addValue( curl.getWaitClassDuration( wcTCPHandshake ) );
            globalstats.wait_class_stats.appconnect.addValue( curl.getWaitClassDuration( wcSSLHandshake ) );
            globalstats.wait_class_stats.pretransfer.addValue( curl.getWaitClassDuration( wcSendStart ) );
            globalstats.wait_class_stats.starttransfer.addValue( curl.getWaitClassDuration( wcWaitEnd ) );
            globalstats.wait_class_stats.endtransfer.addValue( curl.getWaitClassDuration( wcReceiveEnd ) );

            if ( options.hasMode( omDailyTrail ) ) {
              auto &ref = total_date_map[dkey];
              ref.addValues( curl.getWaitClassDuration( wcDNS ), 
                              curl.getWaitClassDuration( wcTCPHandshake ),
                              curl.getWaitClassDuration( wcSSLHandshake ),
                              curl.getWaitClassDuration( wcSendStart ),
                              curl.getWaitClassDuration( wcWaitEnd ),
                              curl.getWaitClassDuration( wcReceiveEnd ) );
            }

            if ( options.hasMode( om24hMap ) || options.hasMode( om24hSlowMap ) ) {
              auto &ref = total_day_map[bucket(tkey,options.day_bucket)];
              ref.addValues( curl.getWaitClassDuration( wcDNS ), 
                              curl.getWaitClassDuration( wcTCPHandshake ),
                              curl.getWaitClassDuration( wcSSLHandshake ),
                              curl.getWaitClassDuration( wcSendStart ),
                              curl.getWaitClassDuration( wcWaitEnd ),
                              curl.getWaitClassDuration( wcReceiveEnd ) );
            }

            if ( options.hasMode( omWeekdayMap ) || options.hasMode( omWeekdaySlowMap ) ) {
              auto &ref = total_dow_map[curl.datetime.wday];
              ref.addValues( curl.getWaitClassDuration( wcDNS ), 
                              curl.getWaitClassDuration( wcTCPHandshake ),
                              curl.getWaitClassDuration( wcSSLHandshake ),
                              curl.getWaitClassDuration( wcSendStart ),
                              curl.getWaitClassDuration( wcWaitEnd ),
                              curl.getWaitClassDuration( wcReceiveEnd ) );
            }

            globalstats.size_upload += curl.size_upload;
            globalstats.size_download += curl.size_download;          

            if ( globalstats.first_time.year == 0 || curl.datetime < globalstats.first_time )
              globalstats.first_time = curl.datetime;
            if ( globalstats.last_time.year == 0 ||  curl.datetime > globalstats.last_time )
              globalstats.last_time = curl.datetime;

            globalstats.timed_probes++;          
            weekmap_qtystats[curl.datetime.wday][bucket(tkey,options.weekmap_bucket)].addValue( curl.total_time );            
          }
        } else {
          qos_by_date[dkey].curl_errors++;
          weekmap_probestats[curl.datetime.wday][bucket(tkey,options.weekmap_bucket)].curl_errors++;
          curl_error_list.push_back( curl );
        }
        globalstats.total_probes++;
      } else {
        cerr << "error on line " << globalstats.total_probes << endl;
      }
    } else {
      comments.addComment( line );      
    }
    getline( in, line );
  }
}



/**
 * Program entry.
 */
int main( int argc, char* argv[] ) {
  try {
    if ( parseArgs( argc, argv, options ) ) {
      read( cin );
      if ( options.output_format == Options::OutputFormat::Text )
        summary_text();
      else if ( options.output_format == Options::OutputFormat::HTML ) {
        HTML html;
        cout << html.generate();
      }
      return 0;
    } else return 1;
  }
  catch ( const std::exception &e ) {
    cerr << "caught std::exception: " << e.what() << endl;
    return 1;
  }
}
