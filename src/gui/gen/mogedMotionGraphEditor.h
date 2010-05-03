#ifndef __mogedMotionGraphEditor__
#define __mogedMotionGraphEditor__

/**
@file
Subclass of MotionGraphEditor, which is generated by wxFormBuilder.
*/

#include <list>
#include <vector>
#include "MogedFrames.h"
#include "Vector.hh"

class AppContext;
class MGEdge;
class ClipDB;
class MotionGraph;
class Pose;
class ClipController;
class Vec3;
class Skeleton;
class Mesh;

/** Implementing MotionGraphEditor */
class mogedMotionGraphEditor : public MotionGraphEditor
{
	enum { TIMING_SAMPLES = 30 };

	AppContext *m_ctx;
	int m_current_state;
	
	typedef std::pair<const MGEdge*,const MGEdge*> EdgePair;
	std::list< EdgePair > m_edge_pairs;

	bool m_stepping;

	void ReadSettings();
	void CreateWorkListAndStart(const MotionGraph* graph);
	void CreateTransitionWorkListAndStart(const MotionGraph* graph);
	bool ProcessNextTransition();
	void UpdateTiming(float num_per_sec);
	void PublishCloudData(bool do_align, Vec3_arg align_translation, float align_rotation);

	struct TransitionWorkingData
	{
		std::vector< int > sample_verts; // vert indices to sample
		int num_clouds;
		Vec3 **clouds;
		int *cloud_lengths;

		float processed_per_second[TIMING_SAMPLES];
		int next_sample_idx;
		TransitionWorkingData();
		~TransitionWorkingData() { clear(); }
		void clear();
	};
	
	typedef std::pair<int,int> TransitionPair;
	struct TransitionFindingData
	{
		int from_idx;
		int to_idx;
		const MGEdge* from;
		const MGEdge* to;
		int from_frame;
		int from_max;
		int to_frame;
		int to_max;

		std::vector< TransitionPair > candidates;
		TransitionFindingData() { clear(); }
		void clear();		
	};

	struct Settings {
		int num_threads;
		float error_threshold;
		float point_cloud_rate;
		float transition_length; // in seconds
		float sample_rate; // fps to sample at
		float weight_falloff;

		int num_samples; // number of samples to gather given the above
		float sample_interval; // time per sample.
		Settings() { clear(); }
		void clear();
	};

	TransitionFindingData m_transition_finding;
	Settings m_settings;
	TransitionWorkingData m_working;

protected:
	// Handlers for MotionGraphEditor events.
	void OnIdle( wxIdleEvent& event );
	void OnPageChanged( wxListbookEvent& event );
	void OnPageChanging( wxListbookEvent& event );
	void OnScrollErrorThreshold( wxScrollEvent& event );
	void OnEditErrorThreshold( wxCommandEvent& event );
	void OnScrollCloudSampleRate( wxScrollEvent& event );
	void OnEditCloudSampleRate( wxCommandEvent& event );
	void OnScrollFalloff( wxScrollEvent& event );
	void OnEditFalloff( wxCommandEvent& event );
	void OnCreate( wxCommandEvent& event );
	void OnCancel( wxCommandEvent& event );
	void OnPause( wxCommandEvent& event );
	void OnNext( wxCommandEvent& event );
	void OnContinue( wxCommandEvent& event );
	void OnNextStage( wxCommandEvent& event );
	void OnTransitionLengthChanged( wxCommandEvent& event ) ;
	void OnClose( wxCloseEvent& event ) ;

public:
	/** Constructor */
	mogedMotionGraphEditor( wxWindow* parent, AppContext* ctx );
};

#endif // __mogedMotionGraphEditor__
