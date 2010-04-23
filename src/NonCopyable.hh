#ifndef INCLUDED_noncopyable_HH
#define INCLUDED_noncopyable_HH

class non_copyable
{
	non_copyable(const non_copyable&);
	non_copyable& operator=(const non_copyable&);
protected:
	inline non_copyable() {}
};

#endif
