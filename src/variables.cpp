#include "variables.h"

GlobalStats globalstats;

map<WaitClass,QtyStats> slow_map;

map<int,ProbeStats> slow_dow_map;

map<int,ProbeStats> total_dow_map;

map<TimeKey,ProbeStats> slow_day_map;

map<TimeKey,ProbeStats> total_day_map;

map<DateKey,ProbeStats> total_date_map;

map<DateKey,ProbeStats> slow_date_map;

map<uint16_t,size_t> curl_error_map;

map<uint16_t,size_t> http_code_map;

map<DateKey,QoS> qos_by_date;

map<WaitClass,size_t> wait_class_map;

list<CURLProbe> curl_error_list;

list<CURLProbe> http_error_list;

list<CURLProbe> slow_repsonse_list;

Comments comments;

list<CURLProbe> recent_probes;

map<int,map<TimeKey,QtyStats>> weekmap_qtystats;

map<int,map<TimeKey,QoS>> weekmap_probestats;