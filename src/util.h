#ifndef util_h
#define util_h

#include <chrono>
#include <cmath>
#include <string>
#include <vector>

using namespace std;

/**
 * Split a string.
 */
vector<string> split( const string& src, char delimiter = ';' );

/**
 * Bucket a real value to its ceil (largest integer above).
 */
double bucket( double v, double bucket );

/**
 * return true if the line is a comment.
 */
bool isCommment( const string& s );

inline std::string& ltrim(std::string& s, const char* t = " \t\n\r\f\v")
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}

inline std::string& rtrim(std::string& s, const char* t = " \t\n\r\f\v")
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

inline std::string& trim(std::string& s, const char* t = " \t\n\r\f\v")
{
    return ltrim(rtrim(s, t), t);
}

  /**
   * StopWatch timing class.
   * @code
   * StopWatch sw;
   * sw.start();
   * ...
   * sw.stop();
   * double seconds = sw.getElapsedSecond();
   * @endcode
   */
  class StopWatch {
    public:

      /**
       * Construct a StopWatch. The start time is taken here, or can be reset by calling start().
       */
      StopWatch() {
        stop_ = std::chrono::high_resolution_clock::now() - std::chrono::hours(24);
        start_ = std::chrono::high_resolution_clock::now();
      };

      /**
       * Start the stopwatch. If calling start() is omitted,
       * start_ is set to the time StopWatch::StopWatch() was called.
       */
      void start() {
        stop_ = std::chrono::high_resolution_clock::now() - std::chrono::hours(24);
        start_ = std::chrono::high_resolution_clock::now();
      }

      /**
       * Stop the stopwatch.
       */
      void stop() { stop_ = std::chrono::high_resolution_clock::now(); }

      /**
       * Return the number of seconds between
       * - start() and stop() calls
       * - start() and time of this call
       * - StopWatch constructor and time of this call.
       * @return The number of seconds.
       */
      double getElapsedSeconds() const {
        std::chrono::duration<double> diff;
        if ( stop_ < start_ ) {
          diff = std::chrono::high_resolution_clock::now() - start_;
        } else {
          diff = stop_ - start_;
        }
        return diff.count();
      };

    private:
      /** Start time */
      std::chrono::time_point<std::chrono::high_resolution_clock> start_;
      /** Stop time */
      std::chrono::time_point<std::chrono::high_resolution_clock> stop_;

  };

#endif