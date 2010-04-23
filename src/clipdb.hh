#ifndef INCLUDED_moged_clipdb_HH
#define INCLUDED_moged_clipdb_HH

#include <vector>
class Clip;

class ClipDB
{
	std::vector< Clip* > m_clips;
public:
	ClipDB();
	~ClipDB();
	void AddClip(Clip* clip);
};

#endif
