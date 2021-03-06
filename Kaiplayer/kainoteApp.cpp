﻿/***************************************************************
 * Name:      kainoteApp.cpp
 * Purpose:   Subtitles editor and player
 * Author:    Bjakja (bjakja@op.pl)
 * Created:   2012-04-23
 * Copyright: Marcin Drob aka Bjakja (http://animesub.info/forum/viewtopic.php?id=258715)
 * License:
 * Kainote is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Kainote is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Kainote.  If not, see <http://www.gnu.org/licenses/>.
 **************************************************************/



#include "KainoteApp.h"
#include <wx/image.h>
#include <wx/ipc.h>
#include <wx/utils.h>
#include <locale.h>
#include "OpennWrite.h"
#include "Config.h"
#include "Hotkeys.h"
#include <wx/intl.h>
#include "KaiMessageBox.h"

//wxDEFINE_EVENT(EVT_OPEN, wxThreadEvent);

IMPLEMENT_APP(kainoteApp);


class KaiConnection : public wxConnection
{

public:
     KaiConnection() : wxConnection() {}
	 ~KaiConnection(){ }
 
	 bool OnExec (const wxString &topic, const wxString &data)
     {
         bool result = wxGetApp().OnSecondInstance(data);
 
         return result;
     }
 

 };

class KaiServer : public wxServer
 {
public:
     virtual wxConnectionBase *OnAcceptConnection (const wxString& topic)
     {
         if (topic != "NewStart")
             return NULL;
         else
             return new KaiConnection;
     }
 };





bool kainoteApp::OnSecondInstance(wxString _paths)
{
	//wxMutexLocker lock(mutex);
	//isopening=true;
	if(Frame->IsIconized()){Frame->Iconize(false);}
	Frame->Raise();
	if(_paths==""){return true;}
	wxStringTokenizer tkn(_paths,"|");
	
	while(tkn.HasMoreTokens()){
		paths.Add(tkn.NextToken());
	}
	timer.Start(500,true);

	return true; 
}

bool kainoteApp::OnInit()
{
    
	m_checker = new wxSingleInstanceChecker();
        
    bool wxsOK = true;

	wxString server="4242";

	if (!m_checker->IsAnotherRunning())
    {
		
		setlocale(LC_NUMERIC, "C");
		setlocale(LC_CTYPE, "C");

		wxImage::AddHandler(new wxPNGHandler);
		wxImage::AddHandler(new wxICOHandler);
		wxImage::AddHandler(new wxCURHandler);

		if ( wxsOK )
		{
		//wxHandleFatalExceptions(true);
			

			if(!Options.LoadOptions()){KaiMessageBox(_("Nie udało się wczytać opcji.\nDziałanie programu zostanie zakończone."),_("Uwaga"));return false;}

			locale=NULL;
			if(Options.GetInt(ProgramLanguage) != 0){
				locale=new wxLocale();
				if(!locale->Init(wxLANGUAGE_ENGLISH, wxLOCALE_DONT_LOAD_DEFAULT)){
					KaiMessageBox("wxLocale cannot initialize, language change failed");
				}
				locale->AddCatalogLookupPathPrefix(Options.pathfull+L"\\Locale\\");
				if(!locale->AddCatalog(L"en",wxLANGUAGE_POLISH,L"UTF-8")){//
					KaiMessageBox("Cannot find translation, language change failed");
				}
				//KaiMessageBox(wxString::Format("isload%i", locale->IsLoaded(L"en")));
			}

			if(!Hkeys.LoadHkeys()){
				KaiMessageBox(_("Nie udało się wczytać skrótów.\nDziałanie programu zostanie zakończone."),_("Uwaga"));
				wxDELETE(locale);return false;
			}

			MyServer=new KaiServer();
			if(MyServer){
				if (!(MyServer->Create(server))){
					delete MyServer;
					MyServer = NULL;
				}
			}
			
			for (int i=1;i<argc;i++) { paths.Add(argv[i]); }

			int posx,posy,sizex,sizey;
			Options.GetCoords(WindowPosition,&posx,&posy);
			Options.GetCoords(WindowSize,&sizex,&sizey);
			if(sizex<500 || sizey<350){
				sizex=800;sizey=650;
			}

			Frame=NULL;
    		Frame = new kainoteFrame(wxPoint(posx,posy),wxSize(sizex,sizey));
			bool opevent=false;
			if(paths.GetCount()>0){
				if(Options.GetBool(VideoFullskreenOnStart)){
					Frame->OpenFiles(paths,false, true);
					Frame->GetTab()->Video->Layout();
				}
				else{
					opevent=true;
					
				}
			}
			
			
			Frame->Show();
    		SetTopWindow(Frame);
			timer.SetOwner(this,1199);
			Connect(1199,wxEVT_TIMER,(wxObjectEventFunction)&kainoteApp::OnOpen);
			if(opevent){
				timer.Start(500,true);
			}
#if _DEBUG
			else if (paths.GetCount()<1){
				if(Frame->subsrec.size()>0){paths.Add(Frame->subsrec[0]);}
				if(Frame->videorec.size()>0){paths.Add(Frame->videorec[0]);}
				timer.Start(50,true);
			}
#endif
			
		}
    }else{
		wxString subs;
		for (int i=1;i<argc;i++) {
			subs.Append(argv[i]);
			if(i+1!=argc){subs+="|";}
		}

		delete m_checker; // OnExit() won't be called if we return false
        m_checker = NULL;
		    
		wxClient *Client=new wxClient;
        KaiConnection * Connection = (KaiConnection*)Client->MakeConnection("", server, "NewStart");

		if (Connection){
			Connection->Execute(subs);
			delete Connection;
		}
		delete Client;
			
			
        return false;
	}
    
    return wxsOK;

}

int kainoteApp::OnExit()
{
	if (m_checker){ delete m_checker; }
	if (MyServer){ delete MyServer; }
	wxDELETE(locale);	
    return 0;
}

//void kainoteApp::OnUnhandledException()
//{
//	//wxString recover;
//	//for(size_t i=0;i<Frame->Tabs->Size();i++)
//		//{
//		//recover<<"Sub"<<i<<": "<<Frame->Tabs->Page(i)->SubsPath<<"\r\n"
//			//<<"Vid"<<i<<": "<<Frame->Tabs->Page(i)->VideoPath<<"\r\n";
//		//}
//	//recover<<Options.GetString("Subs Recent")<<Options.GetString("Video Recent");
//	//OpenWrite op;
//	//op.FileWrite(Options.pathfull+"\\recover.txt",Options.pathfull+"\\recover.txt");
//	//Options.SaveOptions();
//
//	wxLogStatus(_T("Ups, Kainote się skraszował w przyszłości będzie można wznowić sesję po tym kraszu"), "Krasz", wxOK | wxICON_ERROR);
//}

void kainoteApp::OnFatalException()
{
	//wxString recover;
	//for(size_t i=0;i<Frame->Tabs->Size();i++)
		//{
		//recover<<"Sub"<<i<<": "<<Frame->Tabs->Page(i)->SubsPath<<"\r\n"
			//<<"Vid"<<i<<": "<<Frame->Tabs->Page(i)->VideoPath<<"\r\n";
		//}
	//recover<<Options.GetString("Subs Recent")<<"\r\n"<<Options.GetString("Video Recent");
	//OpenWrite op;
	//op.FileWrite(Options.pathfull+"\\recover.txt",recover);
	//Options.SaveOptions();

	KaiMessageBox(_T("Ups, Kainote się skraszował w przyszłości będzie można wznowić sesję po tym kraszu"), "Krasz", wxOK | wxICON_ERROR);
}
void kainoteApp::OnOpen(wxTimerEvent &evt)
{
	if(!IsBusy()){
		Frame->OpenFiles(paths,false);
	}
}

bool kainoteApp::IsBusy()
{
	wxWindowList children = Frame->GetChildren();
	for (wxWindowList::Node *node=children.GetFirst(); node; node = node->GetNext()) {
            wxWindow *current = (wxWindow *)node->GetData();
            if ((current->IsKindOf(CLASSINFO(KaiDialog)) && ((KaiDialog*)current)->IsModal()) || 
				(current->IsKindOf(CLASSINFO(wxDialog)) && ((wxDialog*)current)->IsModal()) || current->GetId() == 31555 )
		{
			return true;
		}
	
	}
	return false;
}

