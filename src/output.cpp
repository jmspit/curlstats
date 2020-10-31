#include "output.h"

/**
 * Write a heading.
 */
void heading( const string &h, char c, int width ) {
  cout << endl << string( width, c ) << endl;
  cout << h << endl;
  cout << string( width, c ) << endl;
}

/**
 * convert a dow-of-week to a string description.
 */
string dowStr( int dow ) {
  switch( dow ) {
    case 0 : return "Sunday";
    case 1 : return "Monday";
    case 2 : return "Tuesday";
    case 3 : return "Wednesday";
    case 4 : return "Thursday";
    case 5 : return "Friday";
    case 6 : return "Saturday";
    default : return "ErrorDay";
  }
};

/**
 * convert a HTTP code to a string description.
 */
string HTTPCode2String( uint16_t code ) {
  stringstream ss;
  switch ( code ) {
    case 200 :
      ss << "OK";
      break;
    case 201 :
      ss << "Created";
      break;
    case 202 :
      ss << "Accepted";
      break;
    case 204 :
      ss << "No Content";
      break;

    case 301:
      ss << "Moved Permanently";
      break;
    case 302:
      ss << "Found / moved temporarily";
      break;
    case 304:
      ss << "Not modified";
      break;
    case 307:
      ss << "Temporary Redirect";
      break;
    case 308:
      ss << "Permanent Redirect";
      break;


    case 400:
      ss << "Bad request";
      break;
    case 401:
      ss << "Unauthorized";
      break;
    case 402:
      ss << "Payment Required";
      break;
    case 403:
      ss << "Forbidden";
      break;
    case 404:
      ss << "Not found";
      break;
    case 405:
      ss << "Method Not Allowed";
      break;
    case 406:
      ss << "Not Acceptable";
      break;
    case 407:
      ss << "Proxy Authentication Required";
      break;
    case 408:
      ss << "Request Timeout";
      break;
    case 409:
      ss << "Conflict";
      break;
    case 410:
      ss << "Gone";
      break;

    case 500:
      ss << "Internal Server Error";
      break;
    case 501:
      ss << "Not Implemented";
      break;
    case 502:
      ss << "Bad Gateway";
      break;
    case 503:
      ss << "Service unavailable";
      break;
    case 504:
      ss << "Gateway timeout";
      break;
  }
  if ( ss.str().size() ) ss << " ";
  ss << "(" << code << ")";
  return ss.str();
};

/**
 * convert a curl error to a string description.
 */
string curlError2String( uint16_t code ) {
  stringstream ss;
  switch ( code ) {
    case 0:
      ss << "CURLE_OK";
      break;
    case 5:
      ss << "CURLE_COULDNT_RESOLVE_PROXY";
      break;
    case 6:
      ss << "CURLE_COULDNT_RESOLVE_HOST";
      break;
    case 7:
      ss << "CURLE_COULDNT_CONNECT";
      break;
    case 8:
      ss << "CURLE_WEIRD_SERVER_REPLY";
      break;
    case 9:
      ss << "CURLE_REMOTE_ACCESS_DENIED";
      break;     
    case 16:
      ss << "CURLE_HTTP2";
      break;     
    case 18:
      ss << "CURLE_PARTIAL_FILE";
      break;                  
    case 28:
      ss << "CURLE_OPERATION_TIMEDOUT";
      break;
    case 35:
      ss << "CURLE_SSL_CONNECT_ERROR";
      break;
    case 47:
      ss << "CURLE_TOO_MANY_REDIRECTS";
      break;      
    case 52:
      ss << "CURLE_GOT_NOTHING";
      break;
    case 55:
      ss << "CURLE_SEND_ERROR";
      break;
    case 56:
      ss << "CURLE_RECV_ERROR";
      break;
    case 67:
      ss << "CURLE_LOGIN_DENIED";
      break;      
    case 92:
      ss << "CURLE_HTTP2_STREAM";
      break;      
  }
  if ( ss.str().length() > 0 ) ss << " ";
  ss << "(" << code << ")";
  return ss.str();
}