/*
 * SerializeBuffer.h:
 *  Buffer for serialization, with a buffer-full callback
 */

#ifndef SERIALIZEBUFFER_H_
#define SERIALIZEBUFFER_H_

#include <string>
#include <cstring>
#include <vector>
#include <cstdio>

#include "../lanarts_defines.h"

#include "../util/callback_util.h"

const int MAX_BUFFER_SIZE = 128 * 1024; //128kb

typedef void (*buffer_flushf)(void* context, const char* data, size_t size);
typedef void (*buffer_fillf)(void* context, std::vector<char>& buffer,
		size_t maxsize);

class SerializeBuffer {
public:
	static SerializeBuffer file_writer(FILE* file);
	static SerializeBuffer file_reader(FILE* file);
	static SerializeBuffer plain_buffer();

	template<class T>
	void write_vector(const std::vector<T>& vec) {
		write((int)vec.size());
		for (int i = 0; i < vec.size(); i++) {
			write(vec[i]);
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

	template<class T>
	void read_vector(std::vector<T>& vec) {
		int size;
		read(size);
		vec.resize(size);
		for (int i = 0; i < vec.size(); i++) {
			read(vec[i]);
		}
	}

	void read_raw(char* bytes, size_t n) {
		if (_buffer.size() - _read_position < n) {
			fill();
		}
		LANARTS_ASSERT(_buffer.size() >= n + _read_position);
		memcpy(bytes, &_buffer[_read_position], n);
		_read_position += n;
	}
	void read(std::string& str) {
		int size;
		read(size);
		str.resize(size);
		if (size > 0) {
			read_raw(&str[0], size);
		}
	}

	template<class T>
	void read_int(T& t) {
		int i;
		read(i);
		t = (T)i;
	}
	template<class T>
	void read(T& t) {
		read_raw((char*)&t, sizeof(T));
	}
	template<class T>
	void peek(T& t) {
		read(t);
		_read_position -= sizeof(T);
	}

	~SerializeBuffer();

	void flush();
	void fill();
private:
	SerializeBuffer(void* context, buffer_flushf flushf, buffer_fillf fillf);

	std::vector<char> _buffer;
	int _read_position;
	void* _context;
	buffer_flushf _flushf;
	buffer_fillf _fillf;
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

#endif /* SERIALIZEBUFFER_H_ */
