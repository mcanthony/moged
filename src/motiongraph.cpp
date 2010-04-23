/*#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <cstring>
#include <cstdlib>

#include "acclaim.hh"
#include "convert.hh"
#include "skeleton.hh"
#include "clip.hh"

using namespace std;

void usage(const char* name, bool full){ 
	printf("usage: %s -g GraphFile.mg [-a animation to add] [-s skeleton] [-p]\n", name);
	if(full)
		printf("\t-a add animation to graph. requires skeleton to be specified if graph is new.\n"
			   "\t-s skeleton must match the animation.\n"
			   "\t-p just load the graph and watch it do things.\n");
}

class Arguments {
	bool m_valid;
public:
	std::string m_graph;
	std::string m_anim_to_add;
	std::string m_skeleton;
	bool m_add_mode;
	bool m_play_mode;

	Arguments(int argc, char **argv) : m_valid(false)
									 , m_add_mode(false)
									 , m_play_mode(false)
		{
			int i = 1;
			while(i < argc) 
			{
				if(strcasecmp(argv[i],"-g") == 0 && (i+1) < argc) {
					++i;
					m_graph = argv[i];
				} 
				else if(strcasecmp(argv[i],"-a") == 0 && (i+1) < argc) {
					++i;
					m_anim_to_add = argv[i];
					m_add_mode = true;
				}
				else if(strcasecmp(argv[i],"-p") == 0) {
					m_play_mode = true;
				}
				else if(strcasecmp(argv[i],"-h") == 0) {
					usage(argv[0],true);
					exit(0);
				} 
				else if(strcasecmp(argv[i],"-s") == 0 && (i+1) < argc) {
					++i;
					m_skeleton = argv[i];
				}
				++i;
			}


			if(m_add_mode) {
				if(access(m_graph.c_str(),F_OK) == 0 &&  // exists
				   access(m_graph.c_str(),W_OK|R_OK) != 0) { // and I can't use it
					printf("graph file is not readable and writeable or does not exist: %s\n", m_graph.c_str());
					return;
				} 
				if(access(m_anim_to_add.c_str(),F_OK|R_OK) != 0) {
					printf("anim file is not readable or does not exist: %s\n", m_anim_to_add.c_str());
					return;
				}
				if(access(m_skeleton.c_str(),F_OK|R_OK) != 0) {
					printf("skeleton is not readable or does not exist: %s\n", m_skeleton.c_str());
					return;
				}
				m_valid = true;
			} 

			if(m_play_mode) {
				if(access(m_graph.c_str(),F_OK|R_OK) != 0) {
					printf("graph file is not readable or does not exist: %s\n", m_graph.c_str());
					return;
				} 
				m_valid = true;
				
			}
		}
	bool Valid() const { return m_valid ; }
};
*/
/*
int main(int argc, char** argv)
{
	Arguments arguments(argc, argv);
	if(!arguments.Valid()) {
		usage(argv[0],false);
		return 1;
	}

	// load skeleton
	char* skeletonFile = loadFileAsString(arguments.m_skeleton.c_str());
	if(skeletonFile == 0) {
		fprintf(stderr, "Error reading skeleton file: %s\n", arguments.m_skeleton.c_str());
		exit(1);
	}
	AcclaimFormat::Skeleton* ac_skel = AcclaimFormat::createSkeletonFromASF( skeletonFile );
	delete[] skeletonFile;

	if(ac_skel == 0) {
		fprintf(stderr, "Failed to parse skeleton file: %s\n", arguments.m_skeleton.c_str());
		exit(1);
	}

	// load animation
	char* animFile = loadFileAsString(arguments.m_anim_to_add.c_str());
	if(animFile == 0) {
		fprintf(stderr, "Error reading anim file: %s\n", arguments.m_anim_to_add.c_str());
		exit(1);
	}
	AcclaimFormat::Clip* ac_clip = AcclaimFormat::createClipFromAMC( animFile, ac_skel );
	delete[] animFile;

	if(ac_clip == 0) {
		fprintf(stderr, "Failed to parse anim file: %s\n", arguments.m_anim_to_add.c_str());
		exit(1);
	}

	Skeleton* skel = convertToSkeleton(ac_skel);
	Clip* clip = convertToClip(ac_clip, ac_skel, 60.f);

	delete ac_skel;
	delete ac_clip;

	// load motion graph

	
	delete skel;
	delete clip;
	return 0;
}
*/
