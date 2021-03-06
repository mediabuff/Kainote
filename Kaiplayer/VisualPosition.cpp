﻿//  Copyright (c) 2016, Marcin Drob

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


#include "Visuals.h"
#include "TabPanel.h"
#include <wx/regex.h>

Position::Position()
	: Visuals()
{
}


void Position::Draw(int time)
{
	wxMutexLocker lock(clipmutex);
	line->SetAntialias(TRUE);
	line->SetWidth(2.0);
	bool nothintoshow=true;
	for(size_t i = 0; i < data.size(); i++){
		auto pos = data[i];
		//pamiętaj sprawdzanie czy czasy mieszczą się w przedziale to czas >= start && czas < koniec
		if(time >= pos.dial->Start.mstime && time < pos.dial->End.mstime){
			DrawCross(pos.pos);
			DrawRect(pos.pos);
			nothintoshow=false;
		}
	}
	
	line->SetAntialias(FALSE);
	oldtime=time;
	if (nothintoshow){ DrawWarning(notDialogue); blockevents = true; }
	else if (blockevents){ blockevents = false; }
}

void Position::OnMouseEvent(wxMouseEvent &evt)
{
	
	if(blockevents){return;}
	bool click = evt.LeftDown();
	bool holding = evt.LeftIsDown();
	
	int x, y;
	evt.GetPosition(&x,&y);
	
	if(evt.RightDown() || evt.LeftDClick()){
		for(size_t i = 0; i < data.size(); i++){
			if(data[i].numpos == tab->Edit->ebrow){
				data[i].pos.x = x;
				data[i].pos.y = y;
				data[i].lastpos = data[i].pos;
				D3DXVECTOR2 diff(data[i].pos.x - data[i].lastpos.x, data[i].pos.y - data[i].lastpos.y);

				for(size_t j = 0; j < data.size(); j++){
					if(j==i){continue;}
					data[j].pos += diff;
					data[j].lastpos = data[j].pos;
				}
				ChangeMultiline(evt.RightDown());
				break;
			}

		}
		return;
	}

	if(evt.LeftUp()){
		if(tab->Video->HasCapture()){tab->Video->ReleaseMouse();}
		ChangeMultiline(true);
		for(size_t i=0; i < data.size(); i++ ){
			data[i].lastpos = data[i].pos;
		}
		if(!hasArrow){tab->Video->SetCursor(wxCURSOR_ARROW);hasArrow=true;}
	}

	if(click){
		if (!tab->Video->HasCapture()){ tab->Video->CaptureMouse(); }
		tab->Video->SetCursor(wxCURSOR_SIZING);
		hasArrow=false;
		wxArrayInt sels;
		tab->Grid->file->GetSelections(sels);
		if(sels.size()!=data.size()){SetCurVisual();tab->Video->Render();}
		firstmove.x=x;
		firstmove.y=y;
		axis=0;
	}else if(holding){
		for(size_t i=0; i < data.size(); i++ ){
			data[i].pos.x = data[i].lastpos.x - (firstmove.x-x);
			data[i].pos.y = data[i].lastpos.y - (firstmove.y-y);
			if(evt.ShiftDown()){
				//wxLogStatus("diff %i, %i", (int)(firstmove.x-x), (int)(firstmove.y-y));
				//if(axis == 0){
					int diffx = abs(firstmove.x-x);
					int diffy = abs(firstmove.y-y);
					if(diffx != diffy){if(diffx > diffy){axis = 2;}else{axis = 1;}}
				//}
				if(axis==1){
					data[i].pos.x = data[i].lastpos.x;
				}else if(axis==2){
					data[i].pos.y = data[i].lastpos.y;
				}
			}
		}
		ChangeMultiline(false);
	}
}
	

wxString Position::GetVisual(int datapos)
{
	return "\\pos("+getfloat(((data[datapos].pos.x/zoomScale.x)+zoomMove.x) * wspw)+","+
		getfloat(((data[datapos].pos.y/zoomScale.y)+zoomMove.y) * wsph)+")";
}

	
void Position::SetCurVisual()
{
	data.clear();
	wxArrayInt sels;
	tab->Grid->file->GetSelections(sels);
	bool pib; wxPoint tp;
	for(size_t i = 0; i < sels.size(); i++){
		//fix by uzyskać reakcję na edycję w editboxie
		Dialogue *dial = (sels[i]==tab->Edit->ebrow)? tab->Edit->line : tab->Grid->GetDialogue(sels[i]);
		if(dial->IsComment){continue;}
		D3DXVECTOR2 pos = GetPos(dial,&pib,&tp);
		data.push_back(PosData(dial, sels[i], D3DXVECTOR2(((pos.x/wspw)-zoomMove.x)*zoomScale.x, 
			((pos.y/wsph)-zoomMove.y)*zoomScale.y), tp, pib));
	}
}

void Position::ChangeMultiline(bool all)
{
	wxString *dtxt = NULL;
	if(!all && !dummytext){
		bool visible=false; 
		selPositions.clear();
		dummytext = tab->Grid->GetVisible(&visible, 0, &selPositions);
		if(selPositions.size() != data.size()){
			wxLogStatus("Sizes mismatch");
			return;
		}
	}
	if(!all){ dtxt = new wxString(*dummytext);}
	bool skipInvisible = !all && tab->Video->GetState() != Playing;
	int _time = tab->Video->Tell();
	int moveLength=0;
	for(size_t i = 0; i < data.size(); i++){
		
		Dialogue *Dial = data[i].dial;
		if(skipInvisible && !(_time >= Dial->Start.mstime && _time <= Dial->End.mstime)){continue;}
		wxString visual = GetVisual(i);
		
		bool istxttl = (tab->Grid->hasTLMode && Dial->TextTl!="");
		wxString txt = (istxttl)? Dial->TextTl : Dial->Text;
		
		if(data[i].putinBracket){visual = "{" + visual + "}";}
		txt.replace(data[i].TextPos.x, data[i].TextPos.y, visual);
		if(all){
			if(istxttl){
				tab->Grid->CopyDialogue(data[i].numpos)->TextTl=txt;
			}else{
				tab->Grid->CopyDialogue(data[i].numpos)->Text=txt;
			}
		}else{
			Dialogue Cpy=Dialogue(*Dial);
			if(istxttl) {
				Cpy.TextTl = txt;
				wxString tlLines;
				Cpy.GetRaw(&tlLines, true);
				Cpy.GetRaw(&tlLines, false,tab->Grid->GetSInfo("TLMode Style"));
				dtxt->insert(selPositions[i] + moveLength,tlLines);
				moveLength += tlLines.Len();
			}else{
				Cpy.Text = txt;
				wxString thisLine;
				Cpy.GetRaw(&thisLine);
				dtxt->insert(selPositions[i] + moveLength,thisLine);
				moveLength += thisLine.Len();
			}
		}


	}

	if(all){
		tab->Video->VisEdit=true;
		if(tab->Edit->splittedTags){tab->Edit->TextEditOrig->modified=true;}
		tab->Grid->SetModified(VISUAL_POSITION,true);
		tab->Grid->Refresh();
	}else{
		
		if(!tab->Video->OpenSubs(dtxt)){wxLogStatus(_("Nie można otworzyć napisów"));}
		tab->Video->VisEdit=true;
		tab->Video->Render();
	}
	
}

