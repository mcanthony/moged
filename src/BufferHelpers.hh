#ifndef INCLUDED_lab_bufferhelpers_HH
#define INCLUDED_lab_bufferhelpers_HH

#include <cstring>

class BufferWriter {
	char* const m_b;
	const unsigned int m_s;
	unsigned int m_pos;
public:
	BufferWriter(char* buffer, unsigned int size) : m_b(buffer),m_s(size),m_pos(0){}
	bool Put(const void* bytes, unsigned int size) {
		if(m_s - m_pos >= size) {
			memcpy(&m_b[m_pos],bytes,size);
			m_pos += size;
			return true;
		} 
		return false;
	}
	bool Advance(unsigned int size) {
		if(m_s - m_pos >= size) {
			m_pos += size;
			return true;
		} 
		return false;
	}

	bool Full() const { return m_s == m_pos; }
};
	
class BufferReader {
	const char* const m_b;
	const unsigned int m_s;
	unsigned int m_pos;
public:
	BufferReader(const char* buffer, unsigned int size) : m_b(buffer),m_s(size),m_pos(0) {}
	bool Get(void* bytes, unsigned int size) {
		if( (m_s - m_pos) >= size) {
			memcpy(bytes,&m_b[m_pos],size);
			m_pos += size;
			return true;
		}
		return false;
	}
	bool Consume(unsigned int size) {
		if( (m_s - m_pos) >= size) {
			m_pos += size;
			return true;
		}
		return false;
	}

	bool Empty() const { return m_s == m_pos; }	   
};


#endif
