#include <wx/filename.h>
#include <vector>
#include "mogedImportClipsDlg.h"
#include "appcontext.hh"
#include "acclaim.hh"
#include "util.hh"
#include "clip.hh"
#include "convert.hh"
#include "skeleton.hh"
#include "entity.hh"
#include "clipdb.hh"
#include "mogedevents.hh"

bool compatibleSkeleton( wxTextCtrl* report, const AcclaimFormat::Skeleton* ac_skel, const Skeleton* skel)
{
	if((int)ac_skel->bones.size() != skel->GetNumJoints())
	{
		(*report) << _("error: different number of joints.\n");
		return false;
	}
	
	int num_bones = ac_skel->bones.size();
	for(int i = 0; i < num_bones; ++i)
	{
		if(strcmp(ac_skel->bones[i]->name.c_str(), skel->GetJointName(i)) != 0)
		{
			(*report) << _("error: bone names do not match.\n");
			return false;
		}
	}
	return true;
}

mogedImportClipsDlg::mogedImportClipsDlg( wxWindow* parent , AppContext *ctx)
: ImportClipsDlg( parent )
, m_ctx(ctx)
{
	m_asf_file->SetPath( wxString(m_ctx->GetBaseFolder(), wxConvUTF8) );
	m_fps->SetValue(_("120.0"));
}

void mogedImportClipsDlg::OnAddClips( wxCommandEvent& event )
{
	// TODO: Implement OnAddClips
	wxFileDialog dlg(this, 
					 _("Open AMC Files"), 
					 wxString(m_ctx->GetBaseFolder(), wxConvUTF8),
					 _(""), 
					 _("Acclaim clips (*.amc)|*.amc"),
					 (wxOPEN|wxFILE_MUST_EXIST|wxMULTIPLE));
	
	
	if(dlg.ShowModal() == wxID_OK) {
		wxArrayString files ;
		dlg.GetPaths(files);
		
		for(int i = 0; i < (int)files.Count(); ++i)
		{
			SortedInsert( m_clip_list, files[i] );
		}
	}
}

void mogedImportClipsDlg::OnRemoveClips( wxCommandEvent& event )
{
	long item = -1;
	do 
	{
		item = m_clip_list->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
		if(item == -1) {
			break;
		} else {
			m_clip_list->DeleteItem(item);
		}
	} while(true);

}

void mogedImportClipsDlg::OnCancel( wxCommandEvent& event )
{
	EndModal(wxID_CANCEL);
}

void mogedImportClipsDlg::OnImport( wxCommandEvent& event )
{
	// TODO: have to put this on a diff thread to make the UI update correctly.
	float fps = atof(m_fps->GetValue().char_str());

	m_report->Clear();
	m_report->AppendText(_("Starting import...\n"));
	m_report->AppendText(_("Assuming FPS of "));
	(*m_report) << fps;
	m_report->AppendText(_("\n"));

	int num_clips = m_clip_list->GetItemCount();
	// setup gauge
	int num_steps = 1 + num_clips;
	m_gauge->SetRange(num_steps);
	m_gauge->SetValue(0);
	
	m_report->AppendText(_("Loading skeleton: "));
	wxString asfFile = m_asf_file->GetPath();
	m_report->AppendText(asfFile);
	m_report->AppendText(_("\n"));

	// load skeleton
	char* skeletonFile = loadFileAsString(asfFile.fn_str());
	if(skeletonFile == 0) {
		m_report->AppendText(_("error: failed to load skeleton file.\n"));
		return;
	}
	
	AcclaimFormat::Skeleton* ac_skel = AcclaimFormat::createSkeletonFromASF( skeletonFile );
	delete[] skeletonFile; skeletonFile = 0;
		
	if(ac_skel == 0) {
		m_report->AppendText(_("error: failed to parse skeleton file.\n"));
		return;
	}

	// check that the skeleton is compatible with the existing skeleton
	const Skeleton* current_skel = m_ctx->GetEntity()->GetSkeleton();
	if(current_skel) {
		if( !compatibleSkeleton(m_report, ac_skel, current_skel)) {
			(*m_report) << _("error: specified skeleton is not compatible with current entity. Did you pick the right one?\n");
			delete ac_skel;
			return;
		}
	} else {
		current_skel = convertToSkeleton( ac_skel );
		if(current_skel) {
			(*m_report) << _("Entity currently lacks a skeleton, importing specified skeleton and creating fresh clip DB...\n");
			m_ctx->GetEntity()->SetSkeleton( current_skel, new ClipDB );
			Events::EntitySkeletonChangedEvent ev;
			m_ctx->GetEventSystem()->Send( &ev );
		} else {
			(*m_report) << _("Failed to load skeleton file.");
		}
	}

	m_gauge->SetValue(m_gauge->GetValue()+1);

	// Now load all of the clips
	std::vector<Clip*> clips;
	std::vector<long> successful_items;
	long item = m_clip_list->GetNextItem(-1);
	while(item != -1)
	{	
		wxString clipFile = m_clip_list->GetItemText(item);
		m_report->AppendText(_("Processing clip file: "));
		m_report->AppendText(clipFile);
		m_report->AppendText(_(" ... "));

		char* fileBuffer = loadFileAsString(clipFile.fn_str());
		if(fileBuffer == 0) {
			m_report->AppendText(_(" file failure\n"));
		} else {
			AcclaimFormat::Clip* ac_clip = AcclaimFormat::createClipFromAMC( fileBuffer , ac_skel );
			delete[] fileBuffer;
			
			if(ac_clip == 0) {
				m_report->AppendText(_(" parse failure\n"));
			} else {
				Clip* clip = convertToClip( ac_clip , ac_skel, fps );
				delete ac_clip;

				wxString basename;
				wxFileName::SplitPath(clipFile, 0, 0, &basename, 0);
				clip->SetName(basename.char_str());
				
				clips.push_back(clip);
				m_report->AppendText(_(" ok\n"));

				successful_items.push_back(item);
			}
		}	

		m_gauge->SetValue(m_gauge->GetValue()+1);
		item = m_clip_list->GetNextItem(item);
	}

	delete ac_skel;	

	// remove items we already imported.
	for(int i = successful_items.size()-1; i >= 0; --i) {
		m_clip_list->DeleteItem(successful_items[i]);
	}

	// add clips to clip database
	int num_imported = clips.size();
	ClipDB* db = m_ctx->GetEntity()->GetClips();
	for(int i = 0; i < num_imported; ++i) 
	{
		Events::ClipAddedEvent ev;
		ev.ClipPtr = clips[i];
		m_ctx->GetEventSystem()->Send(&ev);
		db->AddClip(clips[i]);
	}

	(*m_report) << _("Done.");
}

void mogedImportClipsDlg::OnDone( wxCommandEvent& event )
{
	EndModal(wxID_OK);
}
