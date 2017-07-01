/*
 * SerializeBuffer.cpp:
 *  Buffer for serialization, with a buffer-full callback
 */

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
	int readn = fread(end + &buffer[0], 1, maxsize - end, file);
	buffer.resize(end + readn);
}

static void file_buffer_closef(void* context) {
	fclose((FILE*)context);
}

void SerializeBuffer::clear() {
	_buffer.clear();
	_read_position = 0;
}

SerializeBuffer::SerializeBuffer(void* context, buffer_flushf flushf,
		buffer_fillf fillf, buffer_closef closef) :
		_read_position(0), _context(context), _user_pointer(NULL), _flushf(flushf), _fillf(fillf), _closef(closef) {
}

SerializeBuffer::SerializeBuffer() :
		_read_position(0), _context(NULL), _user_pointer(NULL),  _flushf(NULL), _fillf(NULL), _closef(NULL) {
}

SerializeBuffer::SerializeBuffer(FILE* file, SerializeBuffer::IOType type, bool close_file) :
		_read_position(0), _context(file) {
	_flushf = type == OUTPUT ? file_buffer_flushf : NULL;
	_fillf = type == INPUT ? file_buffer_fillf : NULL;
	_closef = close_file ? file_buffer_closef : NULL;
}

SerializeBuffer::~SerializeBuffer() {
	close();
}

void SerializeBuffer::flush() {
	if (!_buffer.empty() && _flushf != NULL) {
		_flushf(_context, &_buffer[0], _buffer.size());
		_buffer.clear();
	}
}

void SerializeBuffer::close() {
	if (_context != NULL) {
		flush();
		if (_closef) {
			_closef(_context);
		}
		_context = NULL, _flushf = NULL;
		_fillf = NULL, _closef = NULL;
	}
}

void SerializeBuffer::fill() {
	LCOMMON_ASSERT(_fillf);
	_buffer.erase(_buffer.begin(), _buffer.begin() + _read_position);
	_read_position = 0;
	_fillf(_context, _buffer, MAX_BUFFER_SIZE);
}

