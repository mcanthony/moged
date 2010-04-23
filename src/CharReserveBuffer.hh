#ifndef LAB_core_charreservebuffer_HH
#define LAB_core_charreservebuffer_HH

#include "BufferHelpers.hh"

class CharReserveBuffer
{
	char* m_buffer;
	int m_num_written;
	int m_size;

public:
	explicit CharReserveBuffer(int size)
		: m_buffer(0)
		, m_num_written(0)
		, m_size(size)
		{
			m_buffer = new char[size];
		}

	~CharReserveBuffer()
		{
			delete[] m_buffer;
		}

	int CurrentSize() const { return m_num_written; }

	char* Add(unsigned int size)
		{
			char* result = 0;
			int bytes_left = m_size - m_num_written;
			if( (int)size <= bytes_left) {
				result = &m_buffer[m_num_written];
				m_num_written += size;
			}
			return result;
		}

	BufferReader GetBufferReader() const { return BufferReader(m_buffer,CurrentSize()); }
		
	void Clear()
		{
			m_num_written = 0;
		}
};


#endif
