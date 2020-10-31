#include "options.h"

Options options;

void printHelp() {
  cout << endl;
  cout << "curlstats reads from standard input" << endl;
  cout << "see https://github.com/jmspit/curlstats" << endl;
  cout << endl;
  cout << "usage: " << endl;
  cout << "  -b buckets" << endl;
  cout << "     (uint) maximum number of buckets per histogram" << endl;
  cout << "     default: " << DEFAULT_MAX_BUCKETS << " buckets" << endl;
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
}

/**
 * Parse command line arguments.
 */
bool parseArgs( int argc, char* argv[], Options &options ) {
  string mode = "";
  for(;;)
  {
    switch( getopt(argc, argv, "d:tb:T:p:o:h") )
    {
      case 'b':
        try {
          options.histo_max_buckets = stod( optarg );
        }
        catch ( const exception& e ) {
          cerr << "invalid -b value '" << optarg << "'" << endl;
          printHelp();
          return false;
        }
        continue;
      case 'd':
        try {
          options.min_duration = stod( optarg );
        }
        catch ( const exception& e ) {
          cerr << "invalid -d value '" << optarg << "'" << endl;
          printHelp();
          return false;
        }
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
          printHelp();
          return false;
        }
        continue;
      case 'p':
        try {
          options.histo_min_pct = stod( optarg );
        }
        catch ( const exception& e ) {
          cerr << "invalid -p value '" << optarg << "'" << endl;
          printHelp();
          return false;
        }
        if ( options.histo_min_pct > 10.0 ) options.histo_min_pct = DEFAULT_HISTO_MIN_PCT;
        continue;
      case 'T':
        try {
          options.day_bucket = stoi( optarg );
        }
        catch ( const exception& e ) {
          cerr << "invalid -T value '" << optarg << "'" << endl;
          printHelp();
          return false;
        }
        if ( options.day_bucket <= 0 ) options.day_bucket = DEFAULT_DAY_BUCKET;
        if ( options.day_bucket > 60 ) options.day_bucket = DEFAULT_DAY_BUCKET;
        continue;
      case '?':
      case 'h':
      default :
        printHelp();
        return false;
      case -1:
        break;
    }
    break;
  }
  if ( options.output_mode == omNone ) options.output_mode = omAll;
  return true;
}
