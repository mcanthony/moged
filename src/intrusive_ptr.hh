#ifndef INCLUDED_intrusive_ptr_HH
#define INCLUDED_intrusive_ptr_HH

#include "assert.hh"

template<class T>
class reference 
{
	T* m_ptr;
public:
	reference(T* obj = 0) : m_ptr(obj) { if(m_ptr) m_ptr->inc(); }
	~reference() { if(m_ptr) m_ptr->dec(); }

	bool operator!() const { return m_ptr == 0; }
	
	reference(const reference& o) : m_ptr(o.m_ptr) { if(m_ptr) m_ptr->inc(); }
	reference& operator=(const reference& o) { 
		if(this != &o) {
			if(m_ptr && m_ptr != o.m_ptr) m_ptr->dec();
			m_ptr = o.m_ptr;
			if(m_ptr) m_ptr->inc();
		} 
		return *this;
	}
	
	T& operator*() { return *m_ptr; }
	T* operator->() { return m_ptr; }
	const T& operator*() const { return *m_ptr; }
	const T* operator->() const { return m_ptr; }

	T* RawPtr() { return m_ptr; }
	const T* RawPtr() const { return m_ptr; }

	bool Null() const { return m_ptr == 0; }
};

template<class T>
class refcounted_type
{
	// so const ptrs can be updated
	mutable int m_references;
protected:	
	refcounted_type() : m_references(0) {}
	virtual ~refcounted_type() {}
	
private:
	void inc() const { ++m_references; }
	void dec() const { --m_references; ASSERT(m_references >= 0); 
		if(m_references == 0) delete this;
	}

	friend class reference<T>;
};


#endif
