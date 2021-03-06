#include "text.h"

#include "variables.h"
#include "options.h"
#include "output.h"
#include "qtystats.h"

void summary_comments() {
  for ( const auto &c : comments.comments ) {
    cout << c.first << " = " << c.second << endl;
  }
}

void summary_options() {
  heading( "Options in effect" );
  cout << "output format                  : " << FIXED3W7 << Options::formatAsString( options.output_format ) << endl;
  cout << "slowness threshold             : " << FIXED3W7 << options.slow_threshold << " seconds" << endl;
  cout << "histogram max buckets          : " << FIXED3W7 << options.histo_max_buckets << " buckets" << endl;
  cout << "repeating 24h histogram bucket : " << FIXED3W7 << options.day_bucket << " minutes" << endl;
  cout << "histogram minimum display pct  : " << FIXED3W7 << options.histo_min_pct << " %" << endl;
}

void summary_slowtrail() {
  heading( "List of slow (" + options.slowString() + ") probes" );
  for ( const auto &l : slow_repsonse_list ) {
    cout << l.asString() << endl;
  }
}

void show_histogram( const QtyStats& ref ) {
  cout << setw(11) << "bucket" << " " << setw(9) << "count" << setw(7) << "%probe" << setw(7) << "pctile" << setw(8) << "sigma" << endl;
  double percentile = 0.0;
  for ( const auto &b : ref.buckets ) {
    double pct = b.second/(double)globalstats.timed_probes*100.0;
    percentile += pct;
    double sigma = (b.first - ref.getMean())/ref.getSigma();
    if ( pct >= options.histo_min_pct ) {
      cout << "<" << setw(9) << setprecision(6) << b.first;
      cout << "s " << FIXEDINT << b.second;
      cout << " " << FIXEDPCT << pct;
      cout << " " << FIXEDPCT << percentile;
      cout << " " << FIXED3W7 << sigma << endl;
    }
  }
}

void summary_histo() {
  heading( "QoS" );
  cout << FIXED3 << 100.0 - (double)globalstats.items_slow / (double)globalstats.timed_probes * 100.0 << "% ";
  cout << "of probes return within " << FIXED3 << options.slow_threshold << "s" << endl;

  cout << endl << "probe count to total response time distribution, bucket size " << setprecision(6) << globalstats.response_stats.current_bucket << "s" << endl;
  show_histogram( globalstats.response_stats );

  cout << endl << "probe count to DNS wait time distribution, bucket size " << setprecision(6) << globalstats.wait_class_stats.namelookup.current_bucket << "s" << endl;
  show_histogram( globalstats.wait_class_stats.namelookup );

  cout << endl << "probe count to TCP wait time distribution, bucket size " << setprecision(6) << globalstats.wait_class_stats.connect.current_bucket << "s" << endl;
  show_histogram( globalstats.wait_class_stats.connect );

  cout << endl << "probe count to TLS wait time distribution, bucket size " << setprecision(6) << globalstats.wait_class_stats.appconnect.current_bucket << "s" << endl;
  show_histogram( globalstats.wait_class_stats.appconnect );

  cout << endl << "probe count to REQ wait time distribution, bucket size " << setprecision(6) << globalstats.wait_class_stats.pretransfer.current_bucket << "s" << endl;
  show_histogram( globalstats.wait_class_stats.pretransfer );

  cout << endl << "probe count to RSP wait time distribution, bucket size " << setprecision(6) << globalstats.wait_class_stats.starttransfer.current_bucket << "s" << endl;
  show_histogram( globalstats.wait_class_stats.starttransfer );

  cout << endl << "probe count to DAT wait time distribution, bucket size " << setprecision(6) << globalstats.wait_class_stats.endtransfer.current_bucket << "s" << endl;
  show_histogram( globalstats.wait_class_stats.endtransfer );
}

void summary_wait_class() {
  heading( "Slow (" + options.slowString() + ") probe to wait-class distribution" );
  cout << setw(5) << "class";
  cout << setw(10) << "#probes";
  cout << setw(8) << "%time";
  cout << setw(8) << "min";
  cout << setw(8) << "max";
  cout << setw(8) << "avg";
  cout << setw(8) << "stdev";
  cout << endl;
  for ( const auto &w : wait_class_map ) {
    cout << "  " << waitClass2String( w.first ) << " " << FIXEDINT << w.second << " ";
    cout << FIXEDPCT << slow_map[w.first].total / globalstats.total_slow_time * 100.0 << "% ";
    cout << slow_map[w.first].asString(true);
    cout << endl;
  }
}

void summary_slow_probes_to_dow() {
  heading( "Slow (" + options.slowString() + ") probes to day-of-week distribution" );
  cout << setw(9) << "day";
  cout << setw(7) << "%slow";
  cout << setw(8) << "avg";
  cout << setw(6) << "most";
  cout << " ----------DNS----------";
  cout << " ----------TCP----------";
  cout << " ----------TLS----------";
  cout << " ----------REQ----------";
  cout << " ----------RSP----------";
  cout << " ----------DAT----------";
  cout << endl;
  for ( auto d : slow_dow_map ) {
    cout << setw(9) << dowStr(d.first) << " ";
    cout << FIXEDPCT << (double)d.second.getNumItems() / (double)total_dow_map[d.first].getNumItems() * 100.0 << " ";
    cout << FIXED3W7 << slow_dow_map[d.first].probe.getMean() << " ";
    cout << setw(5) << waitClass2String( slow_dow_map[d.first].most() ) << " ";
    cout << slow_dow_map[d.first].namelookup.asString();
    cout << slow_dow_map[d.first].connect.asString();
    cout << slow_dow_map[d.first].appconnect.asString();
    cout << slow_dow_map[d.first].pretransfer.asString();
    cout << slow_dow_map[d.first].starttransfer.asString();
    cout << slow_dow_map[d.first].endtransfer.asString();
    cout << endl;
  }
}

void summary_all_probes_to_dow() {
  heading( "All probes to day-of-week distribution (slow is " + options.slowString() + ")" );
  cout << setw(9) << "day";
  cout << setw(7) << "%slow";
  cout << setw(8) << "avg";
  cout << setw(6) << "most";
  cout << " ----------DNS----------";
  cout << " ----------TCP----------";
  cout << " ----------TLS----------";
  cout << " ----------REQ----------";
  cout << " ----------RSP----------";
  cout << " ----------DAT----------";
  cout << endl;
  for ( auto d : total_dow_map ) {
    cout << setw(9) << dowStr(d.first) << " ";
    cout << FIXEDPCT << (double)slow_dow_map[d.first].getNumItems() / (double)d.second.getNumItems() * 100.0 << " ";
    cout << FIXED3W7 << total_dow_map[d.first].probe.getMean() << " ";
    cout << setw(5) << waitClass2String( total_dow_map[d.first].most() ) << " ";
    cout << total_dow_map[d.first].namelookup.asString();
    cout << total_dow_map[d.first].connect.asString();
    cout << total_dow_map[d.first].appconnect.asString();
    cout << total_dow_map[d.first].pretransfer.asString();
    cout << total_dow_map[d.first].starttransfer.asString();
    cout << total_dow_map[d.first].endtransfer.asString();
    cout << endl;
  }
}

void summary_slow_probes_to_daily() {
  heading( "Slow (" + options.slowString() + ") probe to daily time bucket distribution" );
  cout << "truncated to " << options.day_bucket << " minute buckets, per waitclass min max avg" << endl;
  cout << setw(4) << "hh:mm";
  cout << setw(7) << "%slow";
  cout << setw(8) << "avg";
  cout << setw(6) << "most";
  cout << " ----------DNS----------";
  cout << " ----------TCP----------";
  cout << " ----------TLS----------";
  cout << " ----------REQ----------";
  cout << " ----------RSP----------";
  cout << " ----------DAT----------";
  cout << endl;
  for ( auto d : slow_day_map ) {
    cout << fixed << setw(2) << setfill('0') << d.first.hour << ":"
         << fixed << setw(2) << setfill('0') << d.first.minute << " ";
    cout << FIXEDPCT << (double)d.second.getNumItems() / (double)total_day_map[d.first].getNumItems() * 100.0 << " ";
    cout << FIXED3W7 << slow_day_map[d.first].probe.getMean() << " ";
    cout << setw(5) << waitClass2String( slow_day_map[d.first].most() ) << " ";
    cout << slow_day_map[d.first].namelookup.asString();
    cout << slow_day_map[d.first].connect.asString();
    cout << slow_day_map[d.first].appconnect.asString();
    cout << slow_day_map[d.first].pretransfer.asString();
    cout << slow_day_map[d.first].starttransfer.asString();
    cout << slow_day_map[d.first].endtransfer.asString();
    cout << endl;
  }
}

void summary_all_probes_to_daily() {
  heading( "All probes to daily time bucket distribution (slow is " + options.slowString() + ")" );
  cout << "truncated to " << options.day_bucket << " minute buckets, per waitclass min max avg" << endl;
  cout << setw(4) << "hh:mm";
  cout << setw(7) << "%slow";
  cout << setw(8) << "avg";
  cout << setw(6) << "most";
  cout << " ----------DNS----------";
  cout << " ----------TCP----------";
  cout << " ----------TLS----------";
  cout << " ----------REQ----------";
  cout << " ----------RSP----------";
  cout << " ----------DAT----------";
  cout << endl;
  for ( auto d : total_day_map ) {
    cout << fixed << setw(2) << setfill('0') << d.first.hour << ":"
         << fixed << setw(2) << setfill('0') << d.first.minute << " ";
    cout << FIXEDPCT << (double)slow_day_map[d.first].getNumItems() / (double)d.second.getNumItems() * 100.0 << " ";
    cout << FIXED3W7 << total_day_map[d.first].probe.getMean() << " ";
    cout << setw(5) << waitClass2String( total_day_map[d.first].most() ) << " ";
    cout << total_day_map[d.first].namelookup.asString();
    cout << total_day_map[d.first].connect.asString();
    cout << total_day_map[d.first].appconnect.asString();
    cout << total_day_map[d.first].pretransfer.asString();
    cout << total_day_map[d.first].starttransfer.asString();
    cout << total_day_map[d.first].endtransfer.asString();
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
    cout << FIXEDINT << h.second << " " << HTTPCode2String( h.first ) << endl;
  }
  if ( http_error_list.size() ) {
    cout << endl << "list of HTTP errors" << endl;
    for ( auto h : http_error_list ) {
      cout << h.asString() << endl;
    }
  }
}

void summary_daily_history() {
  heading( "Daily history - all probes (slow is " + options.slowString() + ")" );
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
    cout << setw(4) << setfill('0') << d.first.year;
    cout << '-' << setw(2) << setfill('0') << d.first.month;
    cout << '-' << setw(2) << setfill('0') << d.first.day;
    cout << " ";
    cout << FIXED3W7 << (double)slow_date_map[d.first].getNumItems() / (double)d.second.getNumItems() * 100.0;
    cout << " ";
    cout << FIXED3W7 << d.second.probe.getMean();
    cout << " ";
    cout << setw(5) << waitClass2String( d.second.most() );
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
  heading( "Global stats (slow is " + options.slowString() + ")" );
  cout << "first probe          : " << globalstats.first_time.asString() << endl;
  cout << "last  probe          : " << globalstats.last_time.asString() << endl;
  cout << "#total probes        : " << globalstats.total_probes << endl;  
  cout << "#timed probes        : " << globalstats.timed_probes << endl;
  cout << "#slow probes         : " << globalstats.items_slow << endl;
  size_t total_outside_qos = globalstats.items_slow + curl_error_list.size() + http_error_list.size();
  cout << "#errors curl/http    : " << curl_error_list.size() << "/" << http_error_list.size()  << endl;
  cout << "QoS                  : " << FIXED3 << (1.0-(double)total_outside_qos/(double)globalstats.total_probes)*100.0 << "%" << endl;

  cout << "up/down bytes/req    : " << FIXED3 << (double)globalstats.size_upload/(double)globalstats.timed_probes/1024.0 << "KiB / "
                                    << FIXED3 << (double)globalstats.size_download/(double)globalstats.timed_probes/1024.0 << "KiB" << endl;

  double global_avg_response = globalstats.total_time / globalstats.timed_probes;
  double global_opt_response = globalstats.wait_class_stats.getIdealResponse();

  cout << "average response     : " << FIXED3 << global_avg_response << "s";
  cout << " (" << globalstats.response_stats.consistency() << ")" << endl;
  cout << "ideal response       : " << FIXED3 << global_opt_response << "s" << endl;
  cout << "min/max/sdev response: " << FIXED3 << globalstats.response_stats.min;
  cout << "/" << globalstats.response_stats.max;
  cout << "/" << globalstats.response_stats.getSigma() << "s" << endl;
  cout << "estimate network RTT : " << FIXED3 << globalstats.wait_class_stats.getNetworkRoundtrip()*1000.0 << "ms" << endl;

  cout << setw(4) << "class";
  cout << setw(8) << "%slow";
  cout << setw(8) << "min";
  cout << setw(8) << "max";
  cout << setw(8) << "avg";
  cout << setw(8) << "stddev";
  cout << setw(8) << "%rtrip";
  const unsigned int consistency_width = 20;
  const std::string inconstr = "";
  cout << setw(consistency_width) << "consistency";
  cout << endl;
  cout << setw(5) << waitClass2String( wcDNS ) << " "
       << FIXED3W7 << (double)slow_map[wcDNS].items / (double)globalstats.timed_probes * 100.0 << " "
       << globalstats.wait_class_stats.namelookup.asString(true) << " "
       << FIXEDPCT << globalstats.wait_class_stats.namelookup.total / globalstats.total_time * 100.0;
  if ( globalstats.wait_class_stats.namelookup.total / globalstats.total_time * 100.0 > 1.0 )
	cout << setw(consistency_width) << globalstats.wait_class_stats.namelookup.consistency();
  else
	cout << setw(consistency_width) << inconstr;
  cout <<  endl;

  cout << setw(5) <<  waitClass2String( wcTCPHandshake ) << " "
       << FIXED3W7 << (double)slow_map[wcTCPHandshake].items / (double)globalstats.timed_probes * 100.0 << " "
       << globalstats.wait_class_stats.connect.asString(true) << " "
       << FIXEDPCT << globalstats.wait_class_stats.connect.total / globalstats.total_time * 100.0;
  if ( globalstats.wait_class_stats.connect.total / globalstats.total_time * 100.0 > 1.0 )
	cout << setw(consistency_width) << globalstats.wait_class_stats.connect.consistency();
  else
	cout << setw(consistency_width) << inconstr;
  cout <<  endl;

  cout << setw(5) <<  waitClass2String( wcSSLHandshake ) << " "
       << FIXED3W7 << (double)slow_map[wcSSLHandshake].items / (double)globalstats.timed_probes * 100.0 << " "
       << globalstats.wait_class_stats.appconnect.asString(true) << " "
       << FIXEDPCT << globalstats.wait_class_stats.appconnect.total / globalstats.total_time * 100.0;
  if ( globalstats.wait_class_stats.appconnect.total / globalstats.total_time * 100.0 > 1.0 )
	cout << setw(consistency_width) << globalstats.wait_class_stats.appconnect.consistency();
  else
	cout << setw(consistency_width) << inconstr;
  cout <<  endl;

  cout << setw(5) <<  waitClass2String( wcSendStart ) << " "
       << FIXED3W7 << (double)slow_map[wcSendStart].items / (double)globalstats.timed_probes * 100.0 << " "
       << globalstats.wait_class_stats.pretransfer.asString(true) << " "
       << FIXEDPCT<< globalstats.wait_class_stats.pretransfer.total / globalstats.total_time * 100.0;
  if ( globalstats.wait_class_stats.pretransfer.total / globalstats.total_time * 100.0 > 1.0 )
	cout << setw(consistency_width) << globalstats.wait_class_stats.pretransfer.consistency();
  else
	cout << setw(consistency_width) << inconstr;
  cout <<  endl;

  cout << setw(5) <<  waitClass2String( wcWaitEnd ) << " "
       << FIXED3W7 << (double)slow_map[wcWaitEnd].items / (double)globalstats.timed_probes * 100.0 << " "
       << globalstats.wait_class_stats.starttransfer.asString(true) << " "
       << FIXEDPCT << globalstats.wait_class_stats.starttransfer.total / globalstats.total_time * 100.0;
  if ( globalstats.wait_class_stats.starttransfer.total / globalstats.total_time * 100.0 > 1.0 )
	cout << setw(consistency_width) << globalstats.wait_class_stats.starttransfer.consistency();
  else
	cout << setw(consistency_width) << inconstr;
  cout <<  endl;

  cout << setw(5) <<  waitClass2String( wcReceiveEnd ) << " "
       << FIXED3W7 << (double)slow_map[wcReceiveEnd].items / (double)globalstats.timed_probes * 100.0 << " "
       << globalstats.wait_class_stats.endtransfer.asString(true) << " "
       << FIXEDPCT << globalstats.wait_class_stats.endtransfer.total / globalstats.total_time * 100.0;
  if ( globalstats.wait_class_stats.endtransfer.total / globalstats.total_time * 100.0 > 1.0 )
	cout << setw(consistency_width) << globalstats.wait_class_stats.endtransfer.consistency();
  else
	cout << setw(consistency_width) << inconstr;
  cout <<  endl;
}

void summary_abnormal() {
  if ( globalstats.wait_class_stats.namelookup.getMean() > 2.0 * globalstats.wait_class_stats.connect.getMean() ) {
    globalstats.findings.push_back( "DNS is slow compared to TCP handshakes" );
  }
  if ( globalstats.wait_class_stats.appconnect.getMean() > 6.0 * globalstats.wait_class_stats.connect.getMean() ) {
    globalstats.findings.push_back( "TLS is expensive compared to TCP" );
  }
  if ( globalstats.findings.size() > 0 ) {
    heading( "Findings" );
    for ( auto f : globalstats.findings ) {
      cout << "  - " << f << endl;
    }
  }
}

/**
 * Write summary.
 */
void summary_text() {
  if ( options.hasMode( omComments ) ) summary_comments();
  if ( options.hasMode( omOptions ) ) summary_options();
  if ( options.hasMode( omSlowTrail ) ) summary_slowtrail();
  if ( globalstats.timed_probes >= MINIMUM_PROBES ) {
    if ( options.hasMode( omHistograms ) ) summary_histo();
    if ( globalstats.items_slow > 0 ) {
      if ( options.hasMode( omSlowWaitClass ) ) summary_wait_class();
      if ( options.hasMode( omWeekdaySlowMap ) ) summary_slow_probes_to_dow();
      if ( options.hasMode( om24hSlowMap ) ) summary_slow_probes_to_daily();
    }
    if ( options.hasMode( omWeekdayMap ) ) summary_all_probes_to_dow();
    if ( options.hasMode( om24hMap ) ) summary_all_probes_to_daily();
    if ( options.hasMode( omErrors ) ) summary_curl_errors();
    if ( options.hasMode( omErrors ) ) summary_http_errors();
    if ( options.hasMode( omDailyTrail ) ) summary_daily_history();
    if ( options.hasMode( omGlobal ) ) summary_global_stats();
    if ( options.hasMode( omGlobal ) ) summary_abnormal();
  } else cout << "not enough samples - at least " << MINIMUM_PROBES << " probes required" << endl;
}