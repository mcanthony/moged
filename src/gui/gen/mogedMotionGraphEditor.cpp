#include <cstdio>
#include <ostream>
#include <iomanip>
#include <omp.h>
#include <wx/wx.h>
#include <wx/confbase.h>
#include "mogedMotionGraphEditor.h"
#include "mogedDifferenceFunctionViewer.h"
#include "MathUtil.hh"
#include "motiongraph.hh"
#include "appcontext.hh"
#include "entity.hh"
#include "clipdb.hh"
#include "clip.hh"
#include "mesh.hh"
#include "skeleton.hh"
#include "mogedevents.hh"

using namespace std;

static const int kSampleRateResolution = 10000;
static const int kMaxError = 500;
static const int kErrorResolution = 1000;
static const int kWeightFalloffResolution = 1000;

enum StateType{ 
	StateType_TransitionsIdle = 0,
	StateType_ProcessNextClipPair,
	StateType_FindingTransitions,
	StateType_TransitionsPaused,	
	StateType_TransitionsStepPaused,
	StateType_CreatingBlends,
};

mogedMotionGraphEditor::mogedMotionGraphEditor( wxWindow* parent, AppContext* ctx )
:
MotionGraphEditor( parent )
, m_ctx(ctx)
, m_current_state(StateType_TransitionsIdle)
{
	RestoreSavedSettings();

	m_btn_create->Enable();
	m_btn_cancel->Disable();
	m_btn_pause->Disable();
	m_btn_next->Disable();
	m_btn_continue->Disable();
	m_btn_view_diff->Disable();

	m_listbook->SetPageText(0, _("Transitions"));
	m_listbook->SetPageText(1, _("Graph Pruning"));

	m_stepping = false;
}

mogedMotionGraphEditor::~mogedMotionGraphEditor()
{
	SaveSettings();
}

void mogedMotionGraphEditor::OnIdle( wxIdleEvent& event )
{
	ostream out(m_report);

	const ClipDB* clips = m_ctx->GetEntity()->GetClips();

	switch(m_current_state)
	{
	case StateType_CreatingBlends:
		if(m_ctx->GetEntity()->GetSkeleton() == 0) {
			out << "FATAL ERROR: skeleton became null!" << endl;
			m_current_state = StateType_TransitionsIdle;
			return;
		}

		if(m_working.transition_candidates.empty()) {
			m_btn_create->Enable();
			m_btn_cancel->Disable();
			m_btn_pause->Disable();
			m_btn_next->Disable();
			m_btn_continue->Disable();
			m_btn_view_diff->Disable();
			
			m_current_state = StateType_TransitionsIdle;
		}
		else {
			CreateBlendFromCandidate(out);
		}
		break;
	case StateType_ProcessNextClipPair:
		if(clips == 0) {
			out << "FATAL ERROR: clips has somehow become null!" << endl;
			m_current_state = StateType_TransitionsIdle;
			return;
		} 
	
		if( m_edge_pairs.empty() ) { 
			out << "No pairs left to process. " << endl;
			out << "Found a total of " << m_working.transition_candidates.size() << " suitable transition candidates." << endl;
			m_progress->SetRange(m_working.transition_candidates.size());
			m_progress->SetValue(0);
			m_current_state = StateType_CreatingBlends;
		} else {
			CreateTransitionWorkListAndStart(clips, out);
		}
		break;

	case StateType_FindingTransitions:
		if(clips == 0) {
			out << "FATAL ERROR: clips has somehow become null!" << endl;
			m_current_state = StateType_TransitionsIdle;
			return;
		} 

		if(!ProcessNextTransition()) 
		{
			// Compute minima
			findErrorFunctionMinima( m_transition_finding.error_function_values, 
									 m_transition_finding.to_max,
									 m_transition_finding.from_max,
									 m_transition_finding.minima_indices );

			ExtractTransitionCandidates();

			m_current_state = StateType_ProcessNextClipPair;
			if(m_stepping) {
				m_btn_create->Disable();
				m_btn_cancel->Enable();
				m_btn_pause->Disable();
				m_btn_next->Enable();
				m_btn_continue->Enable();
				m_btn_view_diff->Enable();
				m_stepping = false;
				m_current_state = StateType_TransitionsStepPaused;
			}
		}

		break;

	case StateType_TransitionsStepPaused:
		break;
	case StateType_TransitionsPaused:
		break;	
	case StateType_TransitionsIdle:
		break;
	default:
		break;
	}
	event.Skip();
}

void mogedMotionGraphEditor::OnPageChanged( wxListbookEvent& event )
{
	(void)event;
	// TODO: Implement OnPageChanged
}

void mogedMotionGraphEditor::OnPageChanging( wxListbookEvent& event )
{
	(void)event;
	// TODO: Implement OnPageChanging
	
}

void mogedMotionGraphEditor::OnScrollErrorThreshold( wxScrollEvent& event )
{
	(void)event;
	int slider_value = m_error_slider->GetValue();
	float error_threshold = float(slider_value)/(float(kErrorResolution));
	m_error_value->Clear();
	ostream val(m_error_value);
	val << setprecision(6) << error_threshold;
}

void mogedMotionGraphEditor::OnEditErrorThreshold( wxCommandEvent& event )
{
	(void)event;
	float error_value = atof(m_error_value->GetValue().char_str());
	error_value = Clamp(error_value, m_error_slider->GetMin() / float(kErrorResolution),
						m_error_slider->GetMax() / float(kErrorResolution));
	int slider_value = int(error_value * kErrorResolution);
	m_error_slider->SetValue(slider_value);
}

void mogedMotionGraphEditor::OnScrollCloudSampleRate( wxScrollEvent& event )
{
	(void)event;
	int slider_value = m_point_cloud_rate->GetValue();
	float sample_rate = float(slider_value)/(float(kSampleRateResolution));
	m_point_cloud_rate_value->Clear();
	ostream val(m_point_cloud_rate_value);
	val << setprecision(6) <<  sample_rate;
}

void mogedMotionGraphEditor::OnEditCloudSampleRate( wxCommandEvent& event )
{
	(void)event;
	float sample_rate = atof(m_point_cloud_rate_value->GetValue().char_str());
	sample_rate = Clamp(sample_rate,0.f,1.f);
	int slider_value = int(kSampleRateResolution * sample_rate);
	m_point_cloud_rate->SetValue(slider_value);

}

void mogedMotionGraphEditor::OnScrollFalloff( wxScrollEvent& event )
{
	(void)event;
	float falloff = float(m_weight_falloff->GetValue())/(float(kWeightFalloffResolution));
	m_weight_falloff_value->Clear();
	ostream val(m_weight_falloff_value);
	val << setprecision(6) << falloff;
}

void mogedMotionGraphEditor::OnEditFalloff( wxCommandEvent& event )
{
	(void)event;
	float falloff = atof(m_weight_falloff_value->GetValue().char_str());
	falloff = Clamp(falloff, 0.f, 1.f);
	int slider_value = int(falloff * kWeightFalloffResolution);
	m_weight_falloff->SetValue(slider_value);
}


void mogedMotionGraphEditor::OnTransitionLengthChanged( wxCommandEvent& event ) 
{
	(void)event;
	// compute number of frames given the rate that we will check them
	float transition_length = atof(m_transition_length->GetValue().char_str());
	float fps_rate = atof(m_fps_sample_rate->GetValue().char_str());
	m_transition_frames->Clear();
	(*m_transition_frames) << fps_rate*transition_length;
}


void mogedMotionGraphEditor::OnCreate( wxCommandEvent& event )
{
	(void)event;

	{
		// clear all cloud data from other windows before we delete it (m_working.clear())
		Events::PublishCloudDataEvent ev;
		ev.CloudA = 0;
		ev.CloudALen = 0;
		ev.CloudB = 0;
		ev.CloudBLen = 0;
		m_ctx->GetEventSystem()->Send(&ev);
	}

	m_settings.clear();
	m_working.clear();

	m_time_left->Clear();
	*m_time_left << _("N/A");

	m_report->Clear();
	const Skeleton* skel = m_ctx->GetEntity()->GetSkeleton();
	if(skel == 0)
	{
		wxMessageDialog dlg(this, _("No skeleton available. Cannot create graph!"),
						_("Error"), wxOK|wxICON_ERROR);
		dlg.ShowModal();
		return;
	}

	const Mesh* mesh = m_ctx->GetEntity()->GetMesh();
	if(mesh == 0) {
		wxMessageDialog dlg(this, _("A mesh is required to generate a point cloud for frame comparisons."),
							_("Error"), wxOK|wxICON_ERROR);
		dlg.ShowModal();
		return;
	}

	if(m_ctx->GetEntity()->GetMotionGraph())
	{
		wxMessageDialog dlg(this, _("This will destroy the current graph. Are you sure?"),
							_("Confirm"), wxYES_NO|wxICON_QUESTION);
		if(dlg.ShowModal() != wxID_YES)
			return;
	}

	const ClipDB* clips = m_ctx->GetEntity()->GetClips();
	if(clips == 0 || clips->GetNumClips()==0) {
		wxMessageDialog dlg(this, _("No clips to add to graph."),
							_("Error"), wxOK|wxICON_ERROR);
		dlg.ShowModal();
		return;
	}

	ostream out(m_report);
	
	ReadSettings();

	if(m_settings.transition_length <= 0.f) {
		wxMessageDialog dlg(this, _("Must have a positive transition length."), _("Error"), wxOK|wxICON_ERROR);
		dlg.ShowModal();
		return;
	}

	if(m_settings.sample_rate <= 0.f) {
		wxMessageDialog dlg(this, _("Must have a positive fps sample rate."), _("Error"), wxOK|wxICON_ERROR);
		dlg.ShowModal();
		return;
	}

	int num_points_in_cloud = m_ctx->GetEntity()->GetMesh()->GetNumVerts() * m_settings.point_cloud_rate;
	if(num_points_in_cloud <= 0) {
		wxMessageDialog dlg(this, _("Empty point cloud. Try increasing cloud sample percentage."), _("Error"), wxOK|wxICON_ERROR);
		dlg.ShowModal();
		return;
	}

	int max_point_cloud_size = atoi( m_max_point_cloud_size->GetValue().char_str());
	num_points_in_cloud = Min(max_point_cloud_size, num_points_in_cloud);

	selectMotionGraphSampleVerts(m_ctx->GetEntity()->GetMesh(), num_points_in_cloud, m_working.sample_verts);

	out << "Starting with: " << endl
		<< "No. OMP Threads: " << m_settings.num_threads << endl
		<< "Maximum Error Threshold: " << m_settings.error_threshold << endl
		<< "Point Cloud Sample Rate: " << m_settings.point_cloud_rate << endl
		<< "Points in Cloud: " << num_points_in_cloud << endl
		<< "Sample verts collected: " << m_working.sample_verts.size() << endl
		<< "Transition Length: " << m_settings.transition_length << endl
		<< "FPS Sample Rate: " << m_settings.sample_rate << endl
		<< "Traisition Samples: " << m_settings.num_samples << endl
		<< "Cloud Sample Interval: " << m_settings.sample_interval << endl
		<< "Falloff is " << m_settings.weight_falloff << " - weights will start with a factor of 1.0 and end at " << pow(m_settings.weight_falloff, m_settings.num_samples) << endl;

	// Finally use clips DB to populate a new motiongraph.
	sqlite3_int64 graph_id = NewMotionGraph(m_ctx->GetEntity()->GetDB(), skel->GetID());
	m_ctx->GetEntity()->SetCurrentMotionGraph( graph_id );
	MotionGraph *graph = m_ctx->GetEntity()->GetMotionGraph();
	if(graph == 0) {
		wxMessageDialog dlg(this, _("Failed to create new motion graph."), _("Error"), wxOK|wxICON_ERROR);
		dlg.ShowModal();
		return;
	}
	
	populateInitialMotionGraph(graph, clips, out);

	m_working.num_clouds = graph->GetNumEdges();
	m_working.clouds = new Vec3*[ m_working.num_clouds ];
	memset(m_working.clouds, 0, sizeof(Vec3*)*m_working.num_clouds);
	m_working.cloud_lengths = new int[ m_working.num_clouds ];
	memset(m_working.cloud_lengths, 0, sizeof(int)*m_working.num_clouds);

	omp_set_num_threads(m_settings.num_threads);

	InitJointWeights(skel->GetSkeletonWeights(), mesh);

	out << "Inverse sum of weights is " << m_working.inv_sum_weights << endl;
	CreateWorkListAndStart(graph);
}

void mogedMotionGraphEditor::OnCancel( wxCommandEvent& event )
{
	(void)event;

	wxMessageDialog dlg(this, _("Are you sure you want to cancel? This will undo any progress."), _("Confirm"),
						wxYES_NO|wxICON_QUESTION);
	if(dlg.ShowModal() != wxID_YES) 
		return;

	m_btn_create->Enable();
	m_btn_cancel->Disable();
	m_btn_pause->Disable();
	m_btn_next->Disable();
	m_btn_continue->Disable();
	m_btn_view_diff->Disable();

	ostream out(m_report);
	out << "Cancelled." << endl;
	m_current_state = StateType_TransitionsIdle;
	// TODO: delete cancelled motion graph
	m_ctx->GetEntity()->SetCurrentMotionGraph(0);
}

void mogedMotionGraphEditor::OnPause( wxCommandEvent& event )
{
	(void)event;

	m_btn_create->Disable();
	m_btn_cancel->Enable();
	m_btn_pause->Disable();
	m_btn_next->Enable();
	m_btn_continue->Enable();
	m_btn_view_diff->Enable();

	m_current_state = StateType_TransitionsPaused;
}

void mogedMotionGraphEditor::OnNext( wxCommandEvent& event )
{
	(void)event;
	m_btn_create->Disable();
	m_btn_cancel->Enable();
	m_btn_pause->Disable();
	m_btn_next->Disable();
	m_btn_continue->Disable();
	m_btn_view_diff->Disable();
		
	m_stepping = true;

	if(m_current_state == StateType_TransitionsStepPaused)
		m_current_state = StateType_ProcessNextClipPair;
	else
		m_current_state = StateType_FindingTransitions;
}

void mogedMotionGraphEditor::OnContinue( wxCommandEvent& event )
{
	(void)event;
	m_btn_create->Disable();
	m_btn_cancel->Enable();
	m_btn_pause->Enable();
	m_btn_next->Disable();
	m_btn_continue->Disable();
	m_btn_view_diff->Disable();

	m_current_state = StateType_FindingTransitions;
}

void mogedMotionGraphEditor::OnNextStage( wxCommandEvent& event )
{
	(void)event;
	// TODO: Implement OnNextStage
}

void mogedMotionGraphEditor::OnViewDistanceFunction( wxCommandEvent& event )
{
	(void)event;
	if(m_current_state == StateType_TransitionsStepPaused ||
		m_current_state == StateType_TransitionsPaused)
	{
		ASSERT(!m_transition_finding.from.Null() && !m_transition_finding.to.Null() &&
			m_transition_finding.error_function_values);

		const int dim_y = m_transition_finding.from_max;
		const int dim_x = m_transition_finding.to_max;
		mogedDifferenceFunctionViewer dlg(this, dim_y, dim_x, m_transition_finding.error_function_values,
										  m_transition_finding.current_error_threshold,
										  m_transition_finding.minima_indices,
										  m_transition_finding.from->GetClip()->GetName(),
										  m_transition_finding.to->GetClip()->GetName());
		dlg.ShowModal();
	}
}

void mogedMotionGraphEditor::OnClose( wxCloseEvent& event ) 
{
	// this stuff will be deleted when we exit, so tell other windows to stop looking at it.
	Events::PublishCloudDataEvent ev;
	ev.CloudA = 0;
	ev.CloudALen = 0;
	ev.CloudB = 0;
	ev.CloudBLen = 0;
	m_ctx->GetEventSystem()->Send(&ev);

	event.Skip();
}


////////////////////////////////////////////////////////////////////////////////
mogedMotionGraphEditor::TransitionWorkingData::TransitionWorkingData()
	: num_clouds(0), clouds(0), cloud_lengths(0), joint_weights(0)
{
	clear();
}

void mogedMotionGraphEditor::TransitionWorkingData::clear()
{
	sample_verts.clear();

	for(int i = 0; i < num_clouds; ++i) {
		delete[] clouds[i];
	}
	delete[] clouds; clouds = 0;
	delete[] cloud_lengths; cloud_lengths = 0;
	num_clouds = 0;

	for(int i = 0; i < mogedMotionGraphEditor::TIMING_SAMPLES; ++i)
		processed_per_second[i] = 0.f;
	next_sample_idx = 0;

	delete[] joint_weights; joint_weights = 0;
	inv_sum_weights = 0.f;

	transition_candidates.clear();
	
	working_set.clear();
}

mogedMotionGraphEditor::TransitionFindingData::TransitionFindingData()
	: error_function_values(0), alignment_translations(0), alignment_angles(0)
{ 
	clear() ;
}

void mogedMotionGraphEditor::TransitionFindingData::clear()
{
	from_idx = 0;
	to_idx = 0;
	from = 0;
	to = 0;
	from_frame = 0;
	to_frame = 0;
	from_max = 0;
	to_max = 0;

	current_error_threshold = 0.f;
	delete[] error_function_values; error_function_values = 0;
	minima_indices.clear();

	delete[] alignment_translations; alignment_translations = 0;
	delete[] alignment_angles; alignment_angles = 0;
}

void mogedMotionGraphEditor::Settings::clear()
{
	num_threads = 0;
	error_threshold = 0.f;
	point_cloud_rate = 0.f;
	transition_length = 0.f;
	sample_rate = 0.f;
	weight_falloff = 0.f;
	num_samples = 0;
	sample_interval = 0.f;
}

////////////////////////////////////////////////////////////////////////////////

void mogedMotionGraphEditor::ReadSettings()
{
	m_settings.clear();

	m_settings.num_threads = atoi(m_num_threads->GetValue().char_str());
	m_settings.num_threads = Max(1,m_settings.num_threads);

	float error_value = atof(m_error_value->GetValue().char_str());
	m_settings.error_threshold = Clamp(error_value, m_error_slider->GetMin() / float(kErrorResolution),
									 m_error_slider->GetMax() / float(kErrorResolution));
	
	float sample_rate = atof(m_point_cloud_rate_value->GetValue().char_str());
	m_settings.point_cloud_rate = Clamp(sample_rate,0.f,1.f);
	
	float fps_rate = atof(m_fps_sample_rate->GetValue().char_str());
	m_settings.sample_rate = fps_rate;
	
	float transition_length = atof(m_transition_length->GetValue().char_str());
	m_settings.transition_length = transition_length ;

	float weight_falloff = atof(m_weight_falloff_value->GetValue().char_str());
	m_settings.weight_falloff = Clamp(1.0f - weight_falloff, 0.f, 1.f);
	
	m_settings.num_samples = int(transition_length * fps_rate);
	if(fps_rate > 0.f)
		m_settings.sample_interval = 1.f/fps_rate;
}

// create work list and start state machine
void mogedMotionGraphEditor::CreateWorkListAndStart(const MotionGraph* graph)
{
	m_edge_pairs.clear();

	std::vector< sqlite3_int64 > edges;
	graph->GetEdgeIDs(edges);
	// map the ids to actual handles.
	const int num_edges = edges.size();
	m_working.working_set.resize(num_edges);
	for(int i = 0; i < num_edges; ++i) {
		m_working.working_set[i] = graph->GetEdge( edges[i] );
	}
	
	// now generate pairs from this list - as a side effect this will pre-cache all of the clips.
	int work_size = 0;
	for(int from = 0; from < num_edges; ++from)
	{
		const MGEdgeHandle from_edge = m_working.working_set[from];
		const int from_size = Max(0,int(from_edge->GetClip()->GetClipTime() * m_settings.sample_rate) - m_settings.num_samples);
		for(int to = 0; to < num_edges; ++to) {
			const MGEdgeHandle to_edge = m_working.working_set[to];
			const int to_size = Max(0,int(to_edge->GetClip()->GetClipTime() * m_settings.sample_rate) - m_settings.num_samples);
			work_size += from_size * to_size;
			m_edge_pairs.push_back( make_pair(from,to) );
		}
	}
	
	work_size += num_edges; // for clouds
	m_progress->SetRange(work_size);
	m_progress->SetValue(0);
	m_current_state = StateType_ProcessNextClipPair;
}
	
void mogedMotionGraphEditor::CreateTransitionWorkListAndStart(const ClipDB * clips, ostream& out)
{
	EdgePair pair = m_edge_pairs.front();
	m_edge_pairs.pop_front();

	MGEdgeHandle from_edge = m_working.working_set[pair.first];
	MGEdgeHandle to_edge = m_working.working_set[pair.second];

	out << "Finding transitions from \"" << from_edge->GetClip()->GetName() << "\" to \"" <<
		to_edge->GetClip()->GetName() << "\". " << endl;			

	m_transition_finding.clear();
	m_transition_finding.from_idx = pair.first;
	m_transition_finding.to_idx = pair.second;
	m_transition_finding.from = from_edge;
	m_transition_finding.to = to_edge;	
	m_transition_finding.from_frame = 0;

	m_transition_finding.from_max = Max(0,int(from_edge->GetClip()->GetClipTime() * m_settings.sample_rate) - m_settings.num_samples);
	m_transition_finding.to_max = Max(0,int(to_edge->GetClip()->GetClipTime() * m_settings.sample_rate) - m_settings.num_samples);
	m_transition_finding.to_frame = 0;

	if(m_transition_finding.from_max == 0)
	{
		out << "Discarding pair, \"" << from_edge->GetClip()->GetName() << "\" isn't long enough to have a transition of length " << m_settings.transition_length << endl;
		m_transition_finding.clear();
		return;
	}
	else if(m_transition_finding.to_max == 0)
	{
		out << "Discarding pair, \"" << to_edge->GetClip()->GetName() << "\" isn't long enough to have a transition of length " << m_settings.transition_length << endl;
		m_transition_finding.clear();
		return;
	}

	std::vector< Annotation > from_clip_annotations, to_clip_annotations;
	clips->GetAnnotations(from_clip_annotations, from_edge->GetClip()->GetID());
	clips->GetAnnotations(to_clip_annotations, to_edge->GetClip()->GetID());
	m_transition_finding.current_error_threshold = m_settings.error_threshold;
	int count = from_clip_annotations.size();
	for(int i = 0; i < count; ++i) 
		m_transition_finding.current_error_threshold = Min(m_transition_finding.current_error_threshold,
														   from_clip_annotations[i].GetFidelity());
	count = to_clip_annotations.size();
	for(int i = 0; i < count; ++i) 
		m_transition_finding.current_error_threshold = Min(m_transition_finding.current_error_threshold,
														   to_clip_annotations[i].GetFidelity());

	const int num_error_vals = 	m_transition_finding.from_max * m_transition_finding.to_max;
	m_transition_finding.error_function_values = new float[num_error_vals];
	for(int i = 0; i < num_error_vals; ++i) m_transition_finding.error_function_values[i] = 9999.f;
	m_transition_finding.alignment_translations = new Vec3[num_error_vals];
	memset(m_transition_finding.alignment_translations, 0, sizeof(Vec3)*num_error_vals);
	m_transition_finding.alignment_angles = new float[num_error_vals];
	memset(m_transition_finding.alignment_angles, 0, sizeof(float)*num_error_vals);

	m_current_state = StateType_FindingTransitions;
	m_btn_create->Disable();
	m_btn_cancel->Enable();
	m_btn_pause->Enable();
}

bool mogedMotionGraphEditor::ProcessNextTransition()
{
	static const double kMaxTime = 1.f;
	const int num_from = m_transition_finding.from_max;
	const int num_to = m_transition_finding.to_max;

	const Skeleton* skel = m_ctx->GetEntity()->GetSkeleton();
	const Mesh* mesh = m_ctx->GetEntity()->GetMesh();

	ASSERT(skel && mesh);

	ASSERT(m_working.clouds && m_working.num_clouds > 0 && m_working.cloud_lengths);

	int num_processed = 0;
	double start_time = omp_get_wtime();
	double time_so_far;

	// always do at least one thing every time this function is called, 
	// and don't take longer than kMaxTime. This is enforced through the time_so_far && num_processed checks.

	// Allocate missing point clouds. 
	if(m_working.clouds[m_transition_finding.from_idx] == 0)
	{
		int num_samples = Max(1,int(m_transition_finding.from->GetClip()->GetClipTime() * m_settings.sample_rate));
		int len = m_working.sample_verts.size() * num_samples;
		m_working.clouds[m_transition_finding.from_idx] = new Vec3[ len ];
		m_working.cloud_lengths[m_transition_finding.from_idx] = num_samples;

		getPointCloudSamples(m_working.clouds[m_transition_finding.from_idx],
							 mesh, 
							 skel, 
							 m_working.sample_verts,
							 m_transition_finding.from->GetClip().RawPtr(),
							 num_samples,
							 m_settings.sample_interval, 
							 m_settings.num_threads);

		PublishCloudData(false, Vec3(), 0);
		++num_processed;
	}

	time_so_far = omp_get_wtime() - start_time;
	if( (time_so_far < kMaxTime || num_processed == 0) &&
		m_working.clouds[m_transition_finding.to_idx] == 0)
	{
		int num_samples = Max(1,int(m_transition_finding.to->GetClip()->GetClipTime() * m_settings.sample_rate));
		int len = m_working.sample_verts.size() * num_samples;
		m_working.clouds[m_transition_finding.to_idx] = new Vec3[ len ];
		m_working.cloud_lengths[m_transition_finding.to_idx] = num_samples;

		getPointCloudSamples(m_working.clouds[m_transition_finding.to_idx],
							 mesh, 
							 skel, 
							 m_working.sample_verts,
							 m_transition_finding.to->GetClip().RawPtr(),
							 num_samples,
							 m_settings.sample_interval,
							 m_settings.num_threads);

		PublishCloudData(false, Vec3(), 0);
		++num_processed;
	}

	// Compute difference points.
	int from = m_transition_finding.from_frame,
		to = m_transition_finding.to_frame;

	time_so_far = omp_get_wtime() - start_time;
	while(from < num_from && (time_so_far < kMaxTime || num_processed == 0)) {
		if(m_working.clouds[m_transition_finding.from_idx] &&
		   m_working.clouds[m_transition_finding.to_idx])
		{
			if(to < num_to) {
				// do comparison
				int from_end = from + m_settings.num_samples;
				int to_end = to + m_settings.num_samples;

				int from_cloud_len = m_working.cloud_lengths[m_transition_finding.from_idx];
				int to_cloud_len =  m_working.cloud_lengths[m_transition_finding.to_idx];

				// don't go past the end of what we've allocated - just shorten the comparison. 
				from_end = Min(from_end, from_cloud_len);
				to_end = Min(to_end, to_cloud_len);

				int len = Min(from_end - from, to_end - to);
				ASSERT(len > 0);
				ASSERT(len <= m_settings.num_samples);

				int from_cloud_offset = from * m_working.sample_verts.size();
				int to_cloud_offset = to * m_working.sample_verts.size();

				const Vec3* from_cloud = &m_working.clouds[m_transition_finding.from_idx][ from_cloud_offset ];
				const Vec3* to_cloud = &m_working.clouds[m_transition_finding.to_idx][ to_cloud_offset ];

				ASSERT(from_cloud + len * m_working.sample_verts.size() <= 
					   m_working.clouds[m_transition_finding.from_idx] + from_cloud_len * m_working.sample_verts.size());
				ASSERT(to_cloud + len * m_working.sample_verts.size() <= 
					   m_working.clouds[m_transition_finding.to_idx] + to_cloud_len * m_working.sample_verts.size());

				Vec3 align_translation(0,0,0);
				float align_rotation = 0.f;

				computeCloudAlignment(from_cloud, to_cloud, 
									  m_working.sample_verts.size(), 
									  len, 
									  m_working.joint_weights, 
									  m_working.inv_sum_weights, 
									  align_translation, 
									  align_rotation,
									  m_settings.num_threads);

				float difference = computeCloudDifference(from_cloud, to_cloud, 
														  m_working.joint_weights, 
														  m_working.sample_verts.size(), 
														  len, 
														  align_translation, align_rotation,
														  m_settings.num_threads);							   
				
				if(difference < m_transition_finding.current_error_threshold)
					PublishCloudData(true, align_translation, align_rotation, 
									 from_cloud_offset, len, to_cloud_offset, len);

				m_transition_finding.error_function_values[ from * num_to + to ] = difference;
				m_transition_finding.alignment_translations[ from * num_to + to ] = align_translation;
				m_transition_finding.alignment_angles[ from * num_to + to ] = align_rotation;
				++num_processed;
				++to;
			} else {
				++from;
				to = 0;
			}
		} else {
			break;
		}
		time_so_far = omp_get_wtime() - start_time;
	}


	int num_so_far = m_progress->GetValue() + num_processed;	
 	m_progress->SetValue(num_so_far);	
	double end_time = omp_get_wtime();

	UpdateTiming(num_processed/(float)(end_time - start_time));

	m_transition_finding.from_frame = from ;
	m_transition_finding.to_frame = to ;

	if(from < num_from)
		return true;
 	return false;
}

void mogedMotionGraphEditor::UpdateTiming(float num_per_sec)
{
	m_working.processed_per_second[m_working.next_sample_idx] = num_per_sec;
	m_working.next_sample_idx = (m_working.next_sample_idx+1)%TIMING_SAMPLES;
	float avg = 0.f;
	for(int i = 0; i < TIMING_SAMPLES; ++i)
		avg += m_working.processed_per_second[i];
	avg /= float(TIMING_SAMPLES);
	int num_left = 	m_progress->GetRange() - m_progress->GetValue();
	
	m_time_left->Clear();

	if(avg > 0.f) {
		int num_seconds = int(num_left / avg);
		int num_days = num_seconds / (3600*24);
		int num_hours = (num_seconds % (3600*24)) / (3600);
		int num_minutes = (num_seconds % (3600)) / 60;
		num_seconds = num_seconds % 60;

		if(num_days > 0) 
			*m_time_left << num_days << _("d ");
		if(num_hours > 0) 
			*m_time_left << num_hours << _("h ");
		if(num_minutes > 0)
			*m_time_left << num_minutes << _("m ");
		*m_time_left << num_seconds << _("s ");

	} else {
		*m_time_left << _("N/A") ;
	}
}

void mogedMotionGraphEditor::PublishCloudData(bool do_align, Vec3_arg align_translation, float align_rotation,
											  int from_offset, int from_len , int to_offset, int to_len)
{
	Events::PublishCloudDataEvent ev;
	ev.SamplesPerFrame = m_working.sample_verts.size();
	ev.CloudA = &m_working.clouds[m_transition_finding.from_idx][from_offset];
	ev.CloudALen = from_len > 0 ? from_len : m_working.cloud_lengths[m_transition_finding.from_idx];
	ev.CloudB = &m_working.clouds[m_transition_finding.to_idx][to_offset];
	ev.CloudBLen = to_len > 0 ? to_len : m_working.cloud_lengths[m_transition_finding.to_idx];
	ev.AlignRotation = align_rotation;
	ev.AlignTranslation = align_translation;
	ev.Align = do_align ? 1 : 0;
	m_ctx->GetEventSystem()->Send(&ev);	
}

void mogedMotionGraphEditor::InitJointWeights(const SkeletonWeights& weights, const Mesh* mesh)
{
	const int num_frames = m_settings.num_samples;
	const int samples_per_frame = m_working.sample_verts.size();
	const int numWeights = num_frames * samples_per_frame;
	m_working.joint_weights = new float[numWeights];
	if(numWeights == 0) return;
	memset(m_working.joint_weights, 0, sizeof(float)*numWeights);

	const char *mat_indices = mesh->GetSkinMatricesPtr();
	const float *skin_weights = mesh->GetSkinWeightsPtr();
	float *out_weights = m_working.joint_weights;
	const std::vector<int> &sample_verts = m_working.sample_verts;

	int i = 0;
#pragma omp parallel for private(i) shared(sample_verts, weights, mat_indices, out_weights)
	// get bone influences for each 
	for(i = 0; i < samples_per_frame; ++i) {
		int sample_idx = sample_verts[i];
		int mat_sample = 4*sample_idx;
		
		const float *w = &skin_weights[mat_sample];
		const char* indices = &mat_indices[mat_sample];

		float weight = w[0] * weights.GetJointWeight( indices[0] )
			+ w[1] * weights.GetJointWeight( indices[1] )
			+ w[2] * weights.GetJointWeight( indices[2] )
			+ w[3] * weights.GetJointWeight( indices[3] );

		out_weights[i] = weight;
	}

	int frame = 1;
	for(frame = 1; frame < num_frames; ++frame)
	{
		int last_idx = samples_per_frame*(frame-1);
		int out_idx = samples_per_frame*frame;
		memcpy(&out_weights[out_idx], &out_weights[last_idx], sizeof(float)*samples_per_frame);
		for(int i = 0; i < samples_per_frame; ++i) {
			out_weights[out_idx++] *= m_settings.weight_falloff;
		}
	}

	double sum = 0.0;
#pragma omp parallel for private(i) shared(out_weights) reduction(+:sum)
	for(i = 0; i < numWeights; ++i)
	{
		sum += out_weights[i];
	}

	m_working.inv_sum_weights = float(1.0 / sum);
}

void mogedMotionGraphEditor::RestoreSavedSettings()
{
	wxConfigBase* cfg = wxConfigBase::Get();
	if(cfg == 0) return;
	
	long lval = 0;
	double dval = 0.0;

	m_point_cloud_rate->SetRange(1, kSampleRateResolution);
	if(cfg->Read(_("MotionGraphEditor/PointCloudSampleRate"), &lval)) 
		m_point_cloud_rate->SetValue(lval);
	else 
		m_point_cloud_rate->SetValue(100);
	m_point_cloud_rate_value->Clear();
	ostream cloud_val(m_point_cloud_rate_value);
	cloud_val << setprecision(6) << float(m_point_cloud_rate->GetValue())/kSampleRateResolution;
	
	m_error_slider->SetRange(1, kErrorResolution * kMaxError);
	if(cfg->Read(_("MotionGraphEditor/ErrorThreshold"), &lval)) 
		m_error_slider->SetValue(lval);
	else 
		m_error_slider->SetValue(int(0.5*kErrorResolution));
	m_error_value->Clear();
	ostream error_val(m_error_value);
	error_val << setprecision(6) << float(m_error_slider->GetValue())/(kErrorResolution);

	m_weight_falloff->SetRange(0, kWeightFalloffResolution);
	if(cfg->Read(_("MotionGraphEditor/WeightFalloff"), &lval)) 
		m_weight_falloff->SetValue(lval);
	else 
		m_weight_falloff->SetValue(0.1 * kWeightFalloffResolution);
	m_weight_falloff_value->Clear();
	ostream falloff_val(m_weight_falloff_value);
	falloff_val << setprecision(6) << float(m_weight_falloff->GetValue())/(kWeightFalloffResolution);

	m_transition_length->Clear();
	if(cfg->Read(_("MotionGraphEditor/TransitionLength"), &dval))
		(*m_transition_length) << Max(dval,0.0);
	else
		(*m_transition_length) << 0.25;

	m_fps_sample_rate->Clear();
	if(cfg->Read(_("MotionGraphEditor/FPSSampleRate"), &dval))
		(*m_fps_sample_rate) << Max(dval,1.0);
	else 
		(*m_fps_sample_rate) << 120.0;
		
	m_num_threads->Clear();
	if(cfg->Read(_("MotionGraphEditor/NumOMPThreads"), &lval))
		(*m_num_threads) << Max(lval,1L);
	else
		(*m_num_threads) << omp_get_max_threads();

	m_max_point_cloud_size->Clear();
	if(cfg->Read(_("MotionGraphEditor/MaxPointCloudSize"), &lval))
		(*m_max_point_cloud_size) << Max(lval,1L);
	else
		(*m_max_point_cloud_size) << 100;

}

void mogedMotionGraphEditor::SaveSettings()
{
	wxConfigBase* cfg = wxConfigBase::Get();
	if(cfg == 0) return;
	
	cfg->Write(_("MotionGraphEditor/PointCloudSampleRate"), m_point_cloud_rate->GetValue());
	cfg->Write(_("MotionGraphEditor/ErrorThreshold"), m_error_slider->GetValue());
	cfg->Write(_("MotionGraphEditor/WeightFalloff"), m_weight_falloff->GetValue());
	cfg->Write(_("MotionGraphEditor/TransitionLength"), atof(m_transition_length->GetValue().char_str()));
	cfg->Write(_("MotionGraphEditor/FPSSampleRate"), atof(m_fps_sample_rate->GetValue().char_str()));
	cfg->Write(_("MotionGraphEditor/NumOMPThreads"), atoi(m_num_threads->GetValue().char_str()));	
	cfg->Write(_("MotionGraphEditor/MaxPointCloudSize"), atoi(m_max_point_cloud_size->GetValue().char_str()));
}

void mogedMotionGraphEditor::ExtractTransitionCandidates()
{
	bool self_processing = m_transition_finding.to_idx == m_transition_finding.from_idx;
	const int num_minima = m_transition_finding.minima_indices.size();
	static const float kRootTwo = sqrt(2.f);
	const float minDist = m_settings.num_samples;

	for(int i = 0; i <num_minima; ++i)
	{
		int index = m_transition_finding.minima_indices[i];

		float threshold = m_transition_finding.current_error_threshold;
		if(m_transition_finding.error_function_values[index] < threshold)
		{
			int from_frame = index / m_transition_finding.to_max;
			int to_frame = index % m_transition_finding.to_max;

			// if processing self, ignore any minima within num_samples from the center line (our transition length in frames).
			float dist = kRootTwo * (to_frame - from_frame);
			if(!self_processing || dist > minDist)
			{
				TransitionCandidate c;
				c.from_edge_idx = m_transition_finding.from_idx;
				c.to_edge_idx = m_transition_finding.to_idx;			
				c.from_frame = from_frame;
				c.to_frame = to_frame;			
				c.align_translation = m_transition_finding.alignment_translations[index];
				c.align_rotation = m_transition_finding.alignment_angles[index];
				m_working.transition_candidates.push_back(c);
			}
		}
	}	
}

void mogedMotionGraphEditor::CreateBlendFromCandidate(ostream& out)
{
	TransitionCandidate candidate = m_working.transition_candidates.front();
	m_working.transition_candidates.pop_front();
	m_progress->SetValue( m_progress->GetValue() + 1 );

	ClipHandle from_clip = m_working.working_set[ candidate.from_edge_idx ]->GetClip();
	sqlite3_int64 from_edge_id = m_working.working_set[ candidate.from_edge_idx]->GetID();
	ClipHandle to_clip = m_working.working_set[ candidate.to_edge_idx]->GetClip();
	sqlite3_int64 to_edge_id = m_working.working_set[ candidate.to_edge_idx]->GetID();

	float from_time = candidate.from_frame * m_settings.sample_interval;
	float to_time = candidate.to_frame * m_settings.sample_interval;

	SavePoint save( m_ctx->GetEntity()->GetDB(), "addTransitionEdge");

	sqlite3_int64 newClip = createTransitionClip( m_ctx->GetEntity()->GetDB(), 
												  m_ctx->GetEntity()->GetSkeleton(),
												  from_clip.RawPtr(),
												  to_clip.RawPtr(),
												  from_time,
												  to_time,
												  m_settings.num_samples,
												  m_settings.sample_interval,
												  candidate.align_translation,
												  candidate.align_rotation );

	if(newClip == 0) {
		out << "Error creating blend clip from " << from_clip->GetName() << " to " << to_clip->GetName() << endl;	
	}
	
	MotionGraph* graph = m_ctx->GetEntity()->GetMotionGraph();

	// this transition sends us to to_clip @ to_time + sample_interval * (m_settings.num_samples-1),
	// since we FINISH the transition on that frame.
	int original_clip_frame_from = int(from_time * from_clip->GetClipFPS());
	int original_clip_frame_to = int(to_time * to_clip->GetClipFPS()) + 
		m_settings.sample_interval  * (m_settings.num_samples-1);

	if(original_clip_frame_to >= to_clip->GetNumFrames()) {
		int old = original_clip_frame_to ;
		original_clip_frame_to = Clamp(original_clip_frame_to, 0, to_clip->GetNumFrames() - 1);
		fprintf(stderr, "Warning: transition to clip is beyond the to clip frame count. Clamping %d to %d...\n",
				old, original_clip_frame_to);
	}

	sqlite3_int64 transition_from_node = graph->FindNode(from_clip->GetID(), original_clip_frame_from);
	if(transition_from_node == 0) { // don't already have this start node, so split the original transition.
		transition_from_node = graph->SplitEdge( from_edge_id, original_clip_frame_from );
		if(transition_from_node == 0) { save.Rollback(); return; }
	}
		
	sqlite3_int64 transition_to_node = graph->FindNode(to_clip->GetID(), original_clip_frame_to);
	if(transition_to_node == 0) {
		transition_to_node = graph->SplitEdge( to_edge_id, original_clip_frame_to );
		if(transition_to_node == 0) { save.Rollback(); return; }
	}

	Quaternion align_rotation = make_rotation(candidate.align_rotation, Vec3(0,1,0));
	sqlite3_int64 transition_edge_id = graph->AddTransitionEdge(transition_from_node,
																transition_to_node,
																newClip,
																candidate.align_translation,
																align_rotation);
	if(transition_edge_id == 0) {  save.Rollback(); return; }
				
	Events::ClipAddedEvent ev;
	ev.ClipID = newClip;
	m_ctx->GetEventSystem()->Send(&ev);
}

