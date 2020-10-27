#ifndef curl_h
#define curl_h

#include "datetime.h"
#include "util.h"
#include "waitclass.h"

/**
 * Curl statistics line.
 */
struct CURL {
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
      case wcInvalid :
        throw std::runtime_error( "CURL::getWaitClassDuration invalid WaitClass wcInvalid" );
    }
    return 0.0;
  };

  double getWaitClassPct( WaitClass wc ) {
    return getWaitClassDuration( wc ) / total_time*100.0;
  };


  WaitClass getDominantWaitClass() {
    set<WaitClassOrder> ordered;
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
    ss << datetime.asString() << " ";
    if ( curl_error != 0 ) {
	  ss << curlError2String(curl_error) << " ";
	} else {
      ss << "roundtrip : " << FIXED3 << total_time << "s most " << waitClass2String( getDominantWaitClass() ) << " ";
      ss << fixed << FIXEDPCT << getWaitClassPct( getDominantWaitClass() ) << "% | ";
      ss << waitClass2String( wcDNS ) << "=" << FIXED3 << getWaitClassDuration( wcDNS ) << "s, ";
      ss << waitClass2String( wcTCPHandshake ) << "=" << FIXED3 << getWaitClassDuration( wcTCPHandshake ) << "s, ";
      ss << waitClass2String( wcSSLHandshake ) << "=" << FIXED3 << getWaitClassDuration( wcSSLHandshake ) << "s, ";
      ss << waitClass2String( wcSendStart ) << "=" << FIXED3 << getWaitClassDuration( wcSendStart ) << "s, ";
      ss << waitClass2String( wcWaitEnd )  << "=" << FIXED3 << getWaitClassDuration( wcWaitEnd ) << "s, ";
      ss << waitClass2String( wcReceiveEnd ) << "=" << FIXED3 << getWaitClassDuration( wcReceiveEnd ) << "s";
      ss << " " << HTTPCode2String( http_code );
    }
    return ss.str();
  };

  bool parse( const string &s ) {
    try {
      vector<string> tokens = split( s, ';' );
      if ( tokens.size() >= 12 ) {
        bool ok = datetime.parse( tokens[0] );
        if ( ! ok ) return false;
        curl_error         = stoi(tokens[1]);
        http_code          = stoi(tokens[3]);
        total_time         = stod(tokens[5]);
        time_namelookup    = stod(tokens[6]);
        time_connect       = stod(tokens[7]);
        time_appconnect    = stod(tokens[8]);
        time_pretransfer   = stod(tokens[9]);
        time_starttransfer = stod(tokens[11]);
        if ( tokens.size() == 14 ) {
          size_upload        = stoul(tokens[12]);
          size_download      = stoul(tokens[13]);
        } else {
          size_upload   = 0;
          size_download = 0;
        }
      } else return false;
      return true;
    }
    catch ( const std::runtime_error &e ) {
      return false;
    }
  }
};

#endif