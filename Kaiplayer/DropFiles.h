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

#pragma once

#include <wx/dnd.h>
#include <wx/wxprec.h>
class kainoteFrame;

class DragnDrop : public wxFileDropTarget
{
    private:
    kainoteFrame* Kai;
    public:
    DragnDrop(kainoteFrame* kfparent);
	virtual ~DragnDrop(){ };
    bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames);

};

//class DragScripts : public wxFileDropTarget
//	{
//	private:
//		kainoteFrame* Kai;
//
//	public:
//		DragScripts(kainoteFrame* kfparent);
//		virtual ~DragScripts(){ };
//		bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames);
//	};


