#include <ostream>
#include <cstdio>
#include <wx/wx.h>
#include "mogedDifferenceFunctionViewer.h"
#include "MathUtil.hh"

using namespace std;

mogedDifferenceFunctionViewer::mogedDifferenceFunctionViewer( wxWindow* parent,
															  int from_dim, int to_dim,
															  const float* error_values, 
															  float error_threshold,
															  const std::vector<int>& minima,
															  const char* from_name,
															  const char* to_name )
: DifferenceFunctionViewer( parent )
{
	m_info->Clear();
	ostream out(m_info);
	out << "Showing difference from " << from_name << " to " << to_name << " (" 
		<< from_dim << "x" << to_dim << ")";

	float global_min = 99999.f, global_max = -99999.f;
	const int total = from_dim * to_dim;
	for(int i = 0; i < total; ++i) {
		global_min = Min(global_min, error_values[i]);
		global_max = Max(global_max, error_values[i]);
	}

	// create a temporary image visualizing the error
	wxImage img(to_dim,from_dim);
	for(int y = 0; y < from_dim; ++y) {
		for(int x = 0; x < to_dim; ++x) {
			float error = error_values[x + y*to_dim];
			float scaled = (error - global_min) / (global_max - global_min);
			scaled = 1.f - Clamp(scaled,0.f,1.f);
			unsigned char intensity = Clamp(int(255 * (scaled * scaled)),0,255);
			if(error < error_threshold)
				img.SetRGB(x,y,intensity/2,intensity,intensity/2);
			else
				img.SetRGB(x,y,intensity,intensity,intensity);
		}
	}

	const int num_minima = minima.size();
	for(int i = 0; i < num_minima; ++i)
	{
		int index = minima[i];
		int x = index % to_dim;
		int y = index / to_dim;

		img.SetRGB(x,y,255,0,255);
	}

	m_bitmap = new wxBitmap(img);
	
}

mogedDifferenceFunctionViewer::~mogedDifferenceFunctionViewer()
{
	delete m_bitmap;
}

void mogedDifferenceFunctionViewer::OnPaintView( wxPaintEvent& event )
{
	(void)event;
	wxPaintDC dc(m_view_panel);

 	wxSize size = dc.GetSize();

 	int imgWidth = m_bitmap->GetWidth();
 	int imgHeight = m_bitmap->GetHeight();
 	float scale = 1.f;

	scale = size.GetWidth() / (float)imgWidth;
	imgWidth *= scale;
	imgHeight *= scale;

	float newScale = size.GetHeight() / (float)imgHeight;
	scale *= newScale;
	imgWidth *= newScale;
	imgHeight *= newScale;

	int x_offset = Max((size.GetWidth() - imgWidth) * 0.5f,0.f) / scale;
	int y_offset = Max((size.GetHeight() - imgHeight) * 0.5f,0.f) / scale;
	
	dc.SetUserScale(scale,scale);
	dc.DrawBitmap(*m_bitmap,x_offset,y_offset,false);
}

void mogedDifferenceFunctionViewer::OnCloseButton( wxCommandEvent& event ) 
{
	(void)event;
	EndModal(wxID_OK);
}
