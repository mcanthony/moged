#include <cstdio>
#include "assert.hh"
#include "samplers/skeleton_sampler.hh"
#include "clip.hh"
#include "anim/clipcontroller.hh"
#include "anim/pose.hh"
#include "Mat4.hh"
#include "skeleton.hh"
#include "MathUtil.hh"

SkeletonCloudSampler::SkeletonCloudSampler()
    : m_skel(0)
    , m_samplesPerFrame(0)
    , m_sampleInterval(0.f)
{}

SkeletonCloudSampler::~SkeletonCloudSampler()
{
}

void SkeletonCloudSampler::Init(int requestedNumPoints, const Skeleton* skel, float sampleInterval)
{
    m_skel = skel;
    m_sampleInterval = sampleInterval;
    m_numJointSamples.clear();
    m_samples.clear();
    m_samplesPerFrame = 0;

    float totalLength = 0.f;
    const int numJoints = skel->GetNumJoints();
    const int *parents = skel->GetParents();
    const Vec3 *trans = skel->GetJointTranslations();
    for(int i = 0; i < numJoints; ++i)
    {
        int parent = parents[i];
        if(parent != -1) {
            totalLength += magnitude( trans[i] );
        }
    }

    // each joint should have at least 1 point
    m_numJointSamples.reserve( numJoints );    
    float ptRate = totalLength / requestedNumPoints ;
    for(int i = 0; i < numJoints; ++i)
    {
        int parent = parents[i];
        int numThisJoint = 0;
        float len = magnitude(trans[i]) - ptRate;

        Vec3 endPt = (parent == -1) ? skel->GetRootOffset() : skel->GetJointToSkelOffset(parent);
        Vec3 startPt = skel->GetJointToSkelOffset(i);
        Vec3 toEnd = normalize_safe(endPt - startPt, Vec3(1,0,0));

        float curLen = 0.f;

        do 
        {
            Vec3 pt = startPt + curLen * toEnd;
            m_samples.push_back(pt);
            ++numThisJoint;
            curLen += ptRate;
            len -= ptRate;
        } while(len >= 0);

        m_samplesPerFrame += numThisJoint;
        m_numJointSamples.push_back(numThisJoint);
    }
}

int SkeletonCloudSampler::GetSamplesPerFrame()
{
    return m_samplesPerFrame;
}

void SkeletonCloudSampler::GetSamples(Vec3* allSamples, int sampleCount, const Clip* clip, int numFrames)
{
    const int samplesPerFrame = m_samplesPerFrame;
    const int writeCount = samplesPerFrame * numFrames;
    if(sampleCount < writeCount) {
        fprintf(stderr, "Wrong sampleCount passed to GetSamples(). Expected at least %d, got %d\n",
            writeCount, sampleCount);
        return;
    }

    memset(allSamples, 0, sizeof(Vec3) * writeCount);
    ClipController controller(m_skel);
    controller.SetClip(clip);

    const float sampleInterval = m_sampleInterval;
    const int numJoints = m_skel->GetNumJoints();

    int sampleIdx = 0;
    for(int frame = 0; frame < numFrames; ++frame)
    {
        controller.SetTime( frame * sampleInterval );
        controller.ComputePose();
        controller.ComputeMatrices( (Mat4::ident_t()) );
        const Mat4* mats = controller.GetPose()->GetMatricesPtr();

        int sampleSrcIdx = 0;
        for(int i = 0; i < numJoints; ++i)
        {
            const int numSamples = m_numJointSamples[i];
            for(int sample = 0; sample < numSamples; ++sample)
            {
                allSamples[sampleIdx++] = transform_point( mats[i], m_samples[sampleSrcIdx++] );
            }
        }
    }

    ASSERT(sampleIdx == writeCount);
}

void SkeletonCloudSampler::GetSampleWeights(float *samplesForFrame)
{
	const SkeletonWeights& weights = m_skel->GetSkeletonWeights();

    const int numJoints = m_skel->GetNumJoints();
    int sampleIdx = 0;
    for(int i = 0; i < numJoints; ++i)
    {
        const int numSamples = m_numJointSamples[i];
        for(int sample = 0; sample < numSamples; ++sample)
        {
            samplesForFrame[sampleIdx++] = weights.GetJointWeight(i);
        }
    }

    ASSERT(sampleIdx = m_samplesPerFrame);
}

