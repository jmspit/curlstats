#ifndef waitclass_h
#define waitclass_h

#include "qtystats.h"
#include <set>

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
bool operator<( const WaitClassOrder& o1, const WaitClassOrder& o2 );

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
  void addValue( WaitClass wc, double value );

  double avgResponse() const;

  size_t getNumItems() const { return endtransfer.items; }

  WaitClass most() const;

  double getNetworkRoundtrip() const {
    return connect.min / 1.5;
  }

  int getTLSRoundTrips() const;

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
string waitClass2String( WaitClass wc, bool describe = false );

#endif