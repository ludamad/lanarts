/*
 * GameLogger.h:
 *  Handles detailed logging of game events, which are used to debug game problems.
 *  These can be automatically diff'ed to see differences in online play.
 */

#ifndef GAMELOGGER_H_
#define GAMELOGGER_H_

#include <fstream>
#include <string>

class GameState;

class GameLogger {
public:
	GameLogger();
	void initialize_logs(GameState* gs, const char* input_log,
			const char* output_log);
	~GameLogger();
	void event_log(const char* fmt, va_list ap);
	void event_log(const char* fmt, ...);
        // If we call event_log, will output be printed anywhere?
        bool is_active();
private:
	GameState* gs;
	std::ifstream input_log_file;
	std::ofstream output_log_file;
	std::string line;
};

// If we call event_log, will output be printed anywhere?
bool event_log_is_active();
void event_log(const char* fmt, ...);
void event_log_initialize(GameState* gs, const char* input_file,
		const char* output_file);

#endif /* GAMELOGGER_H_ */
