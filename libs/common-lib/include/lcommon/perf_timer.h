/*
 * perf_timer.h:
 *  Provides timing information on a per-method basis.
 *  Based on a global PerfTimer, but does not expose that class.
 *  Public domain (by Adam Domurad)
 */

#ifndef LCOMMON_PERF_TIMER_H_
#define LCOMMON_PERF_TIMER_H_

// Define a cross-platform function name identifier
#ifdef _MSC_VER
#define FUNCNAME __FUNCSIG__
#else
#ifdef __GNUC__
#define FUNCNAME __PRETTY_FUNCTION__
#else
#define FUNCNAME __func__
#endif
#endif

void perf_timer_begin(const char* funcname);
void perf_timer_end(const char* funcname);
double perf_timer_average_time(const char* funcname);
void perf_timer_clear();
void perf_print_results();

struct PerfCount {
    PerfCount(const char* funcname) : funcname(funcname){
        perf_timer_begin(funcname);
    }
    ~PerfCount() {
        perf_timer_end(funcname);
    }
private:
    const char* funcname;
};

#ifndef NDEBUG
#define PERF_TIMER() PerfCount __perf_count(FUNCNAME)
#else
#define PERF_TIMER() PerfCount __perf_count(FUNCNAME)
#endif


#endif /* LCOMMON_PERF_TIMER_H_ */
