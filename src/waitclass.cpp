#include "waitclass.h"

bool operator<( const WaitClassOrder& o1, const WaitClassOrder& o2 ) {
  return o1.value < o2.value;
}

void ProbeStats::addValues( double vnamelookup, 
                            double vconnect,
                            double vappconnect,
                            double vpretransfer,
                            double vstarttransfer,
                            double vendtransfer ) {
  namelookup.addValue( vnamelookup );                              
  connect.addValue( vconnect );                              
  appconnect.addValue( vappconnect );                              
  pretransfer.addValue( vpretransfer );                              
  starttransfer.addValue( vstarttransfer );                              
  endtransfer.addValue( vendtransfer );     
  probe.addValue( vnamelookup + vconnect + vappconnect + vpretransfer + vstarttransfer + vendtransfer );
}

WaitClass ProbeStats::most() const {
  set<WaitClassOrder> ordered;
  ordered.insert( WaitClassOrder( wcDNS , namelookup.total ) );
  ordered.insert( { wcTCPHandshake , connect.total } );
  ordered.insert( { wcSSLHandshake , appconnect.total } );
  ordered.insert( { wcSendStart , pretransfer.total } );
  ordered.insert( { wcWaitEnd , starttransfer.total } );
  ordered.insert( { wcReceiveEnd , endtransfer.total } );
  return (*ordered.rbegin()).wc;
}

int ProbeStats::getTLSRoundTrips() const {
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