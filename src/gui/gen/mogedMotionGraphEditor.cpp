#include <cstdio>
#include <ostream>
#include <iomanip>
#include <algorithm>
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
#include "samplers/mesh_sampler.hh"

// TODO: major todo: this entire thing could be way more parallel. It would be
// cleaner than the state machine it is now. Instead of a state machine, yield
// at appropriate places or wait on a condition. one thread could be doing
// differences clouds and another could be doing actual transition stuff. It
// wouldn't be a crazy change but I don't really need it right now...

using namespace std;

static const int kSampleRateResolution = 10000;
static const int kMaxError = 500;
static const int kErrorResolution = 1000;
static const int kWeightFalloffResolution = 1000;

enum StateType{ 
	StateType_Idle = 0,
	StateType_ProcessNextClipPair,
	StateType_FindingTransitions,
	StateType_TransitionsPaused,	
	StateType_TransitionsStepPaused,
	StateType_SubdividingEdges,
	StateType_CreatingBlends,
	StateType_PruningGraph,
	StateType_VerifyGraph,
};

mogedMotionGraphEditor::mogedMotionGraphEditor( wxWindow* parent, AppContext* ctx )
:
MotionGraphEditor( parent )
, m_ctx(ctx)
, m_current_state(StateType_Idle)
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

	int count = CountMotionGraphs( m_ctx->GetEntity()->GetDB(), m_ctx->GetEntity()->GetSkeleton()->GetID());
	m_mg_name->Clear();
	*m_mg_name << _("MotionGraph ");
	*m_mg_name << count;

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
	case StateType_Idle:
		break;

	case StateType_ProcessNextClipPair:
		if(clips == 0) {
			out << "FATAL ERROR: clips has somehow become null!" << endl;
			m_current_state = StateType_Idle;
			return;
		} 
	
		if( m_clipPairs.empty() ) { 
			out << "No pairs left to process. " << endl;
			out << "Found a total of " << m_working.transition_candidates.size() << " suitable transition candidates." << endl
				<< "Subdividing graph edges..." << endl;

			m_progress->SetRange(m_working.split_list.size());
			m_progress->SetValue(0);
			m_working.cur_split = 0;
			m_current_state = StateType_SubdividingEdges;
		} else {
			CreateTransitionWorkListAndStart(clips, out);
		}
		break;

	case StateType_FindingTransitions:
		if(clips == 0) {
			out << "FATAL ERROR: clips has somehow become null!" << endl;
			m_current_state = StateType_Idle;
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

	case StateType_SubdividingEdges:
		if(!ProcessSplits()) {
			out << "Subdivided edges with new nodes, creating transition edges..." << endl;
			m_progress->SetRange(m_working.transition_candidates.size());
			m_progress->SetValue(0);
			m_current_state = StateType_CreatingBlends;
		}
		break;

	case StateType_CreatingBlends:
		if(m_ctx->GetEntity()->GetSkeleton() == 0) {
			out << "FATAL ERROR: skeleton became null!" << endl;
			m_current_state = StateType_Idle;
			return;
		}

		if(m_working.transition_candidates.empty()) {
			out << "Finished creating transition edges. " << endl;
			
			m_btn_create->Enable();
			m_btn_cancel->Disable();
			m_btn_pause->Disable();
			m_btn_next->Disable();
			m_btn_continue->Disable();
			m_btn_view_diff->Disable();
			
			out << "Done. Go to the graph pruning page to analyize and optimize the graph." << endl;
			{
				Events::MotionGraphChangedEvent ev;
				ev.MotionGraphID = m_ctx->GetEntity()->GetMotionGraph()->GetID();
				m_ctx->GetEventSystem()->Send(&ev);
			}
			m_current_state = StateType_Idle;
		}
		else {
			CreateBlendFromCandidate(out);
		}
		break;

	case StateType_PruningGraph:
	{
		ostream transition_out(m_transition_report);
		if(!PruneStep(transition_out)) {
			int num_deleted = 0;
			if( m_working.algo_graph->Commit(&num_deleted) ) {
				transition_out << "Graph pruning saved, " << num_deleted << " edges removed." << endl;
			} else {
				transition_out << "Failed to save graph pruning." << endl;
			}

			num_deleted = m_ctx->GetEntity()->GetMotionGraph()->RemoveRedundantNodes();
			transition_out << "Removed " << num_deleted << " redundant nodes." << endl;

			StartVerifyGraph(transition_out);
			m_current_state = StateType_VerifyGraph;
		}
		break;
	}

	case StateType_VerifyGraph:
	{
		ostream transition_out(m_transition_report);
		if(!VerifyGraphStep(transition_out)) {
			transition_out << "Done." << endl;

			{
				Events::MotionGraphChangedEvent ev;
				ev.MotionGraphID = m_ctx->GetEntity()->GetMotionGraph()->GetID();
				m_ctx->GetEventSystem()->Send(&ev);
			}

			m_current_state = StateType_Idle;
		}
		break;
	}

	case StateType_TransitionsStepPaused:
		break;
	case StateType_TransitionsPaused:
		break;	
	default:
		break;
	}
	event.Skip();
}

void mogedMotionGraphEditor::OnPageChanged( wxListbookEvent& event )
{
	(void)event;
	
	if(event.GetSelection() == 1) { // graph pruning page... blah
		m_prune_edit->Clear();
		m_ctx->GetEntity()->GetMotionGraphInfos( m_mg_infos, m_ctx->GetEntity()->GetSkeleton()->GetID() );
		const MotionGraph*graph = m_ctx->GetEntity()->GetMotionGraph();

		int sel = wxNOT_FOUND;
		const int count = m_mg_infos.size();
		for(int i = 0; i < count; ++i) {
			int result = m_prune_edit->Append(wxString( m_mg_infos[i].name.c_str(), wxConvUTF8), (void*)i);
			if( graph && m_mg_infos[i].id == graph->GetID() )
				sel = result;
		}
		m_prune_edit->SetSelection(sel);
	}	
}

void mogedMotionGraphEditor::OnPageChanging( wxListbookEvent& event )
{
	(void)event;

	// only switch if not doing anything
	if(m_current_state != StateType_Idle)
		event.Veto();
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
	ASSERT(skel); // we check for this in mainframe... 

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

	// InitCloudSampler() ... 

	const Mesh* mesh = m_ctx->GetEntity()->GetMesh();
	int num_points_in_cloud = 0; // to be filled in below
	ASSERT(!m_working.sampler);
	if(mesh)
	{
		// Use a Mesh cloud sampler
		num_points_in_cloud = mesh->GetNumVerts() * m_settings.point_cloud_rate;
		if(num_points_in_cloud <= 0) {
			wxMessageDialog dlg(this, _("Empty point cloud. Try increasing cloud sample percentage."), _("Error"), wxOK|wxICON_ERROR);
			dlg.ShowModal();
			return;
		}

		int max_point_cloud_size = atoi( m_max_point_cloud_size->GetValue().char_str());
		num_points_in_cloud = Min(max_point_cloud_size, num_points_in_cloud);

		MeshCloudSampler *meshSampler = new MeshCloudSampler;
		m_working.sampler = meshSampler;
		meshSampler->Init(num_points_in_cloud, skel, mesh, m_settings.num_threads, m_settings.sample_interval);
	}
	else
	{
		// We have no mesh, so sample points on the skeleton.

		ASSERT(false && "to be implemented");
		// TODO m_working.sampler = new SkeletonCloudSampler;...
	}

	out << "Starting with: " << endl
		<< "No. OMP Threads: " << m_settings.num_threads << endl
		<< "Maximum Error Threshold: " << m_settings.error_threshold << endl
		<< "Point Cloud Sample Rate: " << m_settings.point_cloud_rate << endl
		<< "Points in Cloud: " << num_points_in_cloud << endl
		<< "Cloud Samples Per Frame: " << m_working.sampler->GetSamplesPerFrame() << endl
		<< "Transition Length: " << m_settings.transition_length << endl
		<< "FPS Sample Rate: " << m_settings.sample_rate << endl
		<< "Transition Samples: " << m_settings.num_samples << endl
		<< "Cloud Sample Interval: " << m_settings.sample_interval << endl
		<< "Falloff is " << m_settings.weight_falloff << endl;

	// Finally use clips DB to populate a new motiongraph.
	wxString mgName = m_mg_name->GetValue();
	if(mgName.Len() == 0) mgName = _("noname motion graph");
	sqlite3_int64 graph_id = NewMotionGraph(m_ctx->GetEntity()->GetDB(), skel->GetID(), mgName.char_str());
	m_ctx->GetEntity()->SetCurrentMotionGraph( graph_id );
	MotionGraph *graph = m_ctx->GetEntity()->GetMotionGraph();
	if(graph == 0) {
		wxMessageDialog dlg(this, _("Failed to create new motion graph."), _("Error"), wxOK|wxICON_ERROR);
		dlg.ShowModal();
		return;
	}
	
	PopulateInitialMotionGraph(graph, clips, out);

	// TODO: below should probably be in a func: AllocatePointCloudStorage() ...

	m_working.num_clouds = graph->GetNumEdges(); // one cloud array per clip
	m_working.clouds = new Vec3*[ m_working.num_clouds ];
	m_working.cloud_lengths = new int[ m_working.num_clouds ];
	memset(m_working.clouds, 0, sizeof(Vec3*)*m_working.num_clouds);
	memset(m_working.cloud_lengths, 0, sizeof(int)*m_working.num_clouds);

	omp_set_num_threads(m_settings.num_threads);

	InitJointWeights();
	out << "Inverse sum of weights is " << m_working.inv_sum_weights << endl;

	CreateWorkListAndStart();
}

void mogedMotionGraphEditor::OnCancel( wxCommandEvent& event )
{
	(void)event;

	wxMessageDialog dlg(this, _("Are you sure you want to cancel? This will undo any progress."), 
		_("Confirm"), wxYES_NO|wxICON_QUESTION);
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
	m_current_state = StateType_Idle;
	
	if(m_ctx->GetEntity()->GetMotionGraph()) {
		sqlite3_int64 mg_id = m_ctx->GetEntity()->GetMotionGraph()->GetID();
		m_ctx->GetEntity()->SetCurrentMotionGraph(0);
		m_ctx->GetEntity()->DeleteMotionGraph(mg_id);
	}
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

void mogedMotionGraphEditor::OnViewDistanceFunction( wxCommandEvent& event )
{
	(void)event;
	if((m_current_state == StateType_TransitionsStepPaused ||
		m_current_state == StateType_TransitionsPaused) 
        && !m_transition_finding.fromClip.Null() && !m_transition_finding.toClip.Null())
	{
		ASSERT(m_transition_finding.error_function_values);

		const int dim_y = m_transition_finding.from_max;
		const int dim_x = m_transition_finding.to_max;
		mogedDifferenceFunctionViewer dlg(this, dim_y, dim_x, m_transition_finding.error_function_values,
										  m_transition_finding.current_error_threshold,
										  m_transition_finding.minima_indices,
										  m_transition_finding.fromClip->GetName(),
										  m_transition_finding.toClip->GetName());
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

void mogedMotionGraphEditor::OnPruneGraph( wxCommandEvent& event ) 
{ 
	(void)event;

	m_transition_report->Clear();
	ostream out(m_transition_report);

	int sel_idx = m_prune_edit->GetSelection();
	if(sel_idx == wxNOT_FOUND) 
	{
		out << "no graph selected." << endl;
		return;
	}

	int info_idx = (int)(size_t)m_prune_edit->GetClientData(sel_idx);
	if(info_idx >= (int)m_mg_infos.size())
	{
		ASSERT(false);
		out << "fatal error, bad index for motion graph info" << endl;
		return;
	}

	sqlite3_int64 graph_id = m_mg_infos[info_idx].id;

	m_ctx->GetEntity()->SetCurrentMotionGraph(graph_id);
	MotionGraph* graph = m_ctx->GetEntity()->GetMotionGraph();
	if(graph == 0) {
		out << "Fatal error: could not set motion graph to selected graph." << endl;
		return;
	}
	
	out << "Pruning graph ... " << endl;
	m_working.algo_graph = graph->GetAlgorithmGraph();
	m_working.algo_graph->InitializePruning();

	m_working.cur_prune_item = 0;
	m_working.graph_pruning_queue.clear();
	m_working.graph_pruning_queue.push_back(PruneWorkItem(0, "Main"));

	Query get_annos(m_ctx->GetEntity()->GetDB(), "SELECT id,name FROM annotations");
	// TODO: add a work item for each unique set of annotations
	while(get_annos.Step()) {
		m_working.graph_pruning_queue.push_back(PruneWorkItem(get_annos.ColInt64(0),
															  get_annos.ColText(1)));
	}

	m_prune_progress->SetRange(m_working.graph_pruning_queue.size());
	m_prune_progress->SetValue(0);

	out << "Starting..." << endl;
	m_current_state = StateType_PruningGraph;
}

void mogedMotionGraphEditor::OnExportGraphViz( wxCommandEvent& event ) 
{
	(void)event;

	int sel_idx = m_prune_edit->GetSelection();
	if(sel_idx == wxNOT_FOUND) 
		return;

	int info_idx = (int)(size_t)m_prune_edit->GetClientData(sel_idx);
	if(info_idx >= (int)m_mg_infos.size())
	{
		ASSERT(false);
		return;
	}

	sqlite3_int64 graph_id = m_mg_infos[info_idx].id;


	wxString defaultFile = wxString(m_mg_infos[info_idx].name.c_str(),wxConvUTF8) + _(".dot");
	wxString file = wxFileSelector(_("Export GraphViz file"), 
								   wxString(m_ctx->GetBaseFolder(), wxConvUTF8),
								   defaultFile, _(".dot"), _("GraphViz (*.dot)|*.dot"),
								   wxFD_SAVE);
	if( !file.empty() )
	{
		if(!exportMotionGraphToGraphViz( m_ctx->GetEntity()->GetDB(),
										 graph_id, file.char_str())) {
			wxMessageDialog dlg(this, _("Failed to export!"), _("Error :("), wxID_OK|wxICON_HAND);
			dlg.ShowModal();
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
mogedMotionGraphEditor::TransitionWorkingData::TransitionWorkingData()
	: sampler(0), num_clouds(0), clouds(0), cloud_lengths(0), joint_weights(0)
{
	clear();
}

void mogedMotionGraphEditor::TransitionWorkingData::clear()
{
	delete sampler; sampler = 0;

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
	split_list.clear();
	cur_split = 0;
	
	working_set.clear();
    initial_edges.clear();

	algo_graph = AlgorithmMotionGraphHandle();

	graph_pruning_queue.clear();
	cur_prune_item = 0;
}

////////////////////////////////////////////////////////////////////////////////
mogedMotionGraphEditor::TransitionFindingData::TransitionFindingData()
	: error_function_values(0), alignment_translations(0), alignment_angles(0)
{ 
	clear() ;
}

void mogedMotionGraphEditor::TransitionFindingData::clear()
{
	from_idx = 0;
	to_idx = 0;
	fromClip = ClipHandle();
	toClip = ClipHandle();
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
	m_settings.weight_falloff = Clamp(weight_falloff, 0.f, 1.f);
	
	m_settings.num_samples = int(transition_length * fps_rate);
	if(fps_rate > 0.f)
		m_settings.sample_interval = 1.f/fps_rate;
}

void mogedMotionGraphEditor::CreateWorkListAndStart()
{
	m_clipPairs.clear();
    const int numClips = m_working.working_set.size();

	// now generate pairs from this list - as a side effect this will pre-cache all of the clips.
	int work_size = 0;
	for(int from = 0; from < numClips; ++from)
	{
		ClipHandle fromClip = m_working.working_set[from];
		const int from_size = Max(0,int(fromClip->GetClipTime() * m_settings.sample_rate) - m_settings.num_samples);
		for(int to = 0; to < numClips; ++to) {
			ClipHandle toClip = m_working.working_set[to];
			const int to_size = Max(0,int(toClip->GetClipTime() * m_settings.sample_rate) - m_settings.num_samples);
			work_size += from_size * to_size; // for progress bar - work size inc for each comparison
			m_clipPairs.push_back( make_pair(from,to) );
		}
	}

	// create empty buckets for splitting later
	m_working.split_list.clear();
	m_working.split_list.resize(numClips);
	
    // progress bar stuff
	work_size += numClips; // increase work size by one per clip (for splitting?)
	m_progress->SetRange(work_size);
	m_progress->SetValue(0);
	m_current_state = StateType_ProcessNextClipPair;
}
	
void mogedMotionGraphEditor::CreateTransitionWorkListAndStart(const ClipDB * clips, ostream& out)
{
	ClipPair pair = m_clipPairs.front();
	m_clipPairs.pop_front();

	ClipHandle fromClip = m_working.working_set[pair.first];
	ClipHandle toClip = m_working.working_set[pair.second];

	out << "Finding transitions from \"" << fromClip->GetName() << "\" to \"" <<
		toClip->GetName() << "\". " << endl;			

	m_transition_finding.clear();
	m_transition_finding.from_idx = pair.first;
	m_transition_finding.to_idx = pair.second;
	m_transition_finding.fromClip = fromClip;
	m_transition_finding.toClip = toClip;	
	m_transition_finding.from_frame = 0;

	m_transition_finding.from_max = Max(0,int(fromClip->GetClipTime() * m_settings.sample_rate) - m_settings.num_samples);
	m_transition_finding.to_max = Max(0,int(toClip->GetClipTime() * m_settings.sample_rate) - m_settings.num_samples);
	m_transition_finding.to_frame = 0;

	if(m_transition_finding.from_max == 0)
	{
		out << "Discarding pair, \"" << fromClip->GetName() << "\" isn't long enough to have a transition of length " << m_settings.transition_length << endl;
		m_transition_finding.clear();
		return;
	}
	else if(m_transition_finding.to_max == 0)
	{
		out << "Discarding pair, \"" << toClip->GetName() << "\" isn't long enough to have a transition of length " << m_settings.transition_length << endl;
		m_transition_finding.clear();
		return;
	}

	std::vector< Annotation > from_clip_annotations, to_clip_annotations;
	clips->GetAnnotations(from_clip_annotations, fromClip->GetID());
	clips->GetAnnotations(to_clip_annotations, toClip->GetID());
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
	const int samplesPerFrame = m_working.sampler->GetSamplesPerFrame();

	ASSERT(m_working.clouds && m_working.num_clouds > 0 && m_working.cloud_lengths);

	int num_processed = 0;
	double start_time = omp_get_wtime();

	// always do at least one thing every time this function is called, 
	// and don't take longer than kMaxTime. This is enforced through the time_so_far && num_processed checks.

	// Allocate missing point clouds lazily to avoid resampling.
	if(m_working.clouds[m_transition_finding.from_idx] == 0)
	{
		int num_frames = Max(1,
			int(m_transition_finding.fromClip->GetClipTime() * m_settings.sample_rate));
		int len = samplesPerFrame * num_frames;
		m_working.clouds[m_transition_finding.from_idx] = new Vec3[ len ];
		m_working.cloud_lengths[m_transition_finding.from_idx] = num_frames;

		m_working.sampler->GetSamples(m_working.clouds[m_transition_finding.from_idx], len, 
			m_transition_finding.fromClip.RawPtr(), num_frames);
		
		PublishCloudData(false, Vec3(), 0);
		++num_processed;
	}

	double time_so_far = omp_get_wtime() - start_time;
	if( (time_so_far < kMaxTime || num_processed == 0) &&
		m_working.clouds[m_transition_finding.to_idx] == 0)
	{
		int num_frames = Max(1,
			int(m_transition_finding.toClip->GetClipTime() * m_settings.sample_rate));
		int len = samplesPerFrame * num_frames;
		m_working.clouds[m_transition_finding.to_idx] = new Vec3[ len ];
		m_working.cloud_lengths[m_transition_finding.to_idx] = num_frames;

		m_working.sampler->GetSamples(m_working.clouds[m_transition_finding.to_idx], len,
			m_transition_finding.toClip.RawPtr(), num_frames);
		
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

				int from_cloud_offset = from * samplesPerFrame;
				int to_cloud_offset = to * samplesPerFrame;

				const Vec3* from_cloud = &m_working.clouds[m_transition_finding.from_idx][ from_cloud_offset ];
				const Vec3* to_cloud = &m_working.clouds[m_transition_finding.to_idx][ to_cloud_offset ];

				ASSERT(from_cloud + len * samplesPerFrame <= 
					   m_working.clouds[m_transition_finding.from_idx] + from_cloud_len * samplesPerFrame);
				ASSERT(to_cloud + len * samplesPerFrame <= 
					   m_working.clouds[m_transition_finding.to_idx] + to_cloud_len * samplesPerFrame);

				Vec3 align_translation(0,0,0);
				float align_rotation = 0.f;

				computeCloudAlignment(from_cloud, to_cloud, 
									  samplesPerFrame, 
									  len, 
									  m_working.joint_weights, 
									  m_working.inv_sum_weights, 
									  align_translation, 
									  align_rotation,
									  m_settings.num_threads);

				float difference = computeCloudDifference(from_cloud, to_cloud, 
														  m_working.joint_weights, 
														  samplesPerFrame, 
														  len, 
														  align_translation, align_rotation,
														  m_settings.num_threads);							   
				
				if(difference < m_transition_finding.current_error_threshold)
					PublishCloudData(true, align_translation, align_rotation, 
									 from_cloud_offset, len, to_cloud_offset, len);

                int transIndex = from * num_to + to;
				m_transition_finding.error_function_values[transIndex] = difference;
				m_transition_finding.alignment_translations[transIndex] = align_translation;
				m_transition_finding.alignment_angles[transIndex] = align_rotation;
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
	ev.SamplesPerFrame = m_working.sampler->GetSamplesPerFrame();
	ev.CloudA = &m_working.clouds[m_transition_finding.from_idx][from_offset];
	ev.CloudALen = from_len > 0 ? from_len : m_working.cloud_lengths[m_transition_finding.from_idx];
	ev.CloudB = &m_working.clouds[m_transition_finding.to_idx][to_offset];
	ev.CloudBLen = to_len > 0 ? to_len : m_working.cloud_lengths[m_transition_finding.to_idx];
	ev.AlignRotation = align_rotation;
	ev.AlignTranslation = align_translation;
	ev.Align = do_align ? 1 : 0;
	m_ctx->GetEventSystem()->Send(&ev);	
}

void mogedMotionGraphEditor::InitJointWeights()
{
	const int num_frames = m_settings.num_samples;
	const int samples_per_frame = m_working.sampler->GetSamplesPerFrame();
	const int numWeights = num_frames * samples_per_frame;

	// allocate a sample for every point in the cloud.
	m_working.joint_weights = new float[numWeights];
	if(numWeights == 0) return;
	memset(m_working.joint_weights, 0, sizeof(float)*numWeights);

	float *out_weights = m_working.joint_weights;

	// Get the weights for the first frame so we can compute falloff weights.
	// these weights are related to the user set weights for importance of joints.
	m_working.sampler->GetSampleWeights(out_weights);

	int out_idx = samples_per_frame;
	for(int frame = 1; frame < num_frames; ++frame)
	{
		float frac = float(frame) / float(num_frames);
		for(int i = 0; i < samples_per_frame; ++i) {
			// compute falloff start and end weights
			float startw = out_weights[i];
			float endw = startw * m_settings.weight_falloff;

			// use frac to determine how much of the fall off we use and taper off as we near the 
			// end of the comparison window
			out_weights[out_idx++] = (1.0 - frac) * startw + frac * endw;
		}
	}

	// compute 1/(sum of all weights) for use later
	double sum = 0.0;
	for(int i = 0; i < numWeights; ++i)
		sum += out_weights[i];

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
		m_weight_falloff->SetValue(0.75 * kWeightFalloffResolution);
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
				ClipHandle from_clip = m_working.working_set[ m_transition_finding.from_idx ];
				ClipHandle to_clip = m_working.working_set[ m_transition_finding.to_idx];
				
				TransitionCandidate c;
				c.from_clip = from_clip;				
				c.from_frame = from_frame ;
				c.from_time = (from_frame) * m_settings.sample_interval;
				c.from_insert_point = int(c.from_time * from_clip->GetClipFPS()) ;
				
				// this transition sends us to to_clip @ to_time + sample_interval * (m_settings.num_samples-1),
				// since we FINISH the transition on that frame.
				c.to_clip = to_clip;
				c.to_frame = to_frame ;
				c.to_time = (to_frame) * m_settings.sample_interval;
				c.to_insert_point = int( (c.to_time + m_settings.sample_interval * (m_settings.num_samples-1)) * to_clip->GetClipFPS() ) ;

				c.align_translation = m_transition_finding.alignment_translations[index];
				c.align_rotation = m_transition_finding.alignment_angles[index];
				m_working.transition_candidates.push_back(c);

                // Queue edge splits for the given clips at the transition
                // frames. Splits happen in order because it's easy to keep
                // track of new edge ids (and which edges to split,
                // subsequently)
				m_working.split_list[ m_transition_finding.from_idx ].push_back(c.from_insert_point);
				m_working.split_list[ m_transition_finding.to_idx].push_back(c.to_insert_point);
			}
		}
	}	
}

bool mogedMotionGraphEditor::ProcessSplits()
{
    // Check to see if we are finished
	if(m_working.cur_split >= (int)m_working.working_set.size()) {
		return false;
	}

    // Process each split, subdividing the existing edge for an original clip at the list of frames
    // in the split_list for that clip. 
	std::vector< int >& splits = m_working.split_list[ m_working.cur_split ];
	sqlite3_int64 curEdgeId = m_working.initial_edges[m_working.cur_split];
	const int num_frames = m_working.working_set[ m_working.cur_split ]->GetNumFrames();
	++m_working.cur_split;

	MotionGraph* graph = m_ctx->GetEntity()->GetMotionGraph();

	// must be in order to make sure edges are connected properly.
	std::sort(splits.begin(), splits.end());
	
	SavePoint(m_ctx->GetEntity()->GetDB(), "processSplits");
	int lastSplit = -1;
	const int count = splits.size();
	for(int i = 0; i < count; ++i) {
		if(lastSplit != splits[i]) { 
			if(splits[i] > 0 && splits[i] < num_frames-1) { // these nodes already exist, so don't bother
			    sqlite3_int64 new_id = 0;
				if(0 != graph->SplitEdge( curEdgeId, splits[i], 0, &new_id)) {
					curEdgeId = new_id;
				}
			}
			lastSplit = splits[i];
		}
	}

	m_progress->SetValue( m_progress->GetValue() + 1);
	return true;
}

void mogedMotionGraphEditor::CreateBlendFromCandidate(ostream& out)
{
	TransitionCandidate candidate = m_working.transition_candidates.front();
	m_working.transition_candidates.pop_front();
	m_progress->SetValue( m_progress->GetValue() + 1 ); 

	SavePoint save( m_ctx->GetEntity()->GetDB(), "addTransitionEdge");

	MotionGraph* graph = m_ctx->GetEntity()->GetMotionGraph();

	int original_clip_frame_from = candidate.from_insert_point;
	int original_clip_frame_to = candidate.to_insert_point;

	if(original_clip_frame_to >= candidate.to_clip->GetNumFrames()) {
		int old = original_clip_frame_to ;
		original_clip_frame_to = Clamp(original_clip_frame_to, 0, candidate.to_clip->GetNumFrames() - 1);
		fprintf(stderr, "Warning: transition to clip is beyond the to clip frame count. Clamping %d to %d...\n",
				old, original_clip_frame_to);
	}

    // Find the already split nodes. This should have taken place in ProcessSplits
	sqlite3_int64 transition_from_node = graph->FindNode(candidate.from_clip->GetID(), original_clip_frame_from);
	if(transition_from_node == 0) { 
			out << "Failed to find from node." << endl;
			save.Rollback(); 
            return; 
	}
		
	sqlite3_int64 transition_to_node = graph->FindNode(candidate.to_clip->GetID(), original_clip_frame_to);
	if(transition_to_node == 0) {
			out << "Failed to find to node." << endl;
			save.Rollback(); 
            return; 
	}

    // TODO: this is kind of lame - are these variables being used elsewhere in
    // a way that might conflict? should just be m-settings.blendTime
    float blendTime = m_settings.num_samples * m_settings.sample_interval;

	Quaternion align_rotation = make_rotation(candidate.align_rotation, Vec3(0,1,0));
	sqlite3_int64 transition_edge_id = graph->AddTransitionEdge(transition_from_node,
																transition_to_node,
                                                                blendTime,
																candidate.align_translation,
																align_rotation);
	if(transition_edge_id == 0) {  
		out << "Failed to add transition edge." << endl;
		save.Rollback(); return; 
	}
}

std::vector<int>* GetLargestSCC( AlgorithmMotionGraph::SCCList& sccs )
{
	std::vector<int>* largest_scc = 0;
	int best_size = 0;
	AlgorithmMotionGraph::SCCList::iterator cur = sccs.begin(), end = sccs.end();
	while(cur != end) {
		if((int)cur->size() > best_size) {
			best_size = cur->size();
			largest_scc = &(*cur);
		}
		++cur;
	}	
	return largest_scc;
}

bool mogedMotionGraphEditor::PruneStep(ostream& out)
{
	if(m_working.cur_prune_item >= (int)m_working.graph_pruning_queue.size()) return false;
	PruneWorkItem &workItem = m_working.graph_pruning_queue[m_working.cur_prune_item];
	int set_num = m_working.cur_prune_item++;

	AlgorithmMotionGraph::SCCList sccs;	
	m_working.algo_graph->ComputeStronglyConnectedComponents( sccs , workItem.anno );

	if(sccs.empty()) {
		m_prune_progress->SetValue( m_prune_progress->GetValue() + 1 );
		return true;
	}

	std::vector<int>* largest_scc = GetLargestSCC( sccs );
	out << (workItem.anno == 0 ? "Graph " : "Subgraph ") 
		<< workItem.name << ": Largest SCC has " << largest_scc->size() << " nodes." << endl;

	m_working.algo_graph->MarkSetNum( set_num, workItem.anno, *largest_scc);

	m_prune_progress->SetValue( m_prune_progress->GetValue() + 1 );
	return true;
}

void mogedMotionGraphEditor::StartVerifyGraph(std::ostream& out)
{
	out << "Verifying graph..." << endl;
	// need a new read of the algo graph because we just did a commit on the last one
	m_working.algo_graph = m_ctx->GetEntity()->GetMotionGraph()->GetAlgorithmGraph();

	// re-use the pruning queue since it is basically the same thing, but needed to be finished
	// before we can verify.
	m_working.cur_prune_item = 0;

	m_prune_progress->SetRange(m_working.graph_pruning_queue.size());
	m_prune_progress->SetValue( 0 );
}

bool mogedMotionGraphEditor::VerifyGraphStep(std::ostream& out)
{
	if(m_working.cur_prune_item >= (int)m_working.graph_pruning_queue.size()) return false;
	PruneWorkItem &workItem = m_working.graph_pruning_queue[m_working.cur_prune_item];
	++m_working.cur_prune_item;

	AlgorithmMotionGraph::Node* node = m_working.algo_graph->FindNodeWithAnno( workItem.anno );
	if(node) {
		float total_clip_time = m_ctx->GetEntity()->GetMotionGraph()->CountClipTimeWithAnno( workItem.anno );
		out << "Subgraph " << workItem.name << " has " << total_clip_time << "s of clip time." << endl;

		const int num_sets = m_working.cur_prune_item;
		for(int i = 0; i < num_sets; ++i) {
			if(i != m_working.cur_prune_item) {
				PruneWorkItem& otherItem = m_working.graph_pruning_queue[i];
				if(!m_working.algo_graph->CanReachNodeWithAnno( node, otherItem.anno)) {
					out << "Warning: no way to get from subgraph " << workItem.name << " to subgraph " << 
						otherItem.name << endl;
				}
			}
		}
	} else {
		out << "Subgraph " << workItem.name << " seems to have no nodes!" << endl;
	}

	m_prune_progress->SetValue( m_prune_progress->GetValue() + 1 );
	return true;
}

void mogedMotionGraphEditor::PopulateInitialMotionGraph(MotionGraph* graph, 
	const ClipDB* clips,
	std::ostream& out)
{
    // Load all clips into the working set
    std::vector<sqlite3_int64> clipIds;
    clips->GetClipIDs(clipIds);

    const int numClips = (int)clipIds.size();
    m_working.working_set.clear();
	m_working.working_set.resize(numClips);
    for(int i = 0; i < numClips; ++i) {
        m_working.working_set[i] = clips->GetClip( clipIds[i] );
    }

    m_working.initial_edges.clear();
    m_working.initial_edges.resize(numClips, 0);
	for(int i = 0; i < numClips; ++i)
	{
        ClipHandle clip = m_working.working_set[i];
		sqlite3_int64 start = graph->AddNode(clip->GetID() , 0 );
		sqlite3_int64 end = graph->AddNode( clip->GetID(), clip->GetNumFrames() - 1);
		m_working.initial_edges[i] = graph->AddEdge( start, end );
	}
	
	out << "Using " << numClips << " clips." << endl <<
		"Created graph with " << graph->GetNumEdges() << " edges and " << graph->GetNumNodes() << " nodes." << endl;
}


