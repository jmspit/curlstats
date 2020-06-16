- [curlstats](#curlstats)
  * [Examples](#examples)
    + [Some command line examples](#some-command-line-examples)
  * [Build](#build)
  * [Statistics](#statistics)
  * [Anatomy of a round-trip](#anatomy-of-a-round-trip)
    + [DNS resolving (DNS)](#dns-resolving-dns)
    + [TCP Handshake (TCP)](#tcp-handshake-tcp)
    + [TLS Handshake (TLS)](#tls-handshake-tls)
    + [Request send (REQ)](#request-send-req)
    + [Response received (RSP)](#response-received-rsp)
    + [Response all data received (DAT)](#response-all-data-received-dat)
  * [Interpreting the output](#interpreting-the-output)
    + [Comments from data](#comments-from-data)
    + [Curlstats options in effect](#curlstats-options-in-effect)
    + [QoS / Histograms](#qos--histograms)
    + [Slow probes to wait class distribution](#slow-probes-to-wait-class-distribution)
    + [24 hour probe map](#24-hour-probe-map)
    + [Global statistics](#global-statistics)

# curlstats

Generates, parses and analyzes curl timing statistics against supported endpoints. The
[curl_http_timing.sh](src/curl_http_timing.sh) generates probes from the host it is running on. `curlstats` can be
presented that data for processing (and can be run anywhere).

The [curl_http_timing.sh](src/curl_http_timing.sh) script requires bash and curl to generate timed probes. The probed
endpoint is specified through a configuration file. The [curl_config.example](src/curl_config.example) file can be
taken as an example. Be sure to leave the `write-out` configuration option intact as the parsing by curlstats depends
on that format.

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
$ curlstats -h
reads from standard input
see https://github.com/jmspit/curlstats

usage:
  -b seconds
     (real) response time histogram bucket in seconds (-o histo)
     default: 0.2
  -d threshold
     (real) specify a slow threshold in seconds
     default: 1
  -o option
     limit the output, multiple options can be given by repeating -o
       24hmap     : show 24h map of all probes
       24hslowmap : show 24h map of slow probes
       comments   : show comments from input
       daytrail   : show daily history of all probes
       errors     : show errors
       global     : show global stats
       histo      : show wait class histograms
       options    : show options in effect
       slowtrail  : trail of slow probes
       slowwait   : show waits class distribution of slow probes
       wdmap      : show weekday map of all probes
       wdslowmap  : show weekday map of slow probes
     default: 'all'
  -p threshold
     only show histogram buckets with % total probes larger than this value (-o histo)
     default: 0
  -T minutes
     (uint) 24 hour time bucket in minutes ( 0 < x <= 60 ) (-o 24hmap, 24hslowmap)
     default: 30
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
another domain suffix or another DNS server (depending on the client resolving configuration).

There are known caveats when using ipv6 DNS queries on clients that support both ipv4 and ipv6 transparently. Good
software will not depend on the IP address family used (AF_INET or AF_INET6) but specify AF_UNSPEC
instead, letting the OS decide what to use. If the OS has AF_INET6 enabled, and the developer uses the
prescribed getaddrinfo call that supersedes gethostbyname, the OS *will* attempt to use ipv6, and thus send
two DNS queries - one for an A (ipv4) and one for an AAA record (ipv6) *from the same source port*. This means the query
response may lead to two UDP packets being returned on the same port - and intermediate networking software may not
expect that and block either one of the two, leading the client to timeout. In fact, iptables conntrack software
mishandled that and was patched early 2019, but the mistake seems to be widespread as it is still seen in the wild. Use
the `-4` option of curl (specify a `4` in the config file) to prevent ipv6 and ipv6 DNS queries altogether.

*The DNS wait class equates to DNS response time, which is the time the DNS server required to assemble the answer plus
the network time before the response hit the client. In general, DNS resolution performance is so critical to IT
operation that it can be expected to be near the transport RTT, or in the case of DNS caching, close to local memory
access latency. DNS timeouts should not occur at all.*

### TCP Handshake (TCP)

The TCP layer performs a handshake to establish a connection. This is almost entirely bound by network latency (in
absence of socket allocation problems). TCP uses a three-way handshake, the handshake (3 sends) latency is
1.5 times the network latency (2 sends) - so a rough estimate of the network latency can be deduced from the TCP
handshake latency.


*The TCP wait class almost always equates to network latency. If the network latency is demonstrated to be fine
by other means, likely causes for higher TCP are socket and/or CPU starvation problems on either server or client.*

### TLS Handshake (TLS)

If Transport Layer Security is used, TLS needs to handshake as well. The handshake involves establishing
trust on peer identity, negotiating a TLS protocol version and cipher to use, and negotiate a session
key which will be used to encrypt ensuing traffic.

*The TLS wait class equates to network latency and/or CPU starvation on either client or server, although this no longer
is a common problem with the advent of faster TLS handshakes. However, if
network latency is the dominant factor, that should also be visible in the TCP wait class. If TLS is high and
TCP is low during the same probes, check if the server is having trouble producing TLS handshakes.*

### Request send (REQ)

After the TLS handshake completed, the client sends its request data, if any.

*The REQ wait class is the lead time before sending the data to the server, it may include preparation of such data,
but is typically (very) low.*

### Response received (RSP)

The first byte of the response is received, so this is the time the client spent waiting on the server,
the time the server needed to construct the response plus the time it took before the transport
delivered it to the client. If the TCP wait class is low in the same probes, it is safe to assume the time is dominated
by the server response.

*The REQ wait class equates to server response time if TCP wait class is insignificant in the same probes.*

### Response all data received (DAT)

The response may require more than one network packet to return. This is the time between arrival of the first and
last packet of the response.

*The DAT wait class equates to response data transfer. If DAT is high whilst TCP is low, the server may have trouble
presenting the data or the transport speed is the limiting factor on a large response message.*


## Output statistics

curlstats produces lots of structured text on the standard output. The `-o option` can be specified
more than once to select one or more specific representations and interpratations of the probes - or just
specify `-o all`, which is just a silly way to specify no '-o` option at all, as 'all' is also the default.

Note that curlstats will avoid compute on data that would not be relevant to the output selected, so one saves
the planet by limiting the options to what needs be seen.

As curlstats reads from standard in, culrstat expects the user to apply filters, say limit to a particular date

```
$ grep '^2020-06-16' data/gnu.org.dat | curlstats -o global
```

or a certain hour in more detail

```
$ grep '^2020-06-16 21' data/gnu.org.dat | curlstats -o slowtrail -d -0.2
```

### Comments from data

A data source generated with [curl_http_timing.sh](src/curl_http_timing.sh) will include comments that describe
the source the probes were taken from, the curl options in effect - which in turn describe the endpoint.

To extract the comments from the data:

```
$ cat data/gnu.org.dat | curlstats -o comments
```


```
========================================================================================================================
Comments
========================================================================================================================
# probing resumes = 2020-06-16 21:51:14
# interval        = 30s
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
```

Note that the curl config options from the config file are included, except the 'write-out' parameter.

### Curlstats options in effect

To output the options in effect when interpreting the probes.

```
$ cat data/gnu.org.dat | curlstats -o options

========================================================================================================================
Options in effect
========================================================================================================================
Slowness threshold             :   1.000 seconds
Response time histogram bucket :   0.200 seconds
repeating 24h histogram bucket :      30 minutes
histogram minimum display pct  :   0.000 %
```

The output will consider probes exceeding 1 seconds slow, it will use 0.2s buckets in probe count to
response time distribution, use a 30 minute interval to bucket daily statistics, and not show histogram entries that
contribute less than this value to the total.

### QoS / Histograms

Quality of Service depends on the value of the average as well as its standard deviation. Histograms reveal more about
the make-up of the standard deviation.

To output histograms on the total round trip time as well as individual wait classes.

```
$ cat data/gnu.org.dat | curlstats -o histo

========================================================================================================================
QoS
========================================================================================================================
98.039% of probes return within 1.000s

probe count to response time distribution, bucket size 0.200s
   bucket     count %probe
<  0.600s        51  50.00
<  0.800s        34  33.33
<  1.000s        15  14.71
<  1.200s         2   1.96

probe count to DNS wait time distribution, bucket size 0.20s
   bucket     count %probe
<  0.200s       102 100.00

probe count to TCP wait time distribution, bucket size 0.20s
   bucket     count %probe
<  0.200s       101  99.02
<  0.400s         1   0.98

probe count to TLS wait time distribution, bucket size 0.20s
   bucket     count %probe
<  0.200s        53  51.96
<  0.400s        49  48.04

probe count to REQ wait time distribution, bucket size 0.20s
   bucket     count %probe
<  0.200s       102 100.00

probe count to RSP wait time distribution, bucket size 0.20s
   bucket     count %probe
<  0.200s       101  99.02
<  0.400s         1   0.98

probe count to DAT wait time distribution, bucket size 0.20s
   bucket     count %probe
<  0.200s        55  53.92
<  0.400s        47  46.08
```

The QoS section contains a number of time distribution histograms, with a bucket size configured through `-b`.
The first histogram is for the request-response time, subsequent diagrams for each of the wait classes.

The histograms will not show buckets with zero probes. Also, buckets with %probe lower than the `-p` option
(defaults to 0) will be omitted.

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

### Slow probes to wait class distribution

Outputs a histogram of slow probes to wait classes.

```
$ cat data/gnu.org.dat | curlstats -o slowwait

========================================================================================================================
Slow (<1.000s) probe to wait-class distribution
========================================================================================================================
class   #probes  %blame     min     max     avg   stdev
  TCP         6   2.82%   1.054   3.274   1.670   0.749
  TLS       292  89.39%   0.860   3.024   1.088   0.330
  RSP        18   5.09%   1.003   1.033   1.005   0.007
```

The `%blame` column is the percentage of slow time in this class to the slow time of all slow probes - or in short,
the wait class contributing most to the slowness.

### 24 hour probe map

Output a 24h hour map bucketed over only the time-of-day part of probes, useful in detecting
recurring daily patterns. The `-T` option may be used to adjust the default bucket size of 30 minutes.

```
$ cat data/snaak_knmi.dat.old | curlstats -o 24hmap -T 60

================================================================================================================================================================================
All probes to daily time bucket distribution
================================================================================================================================================================================
truncated to 60 minute buckets, per waitclass min max avg
hh:mm  %slow     avg  most ----------DNS---------- ----------TCP---------- ----------TLS---------- ----------REQ---------- ----------RSP---------- ----------DAT----------
00:00   0.00   0.029   TLS   0.000   0.024   0.001   0.004   0.492   0.007   0.015   0.155   0.017   0.000   0.000   0.000   0.003   0.051   0.005   0.000   0.002   0.000
01:00   0.00   0.027   TLS   0.000   0.015   0.001   0.004   0.411   0.005   0.016   0.069   0.017   0.000   0.000   0.000   0.003   0.100   0.005   0.000   0.003   0.000
02:00   0.09   0.029   TLS   0.000   0.031   0.001   0.004   0.343   0.005   0.015   1.026   0.018   0.000   0.000   0.000   0.003   0.272   0.005   0.000   0.001   0.000
03:00   0.09   0.029   TLS   0.000   0.019   0.001   0.004   0.459   0.006   0.015   0.860   0.018   0.000   0.000   0.000   0.003   0.021   0.005   0.000   0.005   0.000
04:00   0.09   0.030   TLS   0.000   0.031   0.001   0.004   0.480   0.006   0.015   0.157   0.017   0.000   0.000   0.000   0.003   1.004   0.006   0.000   0.000   0.000
05:00   0.00   0.028   TLS   0.000   0.005   0.001   0.004   0.437   0.005   0.015   0.036   0.017   0.000   0.000   0.000   0.003   0.036   0.005   0.000   0.000   0.000
06:00   0.19   0.041   TLS   0.000   0.019   0.001   0.004   1.506   0.016   0.015   1.019   0.020   0.000   0.000   0.000   0.003   0.021   0.005   0.000   0.001   0.000
07:00   0.19   0.032   TLS   0.000   0.018   0.001   0.004   1.580   0.009   0.015   1.370   0.018   0.000   0.000   0.000   0.003   0.022   0.005   0.000   0.005   0.000
08:00   0.09   0.027   TLS   0.000   0.016   0.001   0.004   0.019   0.005   0.015   1.020   0.018   0.000   0.000   0.000   0.003   0.023   0.004   0.000   0.001   0.000
09:00   0.19   0.029   TLS   0.000   0.016   0.001   0.004   1.054   0.006   0.015   1.016   0.018   0.000   0.000   0.000   0.003   0.027   0.004   0.000   0.001   0.000
10:00   0.09   0.033   TLS   0.000   0.032   0.001   0.004   3.274   0.010   0.015   0.306   0.018   0.000   0.000   0.000   0.003   0.044   0.005   0.000   0.000   0.000
11:00   0.00   0.029   TLS   0.000   0.025   0.001   0.004   0.448   0.007   0.015   0.094   0.017   0.000   0.000   0.000   0.003   0.165   0.005   0.000   0.001   0.000
12:00   0.09   0.029   TLS   0.000   0.013   0.001   0.004   0.485   0.006   0.016   1.018   0.018   0.000   0.000   0.000   0.003   0.023   0.005   0.000   0.001   0.000
13:00   0.00   0.027   TLS   0.000   0.014   0.001   0.004   0.287   0.005   0.015   0.228   0.017   0.000   0.000   0.000   0.003   0.212   0.005   0.000   0.001   0.000
14:00   0.07   0.028   TLS   0.000   0.029   0.001   0.004   0.509   0.005   0.015   1.016   0.018   0.000   0.000   0.000   0.003   0.040   0.005   0.000   0.000   0.000
15:00   2.80   0.058   TLS   0.000   0.016   0.001   0.004   1.068   0.006   0.015   3.018   0.044   0.000   0.000   0.000   0.003   1.005   0.007   0.000   0.001   0.000
16:00   4.34   0.076   TLS   0.000   0.012   0.001   0.004   0.574   0.007   0.015   3.022   0.062   0.000   0.000   0.000   0.003   1.003   0.007   0.000   0.000   0.000
17:00   6.25   0.099   TLS   0.000   0.018   0.001   0.004   0.494   0.007   0.015   3.024   0.082   0.000   0.000   0.000   0.003   1.004   0.010   0.000   0.001   0.000
18:00   3.49   0.064   TLS   0.000   0.018   0.001   0.004   0.447   0.006   0.015   1.027   0.050   0.000   0.000   0.000   0.003   1.005   0.007   0.000   0.000   0.000
19:00   3.78   0.070   TLS   0.000   0.026   0.001   0.004   0.399   0.005   0.015   3.022   0.057   0.000   0.000   0.000   0.003   1.006   0.007   0.000   0.001   0.000
20:00   0.67   0.039   TLS   0.000   0.025   0.001   0.004   0.417   0.006   0.015   3.024   0.026   0.000   0.000   0.000   0.003   1.033   0.006   0.000   0.002   0.000
21:00   0.00   0.030   TLS   0.000   0.020   0.001   0.004   0.409   0.006   0.016   0.117   0.018   0.000   0.000   0.000   0.003   0.034   0.005   0.000   0.001   0.000
22:00   0.00   0.028   TLS   0.000   0.017   0.001   0.004   0.461   0.005   0.016   0.065   0.017   0.000   0.000   0.000   0.003   0.038   0.005   0.000   0.003   0.000
23:00   0.00   0.028   TLS   0.000   0.022   0.001   0.004   0.502   0.006   0.015   0.099   0.017   0.000   0.000   0.000   0.003   0.077   0.005   0.000   0.001   0.000
```

The `most` column show the wait class that contributed most to the response time.

### 24 hour slow probe map

Output a 24h hour map bucketed over only the time-of-day part of slow probes, useful in detecting
recurring daily patterns. The `-T` option may be used to adjust the default bucket size of 30 minutes.

Out is exactly like `24hmap` except that the `most` column is called `blame` here (as slow already is a moral
judgement).

```
$ cat data/snaak_knmi.dat.old | ./build/curlstats -o 24hslowmap
```

### Weekday probe map

Output a weekday map bucketed over the day-of-week of the probes, useful in detecting weekly patterns.

```
$ cat data/snaak_knmi.dat.old | curlstats -o wdmap

================================================================================================================================================================================
All probes to day-of-week distribution
================================================================================================================================================================================
      day  %slow     avg  most ----------DNS---------- ----------TCP---------- ----------TLS---------- ----------REQ---------- ----------RSP---------- ----------DAT----------
   Sunday   0.07   0.029   TLS   0.000   0.032   0.001   0.004   1.580   0.006   0.015   1.370   0.018   0.000   0.000   0.000   0.003   1.004   0.005   0.000   0.004   0.000
   Monday   0.06   0.030   TLS   0.000   0.031   0.001   0.004   1.506   0.007   0.015   1.026   0.018   0.000   0.000   0.000   0.003   0.267   0.005   0.000   0.005   0.000
  Tuesday   4.19   0.076   TLS   0.000   0.031   0.001   0.004   3.274   0.007   0.015   3.024   0.060   0.000   0.000   0.000   0.003   1.033   0.008   0.000   0.005   0.000
 Saturday   0.02   0.028   TLS   0.000   0.029   0.001   0.004   0.502   0.006   0.015   1.018   0.017   0.000   0.000   0.000   0.003   0.267   0.005   0.000   0.001   0.000
```

### Global statistics

```
===========================================================================
Global stats (slow is >1.000s)
===========================================================================
first probe          : 2020-06-13 12:45:45
last  probe          : 2020-06-16 20:18:31
#probes              : 28509
#slow probes         : 316 (QoS 98.892%)
average response time: 0.041s (excellent)
ideal response time  : 0.023s
estimate network RTT : 4.306ms
class   %slow     min     max     avg  stddev  %rtrip  consistency
  DNS   0.000   0.000   0.032   0.001   0.001    1.30         fair
  TCP   0.021   0.004   3.274   0.006   0.035   15.75     mediocre
  TLS   1.024   0.015   3.024   0.028   0.113   69.21         fair
  REQ   0.000   0.000   0.000   0.000   0.000    0.05          n/a
  RSP   0.063   0.003   1.033   0.006   0.028   13.56     mediocre
  DAT   0.000   0.000   0.005   0.000   0.000    0.12          n/a

```

