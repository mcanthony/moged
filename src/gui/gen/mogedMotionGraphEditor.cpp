#include <cstdio>
#include <ostream>
#include <iomanip>
#include <omp.h>
#include <wx/wx.h>
#include "mogedMotionGraphEditor.h"
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
static const int kMaxError = 10;
static const int kErrorResolution = 1000;

enum StateType{ 
	StateType_TransitionsIdle = 0,
	StateType_ProcessNextClipPair,
	StateType_FindingTransitions,
	StateType_TransitionsPaused,	
};

mogedMotionGraphEditor::mogedMotionGraphEditor( wxWindow* parent, AppContext* ctx )
:
MotionGraphEditor( parent )
, m_ctx(ctx)
, m_current_state(StateType_TransitionsIdle)
{
	m_point_cloud_rate->SetRange(1, kSampleRateResolution);
	m_point_cloud_rate->SetValue(100);
	m_point_cloud_rate_value->Clear();
	ostream cloud_val(m_point_cloud_rate_value);
	cloud_val << setprecision(6) << float(m_point_cloud_rate->GetValue())/kSampleRateResolution;

	m_error_slider->SetRange(1, kErrorResolution * kMaxError);
	m_error_slider->SetValue(int(0.5*kErrorResolution));
	m_error_value->Clear();
	ostream error_val(m_error_value);
	error_val << setprecision(6) << float(m_error_slider->GetValue())/(kErrorResolution);

	(*m_transition_length) << 0.25;
	(*m_fps_sample_rate) << 120.0;
	(*m_num_threads) << omp_get_max_threads();

	m_btn_create->Enable();
	m_btn_cancel->Disable();
	m_btn_pause->Disable();
	m_btn_next->Disable();
	m_btn_continue->Disable();

	m_stepping = false;
}

void mogedMotionGraphEditor::OnIdle( wxIdleEvent& event )
{
	ostream out(m_report);

	const ClipDB* clips = m_ctx->GetEntity()->GetClips();
	MotionGraph* graph = m_ctx->GetEntity()->GetMotionGraph();

	switch(m_current_state)
	{
	case StateType_ProcessNextClipPair:
		if(clips == 0) {
			out << "FATAL ERROR: clips has somehow become null!" << endl;
			m_current_state = StateType_TransitionsIdle;
			return;
		} 
	
		if( m_edge_pairs.empty() ) { 
			out << "No pairs left to process. " << endl;
			m_btn_create->Enable();
			m_btn_cancel->Disable();
			m_btn_pause->Disable();
			m_btn_next->Disable();
			m_btn_continue->Disable();
			m_current_state = StateType_TransitionsIdle;
		} else {
			const EdgePair &pair = m_edge_pairs.front();
			out << "Finding transitions from \"" << pair.first->GetClip()->GetName() << "\" to \"" <<
				pair.second->GetClip()->GetName() << "\". " << endl;
			
			CreateTransitionWorkListAndStart(graph);
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
			m_current_state = StateType_ProcessNextClipPair;
			if(m_stepping) {
				m_btn_create->Disable();
				m_btn_cancel->Enable();
				m_btn_pause->Disable();
				m_btn_next->Enable();
				m_btn_continue->Enable();
				m_stepping = false;
				m_current_state = StateType_TransitionsPaused;
			}
		}

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

	selectMotionGraphSampleVerts(m_ctx->GetEntity()->GetMesh(), num_points_in_cloud, m_working.sample_verts);

	out << "Starting with: " << endl
		<< "No. OMP Threads: " << m_settings.num_threads << endl
		<< "Error Threshold: " << m_settings.error_threshold << endl
		<< "Point Cloud Sample Rate: " << m_settings.point_cloud_rate << endl
		<< "Points in Cloud: " << num_points_in_cloud << endl
		<< "Sample verts collected: " << m_working.sample_verts.size() << endl
		<< "Transition Length: " << m_settings.transition_length << endl
		<< "FPS Sample Rate: " << m_settings.sample_rate << endl
		<< "Num Samples per Cloud: " << m_settings.num_samples << endl
		<< "Cloud Sample Interval: " << m_settings.sample_interval << endl;

	// Finally use clips DB to populate a new motiongraph.
	MotionGraph *graph = new MotionGraph;
	m_ctx->GetEntity()->SetMotionGraph(graph);
	populateInitialMotionGraph(graph, clips, out);

	m_working.num_clouds = graph->GetNumEdges();
	m_working.clouds = new Vec3*[ m_working.num_clouds ];
	memset(m_working.clouds, 0, sizeof(Vec3*)*m_working.num_clouds);
	m_working.cloud_lengths = new int[ m_working.num_clouds ];
	memset(m_working.cloud_lengths, 0, sizeof(int)*m_working.num_clouds);

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

	ostream out(m_report);
	out << "Cancelled." << endl;
	m_current_state = StateType_TransitionsIdle;
	m_ctx->GetEntity()->SetMotionGraph(0);
}

void mogedMotionGraphEditor::OnPause( wxCommandEvent& event )
{
	(void)event;

	m_btn_create->Disable();
	m_btn_cancel->Enable();
	m_btn_pause->Disable();
	m_btn_next->Enable();
	m_btn_continue->Enable();

	m_current_state = StateType_TransitionsPaused;
}

void mogedMotionGraphEditor::OnNext( wxCommandEvent& event )
{
	(void)event;
	if(m_current_state == StateType_TransitionsPaused)
	{
		m_btn_create->Disable();
		m_btn_cancel->Enable();
		m_btn_pause->Disable();
		m_btn_next->Disable();
		m_btn_continue->Disable();
		
		m_stepping = true;
		m_current_state = StateType_FindingTransitions;
	}
}

void mogedMotionGraphEditor::OnContinue( wxCommandEvent& event )
{
	(void)event;
	m_btn_create->Disable();
	m_btn_cancel->Enable();
	m_btn_pause->Enable();
	m_btn_next->Disable();
	m_btn_continue->Disable();

	m_current_state = StateType_FindingTransitions;
}

void mogedMotionGraphEditor::OnNextStage( wxCommandEvent& event )
{
	(void)event;
	// TODO: Implement OnNextStage
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
	: num_clouds(0), clouds(0), cloud_lengths(0)
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

	candidates.clear();
}

void mogedMotionGraphEditor::Settings::clear()
{
	num_threads = 0;
	error_threshold = 0.f;
	point_cloud_rate = 0.f;
	transition_length = 0.f;
	sample_rate = 0.f;
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
	
	m_settings.num_samples = int(transition_length * fps_rate);
	if(fps_rate > 0.f)
		m_settings.sample_interval = 1.f/fps_rate;
}

// create work list and start state machine
void mogedMotionGraphEditor::CreateWorkListAndStart(const MotionGraph* graph)
{
	m_edge_pairs.clear();

	int work_size = 0;
	const int num_edges = graph->GetNumEdges();
	for(int from = 0; from < num_edges; ++from)
	{
		const MGEdge* from_edge = graph->GetEdge(from);
		const int from_size = from_edge->GetClip()->GetNumFrames() - int(m_settings.transition_length * from_edge->GetClip()->GetClipFPS());
		for(int to = 0; to < num_edges; ++to) {
			const MGEdge* to_edge = graph->GetEdge(to);
			work_size += from_size * (to_edge->GetClip()->GetNumFrames() - int(m_settings.transition_length * to_edge->GetClip()->GetClipFPS()));
			m_edge_pairs.push_back( make_pair(from_edge,to_edge) );
		}
	}
	
	work_size += num_edges; // for clouds
	m_progress->SetRange(work_size);
	m_progress->SetValue(0);
	m_current_state = StateType_ProcessNextClipPair;
}
	
void mogedMotionGraphEditor::CreateTransitionWorkListAndStart(const MotionGraph* graph)
{
	EdgePair pair = m_edge_pairs.front();
	m_edge_pairs.pop_front();

	m_transition_finding.clear();
	m_transition_finding.from_idx = graph->IndexOfEdge(pair.first);
	m_transition_finding.to_idx = graph->IndexOfEdge(pair.second);
	m_transition_finding.from = pair.first;
	m_transition_finding.to = pair.second;	
	m_transition_finding.from_frame = 0;
	m_transition_finding.from_max = pair.first->GetClip()->GetNumFrames() - int(m_settings.transition_length * pair.first->GetClip()->GetClipFPS());
	m_transition_finding.to_max = pair.second->GetClip()->GetNumFrames() - int(m_settings.transition_length * pair.second->GetClip()->GetClipFPS());
	m_transition_finding.to_frame = 0;

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
		int num_samples = m_transition_finding.from->GetClip()->GetClipTime() * m_settings.sample_rate;	   
		int len = m_working.sample_verts.size() * num_samples;
		m_working.clouds[m_transition_finding.from_idx] = new Vec3[ len ];
		m_working.cloud_lengths[m_transition_finding.from_idx] = num_samples;

		getPointCloudSamples(m_working.clouds[m_transition_finding.from_idx],
							 mesh, 
							 skel, 
							 m_working.sample_verts,
							 m_transition_finding.from->GetClip(),
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
		int num_samples = m_transition_finding.to->GetClip()->GetClipTime() * m_settings.sample_rate;	   
		int len = m_working.sample_verts.size() * num_samples;
		m_working.clouds[m_transition_finding.to_idx] = new Vec3[ len ];
		m_working.cloud_lengths[m_transition_finding.to_idx] = num_samples;

		getPointCloudSamples(m_working.clouds[m_transition_finding.to_idx],
							 mesh, 
							 skel, 
							 m_working.sample_verts,
							 m_transition_finding.to->GetClip(),
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

				// don't go past the end of what we've allocated - just shorten the comparison. 
				from_end = Min(from_end, m_working.cloud_lengths[m_transition_finding.from_idx] - 1);
				to_end = Min(to_end, m_working.cloud_lengths[m_transition_finding.to_idx] - 1);

				int len = Min(from_end - from, to_end - to);

				const Vec3* from_cloud = &m_working.clouds[m_transition_finding.from_idx][ from * m_working.sample_verts.size() ];
				const Vec3* to_cloud = &m_working.clouds[m_transition_finding.to_idx][ to * m_working.sample_verts.size() ];

				Vec3 align_translation(0,0,0);
				float align_rotation = 0.f;

				computeCloudAlignment(from_cloud, to_cloud, m_working.sample_verts.size(), 
									  len, align_translation, align_rotation );

				PublishCloudData(true, align_translation, align_rotation);

				float difference = computeCloudDifference(from_cloud, to_cloud, m_working.sample_verts.size(), 
														  len, align_translation, align_rotation);

				if(difference < m_settings.error_threshold) {
					m_transition_finding.candidates.push_back( make_pair( from, to ) );
				}
				
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
		int num_hours = num_seconds/ (3600);
		int num_minutes = (num_seconds % (3600)) / 60;
		num_seconds = num_seconds % 60;

		if(num_hours > 0) 
			*m_time_left << num_hours << _("h ");
		if(num_minutes > 0)
			*m_time_left << num_minutes << _("m ");
		*m_time_left << num_seconds << _("s ");

	} else {
		*m_time_left << _("N/A") ;
	}
}

void mogedMotionGraphEditor::PublishCloudData(bool do_align, Vec3_arg align_translation, float align_rotation)
{
	Events::PublishCloudDataEvent ev;
	ev.SamplesPerFrame = m_working.sample_verts.size();
	ev.CloudA = m_working.clouds[m_transition_finding.from_idx];
	ev.CloudALen = m_working.cloud_lengths[m_transition_finding.from_idx];
	ev.CloudB = m_working.clouds[m_transition_finding.to_idx];
	ev.CloudBLen = m_working.cloud_lengths[m_transition_finding.to_idx];
	ev.AlignRotation = align_rotation;
	ev.AlignTranslation = align_translation;
	ev.Align = do_align ? 1 : 0;
	m_ctx->GetEventSystem()->Send(&ev);	
}
