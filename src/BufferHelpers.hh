#ifndef INCLUDED_lab_bufferhelpers_HH
#define INCLUDED_lab_bufferhelpers_HH

#include <cstring>

class BufferWriter {
	char* m_b;
	long m_s;
	long m_pos;
	bool m_error;
public:
	BufferWriter() : m_b(0), m_s(0), m_pos(0), m_error(true) {}
	BufferWriter(char* buffer, long size) : m_b(buffer),m_s(size),m_pos(0),m_error(false){}
	bool Put(const void* bytes, long size) {
		if(m_s - m_pos >= size) {
			memcpy(&m_b[m_pos],bytes,size);
			m_pos += size;
			return true;
		} 
		m_error = true;
		return false;
	}
	bool Advance(long size) {
		if(m_s - m_pos >= size) {
			m_pos += size;
			return true;
		} 
		m_error = true;
		return false;
	}

	long GetPos() const { return m_pos; }

	bool Full() const { return m_s == m_pos; }
	bool Error() const { return m_error; }
};
	
class BufferReader {
	const char* m_b;
	long m_s;
	long m_pos;
	bool m_error;
public:
	BufferReader() : m_b(0), m_s(0), m_pos(0), m_error(true) {}
	BufferReader(const char* buffer, long size) : m_b(buffer),m_s(size),m_pos(0),m_error(false) {}
	bool Get(void* bytes, long size) {
		if( (m_s - m_pos) >= size) {
			memcpy(bytes,&m_b[m_pos],size);
			m_pos += size;
			return true;
		}
		m_error = true;
		return false;
	}
	bool Consume(long size) {
		if( (m_s - m_pos) >= size) {
			m_pos += size;
			return true;
		}
		m_error = true;
		return false;
	}

	long GetPos() const { return m_pos; }

	bool Empty() const { return m_s == m_pos; }	   
	bool Error() const { return m_error; }
};


#endif
