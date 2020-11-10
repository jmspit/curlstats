#!/bin/bash

function printHelp {
  echo "$(basename $0) config-file"
}

function exitError() {
  echo "$1" >&2
  #echo "$1"
  exit 1
}

function findExecutable() {
  #set -x
  local exec_name="$1"
  local exec_path="$(which $1)"
  if [ -x "${exec_path}" ]; then
    local -n outvar=$2
    outvar="${exec_path}"
  else
    exitError "cannot find executable ${exec_name}"
  fi
  #set +x
}

config="$1"
if [ ! -r "${config}" ]; then
  exitError "cannot read config file '${config}'"
fi

interval="$2"
if [[ ${interval} != ?(-)+([0-9]) ]]; then
  exitError "error: need to specify integer INTERVAL"
fi

findExecutable awk AWK_EXEC
findExecutable cat CAT_EXEC
findExecutable curl CURL_EXEC
findExecutable date DATE_EXEC
findExecutable echo ECHO_EXEC
findExecutable grep GREP_EXEC
findExecutable hostname HOSTNAME_EXEC
findExecutable ip IP_EXEC
findExecutable sleep SLEEP_EXEC
findExecutable uname UNAME_EXEC
findExecutable sed SED_EXEC

${ECHO_EXEC} "# probing resumes = $(date +'%Y-%m-%d %H:%M:%S')"
${ECHO_EXEC} "# interval        = ${interval}s"
${ECHO_EXEC} "# client FQDN     = $(${HOSTNAME_EXEC} -f)"
${ECHO_EXEC} "# client kernel   = $(${UNAME_EXEC} -r)"
if [ -f /etc/os-release ]; then
  ${ECHO_EXEC} "# client OS       = $(${GREP_EXEC} ^PRETTY_NAME /etc/os-release | ${AWK_EXEC} -F\" '{print $2}')"
fi
${ECHO_EXEC} "# client IP       = $(${IP_EXEC} -o route get 1 | ${SED_EXEC} -n 's/^.*src \([0-9.]*\) .*$/\1/p' )"

if [ -r /etc/resolv.conf ]; then
  ${GREP_EXEC} -v '^[[:space:]]*#' /etc/resolv.conf | while read LINE
  do
    ${ECHO_EXEC} "# resolv.conf     = ${LINE}"
  done
fi

if [ -r /etc/timezone ]; then
  ${ECHO_EXEC} "# client timezone = $(${CAT_EXEC} /etc/timezone)"
fi

${CURL_EXEC} --version | while read LINE
do
  ${ECHO_EXEC} "# curl            = ${LINE}"
done

${GREP_EXEC} -v "^#" "${config}" | ${GREP_EXEC} -v "^$" | ${GREP_EXEC} -v "^write-out" |
${GREP_EXEC} -v "^output" | while read LINE
do
    ${ECHO_EXEC} "# curl config     = ${LINE}"
done

${ECHO_EXEC} "# YYYY HH:MI:SS ;\
 curl error ;\
 http connect code ;\
 http response code ;\
 ssl verify result ;\
 total_time ;\
 dns lookup ;\
 tcp handshake ;\
 ssl handshake ;\
 dns lookup+tcp+ssl handshake;\
 redirect time;\
 time to first byte sent;\
 bytes uploaded;\
 bytes downloaded"
while true
do
  dstamp="$(${DATE_EXEC} '+%Y-%m-%d %H:%M:%S')"
  curlout="$(${CURL_EXEC} -K ${config})"
  curl_error=$(${ECHO_EXEC} $?)
  ${ECHO_EXEC} "${dstamp};${curl_error};${curlout}"
  ${SLEEP_EXEC} ${interval}
done
