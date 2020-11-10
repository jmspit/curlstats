#include "comments.h"
#include "html.h"
#include "options.h"
#include "variables.h"

#include <iomanip>
#include <sstream>
#include <vector>

using namespace std;

const string color_background_dark = "#a1a1cc";
const string color_background_light = "#efefff";
const string color_background_table_cell = "#e1e1ff";
const string color_background_table_header = "#d1d1ef";
const string color_background_table_good = "#c1d1c1";
const string color_background_table_bad = "#d1c1c1";
const string color_background_tab = "#fafaff";
const string color_table_border = "#c1c1cc";
const string color_anonymous = "#577a7a";

const string color_dns="3366cc";
const string color_tcp="dc3912";
const string color_tls="ff9900";
const string color_req="109618";
const string color_rsp="990099";
const string color_dat="0099c6";

const string wait_class_colors = "['" + color_dns + "','" + color_tcp + "','" + color_tls + 
                                 "','" + color_req + "','" + color_rsp + "','" + color_dat + "']";

const string color_http_error = "maroon";                                 
const string color_curl_error = "#dc3912";                                 
const string color_slow_probe = "orange";                                 
const string color_good = "#0c5922";                            
const string color_mean = "#333366";                            
const string color_stddev = "#aa3333";                            

const string chartarea_linechart = "chartArea:{left:80,top:30,bottom:50,right:120,width:'100%',height:'100%'}";
const string chartarea_piechart  = "chartArea:{left:20,top:20,bottom:20,right:20,width:'100%',height:'100%'}";
const string chartarea_histogram = "chartArea:{left:80,top:50,bottom:50,right:30,width:'100%',height:'100%'}";


const string probe_error_colors = "['" + color_good + "','" + color_slow_probe + "','" +
                                  color_curl_error + "','" + color_http_error + "']";

vector<string> error_index_colors = { "#3366cc","#dc3912","#ff9900","#109618","#990099","#0099c6","#dd4477" }; 

const unsigned int overview_linechart_width = 1800;
const unsigned int overview_linechart_wc_height = 450;
const unsigned int overview_linechart_qos_height = 300;

const unsigned int overview_piechart_height = 300;
const unsigned int overview_piechart_width = 400;

const string github_pages_url = "https://jmspit.github.io/curlstats/";
const string github_repo_url = "https://github.com/jmspit/curlstats";

/**
 *  (100-qos_cutoff)% is threshold below which QoS turns 'to poor' and advances to bad.
 */
const double qos_cutoff = 10.0;

/**
 * RGB color type.
 */
struct RGB {
  int red = 0;
  int green = 0;
  int blue = 0;
};

/**
 * For weekmaps, the gradient runs from this color to color_warn.
 * "Green"
 * @see color_warn
 * @see color_bad
 */
const RGB color_ok = { 0, 180, 0 };

/**
 * For weekmaps, the gradient runs from this color to color_bad.
 * "Yellow"
 * @see color_ok
 * @see color_bad
 */
const RGB color_warn = { 232, 183, 49 };

/**
 * For weekmaps.
 * "Red"
 * @see color_ok
 * @see color_warn
 */
const RGB color_bad = { 255, 0, 0 };


/**
 * Maintain a map of either http or curl error codes to a unqiue id (index).
 */
map<size_t,size_t> error_code_index;

/**
 * Floating point as a string with given precision.
 */
string num( double value, int precision = 3 ) {
  ostringstream oss;
  oss << fixed << setprecision(precision) << value;
  return oss.str();
}

/**
 * Generate JavaScript for managing the tabs.
 */
void generateTabbingJavaScript( ostringstream& oss ) {
  oss << "function openTab(evt, tabName) {" << endl;
  oss << "  var i, tabcontent, tablinks;" << endl;
  oss << "  tabcontent = document.getElementsByClassName(\"tabcontent\");" << endl;
  oss << "  for (i = 0; i < tabcontent.length; i++) {" << endl;
  oss << "    tabcontent[i].style.display = \"none\"" << endl;
  oss << "  }" << endl;

  oss << "  tablinks = document.getElementsByClassName(\"tablinks\");" << endl;
  oss << "  for (i = 0; i < tablinks.length; i++) {" << endl;
  oss << "    tablinks[i].className = tablinks[i].className.replace(\" active\", \"\");" << endl;
  oss << "  }" << endl;

  oss << "  document.getElementById(tabName).style.display = \"block\";" << endl;
  oss << "  evt.currentTarget.className += \" active\";" << endl;
  oss << "  drawCharts();" << endl;
  oss << "}" << endl;
  oss << "window.onload = function(){" << endl;
  oss << "  document.getElementById(\"defaultTab\").click();" << endl;
  oss << "}" << endl;
}

/**
 * Generate the CSS.
 */
void generateCSS( ostringstream& oss ) {
  oss << "<style>" << endl;

  oss << "body {" << endl;
  oss << "  font-family: monospace;" << endl;
  oss << "  font-size: 11px;" << endl;
  oss << "}" << endl;

  oss << "p.gentle {" << endl;
  oss << "  font-family: serif;" << endl;
  oss << "  font-size: 12px;" << endl;
  oss << "  font-style: italic;" << endl;  
  oss << "  color: #444444;" << endl;
  oss << "}" << endl;

  oss << "table.header {" << endl;
  oss << "  padding-bottom: 10px;" << endl;
  oss << "}" << endl;

  oss << "table.header th.caption {" << endl;
  oss << "  font-family: monospace;" << endl;
  oss << "  font-size: 18px;" << endl;  
  oss << "  font-weight: bold;" << endl;  
  oss << "  text-align: left;" << endl;  
  oss << "}" << endl;

  oss << "table.header td, table.header th {" << endl;
  oss << "  font-family: monospace;" << endl;
  oss << "  font-size: 12px;" << endl;  
  oss << "  font-weight: normal;" << endl;  
  oss << "  text-align: left;" << endl;    
  oss << "}" << endl;     

  oss << "table.header th {" << endl;
  oss << "  font-weight: bold;" << endl;  
  oss << "}" << endl;  

  oss << "table.header td {" << endl;
  oss << "  padding-right: 10px;" << endl;   
  oss << "}" << endl;   

  oss << "table.heatmap {" << endl;
  oss << "  background-color : " << color_background_table_header << ";" << endl;
  oss << "  border: 0px solid " << color_table_border << ";" << endl;
  oss << "  border-collapse: collapse;" << endl;  
  oss << "  margin-top: 20px;" << endl;  
  oss << "}" << endl;  

  oss << "table.heatmap caption {" << endl;
  oss << "  font-family: sans;" << endl;
  oss << "  font-size: 14px;" << endl;  
  oss << "  font-weight: bold;" << endl;  
  oss << "  text-align: left;" << endl;  
  oss << "}" << endl;    

  oss << "table.heatmap td, table.heatmap th {" << endl;
  oss << "  border: 1px solid " << color_background_dark << ";" << endl;
  oss << "  border-collapse: collapse;" << endl;
  oss << "  font-family: sans;" << endl;
  oss << "  font-size: 12px;" << endl;  
  oss << "  font-weight: normal;" << endl;  
  oss << "  text-align: left;" << endl;   
  oss << "  padding: 2px;" << endl;  
  oss << "  padding-right: 4px;" << endl;  
  oss << "}" << endl;     

  oss << "table.heatmap th {" << endl;
  oss << "  font-weight: bold;" << endl;  
  oss << "  padding-right: 8px;" << endl;  
  oss << "}" << endl;     

  oss << "table.usertable {" << endl;
  oss << "  border: 1px solid " << color_background_dark << ";" << endl;
  oss << "  border-collapse: collapse;" << endl;
  oss << "}" << endl;  

  oss << "table.usertable th, table.usertable td {" << endl;
  oss << "  text-align: left;" << endl;
  oss << "  border: 1px solid " << color_table_border << ";" << endl;
  oss << "  background-color:" << color_background_table_cell << ";" << endl; 
  oss << "  border-collapse: collapse;" << endl; 
  oss << "  padding: 3px;" << endl;   
  oss << "}" << endl;

  oss << "table.usertable th {" << endl;
  oss << "  background-color:" << color_background_table_header << ";" << endl; 
  oss << "}" << endl;

  oss << "table.usertable td.good {" << endl;
  oss << "  background-color:" << color_background_table_good << ";" << endl; 
  oss << "}" << endl;      

  oss << "table.usertable td.bad {" << endl;
  oss << "  background-color:" << color_background_table_bad << ";" << endl; 
  oss << "}" << endl;        

  oss << ".tab {" << endl;
  oss << "  overflow: hidden;" << endl;
  oss << "  border: 1px solid " << color_background_dark << ";" << endl;
  oss << "  background-color: " << color_background_dark << ";" << endl;
  oss << "}" << endl;

  oss << ".tab button {" << endl;
  oss << "  background-color: inherit;" << endl;
  oss << "  float: left;" << endl;
  oss << "  border: none;" << endl;
  oss << "  outline: none;" << endl;
  oss << "  cursor: pointer;" << endl;
  oss << "  padding: 8px 10px;" << endl;
  oss << "  transition: 0.3s;" << endl;
  oss << "}" << endl;

  oss << ".tab button:hover {" << endl;
  oss << "  background-color: " << color_background_light << ";" << endl;
  oss << "}" << endl;

  oss << ".tab button.active {" << endl;
  oss << "background-color: " << color_background_light << ";" << endl;
  oss << "}" << endl;

  oss << ".tabcontent {" << endl;
  //oss << "  animation: fadeEffect 0.8s;" << endl;
  oss << "  display: none;" << endl;
  oss << "  padding: 6px 12px;" << endl;
  oss << "  border: 1px solid " << color_background_dark << ";" << endl;
  oss << "  border-top: none;" << endl;
  oss << "  background-color: " << color_background_tab << ";" << endl;
  oss << "}" << endl;

  //oss << "@keyframes fadeEffect {" << endl;
  //oss << "  from {opacity: 0;}" << endl;
  //oss << "  to {opacity: 1;}" << endl;
  //oss << "}" << endl;
  oss << "</style>" << endl;
}

/**
 * Generate JavScript options common for Pie (donut) charts.
 */
void generatePieChartOptions( ostringstream& oss, const string& name, const string& title, unsigned width, 
                              unsigned height, const std::string colors, unsigned indent ) {
  oss << string(indent,' ')   << "var " << name << "= {" << endl;
  oss << string(indent+2,' ') << "title: '" << title << "'," << endl;
  oss << string(indent+2,' ') << chartarea_piechart << "," << endl;
  oss << string(indent+2,' ') << "width: " << width << "," << endl;
  oss << string(indent+2,' ') << "height: " << height << "," << endl;
  oss << string(indent+2,' ') << "pieHole: " << 0.4 << "," << endl;
  oss << string(indent+2,' ') << "pieSliceText: 'none'," << endl;
  oss << string(indent+2,' ') << "sliceVisibilityThreshold: 0.0," << endl;
  oss << string(indent+2,' ') << "colors: " << colors << "," << endl;
  oss << string(indent+2,' ') << "backgroundColor: { fill:'transparent' }," << endl;
  oss << string(indent,' ')   << "};" << endl;
}

/**
 * Generate Google chart JavaScript for min-max, mean,+stddev chart.
 */
void generateSummaryMinMaxChart( ostringstream& oss ) {
  oss << "  var minMax_data = google.visualization.arrayToDataTable([" << endl;
  oss << "    ['DNS', " 
      << globalstats.wait_class_stats.namelookup.min << ", " 
      << globalstats.wait_class_stats.namelookup.getMean() << ", " 
      << globalstats.wait_class_stats.namelookup.getMean() + globalstats.wait_class_stats.namelookup.getSigma() << ", " 
      << globalstats.wait_class_stats.namelookup.max 
      << " ]," << endl;
  oss << "    ['TCP', " 
      << globalstats.wait_class_stats.connect.min << ", " 
      << globalstats.wait_class_stats.connect.getMean() << ", " 
      << globalstats.wait_class_stats.connect.getMean() + globalstats.wait_class_stats.connect.getSigma() << ", " 
      << globalstats.wait_class_stats.connect.max 
      << " ]," << endl;
  oss << "    ['TLS', " 
      << globalstats.wait_class_stats.appconnect.min << ", " 
      << globalstats.wait_class_stats.appconnect.getMean() << ", " 
      << globalstats.wait_class_stats.appconnect.getMean() + globalstats.wait_class_stats.appconnect.getSigma() << ", " 
      << globalstats.wait_class_stats.appconnect.max 
      << " ]," << endl;
  oss << "    ['REQ', " 
      << globalstats.wait_class_stats.pretransfer.min << ", " 
      << globalstats.wait_class_stats.pretransfer.getMean() << ", " 
      << globalstats.wait_class_stats.pretransfer.getMean() + globalstats.wait_class_stats.pretransfer.getSigma() << ", " 
      << globalstats.wait_class_stats.pretransfer.max 
      << " ]," << endl;
  oss << "    ['RSP', " 
      << globalstats.wait_class_stats.starttransfer.min << ", " 
      << globalstats.wait_class_stats.starttransfer.getMean() << ", " 
      << globalstats.wait_class_stats.starttransfer.getMean() + globalstats.wait_class_stats.starttransfer.getSigma() << ", " 
      << globalstats.wait_class_stats.starttransfer.max 
      << " ]," << endl;
  oss << "    ['DAT', " 
      << globalstats.wait_class_stats.endtransfer.min << ", " 
      << globalstats.wait_class_stats.endtransfer.getMean() << ", " 
      << globalstats.wait_class_stats.endtransfer.getMean() + globalstats.wait_class_stats.endtransfer.getSigma() << ", " 
      << globalstats.wait_class_stats.endtransfer.max 
      << " ]," << endl;                              
  oss << "    ], true );" << endl;
  
  unsigned int indent = 2;
  oss << string(indent,' ')   << "var minMax_options= {" << endl;
  oss << string(indent+2,' ') << "title: 'Per wait class min-max, mean - mean + standard deviation'," << endl;
  oss << string(indent+2,' ') << chartarea_linechart << "," << endl;
  oss << string(indent+2,' ') << "width: " << 800 << "," << endl;
  oss << string(indent+2,' ') << "height: " << 600 << "," << endl;
  oss << string(indent+2,' ') << "lineWidth: 2," << endl;
  oss << string(indent+2,' ') << "fontSize: 10," << endl;
  oss << string(indent+2,' ') << "colors: ['" << color_anonymous << "']," << endl;
  oss << string(indent+2,' ') << "legend: { position: 'none' }," << endl;
  oss << string(indent+2,' ') << "backgroundColor: { fill:'transparent' }," << endl;
  oss << string(indent+2,' ') << "hAxis: {" << endl;
  oss << string(indent+4,' ') << "title: 'Wait class'," << endl;
  oss << string(indent+2,' ')   << "}," << endl;  
  oss << string(indent+2,' ') << "vAxis: {" << endl;
  oss << string(indent+4,' ') << "title: 'response time (s)'," << endl;
  oss << string(indent+4,' ') << "scaleType: 'log'," << endl;
  oss << string(indent+2,' ')   << "}," << endl;
  oss << string(indent,' ')   << "};" << endl;

  oss << "  var minMax_chart = new google.visualization.CandlestickChart(document.getElementById('minMax'));" << endl;  
  oss << "  minMax_chart.draw( minMax_data, minMax_options );" << endl;  
}

/**
 * Generate Google chart JavaScript for the Summary / QoS pie chart.
 */
void generateSummaryQoSPieChart( ostringstream& oss ) {
  oss << "  var probePie_data = google.visualization.arrayToDataTable([" << endl;
  oss << "  ['Probe counts','count']," << endl;
  oss << "    ['" << "Good " << globalstats.timed_probes - globalstats.items_slow 
      << "', " << globalstats.timed_probes - globalstats.items_slow << "]," << endl;
  oss << "    ['" << "Slow " << globalstats.items_slow << "', " << globalstats.items_slow << "]," << endl;
  oss << "    ['" << "Probe error " << curl_error_list.size() << "', " << curl_error_list.size() << "]," << endl;
  oss << "    ['" << "HTTP error " << http_error_list.size() << "', " << http_error_list.size() << "]," << endl;
  oss << "    ]);" << endl;
  generatePieChartOptions( oss, "probePie_options", "Quality of service", overview_piechart_width, 
                           overview_piechart_height, probe_error_colors, 2 );
  oss << "  var probePie_chart = new google.visualization.PieChart(document.getElementById('probePie'));" << endl;  
  oss << "  probePie_chart.draw( probePie_data, probePie_options );" << endl;  
}

/**
 * Generate Google chart JavaScript for the Summary / Wait class mean / all probes pie chart.
 */
void generateSummaryWCAveragePieChart( ostringstream& oss ) {
  oss << "  var waitClassPie_data = google.visualization.arrayToDataTable([" << endl;
  oss << "  ['Wait class','mean']," << endl;
  oss << "    ['" << "DNS " << num(globalstats.wait_class_stats.namelookup.getMean()*1000.0,2) 
      << "ms', " << globalstats.wait_class_stats.namelookup.getMean()*1000.0 << "]," << endl;
  oss << "    ['" << "TCP " << num(globalstats.wait_class_stats.connect.getMean()*1000.0,2) 
      << "ms', " << globalstats.wait_class_stats.connect.getMean()*1000.0 << "]," << endl;
  oss << "    ['" << "TLS " << num(globalstats.wait_class_stats.appconnect.getMean()*1000.0,2) 
      << "ms', " << globalstats.wait_class_stats.appconnect.getMean()*1000.0 << "]," << endl;
  oss << "    ['" << "REQ " << num(globalstats.wait_class_stats.pretransfer.getMean()*1000.0,2) 
      << "ms', " << globalstats.wait_class_stats.pretransfer.getMean()*1000.0 << "]," << endl;
  oss << "    ['" << "RSP " << num(globalstats.wait_class_stats.starttransfer.getMean()*1000.0,2) 
      << "ms', " << globalstats.wait_class_stats.starttransfer.getMean()*1000.0 << "]," << endl;
  oss << "    ['" << "DAT " << num(globalstats.wait_class_stats.endtransfer.getMean()*1000.0,2) 
      << "ms', " << globalstats.wait_class_stats.endtransfer.getMean()*1000.0 << "]" << endl;
  oss << "    ]);" << endl;
  generatePieChartOptions( oss, "waitClassPie_options", "Wait class mean all probes (ms)", overview_piechart_width, 
                           overview_piechart_height, wait_class_colors, 2 );
  oss << "  var waitClassPie_chart = new google.visualization.PieChart(document.getElementById('waitClassPie'));" << endl;  
  oss << "  waitClassPie_chart.draw( waitClassPie_data, waitClassPie_options );" << endl;  
}

/**
 * Generate Google chart JavaScript for the Summary / Wait class mean / slow probes pie chart.
 */
void generateSummaryWCSlowTotalPieChart( ostringstream& oss ) {
  oss << "  var waitClassSlowPie_data = google.visualization.arrayToDataTable([" << endl;
  oss << "  ['Wait class','mean']," << endl;
  oss << "    ['" << "DNS " << num(slow_map[wcDNS].total) << "', " << slow_map[wcDNS].total << "]," << endl;
  oss << "    ['" << "TCP " << num(slow_map[wcTCPHandshake].total) << "', " << slow_map[wcTCPHandshake].total << "]," << endl;
  oss << "    ['" << "TLS " << num(slow_map[wcSSLHandshake].total) << "', " << slow_map[wcSSLHandshake].total << "]," << endl;
  oss << "    ['" << "REQ " << num(slow_map[wcSendStart].total) << "', " << slow_map[wcSendStart].total << "]," << endl;
  oss << "    ['" << "RSP " << num(slow_map[wcWaitEnd].total) << "', " << slow_map[wcWaitEnd].total << "]," << endl;
  oss << "    ['" << "DAT " << num(slow_map[wcReceiveEnd].total) << "', " << slow_map[wcReceiveEnd].total << "]," << endl;
  oss << "    ]);" << endl;
  generatePieChartOptions( oss, "waitClassSlowPie_options", "Wait class total slow probes", overview_piechart_width,
                           overview_piechart_height, wait_class_colors, 2 );
  oss << "  var waitClassSlowPie_chart = new google.visualization.PieChart(document.getElementById('waitClassSlowPie'));" << endl;  
  oss << "  waitClassSlowPie_chart.draw( waitClassSlowPie_data, waitClassSlowPie_options );" << endl;  
}


/**
 * Generate Google chart JavaScript for the History / Qos chart.
 */
void generateHistoryQosChart( ostringstream& oss ) {
  oss << "  var qosTrail_data = google.visualization.arrayToDataTable([" << endl;
  oss << "  ['date','Good', 'Slow', 'Probe error', 'HTTP error' ]," << endl;
  for ( const auto &d : qos_by_date ) {
    //oss << "    ['" << d.first.asString() 
    oss << "    [ new Date(" << d.first.year << ", " << d.first.month-1 << ", " <<  d.first.day << ").toDateString(),"
        << total_date_map[d.first].probe.items << ", " 
        << d.second.slow << ", " 
        << d.second.curl_errors << ", " 
        << d.second.http_errors <<  "]," << endl;
  }
  oss << "    ]);" << endl;

  unsigned int indent = 2;
  oss << string(indent,' ')   << "var qosTrail_options= {" << endl;
  oss << string(indent+2,' ') << "title: 'QoS history by date'," << endl;
  oss << string(indent+2,' ') << chartarea_linechart << "," << endl;
  oss << string(indent+2,' ') << "width: " << overview_linechart_width << "," << endl;
  oss << string(indent+2,' ') << "height: " << overview_linechart_qos_height << "," << endl;
  oss << string(indent+2,' ') << "seriesType: 'bars'," << endl;
  oss << string(indent+2,' ') << "lineWidth: 2," << endl;
  oss << string(indent+2,' ') << "fontSize: 10," << endl;
  oss << string(indent+2,' ') << "dataOpacity: 0.8," << endl;
  oss << string(indent+2,' ') << "isStacked: 'percent'," << endl;
  oss << string(indent+2,' ') << "colors: " << probe_error_colors << "," << endl;
  oss << string(indent+2,' ') << "legend: { position: 'top' }," << endl;
  oss << string(indent+2,' ') << "backgroundColor: { fill:'transparent' }," << endl;
  oss << string(indent+2,' ') << "showTextEvery: " << total_date_map.size() / 6 << "," << endl;
  oss << string(indent+2,' ') << "hAxis: {" << endl;
  oss << string(indent+4,' ') << "title: 'date'," << endl;
  oss << string(indent+4,' ') << "format: 'EEE MMM d'," << endl;
  oss << string(indent+4,' ') << "gridlines: { count: 3 }," << endl;
  oss << string(indent+2,' ') << "}," << endl;  
  oss << string(indent+2,' ') << "vAxes: {" << endl;
  oss << string(indent+4,' ') << "0: { title: 'QoS' }," << endl;
  oss << string(indent+4,' ') << "1: { title: 'response time (s)' }," << endl;
  oss << string(indent+2,' ') << "}," << endl;
  oss << string(indent,' ')   << "};" << endl;

  oss << "  var qosTrail_chart = new google.visualization.ComboChart(document.getElementById('qosTrail'));" << endl;  
  oss << "  qosTrail_chart.draw( qosTrail_data, qosTrail_options );" << endl;  
}

/**
 * Generate Google chart JavaScript for the Recent chart.
 */
void generateRecentChart( ostringstream& oss ) {
  oss << "  var recentTrail_data = google.visualization.arrayToDataTable([" << endl;
  oss << "    ['datetime','DNS', 'TCP', 'TLS', 'REQ', 'RSP', 'DAT' ]," << endl;  
  for ( const auto &p : recent_probes ) {
    oss << "    [ new Date( " << p.datetime.year << "," << p.datetime.month-1 << "," << p.datetime.day 
        << "," << p.datetime.hour << "," << p.datetime.minute << "," << p.datetime.second << "), ";
    oss << p.getWaitClassDuration( wcDNS ) << ", ";
    oss << p.getWaitClassDuration( wcTCPHandshake ) << ",";
    oss << p.getWaitClassDuration( wcSSLHandshake ) << ",";
    oss << p.getWaitClassDuration( wcSendStart ) << ",";
    oss << p.getWaitClassDuration( wcWaitEnd ) << ",";
    oss << p.getWaitClassDuration( wcReceiveEnd ) << "]," << endl;
  }
  oss << "    ]);" << endl;

  unsigned int indent = 2;
  oss << string(indent,' ')   << "var recentTrail_options= {" << endl;
  oss << string(indent+2,' ') << "title: 'Recent probes'," << endl;
  oss << string(indent+2,' ') << chartarea_linechart << "," << endl;
  oss << string(indent+2,' ') << "width: " << 1800 << "," << endl;
  oss << string(indent+2,' ') << "height: " << 600 << "," << endl;
  oss << string(indent+2,' ') << "isStacked: 'absolute'," << endl;
  oss << string(indent+2,' ') << "lineWidth: 0.1," << endl;
  oss << string(indent+2,' ') << "fontSize: 10," << endl;
  oss << string(indent+2,' ') << "legend: { position: 'top' }," << endl;
  oss << string(indent+2,' ') << "backgroundColor: { fill:'transparent' }," << endl;
  oss << string(indent+2,' ') << "areaOpacity: 0.6," << endl;
  //oss << string(indent+2,' ') << "showTextEvery: " << recent_probes.size() / 6 << "," << endl;
  oss << string(indent+2,' ') << "hAxis: {" << endl;
  oss << string(indent+4,' ') << "title: 'datetime'," << endl;
  oss << string(indent+2,' ')   << "}," << endl;  
  oss << string(indent+2,' ') << "vAxis: {" << endl;
  oss << string(indent+4,' ') << "title: 'seconds'," << endl;
  oss << string(indent+4,' ') << "format: '##.###'," << endl;
  oss << string(indent+2,' ')   << "}," << endl;
  oss << string(indent,' ')   << "};" << endl;

  oss << "  var recentTrail_chart = new google.visualization.AreaChart(document.getElementById('recentTrail'));" << endl;  
  oss << "  recentTrail_chart.draw( recentTrail_data, recentTrail_options );" << endl;  
}

/**
 * Generate Google chart JavaScript for the Error trail chart.
 */
void generateErrorTrail( ostringstream& oss ) {  
  oss << "  var errorTrail_data = new google.visualization.DataTable();" << endl;
  oss << "  errorTrail_data.addColumn('datetime', 'Date');" << endl;
  oss << "  errorTrail_data.addColumn('number', 'error code');" << endl;
  oss << "  errorTrail_data.addColumn({type:'string', role:'style'});" << endl;
  oss << "  errorTrail_data.addColumn({type:'string', role:'tooltip'});" << endl;
  oss << "  errorTrail_data.addRows([" << endl;   
  for ( const auto &p : curl_error_list ) {
    oss << "    [ ";
    oss << "new Date( " << p.datetime.year << "," << p.datetime.month-1 << "," << p.datetime.day << "," 
        << p.datetime.hour << "," << p.datetime.minute << "," << p.datetime.second << "), ";
    oss << error_code_index[p.curl_error] << ", ";    
    oss <<  "'point { fill-color: " << error_index_colors[error_code_index[p.curl_error]] << ";}', ";   
    oss <<  "'" << p.datetime.asString() << ", " << curlError2String( p.curl_error ) << "', ";   
    oss << "]," << endl;    
  }
  for ( const auto &p : http_error_list ) {
    oss << "  [ ";
    oss << "new Date( " << p.datetime.year << "," << p.datetime.month-1 << "," << p.datetime.day << "," 
        << p.datetime.hour << "," << p.datetime.minute << "," << p.datetime.second << "), ";
    oss << error_code_index[p.http_code] << ", ";   
    oss <<  "'point { fill-color: " << error_index_colors[error_code_index[p.http_code]] << ";}', ";  
    oss << "'" << p.datetime.asString() << ", " << HTTPCode2String( p.http_code ) << "', ";   
    oss << "]," << endl;       
  }  
  oss << "    ]);" << endl;

  unsigned int indent = 2;
  oss << string(indent,' ')   << "var errorTrail_options= {" << endl;
  oss << string(indent+2,' ') << "title: 'Probe errors'," << endl;
  oss << string(indent+2,' ') << chartarea_linechart << "," << endl;
  oss << string(indent+2,' ') << "width: " << 1800 << "," << endl;
  oss << string(indent+2,' ') << "height: " << 600 << "," << endl;
  oss << string(indent+2,' ') << "pointSize: 4," << endl;
  oss << string(indent+2,' ') << "fontSize: 10," << endl;
  oss << string(indent+2,' ') << "legend: { position: 'none' }," << endl;
  oss << string(indent+2,' ') << "backgroundColor: { fill:'transparent' }," << endl;
  //oss << string(indent+2,' ') << "showTextEvery: " << recent_probes.size() / 6 << "," << endl;
  oss << string(indent+2,' ') << "hAxis: {" << endl;
  oss << string(indent+4,' ') << "  title: 'datetime'," << endl;
  oss << string(indent+2,' ') << "}," << endl;
 
  oss << string(indent+2,' ') << "vAxis: {" << endl;
  oss << string(indent+2,' ') << "  textStyle: { color : '" << color_background_tab << "' }," << endl;
  oss << string(indent+2,' ') << "  minValue: -1," << endl;
  oss << string(indent+2,' ') << "  gridlines: { count: 0 }," << endl;
  oss << string(indent+2,' ') << "  maxValue: " << error_code_index.size() << "," << endl;
  oss << string(indent+4,' ') << "  baselineColor : '" << color_background_tab << "'," << endl;
  oss << string(indent+2,' ') << "}," << endl;

  oss << string(indent,' ')   << "};" << endl;

  oss << "  var errorTrail_chart = new google.visualization.ScatterChart(document.getElementById('errorTrail'));" << endl;  
  oss << "  errorTrail_chart.draw( errorTrail_data, errorTrail_options );" << endl;  
}

/**
 * Generate Google chart JavaScript for the History / wait class chart.
 */
void generateHistoryWCChart( ostringstream& oss ) {
  oss << "  var dateWaitClassTrail_data = google.visualization.arrayToDataTable([" << endl;
  oss << "  ['date','DNS', 'TCP', 'TLS', 'REQ', 'RSP', 'DAT','Mean','Standard deviation' ]," << endl;
  for ( const auto &d : total_date_map ) {
    oss << "    [ new Date(" << d.first.year << ", " << d.first.month-1 << ", " <<  d.first.day << ").toDateString(),";
    oss << d.second.namelookup.getMean() << ", ";
    oss << d.second.connect.getMean() << ", ";
    oss << d.second.appconnect.getMean() << ", ";
    oss << d.second.pretransfer.getMean() << ", ";
    oss << d.second.starttransfer.getMean() << ", ";
    oss << d.second.endtransfer.getMean() << ", ";
    oss << d.second.probe.getMean() << ", ";
    oss << d.second.probe.getSigma() << "], " << endl;
  }
  oss << "    ]);" << endl;

  unsigned int indent = 2;
  oss << string(indent,' ')   << "var dateWaitClassTrail_options= {" << endl;
  oss << string(indent+2,' ') << "title: 'Wait class history by date'," << endl;
  oss << string(indent+2,' ') << chartarea_linechart << "," << endl;
  oss << string(indent+2,' ') << "width: " << overview_linechart_width << "," << endl;
  oss << string(indent+2,' ') << "height: " << overview_linechart_wc_height << "," << endl;
  oss << string(indent+2,' ') << "isStacked: 'absolute'," << endl;
  oss << string(indent+2,' ') << "series: {" << endl;
  oss << string(indent+4,' ') << "0: {type: 'bars', color: '" + color_dns + "',curveType: 'function', dataOpacity: 0.8}," << endl;
  oss << string(indent+4,' ') << "1: {type: 'bars', color: '" + color_tcp + "',curveType: 'function', dataOpacity: 0.8}," << endl;
  oss << string(indent+4,' ') << "2: {type: 'bars', color: '" + color_tls + "',curveType: 'function', dataOpacity: 0.8}," << endl;
  oss << string(indent+4,' ') << "3: {type: 'bars', color: '" + color_req + "',curveType: 'function', dataOpacity: 0.8}," << endl;
  oss << string(indent+4,' ') << "4: {type: 'bars', color: '" + color_rsp + "',curveType: 'function', dataOpacity: 0.8}," << endl;
  oss << string(indent+4,' ') << "5: {type: 'bars', color: '" + color_dat + "',curveType: 'function', dataOpacity: 0.8}," << endl;
  oss << string(indent+4,' ') << "6: {type: 'line', color: '" + color_mean + "',curveType: 'function', opacity: 0.8}," << endl;
  oss << string(indent+4,' ') << "7: {type: 'line', color: '" + color_stddev + "',curveType: 'function', opacity: 0.8}," << endl;
  oss << string(indent+2,' ') << "}," << endl;   
  oss << string(indent+2,' ') << "lineWidth: 2," << endl;
  oss << string(indent+2,' ') << "fontSize: 10," << endl;
  oss << string(indent+2,' ') << "legend: { position: 'top' }," << endl;
  oss << string(indent+2,' ') << "backgroundColor: { fill:'transparent' }," << endl;
  oss << string(indent+2,' ') << "showTextEvery: " << total_date_map.size() / 6 << "," << endl;
  oss << string(indent+2,' ') << "hAxis: {" << endl;
  oss << string(indent+4,' ') << "title: 'date'," << endl;
  oss << string(indent+2,' ')   << "}," << endl;  
  oss << string(indent+2,' ') << "vAxis: {" << endl;
  oss << string(indent+4,' ') << "title: 'seconds'," << endl;
  oss << string(indent+4,' ') << "format: '##.###'," << endl;
  oss << string(indent+2,' ')   << "}," << endl;
  oss << string(indent,' ')   << "};" << endl;  


  oss << "  var dateWaitClassTrail_chart = new google.visualization.ColumnChart(document.getElementById('dateWaitClassTrail'));" << endl;  
  oss << "  dateWaitClassTrail_chart.draw( dateWaitClassTrail_data, dateWaitClassTrail_options );" << endl;  
}

/**
 * Generate Google chart JavaScript for the Average day chart.
 */
void generateAverageDayChart( ostringstream& oss ) {
  oss << "  var t24hmap_data = google.visualization.arrayToDataTable([" << endl;
  oss << "  ['date','DNS', 'TCP', 'TLS', 'REQ', 'RSP', 'DAT' ]," << endl;
  for ( const auto &d : total_day_map ) {
    oss << "    ['" << d.first.asString() << "', ";
    oss << d.second.namelookup.getMean() << ", ";
    oss << d.second.connect.getMean() << ", ";
    oss << d.second.appconnect.getMean() << ", ";
    oss << d.second.pretransfer.getMean() << ", ";
    oss << d.second.starttransfer.getMean() << ", ";
    oss << d.second.endtransfer.getMean() << "], " << endl;
  }
  oss << "    ]);" << endl;

  unsigned indent = 2;
  oss << string(indent,' ')   << "var t24hmap_options= {" << endl;
  oss << string(indent+2,' ') << "title: 'Wait class to time-of-day'," << endl;
  oss << string(indent+2,' ') << chartarea_linechart << "," << endl;
  oss << string(indent+2,' ') << "isStacked: 'absolute'," << endl;
  oss << string(indent+2,' ') << "width: " << 1800 << "," << endl;
  oss << string(indent+2,' ') << "height: " << 600 << "," << endl;
  oss << string(indent+2,' ') << "curveType: 'function'," << endl;
  oss << string(indent+2,' ') << "lineWidth: 2," << endl;
  oss << string(indent+2,' ') << "fontSize: 10," << endl;
  oss << string(indent+2,' ') << "backgroundColor: { fill:'transparent' }," << endl;
  oss << string(indent+2,' ') << "legend: { position: 'top' }," << endl;
  oss << string(indent+2,' ') << "hAxis: {" << endl;
  oss << string(indent+4,' ') << "title: 'time of day'," << endl;
  oss << string(indent+4,' ') << "showTextEvery: " << total_day_map.size() / 12 << "," << endl;
  oss << string(indent+2,' ')   << "}," << endl;  
  oss << string(indent+2,' ') << "vAxis: {" << endl;
  oss << string(indent+4,' ') << "title: 'seconds'," << endl;
  oss << string(indent+4,' ') << "format: '##.###'," << endl;
  oss << string(indent+2,' ')   << "}," << endl;
  oss << string(indent,' ')   << "};" << endl;

  oss << "  var t24hmap_chart = new google.visualization.AreaChart(document.getElementById('dailyBreakdown'));" << endl;  
  oss << "  t24hmap_chart.draw( t24hmap_data, t24hmap_options );" << endl;  
}

/**
 * Generate Google chart JavaScript for a histogram chart on the given QtyStats.
 */
void generateQtyStatsHistogram( ostringstream& oss, const QtyStats &stats, const string &title, const string &id  ) {
  oss << "  var " << id << "_data = google.visualization.arrayToDataTable([" << endl;
  oss << "    ['bucket','probes' ]," << endl;
  if ( stats.buckets.size() == 0 ) oss << "    ['<0', 0 ]" << endl;

  for ( const auto &b : stats.buckets ) {
    oss << "    ['< " << fixed << b.first << "', " << b.second << "]," << endl;
  }
  oss << "    ]);" << endl;

  unsigned indent = 2;
  oss << string(indent,' ')   << "var " << id << "_options= {" << endl;
  oss << string(indent+2,' ') << "title: '" << title << " (bucket " << stats.current_bucket << "s)'," << endl;
  oss << string(indent+2,' ') << "isStacked: 'absolute'," << endl;
  oss << string(indent+2,' ') << chartarea_histogram << "," << endl;
  oss << string(indent+2,' ') << "width: " << 400 << "," << endl;
  oss << string(indent+2,' ') << "height: " << 300 << "," << endl;
  oss << string(indent+2,' ') << "fontSize: 10," << endl;
  oss << string(indent+2,' ') << "colors: ['" << color_anonymous << "']," << endl;
  oss << string(indent+2,' ') << "backgroundColor: { fill:'transparent' }," << endl;
  oss << string(indent+2,' ') << "legend: { position: 'none' }," << endl;
  oss << string(indent+2,' ') << "hAxis: {" << endl;
  oss << string(indent+4,' ') << "title: 'bucket'," << endl;
  oss << string(indent+2,' ')   << "}," << endl;  
  oss << string(indent+2,' ') << "vAxis: {" << endl;
  oss << string(indent+4,' ') << "title: 'probes'," << endl;
  oss << string(indent+4,' ') << "format: '##.###'," << endl;
  oss << string(indent+4,' ') << "scaleType: 'log'," << endl;
  oss << string(indent+2,' ')   << "}," << endl;
  oss << string(indent,' ')   << "};" << endl;

  oss << "  var " << id << "_chart = new google.visualization.ColumnChart(document.getElementById('" << id << "'));" << endl;  
  oss << "  " << id << "_chart.draw( " << id << "_data, " << id << "_options );" << endl;   
}

/**
 * Generate all histograms.
 */
void generateTotalHistogram( ostringstream& oss ) {
  generateQtyStatsHistogram( oss, globalstats.response_stats, "Total response", "totalHistogram" );
  generateQtyStatsHistogram( oss, globalstats.wait_class_stats.namelookup, "DNS", "dnsHistogram" );
  generateQtyStatsHistogram( oss, globalstats.wait_class_stats.connect, "TCP", "tcpHistogram" );
  generateQtyStatsHistogram( oss, globalstats.wait_class_stats.appconnect, "TLS", "tlsHistogram" );
  generateQtyStatsHistogram( oss, globalstats.wait_class_stats.pretransfer, "REQ", "reqHistogram" );
  generateQtyStatsHistogram( oss, globalstats.wait_class_stats.starttransfer, "RSP", "rspHistogram" );
  generateQtyStatsHistogram( oss, globalstats.wait_class_stats.endtransfer, "DAT", "datHistogram" );
}

/**
 * Generate the HTML header.
 */
void generateHeader( ostringstream& oss ) {
  oss << "<!Doctype html>" << endl;  
  oss << "<html lang=en>" << endl;
  oss << "<head>" << endl;
  oss << "<meta name=\"Author\" content=\"curlstats\">" << endl;
  oss << "<meta charset=\"utf-8\">" << endl;
  oss << "<title>curlstats " << comments.client_fqdn << " " << comments.request << " " << comments.url << "</title>" << endl;
  oss << "<script src=\"https://www.gstatic.com/charts/loader.js\"></script>" << endl;

  oss << "<script>" << endl;  
  
  oss << "function drawCharts() {" << endl;


  generateSummaryQoSPieChart( oss );

  generateSummaryWCAveragePieChart( oss );

  generateSummaryWCSlowTotalPieChart( oss );

  generateSummaryMinMaxChart( oss );

  //generateHistoryResponseChart( oss );

  generateHistoryWCChart( oss );

  generateHistoryQosChart( oss );

  generateRecentChart( oss );

  generateAverageDayChart( oss );

  generateTotalHistogram( oss );

  generateErrorTrail( oss );

  oss << "}" << endl;

  oss << "google.charts.load('current', {'packages':['corechart', 'timeline']});" << endl;
  oss << "google.charts.setOnLoadCallback(drawCharts);" << endl;    

  generateTabbingJavaScript( oss );

  oss << "</script>" << endl;
  generateCSS( oss );

  oss << "</head>" << endl;  
}

/**
 * Generate the history tab.
 */
void generateHistory( ostringstream& oss ) {
  oss << "<div id=\"History\" class=\"tabcontent\">" << endl;
  oss << "<table>" << endl;
  //oss << "<tr>" << endl;
  //oss << "<td><div id=\"dateTrail\"></div></td>" << endl;
  //oss << "</tr>" << endl;
  oss << "<tr>" << endl;
  oss << "<td ><div id=\"dateWaitClassTrail\"></div></td>" << endl;
  oss << "</tr>" << endl;
  oss << "<tr>" << endl;
  oss << "<td ><div id=\"qosTrail\"></div></td>" << endl;
  oss << "</tr>" << endl;
  oss << "</table>" << endl;

  oss << "</div>" << endl;  
}

/**
 * Generate the Summary tab.
 */
void generateSummary( ostringstream& oss ) {
  oss << "<div id=\"Summary\" class=\"tabcontent\">" << endl;

  oss << "<table>" << endl; // left 2x2 table
  oss << "<tr><td>" << endl; // left 2x2 table

  oss << "<table><tr><td>" << endl;

  oss << "<table class=\"usertable\">" << endl;
  oss << "<tr><th>total probes</th><td>" << globalstats.total_probes << "</td></tr>" << endl;
  oss << "<tr><th>timed probes</th><td>" << globalstats.timed_probes << "</td></tr>" << endl;
  oss << "<tr><th>slow probes</th><td>" << globalstats.items_slow << "</td></tr>" << endl;
  oss << "<tr><th>probe errors</th><td>" << curl_error_list.size() << "</td></tr>" << endl;
  oss << "<tr><th>HTTP errors</th><td>" << http_error_list.size() << "</td></tr>" << endl;
  size_t total_outside_qos = globalstats.items_slow + curl_error_list.size() + http_error_list.size();  
  oss << "<tr><th>QoS</th><td>" << num( (1.0-(double)total_outside_qos/(double)globalstats.total_probes)*100.0 ) << "%</td></tr>" << endl;
  oss << "<tr><th>mean response</th><td>" << num( globalstats.total_time / globalstats.timed_probes ) << "s</td></tr>" << endl;
  oss << "<tr><th>ideal response</th><td>" << num( globalstats.wait_class_stats.getIdealResponse() ) << "s</td></tr>" << endl;
  oss << "<tr><th>minimum response</th><td>" << num( globalstats.response_stats.min ) << "s</td></tr>" << endl;
  oss << "<tr><th>maximum response</th><td>" << num( globalstats.response_stats.max ) << "s</td></tr>" << endl;
  oss << "<tr><th>estimate route RTT</th><td>" << num( globalstats.wait_class_stats.getNetworkRoundtrip()*1000.0 ) << "ms</td></tr>" << endl;
  oss << "<tr><th>avg bytes up</th><td>" << num( (double)globalstats.size_upload/(double)globalstats.timed_probes/1024.0 ) << "KiB</td></tr>" << endl;
  oss << "<tr><th>avg bytes down</th><td>" << num( (double)globalstats.size_download/(double)globalstats.timed_probes/1024.0 ) << "KiB</td></tr>" << endl;
  oss << "</table>" << endl;

  oss << "</td><td>" << endl;
  oss << "<div id=\"probePie\"></div>" << endl;    
  oss << "</td></tr>" << endl;

  oss << "<tr><td>" << endl;
  oss << "<div id=\"waitClassPie\"></div>" << endl;
  oss << "</td><td>" << endl;
  oss << "<div id=\"waitClassSlowPie\"></div>" << endl;  
  oss << "</td></tr>" << endl;
  oss << "</table>" << endl;

  oss << "</td><td>" << endl;
  oss << "<div id=\"minMax\"></div>" << endl;
  oss << "</td></tr></table>" << endl;
  
  oss << "</div>" << endl;
}

/**
 * Generate the Errors tab.
 */
void generateErrors( ostringstream& oss ) {
  oss << "<div id=\"Errors\" class=\"tabcontent\">" << endl;
  oss << "<table>" << endl;
  oss << "<tr>" << endl;

  oss << "<td style=\"vertical-align: top;\">" << endl;
  oss << "<table class=\"usertable\">" << endl;
  oss << "<tr><th>Curl error</th><th>probes</th><th>percentage</th><th></th></tr>";
  for ( const auto &h : curl_error_map ) {
    const string td = "<td>";
    oss << "<tr><td>" 
        << curlError2String( h.first ) 
        << "</td><td>" 
        << h.second 
        << "</td><td>"
        << num( (double)h.second/globalstats.total_probes*100.0 );
    if ( h.first > 0 ) 
      oss << "</td><td style=\"background-color: " << error_index_colors[error_code_index[h.first]]<< "\">&nbsp;</td></tr>";
    else
      oss << "</td><td>&nbsp;</td></tr>";
    oss << endl;
  }
  oss << "</table>" << endl;  
  oss << "</td>" << endl;

  oss << "<td style=\"vertical-align: top;\">" << endl;
  oss << "<table class=\"usertable\">" << endl;
  oss << "<tr><th>HTTP code</th><th>probes</th><th>percentage</th><th></th></tr>";
  for ( const auto &h : http_code_map ) {
    const string td = "<td>";
    oss << "<tr><td>" 
        << HTTPCode2String( h.first ) 
        << "</td><td>"
        << h.second 
        << "</td><td>"
        << num( (double)h.second/globalstats.total_probes*100.0 );
    if ( h.first > 200 ) 
      oss << "</td><td style=\"background-color: " << error_index_colors[error_code_index[h.first]]<< "\">&nbsp;</td></tr>";
    else
      oss << "</td><td>&nbsp;</td></tr>";
    oss << endl;
  }
  oss << "</table>" << endl;
  oss << "</td>" << endl;
  oss << "</tr>" << endl;

  oss << "<tr><td colspan=\"2\">" << endl;
  oss << "<table>" << endl;
  oss << "<tr>" << endl;
  oss << "<td><div id=\"errorTrail\"></div></td>" << endl;
  oss << "</tr>" << endl;
  oss << "</table>" << endl;
  oss << "</td></tr>" << endl;


  oss << "</table>" << endl;
  oss << "</div>" << endl;
}

/**
 * Generate the Average day tab.
 */
void generateAverageDay( ostringstream& oss ) {
  oss << "<div id=\"Average_day\" class=\"tabcontent\">" << endl;
  oss << "<p class=\"gentle\">Mean value over " << options.day_bucket 
      << " minute intervals, the last interval covering 23:" << 60 - options.day_bucket << "-00:00.</p>" << endl;
  oss << "<table>" << endl;
  oss << "<tr>" << endl;
  oss << "<td><div id=\"dailyBreakdown\"></div></td>" << endl;
  oss << "</tr>" << endl;
  oss << "</table>" << endl;
  oss << "</div>" << endl;  
}

/**
 * Generate the histograms tab.
 */
void generateHistograms( ostringstream& oss ) {
  oss << "<div id=\"Histograms\" class=\"tabcontent\">" << endl;
  oss << "<table>" << endl;
  oss << "<tr>" << endl;
  oss << "<td><div id=\"totalHistogram\"></div></td>" << endl;
  oss << "<td><div id=\"dnsHistogram\"></div></td>" << endl;
  oss << "<td><div id=\"tcpHistogram\"></div></td>" << endl;
  oss << "<td><div id=\"tlsHistogram\"></div></td>" << endl;
  oss << "</tr><tr>" << endl;
  oss << "<td><div id=\"reqHistogram\"></div></td>" << endl;
  oss << "<td><div id=\"rspHistogram\"></div></td>" << endl;
  oss << "<td colspan=\"2\"><div id=\"datHistogram\"></div></td>" << endl;
  
  oss << "</tr>" << endl;
  oss << "</table>" << endl;
  oss << "</div>" << endl;    
}

/**
 * Generate the Origin tab.
 */
void generateOrigin( ostringstream& oss ) {
  oss << "<div id=\"Origin\" class=\"tabcontent\">" << endl;
  oss << "<table>" << endl;
  oss << "<tr>" << endl;
  oss << "<td>" << endl;

  oss << "<table class=\"usertable\">" << endl;
  for ( const auto &c : comments.comments ) {
    oss << "<tr><th>" << c.first << "</th><td>" << c.second << "</td></tr>" << endl;
  }
  oss << "</table>" << endl;

  oss << "</td>" << endl;
  oss << "</tr>" << endl;
  oss << "</table>" << endl;
  oss << "</div>" << endl;    
}

/**
 * Generate the Recent tab.
 */
void generateRecent( ostringstream& oss ) {
  oss << "<div id=\"Recent\" class=\"tabcontent\">" << endl;
  oss << "<table>" << endl;
  oss << "<tr>" << endl;

  oss << "<td><div id=\"recentTrail\"></div></td>" << endl;

  oss << "</tr>" << endl;
  oss << "</table>" << endl;
  oss << "</div>" << endl;  
}

/**
 * Generate a gradient color.
 * @value The value to generate a color for.
 * @cutoff The cutoff value beyond which the gradient runs from color_warn to color_bad
 * @min The minimum value among those to be presented.
 * @max The minimum value among those to be presented.
 */
string colorGradient( double value, double cutoff, double min, double max ) {
  RGB color_this = { 0, 0, 0 };  
  if ( value < cutoff ) {
    double dr = color_warn.red - color_ok.red;
    double dg = color_warn.green - color_ok.green;
    double db = color_warn.blue - color_ok.blue;
    double ratio = value / (cutoff-min);
    //cout << "value < cutoff " << ratio << " min=" << min << "max=" << max << " cutoff=" << cutoff << endl;    
    color_this  = { color_ok.red + (int)(dr * ratio), 
                    color_ok.green + (int)(dg * ratio), 
                    color_ok.blue + (int)(db * ratio) };             
  } else {
    int dr = color_bad.red - color_warn.red;
    int dg = color_bad.green - color_warn.green;
    int db = color_bad.blue - color_warn.blue;
    double ratio = (value-cutoff) / (max-cutoff);
    //cout << "value >= cutoff " << ratio << endl;    
    color_this  = { color_warn.red + (int)(dr * ratio), 
                    color_warn.green + (int)(dg * ratio), 
                    color_warn.blue + (int)(db * ratio) };    
  }
  ostringstream oss;
  oss << "#" << hex << setfill('0') << setw(2) <<  color_this.red << setw(2) 
      << color_this.green << setw(2) << color_this.blue;
  return oss.str();
}

void generateWeekmapResponse( ostringstream& oss ) {
  double minval = numeric_limits<double>::max();
  double maxval = numeric_limits<double>::min();
  for ( const auto &wd : weekmap_qtystats ) {
    for ( const auto &hh : wd.second ) {
      if ( hh.second.getMean() > maxval ) maxval = hh.second.getMean();
      if ( hh.second.getMean() < minval ) minval = hh.second.getMean();
    }
  }  
  oss << "<table class=\"heatmap\">" << endl;
  oss << "<caption>Response time weekmap</caption>" << endl;
  list<TimeKey> timebuckets;
  int h = 0, m = 0;
  ostringstream hour_headers;
  ostringstream minute_headers;
  hour_headers << "<td></td>";
  minute_headers << "<td></td>";
  for ( int i = 0; i < 24*60/options.weekmap_bucket; i++ ) {
    TimeKey tk( h, m );
    timebuckets.push_back( tk );
    if ( m == 0 ) hour_headers << "<th colspan=\"" << 60 / options.weekmap_bucket << "\">" << setfill('0') << setw(2) << tk.hour << "h</th>" << endl;
    minute_headers << "<td>" << setfill('0') << setw(2) << tk.minute << "m</td>" << endl;
    m += options.weekmap_bucket;
    if ( m == 60 ) { h++; m = 0; }
  }
  oss << "<tr>" << hour_headers.str() << "</tr>" << endl;
  oss << "<tr>" << minute_headers.str() << "</tr>" << endl;

  for ( const auto &wd : weekmap_qtystats ) {
    oss << "<tr><th>" << dowStr( wd.first ) << "</th>" << endl;  
    for ( const auto &t : timebuckets ) {
      const auto i = wd.second.find( t );
      if ( i != wd.second.end() ) {
        oss << "<td style=\"background-color: " 
            << colorGradient( (*i).second.getMean(), options.slow_threshold, minval, maxval ) << "\" ";
        oss << "title=\"" << dowStr( wd.first ) << " " << t.asString() << " response time=" 
            << num( (*i).second.getMean() ) <<  "s\">&nbsp;</td>" << endl;
      } else {
        oss << "<td style=\"background-color: #888888\" ";
        oss << "title=\"no data\">&nbsp;</td>" << endl;
      }
    }
    oss << "</tr>" << endl;  
  }
  oss << "</table>" << endl;
  oss << "<table class=\"heatmaplegend\">" << endl;
  oss << "<tr><td>" << num(minval) << "s </td><td style=\"border: 1px solid black; background-color: " 
      << colorGradient( minval, options.slow_threshold, minval, maxval ) << "\">&nbsp;</td>" << endl;
  oss << "<td>" << num(maxval) << "s </td><td style=\"border: 1px solid black; background-color: " 
      << colorGradient( maxval, options.slow_threshold, minval, maxval ) << "\">&nbsp;</td></tr>" << endl;
  oss << "</table>" << endl;  
}

void generateWeekmapQoS( ostringstream& oss ) {
  double minval = numeric_limits<double>::max();
  double maxval = numeric_limits<double>::min();
  for ( const auto &wp : weekmap_probestats ) {
    for ( const auto &hh : wp.second ) {
      if ( 100.0 - hh.second.getQoS() > maxval ) maxval = 100.0 - hh.second.getQoS();
      if ( 100.0 - hh.second.getQoS() < minval ) minval = 100.0 - hh.second.getQoS();
    }
  }  
  oss << "<table class=\"heatmap\">" << endl;
  oss << "<caption>QoS weekmap</caption>" << endl;
  list<TimeKey> timebuckets;
  int h = 0, m = 0;
  ostringstream hour_headers;
  ostringstream minute_headers;
  hour_headers << "<td></td>";
  minute_headers << "<td></td>";
  for ( int i = 0; i < 24*60/options.weekmap_bucket; i++ ) {
    TimeKey tk( h, m );
    timebuckets.push_back( tk );
    if ( m == 0 ) hour_headers << "<th colspan=\"" << 60 / options.weekmap_bucket << "\">" 
      << setfill('0') << setw(2) << tk.hour << "h</th>" << endl;
    minute_headers << "<td>" << setfill('0') << setw(2) << tk.minute << "m</td>" << endl;
    m += options.weekmap_bucket;
    if ( m == 60 ) { h++; m = 0; }
  }
  oss << "<tr>" << hour_headers.str() << "</tr>" << endl;
  oss << "<tr>" << minute_headers.str() << "</tr>" << endl;

  for ( const auto &wp : weekmap_probestats ) {
    oss << "<tr><th>" << dowStr( wp.first ) << "</th>" << endl;  
    for ( const auto &t : timebuckets ) {
      const auto i = wp.second.find( t );
      if ( i != wp.second.end() ) {
        oss << "<td style=\"background-color: " 
            << colorGradient( 100.0 - (*i).second.getQoS(), qos_cutoff,  minval, maxval ) << "\" ";
        oss << "title=\"" << dowStr( wp.first ) << " " 
            << t.asString() << " QoS=" << num( (*i).second.getQoS(), 1 ) << "% "
            << num( (*i).second.getProbeErrorPct(),1  ) << "% probe errors "
            << num( (*i).second.getHTTPErrorPct(),1  ) << "% http errors "
            << num( (*i).second.getSlowPct(),1  ) << "% slow "
            <<  "\">&nbsp;</td>" << endl;
      } else {
        oss << "<td style=\"background-color: #888888\" ";
        oss << "title=\"no data\">&nbsp;</td>" << endl;
      }
    }
    oss << "</tr>" << endl;  
  }
  oss << "</table>" << endl;
  oss << "<table class=\"heatmaplegend\">" << endl;
  oss << "<tr><td>" << num(100.0 - minval) 
      << "% </td><td style=\"border: 1px solid black; background-color: " 
      << colorGradient( minval, qos_cutoff,  minval, maxval ) << "\">&nbsp;</td>" << endl;
  oss << "<td>" << num(100.0 - maxval) << "% </td><td style=\"border: 1px solid black; background-color: " 
      << colorGradient( maxval, qos_cutoff,  minval, maxval ) << "\">&nbsp;</td></tr>" << endl;
  oss << "</table>" << endl;  
}

void generateWeekmap( ostringstream& oss ) {

  oss << "<div id=\"Weekmap\" class=\"tabcontent\">" << endl;
  generateWeekmapResponse( oss );
  generateWeekmapQoS( oss );
  oss << "</div>" << endl;  
}

std::string HTML::generate() const {
  // generate a mapping from curl+http error to a zero-based index
  size_t index = 0;
  for ( const auto &c : curl_error_map ) {
    if ( c.first ) {
      error_code_index[c.first] = index % error_index_colors.size();
      index++;
    }
  }
  for ( const auto &h : http_code_map ) {
    if ( h.first >= 400 ) {
      error_code_index[h.first] = index % error_index_colors.size();
      index++;
    }
  }  

  ostringstream oss;
  generateHeader( oss );


  oss << "<body>" << endl;
  oss << "<table class=\"header\">" << endl;
  oss << "<tr><th colspan=\"8\" class=\"caption\">" << comments.request << " " 
      << comments.url << "</th></tr>" << endl;
  oss << "<tr>" << endl;
  oss << "<th>from</th><td>" << comments.client_fqdn << "&nbsp;(" << comments.client_ip << ")</td>" << endl;
  oss << "<th>first probe</th><td>" << globalstats.first_time.asString() << "</td>" << endl;
  oss << "<th>last probe</th><td>" << globalstats.last_time.asString() << "</td>" << endl;
  oss << "<th>slow threshold</th><td>" << num( options.slow_threshold ) << "s</td>" << endl;
  oss << "</tr>" << endl;
  oss << "</table>" << endl;


  oss << "<div class=\"tab\">" << endl;
  oss << "<button class=\"tablinks\" onclick=\"openTab(event, 'Summary')\" id=\"defaultTab\">Summary</button>" << endl;
  oss << "<button class=\"tablinks\" onclick=\"openTab(event, 'Recent')\">Recent</button>" << endl;
  oss << "<button class=\"tablinks\" onclick=\"openTab(event, 'History')\">History</button>" << endl;
  oss << "<button class=\"tablinks\" onclick=\"openTab(event, 'Average_day')\">Average day</button>" << endl;
  oss << "<button class=\"tablinks\" onclick=\"openTab(event, 'Weekmap')\">Week map</button>" << endl;
  oss << "<button class=\"tablinks\" onclick=\"openTab(event, 'Histograms')\">Histograms</button>" << endl;
  oss << "<button class=\"tablinks\" onclick=\"openTab(event, 'Errors')\">Errors</button>" << endl;  
  oss << "<button class=\"tablinks\" onclick=\"openTab(event, 'Origin')\">Origin</button>" << endl;
  oss << "<button class=\"tablinks\" onclick=\"openTab(event, 'Help')\">Help</button>" << endl;
  oss << "</div>" << endl;

  generateSummary( oss );
  generateHistory( oss );
  generateRecent( oss );
  generateAverageDay( oss );
  generateWeekmap( oss );
  generateHistograms( oss );
  generateOrigin( oss );
  generateErrors( oss );

  oss << "<div id=\"Help\" class=\"tabcontent\">" << endl;
  oss << "<iframe src=\"" << github_pages_url << "\" title=\"curlstats github pages\" "
         "style=\"position: relative; border-style:none; height: 700px; width: 100%;\"></iframe>" << endl;
  oss << "<p>Repository <a href=\"" << github_repo_url << "\">curlstats on GitHub</a>.</p>" << endl;
  oss << "</div>" << endl;

  oss << "</body>" << endl;
  oss << "</html>" << endl;
  return oss.str();
}
