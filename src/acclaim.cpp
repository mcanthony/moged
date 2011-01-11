#include <ctype.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include "acclaim.hh"
#include "assert.hh"

using namespace std;

namespace AcclaimFormat 
{
	////////////////////////////////////////////////////////////////////////////////
	// Parsing helpers, moving cursors around and such
	////////////////////////////////////////////////////////////////////////////////
	void safe_strncpy(char *dest, const char* src, int len, int max)
	{
		int min_len = std::min(len, max);
		strncpy(dest,src,min_len);
		int last = std::min( min_len, max-1 );
		dest[last] = '\0';
	}

	void line_skipws(const char*& buffer) 
	{
		while(*buffer && isspace(*buffer) && *buffer != '\n') ++buffer;
	}

	void skip_line(const char*& buffer) 
	{
		while(*buffer && *buffer != '\n') ++buffer;
		if(*buffer) ++buffer;
	}

	void advance_line(const char*& buffer)
	{
		skip_line(buffer); line_skipws(buffer);
	}

	const char* find_next_ws(const char* buffer, char lim = -1) {
		while(*buffer && *buffer != '\n' && (lim == -1 ? !isspace(*buffer) : *buffer != lim) )
			++buffer;
		return buffer;
	}

	bool match_token(const char* token, const char* buffer) 
	{
		const char* end = find_next_ws(buffer);
		int len = end - buffer;
		if(len > 0) {
			return strncasecmp(token,buffer,len) == 0;
		}
		else return false;
	}

	void get_token(char* dest, const char* buffer, int max)
	{
		const char* end = find_next_ws(buffer);
		int len = end - buffer;
		safe_strncpy(dest, buffer, len, max);
	}

	bool next_tok(const char*& buffer)
	{
		buffer = find_next_ws(buffer);
		line_skipws(buffer);
		return *buffer && *buffer != '\n';
	}

	float read_as_num(const char* buffer, char lim = -1)
	{
		char temp[32];
		const char* end = find_next_ws(buffer, lim);
		int len = end - buffer;
		safe_strncpy(temp, buffer, len, 32);
		return atof(temp);
	}

	
	////////////////////////////////////////////////////////////////////////////////
	// Parsing functions
	////////////////////////////////////////////////////////////////////////////////
	void parse_dofs(const char*& buffer, std::vector< DOF >& dofs)
	{
		while(next_tok(buffer))
		{
			if(match_token("tx", buffer)) 
				dofs.push_back(DOF(DOF::TX));
			else if(match_token("ty", buffer)) 
				dofs.push_back(DOF(DOF::TY));
			else if(match_token("tz", buffer)) 
				dofs.push_back(DOF(DOF::TZ));
			else if(match_token("rx", buffer)) 
				dofs.push_back(DOF(DOF::RX));
			else if(match_token("ry", buffer)) 
				dofs.push_back(DOF(DOF::RY));
			else if(match_token("rz", buffer)) 
				dofs.push_back(DOF(DOF::RZ));
		}
	}

	void parse_euler_angles(const char*& buffer, EulerAngles& angles)
	{
		if(!next_tok(buffer)) return ;
		float x = read_as_num(buffer);
		if(!next_tok(buffer)) return ;
		float y = read_as_num(buffer);
		if(!next_tok(buffer)) return ;
		float z = read_as_num(buffer);
		if(!next_tok(buffer)) return ;
		const char* end = find_next_ws(buffer);
		int len = end - buffer;
		if(len == 3) {
			for(int i = 0; i < 3; ++i) 
				angles.axis_order[i] = buffer[i];
		} else {
			fprintf(stderr, "invalid euler angles\n");
		}
		angles.angles.set(x,y,z);
	}

	void parse_limits(const char*& buffer, std::vector< DOF >& dofs)
	{
		const int num = dofs.size();
		int count = 0;
		while(*buffer && count < num) 
		{
			while(*buffer && *buffer != '(') ++buffer;
			if(*buffer != '(') return;
			
			++buffer;
			line_skipws(buffer);
			float low = read_as_num(buffer);
			if(!next_tok(buffer)) return;
			float high = read_as_num(buffer, ')');

			while(*buffer && *buffer != ')') ++buffer;
			if(*buffer) ++buffer;

			dofs[count].low = low;
			dofs[count].high = high;
			++count;
		}
	}

	bool parse_units(const char *&buffer, Skeleton* skel)
	{
		while(*buffer && *buffer != ':')
		{
			if(match_token("mass", buffer)) {
				/* ignore */
			} else if(match_token("length", buffer)) {
				if(!next_tok(buffer)) { 
					fprintf(stderr, "missing length number\n");
					return false;
				}

				skel->scale = read_as_num(buffer);
			} else if(match_token("angle", buffer)) {
				if(!next_tok(buffer)) {
					fprintf(stderr, "missing angle spec\n");
					return false;
				}
				
				skel->in_deg = match_token("deg", buffer);
			}			
			advance_line(buffer);
		}
		return true;
	}
	
	bool parse_root_properties(const char *&buffer, Skeleton* skel)
	{
		line_skipws(buffer);
		while(*buffer && *buffer != ':')
		{
			if(match_token("order", buffer)) {
				parse_dofs(buffer, skel->root.dofs);
			} 
			else if(match_token("axis", buffer)) {
				if(next_tok(buffer)) {
					const char* end = find_next_ws(buffer);
					int len = end - buffer;
					if(len == 3) {
						for(int i = 0; i < 3; ++i) 
							skel->root.axis_order[i] = buffer[i];
					} else { 
						fprintf(stderr, "invalid root axis\n");
						return false;
					}
				} else {
					fprintf(stderr, "missing axis\n");
					return false;
				}
			}
			else if(match_token("position", buffer)) {
				if(!next_tok(buffer)) { 
					fprintf(stderr, "missing position x\n");
					return false;
				}
				float x = read_as_num(buffer);
				if(!next_tok(buffer)) { 
					fprintf(stderr, "missing position y\n");
					return false;
				}
				float y = read_as_num(buffer);
				if(!next_tok(buffer)) { 
					fprintf(stderr, "missing position z\n");
					return false;
				}
				float z = read_as_num(buffer);
				skel->root.position.set(x,y,z);
			} 
			else if(match_token("orientation", buffer)) {
				if(!next_tok(buffer)) { 
					fprintf(stderr, "missing orientation x\n");
					return false;
				}
				float x = read_as_num(buffer);
				if(!next_tok(buffer)) { 
					fprintf(stderr, "missing orientation y\n");
					return false;
				}
				float y = read_as_num(buffer);
				if(!next_tok(buffer)) { 
					fprintf(stderr, "missing orientation z\n");
					return false;
				}
				float z = read_as_num(buffer);
				skel->root.orientation.set(x,y,z);
			}
			/* else skip and ignore */
			advance_line(buffer);
		}
		return true;
	}

	bool parse_bonedata(const char*& buffer, Skeleton* skel)
	{
		line_skipws(buffer);
		while(*buffer && *buffer != ':') 
		{
			if(match_token("begin", buffer)) {
				advance_line(buffer);
				BoneData *bone = new BoneData;
				while(*buffer)
				{				   
					if(match_token("end", buffer)) {
						skel->bones.push_back(bone);
						break;
					} else {
						if(match_token("id", buffer)) {
							next_tok(buffer);
							bone->id = (int)read_as_num(buffer);							
						} else if(match_token("name", buffer)) {
							next_tok(buffer);
							const char* end = find_next_ws(buffer);
							int len = end-buffer;
							if(len > 0) 
								bone->name = std::string(buffer, len);
						} else if(match_token("direction", buffer)) {
							if(!next_tok(buffer)) {
								fprintf(stderr, "missing direction x\n");
								return false;
							}
							float x = read_as_num(buffer);
							if(!next_tok(buffer)) {
								fprintf(stderr, "missing direction y\n");
								return false;
							}
							float y = read_as_num(buffer);
							if(!next_tok(buffer)) {
								fprintf(stderr, "missing direction z\n");
								return false;
							}
							float z = read_as_num(buffer);
							bone->direction.set(x,y,z);
						} else if(match_token("length", buffer)) {
							if(!next_tok(buffer)) {
								fprintf(stderr, "missing length\n");
								return false;
							}
							bone->length = read_as_num(buffer);							
						} else if(match_token("axis", buffer)) {
							parse_euler_angles(buffer, bone->axis);							
						} else if(match_token("dof", buffer)) {
							parse_dofs(buffer, bone->dofs);
						} else if(match_token("limits", buffer)) {
							parse_limits(buffer, bone->dofs);
						}
						advance_line(buffer);
					}
				}
			} else {
				fprintf(stderr, "missing begin in hierarchy\n");
				return false;
			}
			advance_line(buffer);
		}

		return true;
	}

	int find_bone(const std::vector<BoneData*> &bones, const char* name)
	{
		// if this ends up being too slow, rewrite with a hash map.. but ~30-60 bones i don't think it matters
		int count = bones.size();
		for(int i = 0; i < count; ++i) {
			if(strcasecmp(name,bones[i]->name.c_str()) == 0) {
				return i;
			}
		}
		return -1;
	}

	bool parse_hierarchy(const char*& buffer, Skeleton* skel)
	{
		line_skipws(buffer);
		if(*buffer && match_token("begin", buffer)) {
			advance_line(buffer);
			while(*buffer) {
				if(match_token("end", buffer)) {
					advance_line(buffer);
					break;
				} else {
					const char* end = find_next_ws(buffer);
					std::string dest_name(buffer,end - buffer) ;
					bool add_to_root = (strcasecmp("root",dest_name.c_str()) == 0);
					int bone_idx = -1;
					if(!add_to_root) {
						bone_idx = find_bone(skel->bones, dest_name.c_str());
						if(bone_idx == -1) {
							fprintf(stderr, "parent bone %s missing\n", dest_name.c_str());
							return false;
						}
					}
					while(next_tok(buffer)) {
						end = find_next_ws(buffer);
						std::string child_name(buffer, end-buffer);
						int child_bone_idx = find_bone(skel->bones, child_name.c_str());
						if(child_bone_idx == -1) {
							fprintf(stderr, "child bone %s missing\n", child_name.c_str());
							return false;						
						}
						
						if(add_to_root)
							skel->root.children.push_back(skel->bones[child_bone_idx]);
						else {
							skel->bones[bone_idx]->children.push_back(skel->bones[child_bone_idx]);
							skel->bones[child_bone_idx]->parent = bone_idx;
						}
					}			
					advance_line(buffer);
				}
			}
		} else {
			fprintf(stderr, "missing hierarchy begin line\n");
			return false;
		}
		return true;
	}
	
	Skeleton::~Skeleton()
	{
		int count = bones.size();
		for(int i = 0; i < count; ++i)
		{
			delete bones[i];
		}
	}

	Clip::~Clip()
	{
		int count = frames.size();
		for(int i = 0; i < count; ++i) 
		{
			delete frames[i];
		}
	}
	
	Frame::Frame(int num_bones) : id(0)
	{
		data.resize(num_bones);
	}

	void dbgPrintDofs(const std::vector< DOF >&dofs )
	{
		int size = dofs.size();
		for(int i = 0; i < size; ++i) {
			switch(dofs[i].type) {
			case DOF::TX:
				printf("tx "); break;
			case DOF::TY:
				printf("ty "); break;
			case DOF::TZ:
				printf("tz "); break;
			case DOF::RX:
				printf("rx "); break;
			case DOF::RY:
				printf("ry "); break;
			case DOF::RZ:
				printf("rz "); break;
			default:
				printf("BAD ");
				break;
			}
		}
	}

	void dbgVerifySkeleton(Skeleton* skel)
	{
		printf("name %s\n", skel->name.c_str());
		printf("skel scale is %f\n", skel->scale);
		printf("skel angles are in %s\n", skel->in_deg ? "deg" : "rad");
		printf("root pos %f %f %f\n", skel->root.position.x, skel->root.position.y, skel->root.position.z);
		printf("root orient %f %f %f\n", skel->root.orientation.x, skel->root.orientation.y, skel->root.orientation.z);
		printf("root axis order %c%c%c\n", skel->root.axis_order[0], skel->root.axis_order[1], skel->root.axis_order[2]);
		printf("root dofs: "); dbgPrintDofs(skel->root.dofs); printf("\n");

		int num_bones = skel->bones.size();
		printf("# bones %d\n", (int)skel->bones.size());

		for(int i = 0; i < num_bones; ++i) 
		{
			printf("bone id: %d\n", skel->bones[i]->id);
			printf("bone name: %s\n", skel->bones[i]->name.c_str());
			printf("bone direction: %f %f %f\n", skel->bones[i]->direction.x, skel->bones[i]->direction.y, skel->bones[i]->direction.z);
			printf("bone len: %f\n", skel->bones[i]->length);
			printf("bone axis: %f %f %f %c%c%c\n", skel->bones[i]->axis.angles.x, skel->bones[i]->axis.angles.y, skel->bones[i]->axis.angles.z,
				   skel->bones[i]->axis.axis_order[0], skel->bones[i]->axis.axis_order[1], skel->bones[i]->axis.axis_order[2]);
			printf("bone dofs: "); dbgPrintDofs(skel->bones[i]->dofs); printf("\n");
			printf("num children: %d\n", (int)skel->bones[i]->children.size());
		}			
	}

	bool parse_anim_data(const char*& buffer, const std::vector<DOF>& dofs, FrameTransformData& data)
	{
		int num_dofs = dofs.size();
		for(int i = 0; i < num_dofs; ++i)
		{
			if(!next_tok(buffer)) {
				fprintf(stderr, "anim data does not match dof spec\n");
				return false;
			}			
			float val = read_as_num(buffer);
			ASSERT(dofs[i].type >= 0 && dofs[i].type < 6);
			data.val[dofs[i].type] = val;
		}
		return true;
	}

	bool parse_root_anim(const char*& buffer, const Skeleton* skel, Frame* frame )
	{
		if(match_token("root", buffer)) 
		{
			if(!parse_anim_data(buffer, skel->root.dofs, frame->root_data))
				return false;
		} else { 
			fprintf(stderr, "missing root anim data for frame %d\n", frame->id);
			return false;
		}
		advance_line(buffer);
		return true;
	}
	

	bool parse_bone_anim( const char*& buffer, const Skeleton* skel, Frame* frame )
	{
		char temp[32];
		get_token(temp, buffer, 32);
		
		int idx = find_bone(skel->bones, temp);
		if(idx == -1)
		{
			fprintf(stderr, "Couldn't find bone: %s. Is this the right skeleton?\n", temp);
			return false;
		}

		if(!parse_anim_data(buffer, skel->bones[idx]->dofs, frame->data[idx]))
			return false;
		
		advance_line(buffer);
		return true;
	}

	void dbgPrintTransformData(const FrameTransformData& data)
	{
		printf("tx %f ty %f tz %f rx %f ry %f rz %f\n",
			   data.val[0], data.val[1], data.val[2], data.val[3], data.val[4], data.val[5]);
	}

	void dbgVerifyClip(const Clip* clip)
	{
		printf("clip with %d frames\n", (int)clip->frames.size());
		printf("clip angles in %s\n", clip->in_deg ? "deg" : "rad");
		for(int i = 0; i < (int)clip->frames.size(); ++i)
		{
			printf("frame id %d\n", clip->frames[i]->id);
			printf("frame root data "); dbgPrintTransformData(clip->frames[i]->root_data); printf("\n");
			for(int j = 0; j < (int)clip->frames[i]->data.size(); ++j)
			{
				printf("bone %d data ", j); dbgPrintTransformData(clip->frames[i]->data[j]); printf("\n");
			}
		}
	}

	typedef std::pair< BoneData*, int > bone_parent_pair;
	struct reverseBoneNameCompare
	{
		bool operator()( const BoneData* lhs, const BoneData* rhs)
			{
				return strcmp(lhs->name.c_str(),rhs->name.c_str()) >= 0 ;
			}
		
		bool operator()( const bone_parent_pair& lhs, const bone_parent_pair& rhs )
			{
				return strcmp(lhs.first->name.c_str(), rhs.first->name.c_str()) >= 0;
			}
	};

	// sort bones by parents & alphabetically - this creates a specific order that other 
	// exporting tools can match
	void sort_bones(Skeleton* skel)
	{
		std::vector< bone_parent_pair > stack;
		int num_bones = skel->bones.size();
		stack.reserve(num_bones);

		for(int i = 0; i < num_bones; ++i)
			if(skel->bones[i]->parent == -1) 
				stack.push_back( std::make_pair(skel->bones[i], -1) );
			
		std::sort( stack.begin(), stack.end(), reverseBoneNameCompare() );

		std::vector< BoneData* >bones; // result vector
		while( !stack.empty() )
		{
			bone_parent_pair curPair = stack.back();
			BoneData* cur = curPair.first;
			int parent_index = curPair.second;
			stack.pop_back();

			cur->parent = parent_index;
			int this_index = bones.size();

			std::vector< BoneData* >sorted_children = cur->children;
			std::sort(sorted_children.begin(), sorted_children.end(), reverseBoneNameCompare() );

			int num_children = sorted_children.size();
			for(int i = 0; i < num_children; ++i) 
			{
				stack.push_back( std::make_pair(sorted_children[i], this_index) );
			}

			bones.push_back(cur);
		}

		ASSERT(skel->bones.size() == bones.size()) ;
		skel->bones = bones;
	}

	////////////////////////////////////////////////////////////////////////////////
	// Public creation/parse functions
	////////////////////////////////////////////////////////////////////////////////

	Skeleton* createSkeletonFromASF( const char* buffer )
	{
		Skeleton* result = new Skeleton;
		bool success = true;
		line_skipws(buffer);
		while(*buffer && success) 
		{
			if(*buffer == '#') advance_line(buffer);
			else if(*buffer == ':')
			{
				++buffer;
				if(match_token("name", buffer)) {
					buffer += 5;
					line_skipws(buffer);
					const char* end = find_next_ws(buffer);
					int len = end - buffer;
					result->name = std::string(buffer, len);
					buffer = end;
					advance_line(buffer);
				} else if(match_token("units", buffer)) {
					advance_line(buffer);
					success = success && parse_units(buffer, result);
				} else if(match_token("root", buffer)) {
					advance_line(buffer);
					success = success && parse_root_properties(buffer, result);
				} else if(match_token("bonedata", buffer)) {
					advance_line(buffer);
					success = success && parse_bonedata(buffer, result);				
				} else if(match_token("hierarchy", buffer)) {
					advance_line(buffer);
					success = success && parse_hierarchy(buffer, result);
				} else {
					// skip until next section
					do {
						advance_line(buffer);
					} while(*buffer && *buffer != ':');
				}
			}
		}

		if(!success) {
			fprintf(stderr,"Failed to parse acclaim skeleton\n");
			delete result;
			result = 0;
		} else {
			sort_bones(result);
		}

//		if(result) dbgVerifySkeleton(result) ;

		return result;
	}


	Clip* createClipFromAMC( const char* buffer, const Skeleton* skel )
	{
		Clip* result = new Clip;
		bool success = true;
		int num_bones = skel->bones.size(); // assume bones are in the same order as in the ASF
		line_skipws(buffer);
		while(*buffer && success)
		{
			if(*buffer == '#') { 
				advance_line(buffer);
			}
			else if(*buffer == ':') {
				++buffer;
				if(match_token("degrees", buffer)) {
					result->in_deg = true;
				} else if(match_token("radians", buffer)) { // this is speculation, I don't think I have any files in radians
					result->in_deg = false;
				}
				advance_line(buffer);
			}
			else {
				// must be a frame!
				Frame* frame = new Frame(num_bones);
				frame->id = (int)read_as_num(buffer);

				advance_line(buffer);
				if(!parse_root_anim( buffer, skel, frame)) {
					fprintf(stderr, "failed to parse root info!\n");
					success = false; 
					delete frame;
					break;
				}

				while(*buffer && !isdigit(*buffer) )
				{
					if(!parse_bone_anim( buffer, skel, frame ))
					{
						fprintf(stderr, "failed to parse bone!\n");
						success = false;
						delete frame;
						break;
					}
				}

				if(success) result->frames.push_back(frame);
				
			}
		}

		if(!success) {
			fprintf(stderr, "Failed to parse acclaim animation\n");
			delete result;
			result = 0;
		}
		
//		if(result) dbgVerifyClip(result);		

		return result;
	}
}

