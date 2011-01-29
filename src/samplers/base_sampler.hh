#ifndef INCLUDED_base_sampler_HH
#define INCLUDED_base_sampler_HH

class CloudSampler
{
public:
	virtual int GetSamplesPerFrame() = 0;
	virtual void GetSamples(Vec3* allSamples, int sampleCount, 
		const Clip* clip, int numFrames) = 0;
	virtual void GetSampleWeights(float *samplesForFrame) = 0;
		
protected:
	CloudSampler() {}
};

#endif

