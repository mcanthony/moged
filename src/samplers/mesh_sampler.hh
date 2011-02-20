#ifndef INCLUDED_mesh_sampler_HH
#define INCLUDED_mesh_sampler_HH

#include <vector>
#include "base_sampler.hh"

class Mesh;
class Skeleton;

class MeshCloudSampler : public CloudSampler
{
	const Mesh *m_mesh;
	const Skeleton *m_skel;
	std::vector<int> m_sampleVerts;	
	int m_numThreads;
	float m_sampleInterval;
public:
	MeshCloudSampler();
	~MeshCloudSampler();

	void Init(int numRequestedSamples, const Skeleton* skel, const Mesh* mesh,
		int numComputeThreads, float sampleInterval);

	// from CloudSampler

	int GetSamplesPerFrame();
	void GetSamples(Vec3 *allSamples, int sampleCount, 
		const Clip* clip, int numFrames);

	void GetSampleWeights(float *samplesForFrame);
};

#endif

