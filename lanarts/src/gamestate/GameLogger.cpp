/*
 * GameLogger.cpp:
 *  Handles detailed logging of game events, which are used to debug game problems.
 *  These can be automatically diff'ed to see differences in online play.
 */

#include <lcommon/strformat.h>

#include "GameState.h"
#include "GameMapState.h"
#include "GameLogger.h"

GameLogger::GameLogger() :
		gs(NULL), input_log_file(), output_log_file() {
}

void GameLogger::initialize_logs(GameState* gs, const char* input_log,
		const char* output_log) {
	this->gs = gs;
	if (input_log) {
		printf("Comparing to log file %s\n", input_log);
		input_log_file.open(input_log, std::ios_base::in);
		if (!input_log_file) {
			printf("Failed to open log file %s\n", input_log);
		}
	}
	if (output_log) {
		printf("Logging to file %s\n", output_log);
		output_log_file.open(output_log, std::ios_base::out);
		if (!output_log_file) {
			printf("Failed to open log file %s\n", input_log);
		}
	}
}

GameLogger::~GameLogger() {
	if (input_log_file) {
		input_log_file.close();
	}
	if (output_log_file) {
		output_log_file.close();
	}
}

void GameLogger::event_log(const char *fmt, va_list ap) {
	static std::string logline;

	if (!gs || !output_log_file || gs->get_level()->id() == -1) {
		va_end(ap);
		return;
	}
	format(logline, "Frame %d Level %d: ",
			gs->frame(), gs->get_level()->id());
	char text[512];
	vsnprintf(text, sizeof(text), fmt, ap);
	va_end(ap);

	logline += text;
	output_log_file.write(logline.c_str(), logline.size());
	if (input_log_file) {
		std::getline(input_log_file, line);
		if (!line.empty()) {
			line += '\n';
			LANARTS_ASSERT(line == logline);
		}
		if (input_log_file.eof()) {
			input_log_file.close();
		}
	}
}

void GameLogger::event_log(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	event_log(fmt, ap);
}

static GameLogger __logger;

void event_log(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	__logger.event_log(fmt, ap);
}
void event_log_initialize(GameState* gs, const char* input_file,
		const char* output_file) {
	__logger.initialize_logs(gs, input_file, output_file);
}
