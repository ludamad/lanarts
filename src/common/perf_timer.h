/*
 * perf_timer.h:
 *  Provides timing information on a per-method basis.
 *  Based on a global PerfTimer, but does not expose that class.
 */

#ifndef LCOMMON_PERF_TIMER_H_
#define LCOMMON_PERF_TIMER_H_

void perf_timer_begin(const char* funcname);
void perf_timer_end(const char* funcname);
void perf_timer_clear();
void perf_print_results();

#endif /* LCOMMON_PERF_TIMER_H_ */
