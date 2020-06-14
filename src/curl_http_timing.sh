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

${ECHO_EXEC} "# probing resumes = $(date +'%Y-%m-%d %H:%M:%S')"
${ECHO_EXEC} "# client FQDN     = $(${HOSTNAME_EXEC} -f)"
${ECHO_EXEC} "# client kernel   = $(${UNAME_EXEC} -r)"
if [ -f /etc/os-release ]; then
  ${ECHO_EXEC} "# client OS       = $(${GREP_EXEC} ^PRETTY_NAME /etc/os-release | ${AWK_EXEC} -F\" '{print $2}')"
fi
${ECHO_EXEC} "# client IP       = $(${IP_EXEC} route get 1 | ${AWK_EXEC} '{print $(NF-2);exit}')"
${CURL_EXEC} --version | while read LINE
do
  ${ECHO_EXEC} "# curl            = ${LINE}"
done

${GREP_EXEC} -v "^#" "${config}" | ${GREP_EXEC} -v "^$" | while read LINE
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
 time to first byte sent"
while true
do
  dstamp="$(${DATE_EXEC} '+%Y-%m-%d %H:%M:%S')"
  curlout="$(${CURL_EXEC} -K ${config})"
  curl_error=$(${ECHO_EXEC} $?)
  ${ECHO_EXEC} "${dstamp};${curl_error};${curlout}"
  ${SLEEP_EXEC} ${interval}
done
