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

  /** 
   * Default constructor.
   */
  WaitClassOrder() : wc(wcInvalid), value(0.0) {}

  /**
   * Construct a WaitClass-value pair.
   * @param wc The WiatClass.
   * @param v The value.
   */
  WaitClassOrder( WaitClass wc, const double &v) : wc(wc), value(v) {}

  /** The WaitClass */
  WaitClass wc;

  /** The value */
  double value;
};

/**
 * Ordering for the Order struct.
 * @param o1 The left key.
 * @param o2 The right key.
 */
bool operator<( const WaitClassOrder& o1, const WaitClassOrder& o2 );

/**
 * Timing statistics for a single probe.
 */
struct ProbeStats {
  /** Statistics for wcDNS */
  QtyStats namelookup;

  /** Statistics for wcTCPHandshake */
  QtyStats connect;

  /** Statistics for wcSSLHandshake */
  QtyStats appconnect;

  /** Statistics for wcSendStart */
  QtyStats pretransfer;

  /** Statistics for wcWaitEnd */
  QtyStats starttransfer;

  /** Statistics for wcReceiveEnd */
  QtyStats endtransfer;

  /** Statistics for the entire probe */
  QtyStats probe;

  /**
   * Add a value for the WaitClass. For each probe, each of the WaitClasses need to have added a value.
   * @param wc The WaitClass to add the value for.
   * @param value The value to add.
   */
  /* void addValue( WaitClass wc, double value ); */

  void addValues( double vnamelookup, 
                  double vconnect,
                  double vappconnect,
                  double vpretransfer,
                  double vstarttransfer,
                  double vendtransfer );

  /**
   * Return the number of probes added.
   * @return The number of probes added.
   */
  size_t getNumItems() const { return endtransfer.items; }

  /**
   * Return the WaitClass that contributes most.
   * @return The WaitClass contributing most.
   */
  WaitClass most() const;

  /**
   * Return the infrastructure RTT as experienced by the TCP handshake.
   * @return The RTT.
   */
  double getNetworkRoundtrip() const {
    return connect.min / 1.5;
  }

  /**
   * Return the estimate TLS round trips based on TCP handskae speed.
   * @return The estimate TLS roundtrips.
   */
  int getTLSRoundTrips() const;

  /**
   * Return the idael repsonse time, consrtcuted by adding the minimum values seen for each WaitClass.
   * @return The ideal response time.
   */
  double getIdealResponse() const {
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
 * @param wc The WaitClass to describe.
 * @param describe If false, return a concise description.
 * @return A description of the WaitClass.
 */
string waitClass2String( WaitClass wc, bool describe = false );

#endif