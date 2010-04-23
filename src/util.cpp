#include <cstdio>
#include <wx/wx.h>
#include "util.hh"

using namespace std;

char* loadFileAsString(const char* filename)
{
	FILE* fp = fopen(filename, "r");
	if(fp == 0) 
		return 0;
	
	fseek(fp,0,SEEK_END);
	long size = ftell(fp);
	rewind(fp);

	char* buffer = new char[size+1]; // room for null term
	size_t num_read = fread(buffer, 1, size, fp);
	if(num_read != (size_t)size) {
		delete[] buffer;
		fclose(fp);
		return 0;
	}

	buffer[size] = '\0';
	
	fclose(fp);
	return buffer;
}

bool SortedInsert( wxListCtrl* list, wxString const& item )
{
	// insert at the first item that is 'less' than mine
	// for now use strcmp()... ideally a natural compare would be better

	int num = list->GetItemCount();
	int i = 0;
	for(; i < num; ++i) {
		wxString text = list->GetItemText(i);
		int result = item.Cmp(text);
		if(result == 0) {
			return false;
		} else if(result < 0) {
			list->InsertItem(i, item);
			return true;
		}
	}
	list->InsertItem(i, item);
	return true;
}


