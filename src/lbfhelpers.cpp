#include <algorithm> 
#include <string>
#include "lbfloader.hh"
#include "lbfhelpers.hh"
#include "BufferHelpers.hh"

LBF::WriteNode* createStdStringTableNode( int type, int id, const std::string* array, int len)
{
	// num strings
	// and offset/len pairs
	long size = sizeof(int) + 2 * len * sizeof(int);
	// plus actual strings
	for(int i = 0; i < len; ++i) {
		size += sizeof(char)*array[i].length()+1;
	}
	LBF::WriteNode* stringTableNode = new LBF::WriteNode(type, id, size);
	BufferWriter offsetWriter = stringTableNode->GetWriter();
	offsetWriter.Put(&len, sizeof(len));

	BufferWriter stringWriter = offsetWriter;
	stringWriter.Advance(2 * len * sizeof(int));

	for(int i = 0; i < len; ++i) {
		int pos = (int)stringWriter.GetPos();
		int len = array[i].length();
		offsetWriter.Put(&pos, sizeof(pos));
		stringWriter.Put(&array[i][0], sizeof(char)*(len + 1));
		offsetWriter.Put(&len, sizeof(len));
	}

	return stringTableNode;
}

void readStdStringTable( const LBF::ReadNode& rn, std::string* table, int len)
{
	int count = 0;
	BufferReader offsetReader = rn.GetReader();
	offsetReader.Get(&count, sizeof(count));

	int min_len = std::min(count,len);
	for(int i = 0; i < min_len; ++i) {
		int pos = 0;
		int len = 0;
		offsetReader.Get(&pos, sizeof(pos));
		offsetReader.Get(&len, sizeof(len));

		const char* strData = rn.GetNodeData() + pos;
		table[i] = std::string(strData, len);
	}
}
