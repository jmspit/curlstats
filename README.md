- [curlstats](#curlstats)
  - [Examples](#examples)
  - [Build](#build)
  - [Statistics](#statistics)
  - [Anatomy of a round-trip](#anatomy-of-a-round-trip)
    - [DNS resolving (DNS)](#dns-resolving-dns)
    - [TCP Handshake (TCP)](#tcp-handshake-tcp)
    - [TLS Handshake (TLS)](#tls-handshake-tls)
    - [Request send (REQ)](#request-send-req)
    - [Response received (RSP)](#response-received-rsp)
    - [Response all data received (DAT)](#response-all-data-received-dat)
  - [Output](#output)
    - [text mode](#text-mode)
    - [html mode](#html-mode)

# curlstats

Generates, parses and analyzes curl timing statistics against (HTTP) endpoints. The
[curl_http_timing.sh](src/curl_http_timing.sh) shell script generates probes against an endpoint. This assembled data
can be fed to the `curlstats` executable for processing and analysis.

The [curl_http_timing.sh](src/curl_http_timing.sh) script requires curl and bash to generate timed probes. The probed
endpoint, along with other curl options, is specified through a configuration file. The 
[curl_config.example](src/curl_config.example) file can be taken as an example. Be sure to leave the `write-out` 
configuration option intact as the parsing by curlstats depends on that output format.

`curlstats` produces either text or html output.

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
write-out "%{http_connect};%{http_code};%{ssl_verify_result};%{time_total};%{time_namelookup};%{time_connect};%{time_appconnect};%{time_pretransfer};%{time_redirect};%{time_starttransfer};%{size_upload};%{size_download}"
```

using the config file the probing can be started, say with a 10 second interval

```
$ nohup curl_http_timing.sh gnu.curl 10 > gnu.data &
$ tail -100f gnu.data
# probing resumes = 2020-11-01 08:03:43
# interval        = 53s
# client FQDN     = snaak.no-ip.org
# client kernel   = 5.4.72-gentoo
# client OS       = Gentoo/Linux
# client IP       = 192.168.178.2
# curl            = curl 7.72.0 (x86_64-pc-linux-gnu) libcurl/7.72.0 OpenSSL/1.1.1g zlib/1.2.11 zstd/1.4.4 nghttp2/1.41.0
# curl            = Release-Date: 2020-08-19
# curl            = Protocols: dict file ftp ftps http https imap imaps pop3 pop3s rtsp smtp smtps tftp
# curl            = Features: AsynchDNS HTTP2 HTTPS-proxy IPv6 Largefile libz NTLM SSL TLS-SRP UnixSockets zstd
# curl config     = url https://www.gnu.org
# curl config     = request GET
# curl config     = connect-timeout 10
# curl config     = max-time 10
# curl config     = header "Accept: text/html"
# curl config     = silent
# YYYY HH:MI:SS ; curl error ; http connect code ; http response code ; ssl verify result ; total_time ; dns lookup ; tcp handshake ; ssl handshake ; dns lookup+tcp+ssl handshake; redirect time; time to first byte sent
2020-11-01 08:03:43;0;000;200;0;0.538611;0.000387;0.088594;0.272252;0.272271;0.000000;0.361655;0;32154
2020-11-01 08:04:37;0;000;200;0;0.521223;0.000481;0.085966;0.263978;0.263999;0.000000;0.350706;0;32154
2020-11-01 08:05:30;0;000;200;0;0.528655;0.000443;0.086945;0.267497;0.267521;0.000000;0.355976;0;32154
2020-11-01 08:06:24;0;000;200;0;0.521126;0.000470;0.086074;0.263584;0.263602;0.000000;0.350534;0;32154
```

The data (in the above case tee-ed to data/gnu.org.dat) can be parsed and analyzed by curlstats:

```
$ cat gnu.data | curlstats
```

curlstats provides several command line options to tweak its behavior

```
$ curlstats -h

curlstats reads from standard input
see https://github.com/jmspit/curlstats

usage: 
  -b buckets
     (uint) maximum number of buckets per histogram
     default: 20 buckets
  -d threshold
     (real) specify a slow threshold in seconds
     default: 1 seconds
  -f format
     (text) specify the output format, either 'text' or 'html'
     default: 'text'
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
     default: 0 %
  -T minutes
     (uint) 24 hour time bucket in minutes ( 0 < x <= 60 ) (-o 24hmap, 24hslowmap)
     default: 60 minutes
  -W minutes
     (uint) 24 hour weekmap time bucket in minutes ( 0 < x <= 60 ). 60 Must be an integer multiple
     of this value
     default: 60 minutes

DNS = DNS name resolution
TCP = TCP handshake
TLS = TLS ('SSL') handshake
REQ = Request send lead time
RSP = Waiting for response
DAT = Waiting for response more data

```


## Build

Building requires cmake and a c++-17 capable C++ compiler stack.

```
$ git clone git@github.com:jmspit/curlstats.git
$ cd curlstats
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ cmake --build . --config Release
```

will produce `curlstats` in the build directory.

## Statistics

To analyze performance (or the lack thereof), a good data set is essential. A good data set

  - has enough probes to capture most of the scenario's that might occur. For example
    - covering full 24 hour
    - covering all days of a week
    - covering multiple weeks
  - is isolated from effects that impact the measurements but do not relate to the analyzed problem, such as sampling from a unstable source.

## Anatomy of a round-trip

Client client/server request/response sequence involves several phases involving a number of network
round-trips. Curl provides timing information for each of these steps, and curlstats labels each step a WaitClass.

| WaitClass | Represents |
|-----------|----------|
| DNS | Resolving of the fqdn to an IP address. |
| TCP | TCP handshake to the remote endpoint. |
| TLS | TLS ('SSL') handshake to the remote endpoint. |
| REQ | Preparing the sending of the request locally. |
| RSP | Waiting for the (first byte of) the endpoint response. |
| DAT | Transfer additional data. |

Each of the wait clases are decribed in more detail below.

### DNS resolving (DNS)

Unless the endpoint is given as an IP (4 or 6) address in the curl config file, the name must be resolved against DNS. DNS queries may timeout and get retried (against another DNS server) showing up as additional latency. The default DNS timeout value on Linux is 5 seconds (can be overridden in /etc/resolv.conf). In case the resolve fails entirely (no DNS server delivered a response), curl will error CURLE_OPERATION_TIMEDOUT and the
probe will not be included in the statistics.

There are known caveats when using ipv6 DNS queries on clients that support both ipv4 and ipv6 transparently. Good
software will not depend on the IP address family used (AF_INET or AF_INET6) but specify AF_UNSPEC
instead, letting the OS decide what capabilities are available. If the OS has AF_INET6 enabled ('has ipv6 support'), and the developer uses the
prescribed getaddrinfo call that supersedes gethostbyname, the OS *will* attempt to use ipv6, and thus send
two DNS queries - one for an A (ipv4) and one for an AAA record (ipv6) *from the same UDP source port*. This means the query
response may lead to two UDP packets being returned on the same port - and intermediate networking apparatus (e.g. firewalls) may not
expect that and block either one of the two, leading the client to timeout and retry. Use
the `-4` option of curl (specify a `ipv4` in the config file) to prevent ipv6 and ipv6 DNS queries altogether - even if the host OS supports ipv6.

As DNS is essential to IT operation, DNS resolving should provide rock solid performance and stability. Infrastructure architects and administrators
should override (better word: *design*) the Linux DNS timeout of 5 seconds - if DNS is not resonding withon 1 second, it is likely not going to respond in the remaining 4 seconds either. adding 4 second sof pointless latency.

*The DNS wait class equates to DNS response time, which is the time the DNS server required to assemble the answer plus
the network time before the response reached the client. In general, DNS resolution performance is so critical to IT
operation that it can be expected to be near the transport RTT, or in the case of DNS caching, close to local memory
access latency.*

### TCP Handshake (TCP)

The TCP layer performs a handshake to establish a TCP connection. This is almost entirely bound by network latency (in
absence of socket allocation problems). TCP uses a three-way handshake, the handshake (3 sends) latency is
1.5 times the network latency (2 sends) - so a rough estimate of the network latency can be deduced from the TCP
handshake latency.


*The TCP wait class typically equates to network latency. Likely causes for higher TCP waits are packet loss (forcing retransmists and thus delays), socket allocation and other problems on either server or client or any intermediate components such as routers, firewalls and loadbalancers.*

### TLS Handshake (TLS)

If Transport Layer Security (formerly known as SSL) is used, TLS needs to handshake as well. The handshake optionally involves establishing trust on peer identity, negotiating a TLS protocol version and cipher to use, and negotiate a session key which will be used to encrypt ensuing traffic.

Note that TLSv3 is not only more secure than TLSv2, TLSv3 is also **much faster** as it involves fewer roundtrips and is less CPU intensive. TLSv1 provides only weak security so it should be disabled.

*The TLS wait class equates to network latency and/or CPU starvation on either client or server, although this no longer is a common problem with the advent of faster TLS handshakes. However, if network latency is the dominant factor, that should also be visible in the TCP wait class. If TLS is high and TCP is low during the same probes, the server may have trouble producing TLS handshakes at the requested rate.*

### Request send (REQ)

After the TLS handshake completed, the client sends its request data, if any. In some cases it takes a bit of time to assemble that data, e.g. reading large JSON file to POST. For simple GET calls against HTTP endpoints this WaitClass should be very close to zero.

*The REQ wait class is the lead time before sending the data to the server, it may include preparation of such data, but is typically (very) low.*

### Response received (RSP)

This wait class meaures the time between the request being (fully) sent and the the first packet of the response is received. This is the time the client spent 'waiting on the server'. 

*The RSP wait class equates to server response time.*

### Response all data received (DAT)

The response may require more than one network packet to return all data. DAT is the time between arrival of the first and last packet of the response.

*The DAT wait class equates to response data transfer. Possible causes of a high DAT value are bandwidth problems or the server having trouble to generate the data in a sytreaming response.*

## Output

### text mode

Curlstats produces analysis as structured text on the standard output. The `-o option` can be specified
more than once to limit the output, or simply use -o all.

As curlstats reads from standard in, the user can apply filters on the input data, say to limit the input to a particular date:

```bash
$ grep '^2020-06-16' data/gnu.org.dat | curlstats -o global
```

### html mode

The HTML mode produces a self-contained html file based on Google charts. If `-f html` is specified, `-o` must eitrher be absent or `-o all`.
