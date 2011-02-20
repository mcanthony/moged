#ifndef INCLUDED_samplers_skeleton_sampler_HH
#define INCLUDED_samplers_skeleton_sampler_HH

#include <vector>
#include "Vector.hh"
#include "samplers/base_sampler.hh"

class Skeleton;

class SkeletonCloudSampler : public CloudSampler
{
    const Skeleton* m_skel;

    int m_samplesPerFrame;                      // sum of values in m_numJointSamples
    std::vector< int > m_numJointSamples;       // number of samples for each joint. (size should be == #joints)
    std::vector< Vec3 > m_samples;              // all of the sample positions in rest position.
    float m_sampleInterval;                     // sample period.. distance between sample points on a joint.

public:
    SkeletonCloudSampler();
    ~SkeletonCloudSampler();

    void Init(int requestedNumPoints, const Skeleton* skel, float sampleInterval);
    
    // CloudSampler interface
    int GetSamplesPerFrame() ;
    void GetSamples(Vec3* allSamples, int sampleCount,
        const Clip* clip, int numFrames);
    void GetSampleWeights(float *samplesForFrame);
};

#endif

