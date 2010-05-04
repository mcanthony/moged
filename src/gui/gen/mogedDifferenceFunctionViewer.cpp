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
															  const char* from_name,
															  const char* to_name )
: DifferenceFunctionViewer( parent )
{
	m_info->Clear();
	ostream out(m_info);
	out << "Showing difference from " << from_name << " to " << to_name << " (" 
		<< from_dim << "x" << to_dim << ")";

	error_threshold *= 5.f;

	// create a temporary image visualizing the error
	wxImage img(to_dim,from_dim);
	for(int y = 0; y < from_dim; ++y) {
		for(int x = 0; x < to_dim; ++x) {
			float error = error_values[x + y*to_dim];
			float scaled = (error_threshold - error) / error_threshold;
			unsigned char intensity = (unsigned char)Clamp(scaled > 0 ? int(255*scaled) : 0,0,255);
			img.SetRGB(x,y,intensity,intensity,intensity);
		}
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
