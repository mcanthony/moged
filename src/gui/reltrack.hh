#ifndef INCLUDED_gui_reltracker_HH
#define INCLUDED_gui_reltracker_HH

class RelativeTracker
{
public:
	RelativeTracker() : m_var(-1),m_rel_var(0) {}

	void Reset() { m_var = -1; m_rel_var = 0; }
	void Update(int newVal) {
		m_rel_var = m_var == -1 ? 0 : newVal - m_var;
		m_var = newVal;
	}

	inline int Get() const { return m_var; }
	inline int GetRel() const { return m_rel_var; }
private:
	int m_var;
	int m_rel_var;
};


#endif
