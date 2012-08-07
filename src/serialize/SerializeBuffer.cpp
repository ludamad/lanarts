/*
 * SerializeBuffer.cpp:
 *  Buffer for serialization, with a buffer-full callback
 */

#include "../lanarts_defines.h"

#include "SerializeBuffer.h"

static void file_buffer_flushf(void* context, const char* data, size_t size) {
	FILE* file = (FILE*)context;
	fwrite(data, size, 1, file);
}
static void file_buffer_fillf(void* context, std::vector<char>& buffer,
		size_t maxsize) {
	FILE* file = (FILE*)context;
	int end = buffer.size();
	//Policy for file buffer reads: Use max buffer size
	buffer.resize(maxsize);
	int readn = fread(&buffer[end], 1, maxsize - end, file);
	buffer.resize(end + readn);
}

SerializeBuffer SerializeBuffer::file_writer(FILE* file) {
	return SerializeBuffer(file, file_buffer_flushf, NULL);
}

SerializeBuffer SerializeBuffer::file_reader(FILE *file) {
	return SerializeBuffer(file, NULL, file_buffer_fillf);
}
SerializeBuffer SerializeBuffer::plain_buffer() {
	return SerializeBuffer(NULL, NULL, NULL);
}

SerializeBuffer::SerializeBuffer(void* context, buffer_flushf flushf,
		buffer_fillf fillf) :
		_read_position(0), _context(context), _flushf(flushf), _fillf(fillf) {
}

SerializeBuffer::~SerializeBuffer() {
	flush();
}

void SerializeBuffer::flush() {
	if (!_buffer.empty() && _flushf != NULL) {
		_flushf(_context, &_buffer[0], _buffer.size());
		_buffer.clear();
	}
}

void SerializeBuffer::fill() {
	LANARTS_ASSERT(_fillf);
	_buffer.erase(_buffer.begin(), _buffer.begin() + _read_position);
	_read_position = 0;
	_fillf(_context, _buffer, MAX_BUFFER_SIZE);
}

