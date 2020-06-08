# Build

requires cmake and a C++ compiler.

mkdir build
cd build
cmake ..
make

# Run

curlstats reads (only) from standard in.

$ ./curlstats -h
usage:
  -b seconds
     (real) 24h time distribution bucket
  -d minimum
     (real) specify a slow threshold filter in seconds
  -t
     include a full list of slow probes
  -T minutes
     (uint) 24 hour time bucket ( 0 < x <= 60 )


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
