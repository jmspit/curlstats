#ifndef waitclass_h
#define waitclass_h

#include "qtystats.h"

/**
 * Enumerate the roundtrip wait classes (steps).
 */
enum WaitClass {
  wcDNS,            /**< The target name needs be resolved to an IP address. */
  wcTCPHandshake,   /**< The TCP layer needs to handshake. */
  wcSSLHandshake,   /**< The TLS layer needs to handshake. */
  wcSendStart,      /**< The client may need time to prepare the data before sending. */
  wcWaitEnd,        /**< The client waited on the first response packet from the peer. */
  wcReceiveEnd,     /**< The client spend time retrieving additional network packets. */
  wcInvalid         /**< An init value. */
};

/**
 * Structure to order say std::set<Order> WaitClasses so WaitClass with highest
 * value can easily be retrieved.
 */
struct WaitClassOrder {
  WaitClassOrder() : wc(wcInvalid), value(0.0) {}
  WaitClassOrder( WaitClass wc, const double &v) : wc(wc), value(v) {}
  WaitClass wc;
  double value;
};

/**
 * Ordering for the Order struct.
 */
bool operator<( const WaitClassOrder& o1, const WaitClassOrder& o2 ) {
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

  WaitClass most() const {
    set<WaitClassOrder> ordered;
    ordered.insert( WaitClassOrder( wcDNS , namelookup.total ) );
    ordered.insert( { wcTCPHandshake , connect.total } );
    ordered.insert( { wcSSLHandshake , appconnect.total } );
    ordered.insert( { wcSendStart , pretransfer.total } );
    ordered.insert( { wcWaitEnd , starttransfer.total } );
    ordered.insert( { wcReceiveEnd , endtransfer.total } );
    return (*ordered.rbegin()).wc;
  }

  double getNetworkRoundtrip() const {
    return connect.min / 1.5;
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
 * Convert a WaitClass to an acronym (describe==false) or a full description (describe==true).
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

#endif