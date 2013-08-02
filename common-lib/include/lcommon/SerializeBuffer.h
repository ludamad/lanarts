/*
 * SerializeBuffer.h:
 *  Buffer for serialization, with a buffer-full callback
 */

#ifndef SERIALIZEBUFFER_H_
#define SERIALIZEBUFFER_H_

#include <stdexcept>
#include <string>
#include <cstring>
#include <vector>
#include <cstdio>

#include "strformat.h"
#include "lcommon_assert.h"

const int MAX_ALLOC_SIZE = 32 * 1024 * 1024; //32MB
const int MAX_BUFFER_SIZE = 128 * 1024; //128kb

typedef void (*buffer_flushf)(void* context, const char* data, size_t size);
typedef void (*buffer_fillf)(void* context, std::vector<char>& buffer,
		size_t maxsize);
typedef void (*buffer_closef)(void* context);

class SerializeBufferError : public std::runtime_error {
public:
	SerializeBufferError(const std::string& msg) : runtime_error(msg) {
	}
};

#ifndef LCOMMON_NO_EXCEPTIONS

inline void serialize_buffer_error(const std::string& msg) {
	throw SerializeBufferError(msg);
}

#else

inline void serialize_buffer_error(const std::string& msg) {
	fprintf(stderr, "Serialization error has occurred: %s\n", msg.c_str());
	abort();
}

#endif

#define LSERIALIZE_CHECK(check) \
	if (!(check)) { serialize_buffer_error( format( "SerializeBuffer line %d: %s", __LINE__, #check) ); }


class SerializeBuffer {
public:
	enum IOType {
		INPUT,
		OUTPUT
	};

	SerializeBuffer(void* context, buffer_flushf flushf, buffer_fillf fillf, buffer_closef closef);
	SerializeBuffer();
	SerializeBuffer(FILE* file, IOType type, bool close_file = false);

	template<class T>
	void write_container(const T& t) {
		write((int)t.size());
		for (typename T::const_iterator it = t.begin(); it != t.end(); ++it) {
			write(*it);
		}
	}

	void write_raw(const char* bytes, size_t n) {
		if (_buffer.size() + n > MAX_BUFFER_SIZE) {
			flush();
		}
		_buffer.insert(_buffer.end(), bytes, bytes + n);
	}
	void write(const std::string& str) {
		write((int)str.size());
		write_raw(str.c_str(), str.size());
	}
	template<class T>
	void write(const T& t) {
		write_raw((const char*)&t, sizeof(T));
	}

	void write_int(int i) {
		write(i);
	}

	void write_byte(int i) {
		write((char)i);
	}
	template<class T>
	void read_byte(T& i) {
		char byte;
		read(byte);
		i = byte;
	}

	template<class T>
	void read_container(T& t) {
		int size;
		read(size);
		LSERIALIZE_CHECK(size_t(size * sizeof(T)) < MAX_ALLOC_SIZE);
		t.resize(size);
		for (typename T::iterator it = t.begin(); it != t.end(); ++it) {
			read(*it);
		}
	}

	void read_raw(char* bytes, size_t n) {
		if (_buffer.size() - _read_position < n) {
			fill();
		}
		LSERIALIZE_CHECK(_buffer.size() >= n + _read_position);
		memcpy(bytes, &_buffer[_read_position], n);
		_read_position += n;
	}
	void read(std::string& str) {

		int size;
		read(size);
		LSERIALIZE_CHECK(size_t(size) < MAX_ALLOC_SIZE);
		str.resize(size);
		if (size > 0) {
			read_raw(&str[0], size);
		}
	}

	// For vector<bool>! Blah.
	void read(std::vector<bool>::reference ref) {
		bool bit;
		read(bit);
		ref = bit;
	}

	template<class T>
	void read_int(T& t) {
		int i;
		read(i);
		t = (T)i;
	}

	int read_int() {
		int integer;
		read_int(integer);
		return integer;
	}

	template<class T>
	void read(T& t) {
		read_raw((char*)&t, sizeof(T));
	}

	~SerializeBuffer();

	const char* data() const {
		return &_buffer[0];
	}
	size_t size() const {
		return _buffer.size();
	}
	int read_position() const {
		return _read_position;
	}

	void close();
	void clear();
	void flush();
	void fill();
	void move_read_position(int len) {
		_read_position += len;
		LSERIALIZE_CHECK(_read_position >= 0);
	}
	bool empty() const {
		return _buffer.empty();
	}

private:
	std::vector<char> _buffer;
	int _read_position;
	void* _context;
	buffer_flushf _flushf;
	buffer_fillf _fillf;
	buffer_closef _closef;
};

// The following functions are hacks for quick and dirty serialization of known POD regions
// Writes plain-old-data
#define SERIALIZE_POD_REGION(serializer, obj, member1, member2) \
	serializer.write_raw( (char*) (& obj-> member1 ),\
		(char*)(1 + & obj-> member2) - (char*)(& obj-> member1))

// Reads plain-old-data
#define DESERIALIZE_POD_REGION(serializer, obj, member1, member2) \
	serializer.read_raw( (char*) (& obj-> member1 ),\
		(char*)(1 + & obj-> member2) - (char*)(& obj-> member1))

template<typename T>
inline bool serializer_equals_read(SerializeBuffer& sb, const T& value) {
	T t;
	sb.read(t);
	return t == value;
}

#endif /* SERIALIZEBUFFER_H_ */
