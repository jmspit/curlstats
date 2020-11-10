#include "options.h"

#include <cstring>

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
  cout << "     default: " << DEFAULT_SLOW_DURATION << " seconds" << endl;
  cout << "  -f format" << endl;
  cout << "     (text) specify the output format, either 'text' or 'html'" << endl;
  cout << "     default: 'text'" << endl;
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
  cout << "  -W minutes" << endl;
  cout << "     (uint) 24 hour weekmap time bucket in minutes ( 0 < x <= 60 ). 60 Must be an integer multiple" << endl;
  cout << "     of this value" << endl;
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
    switch( getopt(argc, argv, "d:tb:T:W:p:o:f:h") )
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
          options.slow_threshold = stod( optarg );
        }
        catch ( const exception& e ) {
          cerr << "invalid -d value '" << optarg << "'" << endl;
          printHelp();
          return false;
        }
        continue;
      case 'f':
        if ( strncmp( optarg, "text", 4) == 0 ) 
          options.output_format = Options::OutputFormat::Text;
        else if ( strncmp( optarg, "html", 4) == 0 ) 
          options.output_format = Options::OutputFormat::HTML;
        else {
          cerr << "invalid output format (-f) '" << optarg << "'" << endl;
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
        if ( options.day_bucket <= 0 ||options.day_bucket > 60 ) {
          cerr << "-T value must be > 0 and <= 60 '" << optarg << "'" << endl;
          printHelp();
          return false;
        }
        continue;
      case 'W':
        try {
          options.weekmap_bucket = stoi( optarg );
        }
        catch ( const exception& e ) {
          cerr << "invalid -w value '" << optarg << "'" << endl;
          printHelp();
          return false;
        }
        if ( 60 % options.weekmap_bucket != 0 ) {
          cerr << "60 must be an integer multiple of the -w value '" << optarg << "'" << endl;
          printHelp();
          return false;
        }
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
  if ( options.output_format == Options::OutputFormat::HTML && options.output_mode != omAll ) {
    cerr << "cannot specify -o mode with -f html unless mode is 'all'" << endl;
    return false;
  }
  return true;
}


bool Options::hasMode( OutputMode mode ) const {
  unsigned long t = static_cast<unsigned long>(output_mode);
  unsigned long m = static_cast<unsigned long>(mode);
  return ((t & m) || (t & omAll));
}