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


#include "ChangeTime.h"
#include "Config.h"
#include "Stylelistbox.h"
#include "KainoteMain.h"
#include "EditBox.h"
//#include "ColorPicker.h"


ShiftTimesWindow::ShiftTimesWindow(wxWindow* parent,kainoteFrame* kfparent,wxWindowID id,const wxPoint& pos,const wxSize& size,long style)
	: wxWindow/*wxScrolled<wxWindow>*/(parent, id, pos, size, style|wxVERTICAL)
{
    Kai=kfparent;
    form=ASS;
    panel = new wxWindow(this,1);
	SetForegroundColour(Options.GetColour(WindowText));
	SetBackgroundColour(Options.GetColour(WindowBackground));
	scroll=new KaiScrollbar(this,5558,wxDefaultPosition, wxDefaultSize, wxVERTICAL);
	scroll->Hide();
	scroll->SetScrollRate(30);
	isscrollbar=false;
	//SetScrollRate(0,5);
	scPos=0;
	
	wxAcceleratorEntry ctentries[1];
    ctentries[0].Set(wxACCEL_NORMAL, WXK_RETURN, ID_MOVE);
	
    wxAcceleratorTable ctaccel(1, ctentries);
    SetAcceleratorTable(ctaccel);
	CreateControls(Options.GetInt(PostprocessorEnabling) < 16);
	RefVals();
	
}

ShiftTimesWindow::~ShiftTimesWindow()
{
	SaveOptions();
}

bool ShiftTimesWindow::SetBackgroundColour(const wxColour &col)
{
	wxWindow::SetBackgroundColour(Options.GetColour(WindowBackground));
	panel->SetBackgroundColour(Options.GetColour(WindowBackground));
	return true;
}

bool ShiftTimesWindow::SetForegroundColour(const wxColour &col)
{
	wxWindow::SetForegroundColour(Options.GetColour(WindowText));
	panel->SetForegroundColour(Options.GetColour(WindowText));
	return true;
}

void ShiftTimesWindow::Contents(bool addopts)
{
	bool state;
	form=Kai->GetTab()->Grid->subsFormat;
	VideoCtrl *vb = Kai->GetTab()->Video;
	if(form<SRT){
		state=true;
		WhichLines->EnableItem(3);
		WhichLines->EnableItem(4);
	}else{
		WhichLines->EnableItem(3,false);
		WhichLines->EnableItem(4,false);
		state=false;
	}
	if (!LeadIn){
		bool lastEnable = DisplayFrames->IsEnabled();
		DisplayFrames->Enable(vb->VFF != NULL);
		bool Enable = DisplayFrames->IsEnabled();
		bool dispFrames = DisplayFrames->GetValue();
		if (lastEnable != Enable){
			if (!vb->VFF && (dispFrames || !Enable)){
				ChangeDisplayUnits(true);
			}
			else if (vb->VFF && dispFrames){
				ChangeDisplayUnits(false);
			}
		}
		MoveTagTimes->Enable(state);
	}
	AddStyles->Enable(state);
	Stylestext->Enable(state);
	if (!LeadIn){
		WhichTimes->Enable(form != TMP);
		if (vb->GetState() != None){ state = true; }
		else{ state = false; }
		videotime->Enable(state);
		state = (Kai->GetTab()->Edit->ABox && Kai->GetTab()->Edit->ABox->audioDisplay->hasMark);
		audiotime->Enable(state);
	}
	if(LeadIn){
		state=(form!=TMP);
		LeadIn->Enable(state);
		LeadOut->Enable(state);
		Continous->Enable(state);
		SnapKF->Enable(state && vb->VFF);
	}
	//if(addopts){RefVals();}
	
}



void ShiftTimesWindow::OnAddStyles(wxCommandEvent& event)
{
	wxString result = GetCheckedElements(Kai);
	Stylestext->SetValue(result);
	if(result != ""){WhichLines->SetSelection(4);}
}

void ShiftTimesWindow::SaveOptions()
{
	if (!LeadIn){
		if (TimeText->HasShownFrames()){
			Options.SetInt(MoveTimesFrames, TimeText->GetTime().orgframe);
		}
		else{
			Options.SetInt(MoveTimesTime, TimeText->GetTime().mstime);
		}

		//1 forward / backward, 2 Start Time For V/A Timing, 4 Move to video time, 
		//8 Move to audio time 16 display times / frames 32 move tag times;
		Options.SetInt(MoveTimesOptions, (int)Forward->GetValue() | ((int)StartVAtime->GetValue() << 1) |
			((int)videotime->GetValue() << 2) | ((int)(audiotime->GetValue()) << 3) |
			((int)DisplayFrames->GetValue() << 4) | ((int)MoveTagTimes->GetValue() << 5));

		Options.SetInt(MoveTimesWhichTimes, WhichTimes->GetSelection());
		Options.SetInt(MoveTimesCorrectEndTimes, CorTime->GetSelection());
	}
	Options.SetInt(MoveTimesWhichLines, WhichLines->GetSelection());
	if (form == ASS){
		wxString sstyles = Stylestext->GetValue();
		Options.SetString(MoveTimesStyles, sstyles);
	}
	if(LeadIn){
		Options.SetInt(PostprocessorLeadIn,LITime->GetInt());
		Options.SetInt(PostprocessorLeadOut,LOTime->GetInt());
		Options.SetInt(PostprocessorThresholdStart,ThresStart->GetInt());
		Options.SetInt(PostprocessorThresholdEnd,ThresEnd->GetInt());
		Options.SetInt(PostprocessorKeyframeBeforeStart,BeforeStart->GetInt());
		Options.SetInt(PostprocessorKeyframeAfterStart,AfterStart->GetInt());
		Options.SetInt(PostprocessorKeyframeBeforeEnd,BeforeEnd->GetInt());
		Options.SetInt(PostprocessorKeyframeAfterEnd,AfterEnd->GetInt());
		//1 Lead In, 2 Lead Out, 4 Make times continous, 8 Snap to keyframe;
		//int peres= (LeadIn->GetValue())? 1 : 0
		Options.SetInt(PostprocessorEnabling,(int)LeadIn->GetValue()+((int)LeadOut->GetValue()*2)+((int)Continous->GetValue()*4)+((int)SnapKF->GetValue()*8)+16);
	}
}

void ShiftTimesWindow::CreateControls(bool normal /*= true*/)
{
	wxFont thisFont(8, wxSWISS, wxFONTSTYLE_NORMAL, wxNORMAL, false, "Tahoma", wxFONTENCODING_DEFAULT);
	panel->SetFont(thisFont);
	panel->SetForegroundColour(Options.GetColour(WindowText));
	panel->SetBackgroundColour(Options.GetColour(WindowBackground));
	Main = new wxBoxSizer(wxVERTICAL);

	coll = new MappedButton(panel, 22999, (normal)? _("Post processor") : _("Przesuwanie czasów"));
	Main->Add(coll, 0, wxEXPAND | wxLEFT | wxRIGHT, 6);

	wxArrayString choices;
	KaiStaticBoxSizer *linesizer = new KaiStaticBoxSizer(wxVERTICAL, panel, _("Które linijki"));
	choices.Add(_("Wszystkie linijki"));
	choices.Add(_("Zaznaczone linijki"));
	choices.Add(_("Od zaznaczonej linijki"));
	choices.Add(_("Czasy wyższe i równe"));
	choices.Add(_("Według wybranych stylów"));
	WhichLines = new KaiChoice(panel, -1, wxDefaultPosition, wxDefaultSize, choices, KAI_SCROLL_ON_FOCUS);

	wxBoxSizer *stylesizer = new wxBoxSizer(wxHORIZONTAL);
	AddStyles = new MappedButton(panel, ID_BSTYLE, "+", "", wxDefaultPosition, wxSize(22, 22), -1);
	Stylestext = new KaiTextCtrl(panel, -1, "", wxDefaultPosition, wxSize(22, 22), wxTE_PROCESS_ENTER);
	stylesizer->Add(AddStyles, 0, wxALL, 2);
	stylesizer->Add(Stylestext, 1, wxEXPAND | wxBOTTOM | wxTOP | wxRIGHT, 2);

	linesizer->Add(WhichLines, 0, wxEXPAND | wxRIGHT | wxTOP | wxLEFT, 2);
	linesizer->Add(stylesizer, 1, wxEXPAND);

	if (normal){
		//ramka czasu
		KaiStaticBoxSizer *timesizer = new KaiStaticBoxSizer(wxVERTICAL, panel, _("Czas"));
		wxGridSizer *timegrid = new wxGridSizer(2, 0, 0);
		MoveTime = new MappedButton(panel, ID_MOVE, _("Przesuń"), _("Przesuń czas napisów"), wxDefaultPosition, wxSize(60, 22), GLOBAL_HOTKEY);
		TimeText = new TimeCtrl(panel, -1, "0:00:00.00", wxDefaultPosition, wxSize(60, 20), wxALIGN_CENTER | wxTE_PROCESS_ENTER);
		Forward = new KaiRadioButton(panel, -1, _("W przód"));
		Backward = new KaiRadioButton(panel, -1, _("W tył"));
		DisplayFrames = new KaiCheckBox(panel, 31221, _("Klatki"));
		MoveTagTimes = new KaiCheckBox(panel, -1, _("Czasy tagów"));
		Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &ShiftTimesWindow::OnChangeDisplayUnits, this, 31221);
		timegrid->Add(TimeText, 0, wxEXPAND | wxLEFT | wxRIGHT, 2);
		timegrid->Add(MoveTime, 0, wxEXPAND | wxRIGHT, 2);
		timegrid->Add(Forward, 1, wxEXPAND | wxLEFT | wxRIGHT, 2);
		timegrid->Add(Backward, 1, wxEXPAND | wxRIGHT, 2);
		timegrid->Add(DisplayFrames, 1, wxEXPAND | wxLEFT | wxRIGHT, 2);
		timegrid->Add(MoveTagTimes, 0, wxEXPAND | wxRIGHT, 2);

		timesizer->Add(timegrid, 0, wxEXPAND, 0);


		//ramka przesuwania wg audio / wideo
		KaiStaticBoxSizer *VAtiming = new KaiStaticBoxSizer(wxVERTICAL, panel, _("Przesuwanie wg wideo / audio"));

		wxBoxSizer *SE = new wxBoxSizer(wxHORIZONTAL);
		StartVAtime = new KaiRadioButton(panel, -1, _("Początek"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
		EndVAtime = new KaiRadioButton(panel, -1, _("Koniec"));

		SE->Add(StartVAtime, 1, wxEXPAND | wxLEFT | wxRIGHT, 2);
		SE->Add(EndVAtime, 1, wxEXPAND | wxRIGHT, 2);

		videotime = new KaiCheckBox(panel, ID_VIDEO, _("Przesuń znacznik\ndo czasu wideo"));
		videotime->SetForegroundColour(WindowWarningElements);
		videotime->Enable(false);

		audiotime = new KaiCheckBox(panel, ID_AUDIO, _("Przesuń znacznik\ndo czasu audio"));
		audiotime->SetForegroundColour(WindowWarningElements);
		audiotime->Enable(false);

		Connect(ID_VIDEO, ID_AUDIO, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&ShiftTimesWindow::AudioVideoTime);
		//TextColorPicker *picker = new TextColorPicker(this, AssColor(wxString("#AABBCC")));
		VAtiming->Add(SE, 0, wxEXPAND | wxTOP, 2);
		VAtiming->Add(videotime, 1, wxEXPAND | wxLEFT, 2);
		VAtiming->Add(audiotime, 1, wxEXPAND | wxLEFT, 2);
		//VAtiming->Add(picker,0,wxEXPAND|wxLEFT,2);
		
		KaiStaticBoxSizer *timessizer = new KaiStaticBoxSizer(wxVERTICAL, panel, _("Sposób przesuwania czasów"));
		choices.clear();
		choices.Add(_("Obydwa czasy"));
		choices.Add(_("Czas początkowy"));
		choices.Add(_("Czas końcowy"));

		WhichTimes = new KaiChoice(panel, -1, wxDefaultPosition, wxDefaultSize, choices, KAI_SCROLL_ON_FOCUS);
		WhichTimes->Enable(form != TMP);

		timessizer->Add(WhichTimes, 0, wxEXPAND | wxRIGHT | wxTOP | wxLEFT, 2);

		KaiStaticBoxSizer *cesizer = new KaiStaticBoxSizer(wxVERTICAL, panel, _("Korekcja czasów końcowych"));
		choices.clear();
		choices.Add(_("Pozostaw bez zmian"));
		choices.Add(_("Skoryguj nachodzące czasy"));
		choices.Add(_("Nowe czasy"));
		CorTime = new KaiChoice(panel, -1, wxDefaultPosition, wxSize(130, -1), choices, KAI_SCROLL_ON_FOCUS);
		CorTime->SetSelection(0);
		cesizer->Add(CorTime, 0, wxEXPAND | wxLEFT | wxRIGHT, 2);
		
		LeadIn = NULL;

		Main->Add(timesizer, 0, wxEXPAND | wxALL, 2);
		Main->Add(VAtiming, 0, wxEXPAND | wxALL, 2);
		Main->Add(linesizer, 0, wxEXPAND | wxALL, 2);
		Main->Add(timessizer, 0, wxEXPAND | wxALL, 2);
		Main->Add(cesizer, 0, wxEXPAND | wxALL, 2);

		panel->SetSizerAndFit(Main);
	}
	else{
		int pe = Options.GetInt(PostprocessorEnabling);
		liosizer = new KaiStaticBoxSizer(wxVERTICAL, panel, _("Wstęp i zakończenie"));
		MoveTime = new MappedButton(panel, ID_MOVE, _("Uruchom postprocessor"), _("Uruchom postprocessor"), wxDefaultPosition, wxDefaultSize, GLOBAL_HOTKEY);
		Main->Add(MoveTime, 0, wxEXPAND | wxTOP | wxLEFT | wxRIGHT, 6);
		wxBoxSizer *leadinSizer = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *leadoutSizer = new wxBoxSizer(wxHORIZONTAL);
		LeadIn = new KaiCheckBox(panel, -1, _("Wstęp"), wxDefaultPosition, wxSize(-1, -1));
		LeadIn->SetValue((pe & 1) > 0);
		LITime = new NumCtrl(panel, -1, Options.GetString(PostprocessorLeadIn), -10000, 10000, true, wxDefaultPosition, wxSize(40, -1), SCROLL_ON_FOCUS);
		LeadOut = new KaiCheckBox(panel, -1, _("Zakończenie"), wxDefaultPosition, wxSize(-1, -1));
		LeadOut->SetValue((pe & 2) > 0);
		LOTime = new NumCtrl(panel, -1, Options.GetString(PostprocessorLeadOut), -10000, 10000, true, wxDefaultPosition, wxSize(40, -1), SCROLL_ON_FOCUS);

		leadinSizer->Add(LeadIn, 1, wxEXPAND, 0);
		leadinSizer->Add(LITime, 0);
		leadoutSizer->Add(LeadOut, 1, wxEXPAND , 0);
		leadoutSizer->Add(LOTime, 0);

		liosizer->Add(leadinSizer, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 2);
		liosizer->Add(leadoutSizer, 1, wxEXPAND | wxLEFT | wxRIGHT, 2);

		consizer = new KaiStaticBoxSizer(wxVERTICAL, panel, _("Ustaw czasy jako ciągłe"));
		Continous = new KaiCheckBox(panel, -1, _("Włącz"));
		Continous->SetValue((pe & 4) > 0);

		consizer->Add(Continous, 0, wxEXPAND | wxALL, 2);

		wxBoxSizer *ThresStartSizer = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *ThresEndSizer = new wxBoxSizer(wxHORIZONTAL);
		ThresStart = new NumCtrl(panel, -1, Options.GetString(PostprocessorThresholdStart), 0, 10000, true, wxDefaultPosition, wxSize(40, -1), SCROLL_ON_FOCUS);
		ThresEnd = new NumCtrl(panel, -1, Options.GetString(PostprocessorThresholdEnd), 0, 10000, true, wxDefaultPosition, wxSize(40, -1), SCROLL_ON_FOCUS);

		ThresStartSizer->Add(new KaiStaticText(panel, -1, _("Próg czasu początku"), wxDefaultPosition, wxSize(-1, -1)), 1, wxEXPAND | wxLEFT, 4);
		ThresStartSizer->Add(ThresStart, 0);
		ThresEndSizer->Add(new KaiStaticText(panel, -1, _("Próg czasu końca"), wxDefaultPosition, wxSize(-1, -1)), 1, wxEXPAND | wxLEFT, 4);
		ThresEndSizer->Add(ThresEnd, 0);

		consizer->Add(ThresStartSizer, 1, wxEXPAND | wxBOTTOM | wxRIGHT, 2);
		consizer->Add(ThresEndSizer, 1, wxEXPAND | wxBOTTOM | wxRIGHT, 2);

		snapsizer = new KaiStaticBoxSizer(wxVERTICAL, panel, _("Wyrównaj do klatek kluczowych"));
		SnapKF = new KaiCheckBox(panel, -1, _("Włącz"));
		SnapKF->Enable(false);
		SnapKF->SetValue((pe & 8) > 0);
		snapsizer->Add(SnapKF, 0, wxEXPAND | wxALL, 2);

		wxBoxSizer *BeforeStartSizer = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *AfterStartSizer = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *BeforeEndSizer = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer *AfterEndSizer = new wxBoxSizer(wxHORIZONTAL);
		BeforeStart = new NumCtrl(panel, -1, Options.GetString(PostprocessorKeyframeBeforeStart), 0, 1000, true, wxDefaultPosition, wxSize(40, -1), SCROLL_ON_FOCUS);
		AfterStart = new NumCtrl(panel, -1, Options.GetString(PostprocessorKeyframeAfterStart), 0, 1000, true, wxDefaultPosition, wxSize(40, -1), SCROLL_ON_FOCUS);
		BeforeEnd = new NumCtrl(panel, -1, Options.GetString(PostprocessorKeyframeBeforeEnd), 0, 1000, true, wxDefaultPosition, wxSize(40, -1), SCROLL_ON_FOCUS);
		AfterEnd = new NumCtrl(panel, -1, Options.GetString(PostprocessorKeyframeAfterEnd), 0, 1000, true, wxDefaultPosition, wxSize(40, -1), SCROLL_ON_FOCUS);

		BeforeStartSizer->Add(new KaiStaticText(panel, -1, _("Przed czasem początku"), wxDefaultPosition, wxSize(-1, -1)), 1, wxEXPAND | wxLEFT, 4);
		BeforeStartSizer->Add(BeforeStart, 0);
		AfterStartSizer->Add(new KaiStaticText(panel, -1, _("Po czasie początku"), wxDefaultPosition, wxSize(-1, -1)), 1, wxEXPAND | wxLEFT, 4);
		AfterStartSizer->Add(AfterStart, 0);
		BeforeEndSizer->Add(new KaiStaticText(panel, -1, _("Przed czasem końca"), wxDefaultPosition, wxSize(-1, -1)), 1, wxEXPAND | wxLEFT, 4);
		BeforeEndSizer->Add(BeforeEnd, 0);
		AfterEndSizer->Add(new KaiStaticText(panel, -1, _("Po czasie końca"), wxDefaultPosition, wxSize(-1, -1)), 1, wxEXPAND | wxLEFT, 4);
		AfterEndSizer->Add(AfterEnd, 0);

		snapsizer->Add(BeforeStartSizer, 1, wxEXPAND | wxBOTTOM | wxRIGHT, 2);
		snapsizer->Add(AfterStartSizer, 1, wxEXPAND | wxBOTTOM | wxRIGHT, 2);
		snapsizer->Add(BeforeEndSizer, 1, wxEXPAND | wxBOTTOM | wxRIGHT, 2);
		snapsizer->Add(AfterEndSizer, 1, wxEXPAND | wxBOTTOM | wxRIGHT, 2);

		Main->Add(linesizer, 0, wxEXPAND | wxALL, 2);
		Main->Add((wxSizer*)liosizer, 0, wxEXPAND | wxALL, 2);
		Main->Add((wxSizer*)consizer, 0, wxEXPAND | wxALL, 2);
		Main->Add((wxSizer*)snapsizer, 0, wxEXPAND | wxALL, 2);
		panel->SetSizerAndFit(Main);
	}
	DoTooltips(normal);
	Connect(ID_MOVE, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ShiftTimesWindow::OnOKClick);
	Connect(ID_BSTYLE, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ShiftTimesWindow::OnAddStyles);
	Connect(22999, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ShiftTimesWindow::CollapsePane);
}

void ShiftTimesWindow::OnOKClick(wxCommandEvent& event)
{
	SaveOptions();
	int acid=event.GetId();
	TabPanel *tab = Kai->GetTab();
	if (acid==ID_MOVE){
		tab->Grid->ChangeTimes((!LeadIn)? TimeText->HasShownFrames() : false);
	}else if(acid==ID_CLOSE){
		Hide();
		tab->BoxSizer1->Layout();
	}
	tab->Grid->SetFocus();
}


void ShiftTimesWindow::OnSize(wxSizeEvent& event)
{
	int h,gw,gh;
	TabPanel* cur=(TabPanel*)GetParent();
	cur->Grid->GetClientSize(&gw,&gh);
	int w;
	panel->GetBestSize(&w,&h);
	int ctw, cth;
	GetSize(&ctw,&cth);
	if(!isscrollbar && gh < h)//pojawianie scrollbara
	{
		isscrollbar=true;
		SetMinSize(wxSize(w+17,h));
		cur->BoxSizer3->Layout();
		scroll->SetSize(w-1, 0, 17, gh);
		scroll->SetScrollbar(scPos, gh, h, gh-10);
		scroll->Show();
		
	}
	else if(isscrollbar && gh >= h )//znikanie scrollbara
	{
		isscrollbar=false;
		scPos=0;
		scroll->Hide();
		scroll->SetScrollbar(scPos, gh, h, gh-10);
		SetMinSize(wxSize(w,h));
		panel->SetPosition(wxPoint(0,scPos));
		cur->BoxSizer3->Layout();
	}else if(scroll->IsShown()){
		scroll->SetSize(ctw-18, 0, 17, gh);
		scroll->SetScrollbar(scPos, gh, h, gh-10);
		if(scPos != scroll->GetScrollPos()){
			scPos = scroll->GetScrollPos();
			panel->SetPosition(wxPoint(0,-scPos));
			//panel->Refresh(false);
		}
	}
	
}

void ShiftTimesWindow::DoTooltips(bool normal /*= true*/)
{
	WhichLines->SetToolTip(_("Wybór linijek do przesunięcia"));
	AddStyles->SetToolTip(_("Wybierz style z listy"));
	Stylestext->SetToolTip(_("Przesuń według następujących stylów (oddzielone średnikiem)"));
	if (normal){
		TimeText->SetToolTip(_("Czas przesunięcia"));
		videotime->SetToolTip(_("Przesuwanie zaznaczonej linijki\ndo czasu wideo ± czas przesunięcia"));
		audiotime->SetToolTip(_("Przesuwanie zaznaczonej linijki do czasu\nznacznika audio ± czas przesunięcia"));
		StartVAtime->SetToolTip(_("Przesuwa czas początkowy do czasu wideo / audio"));
		EndVAtime->SetToolTip(_("Przesuwa czas końcowy do czasu wideo / audio"));
		Forward->SetToolTip(_("Przesunięcie w przód / w tył"));
		DisplayFrames->SetToolTip(_("Przesuwa napisy o ustawiony czas / klatki"));
		MoveTagTimes->SetToolTip(_("Przesuwa czasy tagów \\move, \\t, \\fad tak,\nby ich pozycja na wideo się nie zmieniła\n(spowalnia przesuwanie czasów)"));
		WhichTimes->SetToolTip(_("Wybór czasów do przesunięcia"));
		CorTime->SetToolTip(_("Korekcja czasów końcowych, gdy są niewłaściwe albo nachodzą na siebie"));
	}
	else{
		LeadIn->SetToolTip(_("Wstawia wstęp do czasu początkowego, dobre przy stosowaniu fad"));
		LITime->SetToolTip(_("Czas wstępu w milisekundach"));
		LeadOut->SetToolTip(_("Wstawia zakończenie do czasu końcowego, dobre przy stosowaniu fad"));
		LOTime->SetToolTip(_("Czas zakończenia w milisekundach"));
		ThresStart->SetToolTip(_("Próg wydłużania czasu początkowego"));
		ThresEnd->SetToolTip(_("Próg wydłużania czasu końcowego"));
		BeforeStart->SetToolTip(_("Maksymalne przesunięcie do klatki kluczowej\nprzed czasem początkowym w milisekundach"));
		AfterStart->SetToolTip(_("Maksymalne przesunięcie do klatki kluczowej\npo czasie początkowym w milisekundach"));
		BeforeEnd->SetToolTip(_("Maksymalne przesunięcie do klatki kluczowej\nprzed czasem końcowym w milisekundach"));
		AfterEnd->SetToolTip(_("Maksymalne przesunięcie do klatki kluczowej\npo czasie końcowym w milisekundach"));
	}
}

void ShiftTimesWindow::AudioVideoTime(wxCommandEvent &event)
{
	int id=event.GetId();
	if (id==ID_VIDEO && videotime->GetValue()){
		audiotime->SetValue(false);
	}
	else if (id==ID_AUDIO && audiotime->GetValue()){
		videotime->SetValue(false);
	}
}

void ShiftTimesWindow::RefVals(ShiftTimesWindow *secondWindow)
{
	//1 forward / backward, 2 Start Time For V/A Timing, 4 Move to video time, 
	//8 Move to audio time 16 display times / frames 32 move tag times;
	if (secondWindow && (!secondWindow->LeadIn != !LeadIn)){
		wxCommandEvent evt;
		evt.SetId(22999);
		evt.SetInt(1000);
		CollapsePane(evt);
	}
	int mto = Options.GetInt(MoveTimesOptions);
	
	if (!LeadIn){
		
		STime ct = (secondWindow) ? secondWindow->TimeText->GetTime() : STime(Options.GetInt(MoveTimesTime), Options.GetInt(MoveTimesFrames));
		bool dispTimes = DisplayFrames->GetValue();
		DisplayFrames->SetValue((secondWindow) ? secondWindow->DisplayFrames->GetValue() : (mto & 16) > 0);
		TabPanel *tab = ((TabPanel*)GetParent());
		if (secondWindow && (secondWindow->DisplayFrames->GetValue() != dispTimes)){
			if (DisplayFrames->GetValue()){
				ChangeDisplayUnits(false);
			}
			else if (!DisplayFrames->GetValue()){
				ChangeDisplayUnits(true);
			}


		}
		else if (!tab->Video->VFF){
			if (DisplayFrames->GetValue()){ ChangeDisplayUnits(true); }
			DisplayFrames->Enable(false);
		}
		TimeText->SetTime(ct);

		videotime->SetValue((secondWindow) ? secondWindow->videotime->GetValue() : (mto & 4) > 0);
		audiotime->SetValue((secondWindow) ? secondWindow->audiotime->GetValue() : (mto & 8) > 0);
		Forward->SetValue((secondWindow) ? secondWindow->Forward->GetValue() : (mto & 1) > 0);
		Backward->SetValue((secondWindow) ? secondWindow->Backward->GetValue() : (mto & 1) == 0);
		DisplayFrames->SetValue((secondWindow) ? secondWindow->DisplayFrames->GetValue() : (mto & 16) > 0);
		MoveTagTimes->SetValue((secondWindow) ? secondWindow->MoveTagTimes->GetValue() : (mto & 32) > 0);
	}
	Stylestext->SetValue( (secondWindow)? secondWindow->Stylestext->GetValue() : Options.GetString(MoveTimesStyles) );

	int cm= (secondWindow)? secondWindow->WhichLines->GetSelection() : Options.GetInt(MoveTimesWhichLines);
	if(cm>(int)WhichLines->GetCount()){cm=0;}
	WhichLines->SetSelection(cm);
	if (!LeadIn){
		WhichTimes->SetSelection((secondWindow) ? secondWindow->WhichTimes->GetSelection() : Options.GetInt(MoveTimesWhichTimes));

		if ((secondWindow) ? secondWindow->StartVAtime->GetValue() : (mto & 2) > 0){ StartVAtime->SetValue(true); }
		else{ EndVAtime->SetValue(true); }

		CorTime->SetSelection((secondWindow) ? secondWindow->CorTime->GetSelection() : Options.GetInt(MoveTimesCorrectEndTimes));
	}
	int enables = Options.GetInt(PostprocessorEnabling);
	
	if(LeadIn && secondWindow && secondWindow->LeadIn){
		LeadIn->SetValue((secondWindow)? secondWindow->LeadIn->GetValue() : (enables & 1)>0);
		LeadOut->SetValue((secondWindow)? secondWindow->LeadOut->GetValue() : (enables & 2)>0);
		Continous->SetValue((secondWindow)? secondWindow->Continous->GetValue() : (enables & 4)>0);
		SnapKF->SetValue((secondWindow)? secondWindow->SnapKF->GetValue() : (enables & 8)>0);
		LITime->SetInt((secondWindow)? secondWindow->LITime->GetInt() : Options.GetInt(PostprocessorLeadIn));
		LOTime->SetInt((secondWindow)? secondWindow->LOTime->GetInt() : Options.GetInt(PostprocessorLeadOut));
		ThresStart->SetInt((secondWindow)? secondWindow->ThresStart->GetInt() : Options.GetInt(PostprocessorThresholdStart));
		ThresEnd->SetInt((secondWindow)? secondWindow->ThresEnd->GetInt() : Options.GetInt(PostprocessorThresholdEnd));
		BeforeStart->SetInt((secondWindow)? secondWindow->BeforeStart->GetInt() : Options.GetInt(PostprocessorKeyframeBeforeStart));
		AfterStart->SetInt((secondWindow)? secondWindow->AfterStart->GetInt() : Options.GetInt(PostprocessorKeyframeAfterStart));
		BeforeEnd->SetInt((secondWindow)? secondWindow->BeforeEnd->GetInt() : Options.GetInt(PostprocessorKeyframeBeforeEnd));
		AfterEnd->SetInt((secondWindow)? secondWindow->AfterEnd->GetInt() : Options.GetInt(PostprocessorKeyframeAfterEnd));
	}
   
}

void ShiftTimesWindow::CollapsePane(wxCommandEvent &event)
{
	bool collapsed = (LeadIn==NULL);
	int pe = Options.GetInt(PostprocessorEnabling);
	Options.SetInt(PostprocessorEnabling, pe ^ 16);
	if (!collapsed){
		Options.SetInt(PostprocessorLeadIn, LITime->GetInt());
		Options.SetInt(PostprocessorLeadOut, LOTime->GetInt());
		Options.SetInt(PostprocessorThresholdStart, ThresStart->GetInt());
		Options.SetInt(PostprocessorThresholdEnd, ThresEnd->GetInt());
		Options.SetInt(PostprocessorKeyframeBeforeStart, BeforeStart->GetInt());
		Options.SetInt(PostprocessorKeyframeAfterStart, AfterStart->GetInt());
		Options.SetInt(PostprocessorKeyframeBeforeEnd, BeforeEnd->GetInt());
		Options.SetInt(PostprocessorKeyframeAfterEnd, AfterEnd->GetInt());
		//1 Lead In, 2 Lead Out, 4 Make times continous, 8 Snap to keyframe;
		//int peres= (LeadIn->GetValue())? 1 : 0
		Options.SetInt(PostprocessorEnabling, (int)LeadIn->GetValue() + ((int)LeadOut->GetValue() * 2) + ((int)Continous->GetValue() * 4) + ((int)SnapKF->GetValue() * 8));
	}
	panel->Destroy();
	LeadIn = NULL;
	panel = new wxWindow(this, 1);
	CreateControls(!collapsed);

	if(collapsed){
		
		if(event.GetId()==22999){
			//((TabPanel*)GetParent())->BoxSizer3->Layout();
			isscrollbar=false;
			wxSizeEvent evt;
			OnSize(evt);
		}
		//wxSizeEvent evt;
		//OnSize(evt);
	}else{
		
		//wxSizeEvent evt;
		//OnSize(evt);
		int w, h,gw,gh;
		TabPanel* cur=(TabPanel*)GetParent();
		cur->Grid->GetClientSize(&gw,&gh);
		panel->GetBestSize(&w,&h);
		scPos = 0;
		if(gh < h)//pojawianie scrollbara
		{
			SetMinSize(wxSize(w+17,h));
			cur->BoxSizer3->Layout();
			scroll->SetSize(w, 0, 17, gh);
			scroll->SetScrollbar(scPos, gh, h, gh-10);
		
		}
		else if(gh >= h )//znikanie scrollbara
		{
			isscrollbar=false;
			scroll->Hide();
			scroll->SetScrollbar(scPos, gh, h, gh-10);
			SetMinSize(wxSize(w,h));
			cur->BoxSizer3->Layout();
		}
		
		panel->SetPosition(wxPoint(0,-scPos));
		//panel->Update();
	}
	//trick to prevent recursive stack overflow
	if (event.GetInt() != 1000){
		Contents(false);
		RefVals();
	}
}

void ShiftTimesWindow::OnScroll(wxScrollEvent& event)
{
	int newPos = event.GetPosition();
	//wxLogStatus("scroll %i %i", newPos, scPos);
	if (scPos != newPos) {
		scPos = newPos;
		panel->SetPosition(wxPoint(0,-scPos));
		panel->Update();
	}
}

void ShiftTimesWindow::OnMouseScroll(wxMouseEvent& event)
{
	if(event.GetWheelRotation() != 0){
		int step = 30 * event.GetWheelRotation() / event.GetWheelDelta();
		scPos = scroll->SetScrollPos(scPos-step); 
		panel->SetPosition(wxPoint(0,-scPos));
		panel->Update();
	}
}

void ShiftTimesWindow::OnChangeDisplayUnits(wxCommandEvent& event)
{
	ChangeDisplayUnits(!DisplayFrames->GetValue());
}
	
void ShiftTimesWindow::ChangeDisplayUnits(bool times)
{
	STime ct = TimeText->GetTime();
	if(times){
		TimeText->ShowFrames(false);
		ct.mstime = Options.GetInt(MoveTimesTime);
		TimeText->SetTime(ct);
	}else{
		TimeText->ShowFrames(true);
		ct.orgframe = Options.GetInt(MoveTimesFrames);
		TimeText->SetTime(ct);
	}
}
BEGIN_EVENT_TABLE(ShiftTimesWindow,wxWindow)
EVT_SIZE(ShiftTimesWindow::OnSize)
EVT_COMMAND_SCROLL_THUMBTRACK(5558,ShiftTimesWindow::OnScroll)
EVT_MOUSEWHEEL(ShiftTimesWindow::OnMouseScroll)
END_EVENT_TABLE()
