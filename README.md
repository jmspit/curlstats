# curlstats

Generates, parses and analyzes curl timing statistics against supported endpoints.

Requires curl to generate timed probes.

## Statistics

A good data set has enough statistics to capture most of the scenario's that might occur. For example

  - covering full 24 hour
  - covering all days of a week
  - covering multiple weeks

There is little point in probing very frequently, say 10 up to 30 seconds intervals are fine if one seeks the ability
to pinpoint recurring problems with adequate accuracy. Even higher values can be used to at least spot problems that
can be investigated with more detail.

## Anatomy of a round-trip

Client client/server request/response sequence involves several logical steps involving a number of network
round-trips. Curl provides timing information for each of these steps, and curlstats calls them WaitStates.

```
enum WaitClass {
  wcDNS,            /**< The target name needs be resolved to an IP address. */
  wcTCPHandshake,   /**< The TCP layer needs to handshake. */
  wcSSLHandshake,   /**< The TLS layer needs to handshake. */
  wcSendStart,      /**< The client may need time to prepare the data before sending. */
  wcWaitEnd,        /**< The client waited on the first response packet from the peer. */
  wcReceiveEnd      /**< The client spend time retrieving additional network packets. */
};
```

### DNS resolving (DNS)

If the endpoint is not given as an IP (4 or 6) address, the name must be resolved against DNS. DNS queries may
timeout and get retried (elsewhere) showing up as additional spikes in the probe count to response time distribution.
The default timeout value on Linux is 5 seconds (can be overridden in /etc/resolv.conf) so secondary spikes
around 5 seconds indicate DNS response (not resolving) problems. In case the resolve fails entirely, curl will error
CURLE_OPERATION_TIMEDOUT and the probe will not be included in the statistics.

There are known caveats when using ipv6 DNS queries on clients that support both ipv4 and ipv6 transparently
and let the OS decide through AF_UNSPEC what to try and use. The Linux gettadrrinfo call will attempt an ipv6 lookup if
the client supports it, and blocks on the query response (regardless if it is NXDOMAIN).

*The DNS wait class equates to DNS response time, which is the time the DNS server required to assemble the answer plus
the network time before the response hit the client.*

### TCP Handshake (TCP)

The TCP layer performs a handshake to establish a connection. This is almost entirely bound by network latency (in
absence of socket allocation problems). A TCP involves a three-way handshake, the handshake latency is
about 1.5 time the network latency - so a rough estimate of the network latency can be deduced from the TCP
handshake latency.


*The TCP wait class almost always equates to network latency. If the network latency is demonstrated to be fine
by other means, likely other causes are socket and/or CPU starvation problems on either server or client.*

### TLS Handshake (TLS)

If Transport Layer Security is used, TLS needs to handshake as well. The handshake involves establishing
trust on mutual identity, establishing a TLS protocol version and cipher to use, and negotiate a session
key which will be used to encrypt ensuing traffic.

*The TLS wait class equates to network latency and/or CPU starvation on either client or server. However, if
network latency is the dominant factor, that should also be visible in the TCP wait class. If TLS is high and
TCP is low during the same probe, check if the server is having trouble calculating through TLS handshakes.*

### Request send (REQ)

After the TLS handshake completed, the client sends its request data, if any.

*The REQ wait class should be in step with the network latency and network packet size.*

### Response received (RSP)

The first byte of the response is received - so this is the time the client spent waiting on the server's answer,
which is the time the server needed to construct the response plus the time it took before the network
delivered it to the client. If the TCP wait class is low in the same probes, it is safe to assume the time is dominated
by the server response.

*The REQ wait class equates to server response time if TCP wait class is insignificant in the same probes.*

### Response all data received (DAT)

The response may require more than one network packet to return. This is the time it took until the last one was
received.

*The DAT wait class equates to response data transfer. If DAT is high whilst TCP is low, the server may have trouble
presenting the data.*

# Interpreting the output

Any comments in the data (lines starting with a '#') are dumped to stdout as is. The curl_http_timing.sh
script dumps some probing information as comments to the data file, so the context of the output is clear. In the below
case, a 10 second interval sample against www.gnu.org  with a timeout of 10 seconds, from a client named
me.no-ip.org using a local network address, with any optional arguments.

```
#url      = https://www.gnu.org
#interval = 10
#timeout  = 10
#client   = me.no-ip.org
#IP       = 192.168.1.2
#args     =
```

The next sections shows options in effect

```
====================================================
Options in effect
====================================================
Slowness threshold                :   0.050 seconds
Response time distribution bucket :    0.01 seconds
repeating 24h bucket              :      30 minutes
Show trail of slow probes         :       0
```

so the output will consider probes exceeding 0.05 seconds slow, it will use 0.01s buckets in probe count to
response time distribution, use a 30 minute interval to bucket daily statistics, and not show a full trail
of slow probes. See command line examples below.

The QoS section show the % of probes within the specified (-d) slowness threshold. Additionally,
a histogram is shown against response time buckets (-b). The histogram shows that the majority of probes complete within
0.5 seconds, but there is a secondary peak at <  3.500s, 3 seconds above the peak <0.500s. The client producing this
output has a DNS timeout set to 3 seconds - and DNS timeouts are causing the second peak.

```
=============================================================
QoS
=============================================================
94.730% of probes return within 0.500s

probe count to response time distribution, bucket size 0.500s
   bucket    count  %probe
<  0.500s      5321  94.73
<  1.000s        73   1.30
<  1.500s         7   0.12
<  2.000s         2   0.04
<  2.500s         4   0.07
<  3.000s         6   0.11
<  3.500s       172   3.06
<  4.000s        13   0.23
<  4.500s         4   0.07
<  5.000s         3   0.05
<  5.500s         4   0.07
<  6.000s         2   0.04
<  6.500s         1   0.02
<  7.000s         1   0.02
<  7.500s         1   0.02
<  8.000s         1   0.02
<  9.500s         2   0.04
```


```
============================================================================
Global stats
============================================================================
first data point     : 2020-06-13 01:01:01
last  data point     : 2020-06-13 21:13:01
#probes              : 6742
#slow probes         : 162
average response time: 0.334s
estimate network RTT : 16.095ms
estimate #TLS rtrips : 8
class   %slow     min     max     avg  stddev  %rtrip             stability
  DNS   0.489   0.025   9.023   0.048   0.280   14.37         bad (  0.089)
  TCP   0.015   0.024   1.026   0.029   0.013    8.65        good (  1.859)
  TLS   0.237   0.135   0.788   0.188   0.042   56.21   excellent (  3.210)
  REQ   0.000   0.000   0.022   0.000   0.001    0.11         n/a (  0.047)
  RSP   1.661   0.026  16.865   0.069   0.341   20.58         bad (  0.075)
  DAT   0.000   0.000   0.021   0.000   0.001    0.08         n/a (  0.008)

```

## Command line examples

```
usage:
  -b seconds
     (real) 24h time distribution bucket
  -d minimum
     (real) specify a slow threshold filter in seconds
  -t
     include a full list of slow probes
  -T minutes
     (uint) 24 hour time bucket in minutes ( 0 < x <= 60 )

DNS = DNS name resolution
TCP = TCP handshake
TLS = TLS ('SSL') handshake
REQ = Request send lead time
RSP = Waiting for response
DAT = Waiting for response more data
```

parse data in curl.log, responses exceeding 0.5 seconds considered 'slow'.

```
$ cat curl.log | curlstats -d 0.5
```

parse data in curl.log, responses exceeding 0.5 seconds considered 'slow', use a 100ms/0.1s call-to-time distribution
bucket

```
$ cat curl.log | curlstats -d 0.5 -b 0.1
```

parse data in curl.log, responses exceeding 0.5 seconds considered 'slow', use a 30 minute 24h time bucket

```
$ cat curl.log | curlstats -d 0.5 -T 30
```

## Build

requires cmake and a contemporary C++ (c++17) compiler.

```
$ mkdir build
$ cd build
$ cmake .. -DCMAKE_BUILD_TYPE=RELEASE
$ make
```

