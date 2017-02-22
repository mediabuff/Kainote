//  Copyright (c) 2016, Marcin Drob

//  Kainote is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  Kainote is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with Kainote.  If not, see <http://www.gnu.org/licenses/>.

#ifndef _KAI_FRAME_
#define _KAI_FRAME_


#include <wx/toplevel.h>
//#include <wx/frame.h>
#include <wx/sizer.h>


class KaiFrame : public wxTopLevelWindow
{
public:
	KaiFrame(wxWindow *parent, wxWindowID id, const wxString& title="", const wxPoint& pos=wxDefaultPosition, const wxSize& size=wxDefaultSize, long style=0);
	virtual ~KaiFrame();
	void SetLabel(const wxString &text);

private:
	void OnPaint(wxPaintEvent &evt);
	void OnSize(wxSizeEvent &evt);
	void OnMouseEvent(wxMouseEvent &evt);
	void OnActivate(wxActivateEvent &evt);
	WXLRESULT MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam);
	void DoGetClientSize(int *w, int *h);
	void OnEraseBackground(wxEraseEvent()){}
	
	long style;
	wxPoint diff;
	bool enterClose;
	bool pushedClose;
	bool enterMaximize;
	bool pushedMaximize;
	bool enterMinimize;
	bool pushedMinimize;
	bool isActive;
	wxDECLARE_ABSTRACT_CLASS(KaiFrame);
};

#endif