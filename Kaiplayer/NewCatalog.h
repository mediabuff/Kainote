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

#ifndef NEWCATALOG_H
#define NEWCATALOG_H

//(*Headers(NewCatalog)
#include "KaiTextCtrl.h"
#include "MappedButton.h"
#include "KaiDialog.h"
//*)

class NewCatalog: public KaiDialog
{
	public:

		NewCatalog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~NewCatalog();

		
		MappedButton* Button1;
		MappedButton* Button2;
		KaiTextCtrl* TextCtrl1;
		
};

#endif
