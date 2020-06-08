# Build

mkdir build
cd build
cmake ..
make

# Run

curlstats reads (only) from standard in.

usage:
curlstats [flags]
  -b seconds
     (real) slow call time distribution bucket (default 0.1s)
  -d slow
     (real) specify a slow threshold filter in seconds (default 1.0s)
  -t
     include a full list of slow probes (default false)
  -T minutes
     (uint) 24 hour time bucket ( 0 < x <= 60 ) (default 30 minutes)

## Examples

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
