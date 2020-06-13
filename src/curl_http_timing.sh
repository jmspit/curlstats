#!/bin/bash
#set -x

timeout_default=10

url="$1"
interval="$2"
timeout="$3"
extra_args="$4"

function printHelp {
  echo "$(basename $0) URL INTERVAL"
  echo "  URL      endpoint to GET"
  echo "  INTERVAL number of seconds between gets"
  echo "  TIMEOUT  connect timeout default ${timeout_default}"
  echo "  extra    arguments passed to curl"
}

if [ -z "${url}" ]; then
  echo "error: need to specify URL"
  printHelp
  exit 1
fi

if [[ ${interval} != ?(-)+([0-9]) ]]; then
  echo "error: need to specify integer INTERVAL"
  printHelp
  exit 1
fi

if [[ ${timeout} != ?(-)+([0-9]) ]]; then
  timeout=${timeout_default}
fi

echo "#url      = ${url}"
echo "#interval = ${interval}"
echo "#timeout  = ${timeout}"
echo "#client   = $(hostname -f)"
echo "#IP       = $(ip route get 1 | awk '{print $(NF-2);exit}')"
echo "#args     = ${extra_args}"

echo "#YYYY HH:MI:SS;curl error;http connect code;http response code;ssl verify result;total_time;dns lookup;tcp handshake;ssl handshake;dns lookup+tcp+ssl handshake;redirect time;time to first byte sent"
while true
do
  dstamp="$(date '+%Y-%m-%d %H:%M:%S')"
  curlout="$(curl $extra_args -siw '%{http_connect};%{http_code};%{ssl_verify_result};%{time_total};%{time_namelookup};%{time_connect};%{time_appconnect};%{time_pretransfer};%{time_redirect};%{time_starttransfer}' --connect-timeout ${timeout} --output /dev/null -H 'Accept: application/json' -H 'Content-Type: application/json' -X GET ${url})"
  curl_error=$(echo $?)
  echo "${dstamp};${curl_error};${curlout}"
  sleep ${interval}
done
