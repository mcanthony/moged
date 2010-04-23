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
	void RemoveClip(Clip* clip);

	int GetNumClips() const ;
	const Clip* GetClip(int index) const ;
};

#endif
