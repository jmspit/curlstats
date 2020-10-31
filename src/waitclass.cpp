#include "waitclass.h"

bool operator<( const WaitClassOrder& o1, const WaitClassOrder& o2 ) {
  return o1.value < o2.value;
}

void WaitClassStats::addValue( WaitClass wc, double value ) {
  size_t compare = 0;
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
      compare = endtransfer.items;
      if ( namelookup.items  != compare ||
            connect.items     != compare ||
            appconnect.items  != compare ||
            pretransfer.items != compare ||
            starttransfer.items != compare )
        throw std::runtime_error( "WaitClassStats::addValue WaitClassStats to item count inconsistency" );
      break;
    case wcInvalid:
      throw std::runtime_error("WaitClassStats::addValue invalid WaitClass wcInvalid");
      break;
  }
}

double WaitClassStats::avgResponse() const {
  if ( endtransfer.items > 0 ) return ( namelookup.total +
                                        connect.total +
                                        appconnect.total +
                                        pretransfer.total +
                                        starttransfer.total +
                                        endtransfer.total ) / (double)endtransfer.items;
  else return 0.0;
}

WaitClass WaitClassStats::most() const {
  set<WaitClassOrder> ordered;
  ordered.insert( WaitClassOrder( wcDNS , namelookup.total ) );
  ordered.insert( { wcTCPHandshake , connect.total } );
  ordered.insert( { wcSSLHandshake , appconnect.total } );
  ordered.insert( { wcSendStart , pretransfer.total } );
  ordered.insert( { wcWaitEnd , starttransfer.total } );
  ordered.insert( { wcReceiveEnd , endtransfer.total } );
  return (*ordered.rbegin()).wc;
}

int WaitClassStats::getTLSRoundTrips() const {
  return static_cast<int>( floor( appconnect.min / getNetworkRoundtrip() ) );
}

/**
 * Convert a WaitClass to an acronym (describe==false) or a full description (describe==true).
 */
string waitClass2String( WaitClass wc, bool describe ) {
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