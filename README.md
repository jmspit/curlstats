- [curlstats](#curlstats)
  * [Examples](#examples)
    + [Some command line examples](#some-command-line-examples)
  * [Build](#build)
  * [Statistics](#statistics)
  * [Anatomy of a round-trip](#anatomy-of-a-round-trip)
    + [DNS resolving (DNS)](#dns-resolving--dns-)
    + [TCP Handshake (TCP)](#tcp-handshake--tcp-)
    + [TLS Handshake (TLS)](#tls-handshake--tls-)
    + [Request send (REQ)](#request-send--req-)
    + [Response received (RSP)](#response-received--rsp-)
    + [Response all data received (DAT)](#response-all-data-received--dat-)
  * [Interpreting the output](#interpreting-the-output)
    + [Comments from data](#comments-from-data)
    + [Options in effect](#options-in-effect)
    + [QoS](#qos)
    + [Global statistics](#global-statistics)

# curlstats

Generates, parses and analyzes curl timing statistics against supported endpoints.

Requires bash and curl to generate timed probes. Probes are generated by [curl_http_timing.sh](src/curl_http_timing.sh)
script along with a configuration file specifying the endpoint to be called and options to be used. The
[curl_config.example](src/curl_config.example) file can be used as an example. Be sure to leave the `write-out`
configuration option intact as the parsing by curlstats depends on that format.

## Examples

The generate statistics, copy and edit the [curl_config.example](src/curl_config.example) script

```
# mandatory - must specify
url https://www.gnu.org
request GET

# customize
connect-timeout 10
max-time 10
header "Accept: text/html"

# leave alone
silent
output /dev/null
write-out "%{http_connect};%{http_code};%{ssl_verify_result};%{time_total};%{time_namelookup};%{time_connect};%{time_appconnect};%{time_pretransfer};%{time_redirect};%{time_starttransfer}"
```

using the config file the probing can be started, say with a 10 second interval

```
$ src/curl_http_timing.sh src/curl_config.example 10 | tee data/gnu.org.dat
# probing resumes = 2020-06-15 00:10:03
# client FQDN     = snaak.no-ip.org
# client kernel   = 5.4.38-gentoo
# client OS       = Gentoo/Linux
# client IP       = 192.168.178.2
# curl            = curl 7.69.1 (x86_64-pc-linux-gnu) libcurl/7.69.1 OpenSSL/1.1.1g zlib/1.2.11
# curl            = Release-Date: 2020-03-11
# curl            = Protocols: dict file ftp ftps http https imap imaps ldap ldaps pop3 pop3s rtsp smtp smtps tftp
# curl            = Features: AsynchDNS HTTPS-proxy IPv6 Largefile libz NTLM SSL TLS-SRP UnixSockets
# curl config     = url https://www.gnu.org
# curl config     = request GET
# curl config     = connect-timeout 10
# curl config     = max-time 10
# curl config     = header "Accept: text/html"
# curl config     = silent
# YYYY HH:MI:SS ; curl error ; http connect code ; http response code ; ssl verify result ; total_time ; dns lookup ; tcp handshake ; ssl handshake ; dns lookup+tcp+ssl handshake; redirect time; time to first byte sent
2020-06-15 00:10:03;0;000;200;0;0.744491;0.000352;0.092665;0.284233;0.284257;0.000000;0.377898
2020-06-15 00:10:14;0;000;200;0;0.736170;0.000621;0.091463;0.280857;0.280884;0.000000;0.374935
2020-06-15 00:10:25;0;000;200;0;0.713518;0.000461;0.088908;0.273343;0.273368;0.000000;0.363101
2020-06-15 00:10:35;0;000;200;0;0.690696;0.000499;0.085627;0.263239;0.263264;0.000000;0.350330
2020-06-15 00:10:46;0;000;200;0;0.746183;0.000426;0.092519;0.284151;0.284176;0.000000;0.378483
2020-06-15 00:10:57;0;000;200;0;0.739484;0.000386;0.091974;0.280912;0.280955;0.000000;0.373567
2020-06-15 00:11:07;0;000;200;0;0.743964;0.000484;0.092424;0.283790;0.283815;0.000000;0.377248
2020-06-15 00:11:18;0;000;200;0;0.717048;0.000441;0.088519;0.273302;0.273336;0.000000;0.366548
2020-06-15 00:11:29;0;000;200;0;0.739107;0.000468;0.091706;0.281908;0.281933;0.000000;0.375991
2020-06-15 00:11:40;0;000;200;0;0.721408;0.000438;0.089917;0.276119;0.276145;0.000000;0.366683
```

The data (in the above case tee-ed to data/gnu.org.dat) can be parsed and analyzed by curlstats:

```
cat data/gnu.org.dat | curlstats
```

curlstats provides several command line options to tweak its behavior

```
usage:
  -b seconds
     (real) response time histogram bucket in seconds
     default: 0.2
  -d minimum
     (real) specify a slow threshold filter in seconds
     default: 1
  -p minimum
     only show histogram buckets with % total probes larger than this value
     default: 0
  -t
     include a full list of slow probes
     default: false
  -T minutes
     (uint) 24 hour time bucket in minutes ( 0 < x <= 60 )
     default: 30
```

### Some command line examples

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

Isolate a single date, responses exceeding 0.5 seconds considered 'slow', use a 5 minute 24h time bucket
and a 0.05s histogram bucket

```
$ grep '^2020-04-20' curl.log | curlstats -d 0.5 -T 5 -b 0.05
```

## Build

requires cmake and c++-17 capable C++ compiler stack.

```
$ git clone git@github.com:jmspit/curlstats.git
$ cd curlstats
$ mkdir build
$ cd build
$ cmake .. -DCMAKE_BUILD_TYPE=RELEASE
$ make
```

will produce `curlstats` in the build directory.

## Statistics

To analyze performance (or the lack thereof), a good data set is key. A good data set

  - has enough probes to capture most of the scenario's that might occur. For example
    - covering full 24 hour
    - covering all days of a week
    - covering multiple weeks
  - is isolated from effects that impact but do not relate to the analysis, such as sampling from a unstable source.

There is little point in probing very frequently, say 10 up to 30 seconds intervals are fine if one seeks the ability
to pinpoint recurring problems with adequate accuracy. Even higher values can be used to at least spot problems that
can be investigated with more detail.

Often, IT related problems correlate with days-of-week and time-of-day as they are patterns in human behavior
(work, sleep, weekend, backups, maintenance, ...). If problem effects are seen but its cause hard to locate
it can be essential to pinpoint the exact times at which they occur. Or, conversely, by looking for such
patterns they are not to be seen - meaning the problem is truly consistent (even if it is consistently random).

## Anatomy of a round-trip

Client client/server request/response sequence involves several logical steps involving a number of network
round-trips. Curl provides timing information for each of these steps, and curlstats calls each step a WaitClass.

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
timeout and get retried (against another DNS server) showing up as additional spikes in the probe count to response
time distribution. The default timeout value on Linux is 5 seconds (can be overridden in /etc/resolv.conf). In case the
resolve fails entirely (no DNS server delivered a response), curl will error CURLE_OPERATION_TIMEDOUT and the
probe will not be included in the statistics.

Note that receiving NXDOMAIN *is* a response - just not one that is very useful and will lead to a retry against
another DNS server (if more than one is configured).

There are known caveats when using ipv6 DNS queries on clients that support both ipv4 and ipv6 transparently. Good
developers will not make their software depend on the IP address family used (AF_INET or AF_INET6) but specify AF_UNSPEC
instead, letting the OS decide what to use. If the OS has AF_INET6 enabled, and the developer uses the
prescribed getaddrinfo call that supersedes gethostbyname, the OS *will* attempt to use ipv6, and thus send
two DNS queries - one for an A (ipv4) and one for an AAA record (ipv6) *from the same source port*. This means the query
response may lead to two UDP packets being returned on the same port - and intermediate networking software may not
expect that and block either one of the two, leading the client to timeout. In fact, iptables conntrack software
mishandled that and was patched early 2019, but the mistake seems to be widespread as it is still seen in the wild.

*The DNS wait class equates to DNS response time, which is the time the DNS server required to assemble the answer plus
the network time before the response hit the client. In general, DNS resolution performance is so critical to IT
operation that it can be expected to be near the transport RTT, or in the case of DNS caching, close to local memory
access latency. DNS timeouts should not occur at all.*

### TCP Handshake (TCP)

The TCP layer performs a handshake to establish a connection. This is almost entirely bound by network latency (in
absence of socket allocation problems). A TCP involves a three-way handshake, the handshake (3 sends) latency is
1.5 times the network latency (2 sends) - so a rough estimate of the network latency can be deduced from the TCP
handshake latency.


*The TCP wait class almost always equates to network latency. If the network latency is demonstrated to be fine
by other means, likely causes for higher TCP are socket and/or CPU starvation problems on either server or client.*

### TLS Handshake (TLS)

If Transport Layer Security is used, TLS needs to handshake as well. The handshake involves establishing
trust on peer identity, establishing a TLS protocol version and cipher to use, and negotiate a session
key which will be used to encrypt ensuing traffic.

*The TLS wait class equates to network latency and/or CPU starvation on either client or server, although this no longer
is a common problem with the advent of faster TLS handshakes. However, if
network latency is the dominant factor, that should also be visible in the TCP wait class. If TLS is high and
TCP is low during the same probe, check if the server is having trouble producing TLS handshakes.*

### Request send (REQ)

After the TLS handshake completed, the client sends its request data, if any.

*The REQ wait class is the lead time before sending the data to the server, it may include preparation of such data,
but is typically (very) low.*

### Response received (RSP)

The first byte of the response is received - so this is the time the client spent waiting on the server's answer,
which is the time the server needed to construct the response plus the time it took before the network
delivered it to the client. If the TCP wait class is low in the same probes, it is safe to assume the time is dominated
by the server response.

*The REQ wait class equates to server response time if TCP wait class is insignificant in the same probes.*

### Response all data received (DAT)

The response may require more than one network packet to return. This is the time between arrival of the first and
last packet of the response.

*The DAT wait class equates to response data transfer. If DAT is high whilst TCP is low, the server may have trouble
presenting the data or the transport speed is the limiting factor.*

## Interpreting the output

curlstats produces lots of structured text on the standard output.

### Comments from data

Any comments in the data (lines starting with a '#') are dumped to stdout as is. The
[curl_http_timing.sh](src/curl_http_timing.sh) script dumps some probing information as comments to the probe data file,
so the context of the output is clear.

```
# probing resumes = 2020-06-15 01:20:07
# client FQDN     = snaak.no-ip.org
# client kernel   = 5.4.38-gentoo
# client OS       = Gentoo/Linux
# client IP       = 192.168.178.2
# curl            = curl 7.69.1 (x86_64-pc-linux-gnu) libcurl/7.69.1 OpenSSL/1.1.1g zlib/1.2.11
# curl            = Release-Date: 2020-03-11
# curl            = Protocols: dict file ftp ftps http https imap imaps ldap ldaps pop3 pop3s rtsp smtp smtps tftp
# curl            = Features: AsynchDNS HTTPS-proxy IPv6 Largefile libz NTLM SSL TLS-SRP UnixSockets
# curl config     = url https://www.gnu.org
# curl config     = request GET
# curl config     = connect-timeout 10
# curl config     = max-time 10
# curl config     = header "Accept: text/html"
# curl config     = silent
# YYYY HH:MI:SS ; curl error ; http connect code ; http response code ; ssl verify result ; total_time ; dns lookup ; tcp handshake ; ssl handshake ; dns lookup+tcp+ssl handshake; redirect time; time to first byte sent
```

### Options in effect

```
================================================
Options in effect
================================================
Slowness threshold             :   0.100 seconds
Response time histogram bucket :   0.010 seconds
repeating 24h histogram bucket :      30 minutes
histogram minimum display pct  :   0.000 %
Show trail of slow probes      :       0
```

so the output will consider probes exceeding 0.05 seconds slow, it will use 0.01s buckets in probe count to
response time distribution, use a 30 minute interval to bucket daily statistics, and not show a full trail
of slow probes. See command line examples below.

### QoS

```
======================================
QoS
======================================
99.581% of probes return within 0.100s
...
```

The QoS section contains a number of time distribution histograms, with a bucket size configured through `-b`.
The first histogram is for the request-response time, subsequent diagrams for each of the wait classes.

The histograms will not show buckets with zero probes. Also, buckets with %probe lower than the `-p` option
will be omitted.

Buckets are taken as the ceil(ing) of a given value with respect to the bucket size. So a bucket size `-b 0.1`
will bucket the value 0.302 to `< 0.400` and 0.178 to `< 0.200`.

The histograms may show secondary peaks, which may be caused by timeouts, the effect of cache hits and misses,
different storage tiers accessed and so on. For example, this histogram shows a problem with DNS resolving:

```
probe count to DNS wait time distribution, bucket size 0.05s
   bucket     count %probe
<  0.050s     12446  94.22
<  0.100s       219   1.66
<  0.150s        14   0.11
<  3.050s        27   0.20
<  3.100s       444   3.36
```

on a machine with the DNS timeout set to 3 seconds, roughly 3.5% of DNS resolving requests time out (but do succeed
eventually as the curl error is CURLE_OK).

The QoS section show the % of probes within the specified (-d) slowness threshold. Additionally,
a histogram is shown against response time buckets (-b). The histogram shows that the majority of probes complete within
0.5 seconds, but there is a secondary peak at <  3.500s, 3 seconds above the peak <0.500s. The client producing this
output has a DNS timeout set to 3 seconds - and DNS timeouts are causing the second peak.

### Global statistics

```
============================================================================
Global stats
============================================================================
first probe          : 2020-06-13 01:01:01
last  probe          : 2020-06-14 08:46:34
#probes              : 10587
#slow probes         : 242
%slow probes         : 2.286
average response time: 0.331s
optimal response time: 0.208s
estimate network RTT : 19.090ms
class   %slow     min     max     avg  stddev  %rtrip             stability
  DNS   0.472   0.025   9.023   0.047   0.281   14.36         bad (  0.088)
  TCP   0.009   0.023   1.026   0.029   0.011    8.66        good (  2.175)
  TLS   0.170   0.134   0.788   0.184   0.040   55.54   excellent (  3.349)
  REQ   0.000   0.000   0.022   0.000   0.001    0.11         n/a (  0.051)
  RSP   1.634   0.026  16.865   0.070   0.328   21.25         bad (  0.078)
  DAT   0.000   0.000   0.021   0.000   0.001    0.08         n/a (  0.009)
```

