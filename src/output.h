#ifndef output_h
#define output_h

#include <iomanip>
#include <iostream>

using namespace std;

#define FIXED3 fixed << setfill(' ') << setprecision(3)
#define FIXED3W7 fixed << setfill(' ') << setprecision(3) << setw(7)
#define FIXED3W10 fixed << setfill(' ') << setprecision(3) << setw(10)
#define FIXEDINT fixed << setfill(' ') << setprecision(0) << setw(9)
#define FIXEDPCT fixed << setfill(' ') << setprecision(2) << setw(6)

enum OutputMode : unsigned long {
  omNone                 = 0b0000000000000000, /**< Output nothing */
  omAll                  = 0b0000000000000001, /**< Output everything 'all'*/
  omSlowTrail            = 0b0000000000000010, /**< Output a trail of slow probes 'slowtrail' */
  omHistograms           = 0b0000000000000100, /**< Output wait class histograms 'histo' */
  omDailyTrail           = 0b0000000000001000, /**< Output the daily trail 'daytrail'*/
  om24hMap               = 0b0000000000010000, /**< Output the 24h map '24hmap' */
  omWeekdayMap           = 0b0000000000100000, /**< Output the Weekday map 'wdmap'  */
  om24hSlowMap           = 0b0000000001000000, /**< Output the 24h slow map '24hslowmap' */
  omWeekdaySlowMap       = 0b0000000010000000, /**< Output the Weekday slow map 'wdslowmap' */
  omErrors               = 0b0000000100000000, /**< Output trail of errors 'errortrail' */
  omGlobal               = 0b0000001000000000, /**< Output global stats 'global' */
  omOptions              = 0b0000010000000000, /**< Output options 'options' */
  omComments             = 0b0000100000000000, /**< Output comments 'comments' */
  omSlowWaitClass        = 0b0001000000000000, /**< Output comments 'slowwait' */
};

inline OutputMode operator|( OutputMode a, OutputMode b ) {
  return static_cast<OutputMode>(static_cast<unsigned long>(a) | static_cast<unsigned long>(b));
}

inline OutputMode& operator|=( OutputMode &a, OutputMode b ) {
  a = static_cast<OutputMode>(static_cast<unsigned long>(a) | static_cast<unsigned long>(b));
  return a;
}

/**
 * Write a heading.
 */
void heading( const string &h, char c = '=', int width = 176 );

/**
 * convert a dow-of-week to a string description.
 */
string dowStr( int dow );

/**
 * convert a HTTP code to a string description.
 */
string HTTPCode2String( uint16_t code );

/**
 * convert a curl error to a string description.
 */
string curlError2String( uint16_t code );

#endif