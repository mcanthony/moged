#include <cstdio>
#include <unistd.h>
#include "assert.hh"
#include "lbfloader.hh"
#include "BufferHelpers.hh"
using namespace std;

namespace LBF
{
	ReadNode::ReadNode() : m_data(0), m_size(0) {}

	ReadNode::ReadNode(char *chunk_start, long size)
		: m_data(chunk_start), m_size(size)
	{
		
	}

	int ReadNode::GetType() const 
	{
		if(m_data) {
			const ChunkHeader* header = reinterpret_cast<const ChunkHeader*>(m_data);
			return header->type;
		}
		else 
			return DONTCARE;		
	}

	int ReadNode::GetID() const 
	{
		if(m_data) {
			const ChunkHeader* header = reinterpret_cast<const ChunkHeader*>(m_data);
			return header->id;
		}
		else 
			return -1;		
			
	}

	bool ReadNode::GetData(void* dest, long size) const
	{
		BufferReader reader(GetNodeData(), GetNodeDataLength());
		reader.Get(dest, size);
		return !reader.Error();
	}

	BufferReader ReadNode::GetReader() const 
	{
		return BufferReader(GetNodeData(), GetNodeDataLength());
	}

	const char* ReadNode::GetNodeData() const 
	{
		if(m_data) 
			return m_data + sizeof(ChunkHeader);
		else 
			return 0;
	}

	int ReadNode::GetNodeDataLength() const 
	{
		if(m_data) {
			const ChunkHeader* header = reinterpret_cast<const ChunkHeader*>(m_data);
			return header->child_offset - sizeof(ChunkHeader) ;
		} else 
			return 0;
	}

	ReadNode ReadNode::GetNext(int type, int id) const
	{
		if(m_data == 0) return ReadNode();
		const ChunkHeader* header = reinterpret_cast<const ChunkHeader*>(m_data);
		long nextSize = m_size - header->length;
		if(nextSize > 0) {
			if(type == DONTCARE)
				return ReadNode(m_data + header->length, nextSize);
			else {
				ReadNode rn(m_data + header->length, nextSize);
				while(rn.Valid() && (rn.GetType() != type ||
									 (id != DONTCARE && id != rn.GetID()))) {
					rn = rn.GetNext();
				}
				return rn;
			}
		} else {
			ASSERT(nextSize == 0);
			return ReadNode();
		}
	}

	ReadNode ReadNode::GetFirstChild(int type, int id) const 
	{
		if(m_data == 0) return ReadNode();

		const ChunkHeader* header = reinterpret_cast<const ChunkHeader*>(m_data);
		long sizeForChildren = header->length - header->child_offset;
		if(sizeForChildren > 0) {
			if(type == DONTCARE)
				return ReadNode( m_data + header->child_offset, sizeForChildren );
			else {
				ReadNode rn( m_data + header->child_offset, sizeForChildren );
				while(rn.Valid() && (rn.GetType() != type ||
									 (id != DONTCARE && id != rn.GetID()))) {
					rn = rn.GetNext();
				}
				return rn;
			}
		}
		else
			return ReadNode();
	}

	WriteNode::WriteNode(int type, int id, long length, int flags)
		: m_type(type)
		, m_data(0)
		, m_data_length(length)
		, m_first_child(0)
		, m_next(0)
		, m_id(id)
		, m_write_flags(flags)
	{
		m_data = new char[length];
		memset(m_data,0,sizeof(char)*length);
	}

	WriteNode::WriteNode(const WriteNode& other)
		: m_type(other.m_type)
		, m_data(0)
		, m_data_length(other.m_data_length)
		, m_first_child(0)
		, m_next(0)
		, m_id(other.m_id)
	{
		m_data = new char[m_data_length] ;
		memcpy( m_data, other.m_data, sizeof(char)*m_data_length);
		
		WriteNode* working = 0;
		const WriteNode* oChild = other.GetFirstChild();
		while(oChild) {
			WriteNode* childCopy = new WriteNode ( *oChild );
			if(working) {
				working->AddSibling(childCopy);
			} else {
				AddChild(childCopy);
			}
			working = childCopy;
			oChild = oChild->GetNext();
		}
	}

	WriteNode::~WriteNode() 
	{
		delete[] m_data;
		delete m_first_child;
		delete m_next;
	}

	void WriteNode::ReplaceData(const WriteNode* other)
	{
		delete[] m_data;
		m_data = new char[other->m_data_length];
		m_data_length = other->m_data_length;
		memcpy(m_data, other->m_data, m_data_length);
	}

	void WriteNode::ReplaceChildren(const WriteNode* other)
	{
		delete m_first_child; m_first_child = 0;
		if(other->m_first_child) {
			m_first_child = new WriteNode( *other->m_first_child );

			WriteNode* curInsert = m_first_child;
			const WriteNode* cur = other->m_first_child;
			cur = cur->GetNext();

			while(cur)
			{
				WriteNode* childCopy = new WriteNode( *cur );
				curInsert->AddSibling(childCopy);
				curInsert = childCopy;
				cur = cur->GetNext();
			}			
		}
	}	

	void WriteNode::AddChild(WriteNode* node) 
	{
		if(m_first_child) {
			m_first_child->AddSibling(node);
		} else {
			m_first_child = node;
		}
	}

	void WriteNode::AddSibling(WriteNode* node) 
	{
		if(m_next) {
			m_next->AddSibling(node);
		} else {
			m_next = node;
		}
	}

	WriteNode* WriteNode::GetNext( ) 
	{
		return m_next;
	}

	const WriteNode* WriteNode::GetNext( ) const 
	{
		return m_next;
	}

	WriteNode* WriteNode::GetFirstChild( ) 
	{
		return m_first_child;
	}

	const WriteNode* WriteNode::GetFirstChild( ) const 
	{
		return m_first_child;
	}

	BufferWriter WriteNode::GetWriter() 
	{
		return BufferWriter(m_data, m_data_length);
	}

	bool WriteNode::PutData(const void* data, long size) 
	{
		BufferWriter writer(m_data, m_data_length);
		writer.Put(data, size);
		return !writer.Error();
	}

	const char* WriteNode::GetData() const
	{
		return m_data;
	}

	long WriteNode::GetDataLength() const 
	{
		return m_data_length;
	}

	LBFData::LBFData(char* top_ptr, long size, bool owner)
		: m_file_data(top_ptr)
		, m_file_size(size)
		, m_owner(owner)
	{
	}

	LBFData::~LBFData()
	{
		if(m_owner) {
			delete[] m_file_data;
		}
	}

	ReadNode LBFData::GetFirstNode(int type, int id) const
	{
		long lengthLeft = m_file_size - sizeof(FileHeader);
		if(lengthLeft > 0) {
			char* firstNode = m_file_data + sizeof(FileHeader);
			ReadNode rn(firstNode, lengthLeft);
			if(type == DONTCARE)
				return rn;
			else {
				while(rn.Valid() && (rn.GetType() != type ||
									 (id != DONTCARE && rn.GetID() != id))) {
					rn = rn.GetNext();
				} 
				return rn;
			}
		} else {
			return ReadNode();
		}
	}

	bool verifyLBFChunk(BufferReader& reader, ChunkHeader& header)
	{
		// consume data section
		reader.Consume( header.child_offset - sizeof(ChunkHeader) ) ; // data section of THIS chunk.
		long dataLeft = header.length - header.child_offset; // bytes for children.
		while(dataLeft > 0) {
			ChunkHeader childHeader;
			if(!reader.Get(&childHeader, sizeof(childHeader))) return false;
			dataLeft -= childHeader.length;
			if(!verifyLBFChunk(reader, childHeader)) return false;
		}
		return dataLeft == 0;
	}
	
	bool verifyLBFSize(const char* buffer, long fileSize) 
	{
		BufferReader reader(buffer, fileSize);
		if(!reader.Consume(sizeof(FileHeader)))
			return false;
		
		ChunkHeader header;
		do {
			if(!reader.Get(&header, sizeof(header))) return false;
			if(!verifyLBFChunk(reader, header)) return false;
		} while (!reader.Empty());
		return true;
	}

	int parseLBF( char* buffer, long buffer_size, LBFData*& result, bool takeOwnership)
	{
		result = new LBFData(buffer, buffer_size, takeOwnership);

		static const char* kTag = "LBF_";

		const FileHeader* header = reinterpret_cast<const FileHeader*>(buffer);
		if( memcmp(header->tag, kTag, 4) != 0 ) {
			delete result; result = 0;
			return ERR_PARSE;
		}

		if(header->version_major != LBF_VERSION_MAJOR ||
		   header->version_minor > LBF_VERSION_MINOR)
		{
			delete result; result = 0;
			return ERR_BAD_VERSION;
		}		   
		
		if(!verifyLBFSize(buffer, buffer_size)) {
			delete result; result = 0;
			return ERR_PARSE;
		}		

		return OK;
	}

	// same as parse, but opens file, allocates data and passes ownership to the result.
	int openLBF( const char* filename, LBFData*& out )
	{
		FILE* fp = fopen(filename, "rb");
		if(fp == 0) {
			return ERR_OPEN_R;
		}

		fseek(fp, 0, SEEK_END);
		long size = ftell(fp);
		rewind(fp);

		char* tempIn = new char[size];
		if(tempIn == 0) {
			fclose(fp);
			return ERR_MEM;
		}

		size_t bytesRead = fread(tempIn, 1, size, fp);
		fclose(fp);

		if(bytesRead != (size_t)size) {
			delete[] tempIn;
			return ERR_READ;
		}

		out = 0;
		int err = parseLBF( tempIn, size, out, true);
		if(err) return err;

		return OK;
	}

	long computeWriteSize(const WriteNode* node )
	{
		long writeSize = sizeof(ChunkHeader);
		writeSize += node->GetDataLength();
		
		const WriteNode* child = node->GetFirstChild();
		while(child) {
			writeSize += computeWriteSize(child);
			child = child->GetNext();
		}

		return writeSize;		
	}
	
	void writeNode( BufferWriter& writer, const WriteNode* node)
	{
		long size = computeWriteSize(node);
		ChunkHeader header;
		memset(&header,0,sizeof(header));
		header.type = node->GetType();
		header.length = size;
		header.child_offset = sizeof(header) + node->GetDataLength();
		header.id = node->GetID();

		writer.Put(&header, sizeof(header));
		writer.Put(node->GetData(), node->GetDataLength());

		if(writer.Error()) {
			return;
		}
		
		const WriteNode* child = node->GetFirstChild();
		while(child) {
			writeNode(writer, child);
			child = child->GetNext();
		}
	}

	WriteNode* convertToWriteNode( const ReadNode& node )
	{
		ASSERT(node.Valid());
		WriteNode* wnode = new WriteNode( node.GetType(),
										 node.GetID(),
										 node.GetNodeDataLength() );
		wnode->PutData( node.GetNodeData(), wnode->GetDataLength() );
		
		WriteNode* workingChild = 0;
		ReadNode child = node.GetFirstChild();
		while(child.Valid()) 
		{
			WriteNode* childNode = convertToWriteNode(child);
			if(workingChild) {
				workingChild->AddSibling(childNode);
			} else {
				wnode->AddChild(childNode);
			}
			workingChild = childNode;
			child = child.GetNext();
		}

		return wnode;
	}

	WriteNode* convertToWriteNodes( const LBFData* existing )
	{
		WriteNode* first = 0; 
		WriteNode* working = 0;

		ReadNode cur = existing->GetFirstNode();
		while(cur.Valid()) {
			WriteNode* newCur = convertToWriteNode(cur);
			if(first == 0) { 
				first = newCur;
			} else {
				working->AddSibling(newCur);
			}
			working = newCur;
			cur = cur.GetNext();
		}

		return first;
	}

	void mergeWriteNodeTrees( WriteNode* dest, const WriteNode* src )
	{
		// for each item in src, find in dest. if found, replace, otherwise append.
		const WriteNode* cur = src;
		while(cur)
		{
			int type = cur->GetType();
			int id = cur->GetID();
			WriteNode* curDest = dest, *lastDest = 0;
			while(curDest && (curDest->GetType() != type || curDest->GetID() != id)) {
				lastDest = curDest;
				curDest = curDest->GetNext();
			}

			if(curDest == 0) {
				lastDest->AddSibling( new WriteNode( *cur ) );
			} else {
				curDest->ReplaceData(cur);

				// non-data objects acts as folders, so their children get merged instead of
				// replaced.
				if( (src->GetFlags() & WriteNodeFlag_DataObj) == 0) 
				{
					WriteNode* destChild = curDest->GetFirstChild();
					const WriteNode* srcChild = src->GetFirstChild();

					if(srcChild) {
						if(destChild == 0) {
							WriteNode* childCopy = new WriteNode( *srcChild );
							curDest->AddChild(childCopy);

							srcChild = srcChild->GetNext();
							destChild = childCopy ;
							if(srcChild) {
								mergeWriteNodeTrees(destChild, srcChild);
							}
						} else {
							mergeWriteNodeTrees(destChild, srcChild);
						}
					}
				}
				else
				{
					curDest->ReplaceChildren(cur);
				}
			}

			cur = cur->GetNext();
		}
	}

	int compileLBF( const WriteNode* newData, char *& outBuffer, long& outLen )
	{
		long writeSize = sizeof(FileHeader);
		const WriteNode* cur = newData;
		while(cur) {
			writeSize += computeWriteSize(cur);
			cur = cur->GetNext();
		}

		outBuffer = new char[writeSize];
		if(outBuffer == 0) {
			return ERR_MEM;
		}

		outLen = writeSize;
		BufferWriter writer(outBuffer, writeSize);
			
		FileHeader header;
		memset(&header,0,sizeof(header));
		memcpy(header.tag, "LBF_", 4);
		header.version_major = LBF_VERSION_MAJOR;
		header.version_minor = LBF_VERSION_MINOR;

		writer.Put(&header, sizeof(header));
		cur = newData;
		while(cur) {
			writeNode( writer, cur );
			cur = cur->GetNext();
		}

		if(writer.Error()) {
			delete[] outBuffer; outBuffer = 0;
			return ERR_SIZE;
		}

		return OK;
	}

	int mergeLBF( const WriteNode* newData, const LBFData* existing, char *& outBuffer, long& outLen )
	{
		if(existing) 
		{
			// parse existing, if newData has match, write newData instead
			WriteNode* existingData = convertToWriteNodes( existing );
			mergeWriteNodeTrees(existingData, newData);
			return compileLBF(existingData, outBuffer, outLen);			
		}
		else 
		{
			return compileLBF(newData, outBuffer, outLen);
		}
	}

// save newData as file, optionally merging 
	int saveLBF( const char* filename, WriteNode* newData, bool doMerge )
	{
		LBFData *existingData = 0;
		if(doMerge) {
			if(access(filename, F_OK) == 0) {
				int err = openLBF(filename, existingData);
				if(err) return err;
			}
		}

		char* newDataBuffer = 0;
		long newLength = 0;
		int err = mergeLBF( newData, existingData, newDataBuffer, newLength );
		if(newDataBuffer == 0) {
			delete existingData;
			return err;
		} else {
			FILE* fp = fopen(filename, "wb");
			if(fp == 0) {
				delete existingData;
				delete[] newDataBuffer;
				return ERR_OPEN_W;
			}

			size_t bytesWritten = fwrite(newDataBuffer, 1, newLength, fp);
			if(bytesWritten != (size_t)newLength) {
				delete existingData;
				delete[] newDataBuffer;
				return ERR_WRITE;
			}

			fclose(fp);
		}

		delete existingData;
		delete[] newDataBuffer;
		return OK;
	}
}
