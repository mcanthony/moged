#include <omp.h>
#include <algorithm>
#include <cstdio>
#include "clip.hh"
#include "anim/clipcontroller.hh"
#include "anim/pose.hh"
#include "Mat4.hh"
#include "skeleton.hh"
#include "mesh_sampler.hh"
#include "MathUtil.hh"
#include "assert.hh"
#include "mesh.hh"

// Support functions
static void poseSamples(Vec3* out, int num_samples, const std::vector<int>& sample_indices, 
				const Mesh* mesh, const Pose* pose);

////////////////////////////////////////////////////////////////////////////////
MeshCloudSampler::MeshCloudSampler()
	: m_mesh(0)
	, m_skel(0)
	, m_numThreads(0)
	, m_sampleInterval(0.0)
{
}

MeshCloudSampler::~MeshCloudSampler()
{
}

void MeshCloudSampler::Init(int numRequestedSamples, 
	const Skeleton* skel, const Mesh* mesh,
	int numComputeThreads, float sampleInterval)
{
	m_mesh = mesh;
	m_skel = skel;
	m_numThreads = Max(numComputeThreads,1);
	m_sampleInterval = sampleInterval;

	// choose random vertices of the mesh.
	// This may not give the best distribution over the character necessarily since
	// it doesn't take into account density of mesh (where we probably want number of points
	// in an area to be roughly the same).
	// If it is a problem, I can try a stratified sampling.

	m_sampleVerts.clear();

	std::vector<int> candidates ;
	const int num_verts = mesh->GetNumVerts();

	candidates.reserve(num_verts);
	
	for(int i = 0; i < num_verts; ++i) 
		candidates.push_back(i);
	
	numRequestedSamples = Min(num_verts, numRequestedSamples);
	m_sampleVerts.reserve(numRequestedSamples);

	for(int i = 0; i < numRequestedSamples; ++i) 
	{
		int rand_index = rand() % candidates.size();

		m_sampleVerts.push_back(candidates[rand_index]);
		candidates.erase( candidates.begin() + rand_index );
	}

	// sort the list so we at least access data in a predictable way
	std::sort( m_sampleVerts.begin(), m_sampleVerts.end());
}

int MeshCloudSampler::GetSamplesPerFrame()
{
	return (int)m_sampleVerts.size();
}

void MeshCloudSampler::GetSamples(Vec3* allSamples, int sampleCount, 
	const Clip* clip, int numFrames)
{
	ASSERT(allSamples);
	int i;
	int frame_offset, tid;
	// init

	const int samplesPerFrame = m_sampleVerts.size();
	const int writeCount = samplesPerFrame * numFrames;
	if(sampleCount < writeCount) {
		fprintf(stderr, "wrong size passed to GetSamples(). Expected %d, got %d\n", writeCount, sampleCount);
		return;
	}
	memset(allSamples, 0, sizeof(Vec3) * writeCount);

	ClipController **controllers = new ClipController*[m_numThreads];
	for(int i = 0; i < m_numThreads; ++i) {
		controllers[i] = new ClipController(m_skel);
		controllers[i]->SetClip(clip);
	}

	// variables to make OpenMP happy
	const int numThreads = m_numThreads;
	const Mesh* mesh = m_mesh;
	const std::vector<int>& sampleVerts = m_sampleVerts;
	float sampleInterval = m_sampleInterval;
	
	omp_set_num_threads(numThreads);

#pragma omp parallel for												\
	shared(controllers,mesh,sampleVerts,allSamples,sampleInterval,numFrames) \
	private(i,tid,frame_offset)
	for(i = 0; i < numFrames; ++i)
	{
		tid = omp_get_thread_num();
		ASSERT(tid < numThreads);
		frame_offset = i * samplesPerFrame;

		controllers[tid]->SetTime( i * sampleInterval );
		controllers[tid]->ComputePose();
		controllers[tid]->ComputeMatrices( mesh->GetTransform() );

		poseSamples(&allSamples[frame_offset],
					samplesPerFrame,
					sampleVerts, 
					mesh, 
					controllers[tid]->GetPose());
	}

	// clean up
	for(int i = 0; i < numThreads; ++i) {
		delete controllers[i];
	}
	delete[] controllers;
}


void MeshCloudSampler::GetSampleWeights(float *samplesForFrame)
{ 
	const SkeletonWeights& weights = m_skel->GetSkeletonWeights();

	const char *mat_indices = m_mesh->GetSkinMatricesPtr();
	const float *skin_weights = m_mesh->GetSkinWeightsPtr();
	const int numWeights = m_sampleVerts.size();

	for(int sampleInFrame = 0; sampleInFrame < numWeights; ++sampleInFrame)
	{
		int sample_idx = m_sampleVerts[sampleInFrame];
		int mat_sample = 4 * sample_idx; // 4 values per idx

		const float *w = &skin_weights[mat_sample];
		const char *indices = &mat_indices[mat_sample];

		float weight = w[0] * weights.GetJointWeight( indices[0] )
			+ w[1] * weights.GetJointWeight( indices[1] )
			+ w[2] * weights.GetJointWeight( indices[2] )
			+ w[3] * weights.GetJointWeight( indices[3] );
		samplesForFrame[sampleInFrame] = weight;
	}
}

// Get point from a clip controller. 
// 'samples' must be of at least num_samples * sample_indicies.size() in length.
// It begins by starting at a particular frame, and then sampling every sample_interval for num_samples iterations.
// This allows for clips that are running at variable FPS to be used.
// Pose must be initialized with the same skeleton as the clip_controller.

static void poseSamples(Vec3* out, int num_samples, const std::vector<int>& sample_indices, 
						const Mesh* mesh, const Pose* pose)
{
	const float *vert_data = mesh->GetPositionPtr();
	const char *mat_indices = mesh->GetSkinMatricesPtr();
	const float *weights = mesh->GetSkinWeightsPtr();
	
	const Mat4* mats = pose->GetMatricesPtr();

	Vec3 v0,v1,v2,v3;
	int i = 0;

	for(i = 0; i < num_samples; ++i) {
		int sample = sample_indices[i];
		int vert_sample = sample*3;
		int mat_sample = sample*4;
		Vec3 mesh_vert(vert_data[vert_sample],vert_data[vert_sample+1],vert_data[vert_sample+2]);
		const float *w = &weights[mat_sample];
		const char* indices = &mat_indices[mat_sample];
		ASSERT(indices[0] < pose->GetNumJoints() && 
			   indices[1] < pose->GetNumJoints() && 
			   indices[2] < pose->GetNumJoints() && 
			   indices[3] < pose->GetNumJoints());
		const Mat4& m1 = mats[ int(indices[0]) ];
		const Mat4& m2 = mats[ int(indices[1]) ];
		const Mat4& m3 = mats[ int(indices[2]) ];
		const Mat4& m4 = mats[ int(indices[3]) ];
	
		v0 = transform_point( m1, mesh_vert );
		v1 = transform_point( m2, mesh_vert );
		v2 = transform_point( m3, mesh_vert );
		v3 = transform_point( m4, mesh_vert );

		(void)w;
		out[i] = w[0] * v0 + 
			w[1] * v1 + 
			w[2] * v2 + 
			w[3] * v3;
	}
}
