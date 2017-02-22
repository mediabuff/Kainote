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

#include "FontEnumerator.h"
#undef CreateDialog
#include "kainoteMain.h"
#include <wx/log.h>
#include <wx/filefn.h>
#include <Usp10.h>
#include <unicode/utf16.h>
//#pragma comment(lib,"Usp10.lib")
	



FontEnumerator::FontEnumerator()
{
	Fonts = new wxArrayString();
	FontsTmp = new wxArrayString();
}

FontEnumerator::~FontEnumerator()
{
	SetEvent(eventKillSelf);
	WaitForSingleObject(checkFontsThread, 2000);
	delete Fonts;
	delete FontsTmp;
}

void FontEnumerator::StartListening(kainoteFrame* _parent)
{
	parent = _parent;
	checkFontsThread = CreateThread( NULL, 0,  (LPTHREAD_START_ROUTINE)CheckFontsProc, this, 0, 0);
	SetThreadPriority(checkFontsThread,THREAD_PRIORITY_LOWEST);
}

void FontEnumerator::EnumerateFonts(bool reenumerate)
{
	wxMutexLocker lock(enumerateMutex);
	FontsTmp->Clear();
	LOGFONT lf;
	lf.lfCharSet = DEFAULT_CHARSET;
	wxStrlcpy(lf.lfFaceName, L"\0", WXSIZEOF(lf.lfFaceName));
	lf.lfPitchAndFamily = 0;
	HDC hDC = ::GetDC(NULL);
	EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROC)FontEnumeratorProc,
		(LPARAM)this, 0 /* reserved */);
	FontsTmp->Sort([](const wxString &i, const wxString &j){return i.CmpNoCase(j);});
	wxArrayString *tmp = FontsTmp;
	FontsTmp = Fonts;
	Fonts = tmp;

	::ReleaseDC(NULL, hDC);
}

wxArrayString *FontEnumerator::GetFonts(const wxWindow *client, std::function<void()> func)
{
	wxMutexLocker lock(enumerateMutex);
	if(Fonts->size()<1){
		EnumerateFonts(false);
	}
	if(client){observers[client] = func;}
	return Fonts;
}
void FontEnumerator::AddClient(const wxWindow *client, std::function<void()> func)
{
	if(client){observers[client] = func;}
}

void FontEnumerator::RemoveClient(const wxWindow *client)
{
	auto it = observers.find(client);
	if(it!=observers.end()){
		observers.erase(it);
	}
}

void FontEnumerator::RefreshClientsFonts()
{
	for(auto it = observers.begin(); it!=observers.end(); it++){
		auto func = it->second;
		func();
	}
}

int CALLBACK FontEnumerator::FontEnumeratorProc(LPLOGFONT lplf, LPTEXTMETRIC lptm,
												DWORD WXUNUSED(dwStyle), LPARAM lParam)
{
	FontEnumerator *Enum = (FontEnumerator*)lParam;
	if(Enum->FontsTmp->Index(lplf->lfFaceName,false)==wxNOT_FOUND){
		Enum->FontsTmp->Add(lplf->lfFaceName);
	}
	return true;
}

void FontEnumerator::RefreshVideo()
{
	//na razie tak, ale trzeba zrobi� to tak by �aden idiota przy od�wie�aniu nie zmieni� zak�adki.
	//TODO: Mo�liwy b��d Devilkana, czyli mix napis�w w zak�adkach, pami�taj o tym.
	// nigdy nie pozw�l by zmieni�a si� zak�adka gdy od�wie�amy albo od�wie�amy napisy na innej zak�adce ni� widoczna.
	TabPanel *tab = parent->GetTab();
	if(tab->Video->GetState()!=None){
		tab->Video->OpenSubs(tab->Grid1->SaveText());
		tab->Video->Render();
	}
}

DWORD FontEnumerator::CheckFontsProc(void* fontEnum)
{
	FontEnumerator *fe=(FontEnumerator*)fontEnum;
	if(!fontEnum){wxLogMessage(_("Brak wska�nika klasy magazynu styl�w.")); return 0;}

	HANDLE hDir  = NULL; 
	fe->eventKillSelf = CreateEvent(0, FALSE, FALSE, 0);
	wxString fontrealpath=wxGetOSDirectory() + "\\fonts\\";

	hDir = FindFirstChangeNotification( fontrealpath.wc_str(), TRUE, FILE_NOTIFY_CHANGE_FILE_NAME);// | FILE_NOTIFY_CHANGE_LAST_WRITE

	if(hDir == INVALID_HANDLE_VALUE ){wxLogMessage(_("Nie mo�na stworzy� uchwytu notyfikacji zmian folderu czcionek.")); return 0;}
	HANDLE events_to_wait[] = {
		hDir,
		fe->eventKillSelf
	};
	while(1){
		DWORD wait_result = WaitForMultipleObjects(sizeof(events_to_wait)/sizeof(HANDLE), events_to_wait, FALSE, INFINITE);
		if(wait_result == WAIT_OBJECT_0+0){
			fe->EnumerateFonts(true);
			fe->RefreshClientsFonts();
			fe->RefreshVideo();
			if( FindNextChangeNotification( hDir ) == 0 ){
				wxLogStatus(_("Nie mo�na stworzy� nast�pnego uchwytu notyfikacji zmian folderu czcionek."));
				return 0;
			}
		}else {
			break;
		}
	}

	return FindCloseChangeNotification( hDir );
}

//w Dc ma by� ustawiona czcionka
bool FontEnumerator::CheckGlyphsExists(HDC dc, const wxString &textForCheck, wxString &missing)
{
	std::wstring utf16characters = textForCheck.wc_str();
	
	bool succeeded = true;
	//code taken from Aegisub, fixed by me.
	SCRIPT_CACHE cache = NULL;
	WORD *indices = new WORD[utf16characters.size()];

	// First try to check glyph coverage with Uniscribe, since it
	// handles non-BMP unicode characters
	HRESULT hr = ScriptGetCMap(dc, &cache, utf16characters.data(),
		utf16characters.size(), 0, indices);

	// Uniscribe doesn't like some types of fonts, so fall back to GDI
	if (hr == E_HANDLE) {
		succeeded = (GetGlyphIndicesW(dc, utf16characters.data(), utf16characters.size(),
			indices, GGI_MARK_NONEXISTING_GLYPHS)!=GDI_ERROR);
		for (size_t i = 0; i < utf16characters.size(); ++i) {
			if (U16_IS_SURROGATE(utf16characters[i]))
				continue;
			if (indices[i] == 65535)
				missing<<utf16characters[i];
		}
	}
	else if (hr == S_FALSE) {
		for (size_t i = 0; i < utf16characters.size(); ++i) {
			// Uniscribe doesn't report glyph indexes for non-BMP characters,
			// so we have to call ScriptGetCMap on each individual pair to
			// determine if it's the missing one
			if (U16_IS_SURROGATE(utf16characters[i])) {
				hr = ScriptGetCMap(dc, &cache, &utf16characters[i], 2, 0, &indices[i]);
				if (hr == S_FALSE) {
					missing<<utf16characters[i];
					missing<<utf16characters[i+1];
				}
				++i;
			}
			else if (indices[i] == 0) {
				missing<<utf16characters[i];
			}
		}
	}else if(hr != S_OK){
		succeeded=false;
	}
	ScriptFreeCache(&cache);
	delete[] indices;
	return succeeded;
}

FontEnumerator FontEnum;