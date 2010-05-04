#ifndef INCLUDED_moged_clipdb_HH
#define INCLUDED_moged_clipdb_HH

#include <vector>
class Clip;

// DBQuery query = clips->Select().Col("clips.id").Col("clips.name").From("clips").Where("clips.id", Equals(5));
// DBResult result = query.Execute() ;
// if result.Valid() == false: printf("error : %s\n", result.GetError());
// for(int i = 0; i < result.Count(); ++i) {
//   printf("%d: %s\n", result.Int(i,0), result.Str(i,1));
// }

// DBQuery query = clips->Select().Col("id").Col("name").From("clips").

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

namespace LBF { class WriteNode; class ReadNode; }
LBF::WriteNode* createClipsWriteNode( const ClipDB* clips );
ClipDB* createClipsFromReadNode( const LBF::ReadNode& rn );

#endif
