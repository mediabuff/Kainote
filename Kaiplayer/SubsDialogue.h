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

#ifndef ZEROIT
#define ZEROIT(a) ((a/10)*10)
#endif


#include "SubsTime.h"
#include <wx/colour.h>
#include <vector>

//isVisible helper class
class StoreHelper {
public:
	StoreHelper(){}
	StoreHelper(const StoreHelper &sh){
		Store(sh, false);
	}
	~StoreHelper(){
		if (*deleteReference < 1 && stored){ 
			delete stored; stored = NULL; delete deleteReference; deleteReference = NULL; 
		}
		else{ 
			(*deleteReference)--; 
		}
	};
	void Store(const StoreHelper &sh, bool copy){ 
		assert(sh.stored);
		if (*deleteReference < 1 && stored != sh.stored){
			delete stored; stored = NULL;
			delete deleteReference; deleteReference = NULL;
		}
		if (copy){
			stored = new unsigned char(*sh.stored);
			if (*stored < 1){ *stored = 1; }
			deleteReference = new size_t(0);
		}
		else{
			stored = sh.stored;
			//if (deleteReference){ delete deleteReference; }
			deleteReference = sh.deleteReference;
			(*deleteReference)++;
		}
	};
	StoreHelper &operator =(const StoreHelper &sh){
		Store(sh, false);
		return *this;
	}
	void operator =(const unsigned char value){
		assert(stored);
		*stored = value;
	}
	bool operator ==(const unsigned char value){
		assert(stored);
		return *stored == value;
	}
	bool operator !=(const unsigned char value){
		return *stored != value;
	}
	bool operator >(const unsigned char value){
		assert(stored);
		return *stored > value;
	}
	bool operator <(const unsigned char value){
		assert(stored);
		return *stored < value;
	}
	bool operator !(){
		assert(stored);
		return !(*stored);
	}
	unsigned char &operator *(){ 
		assert(stored);
		return *stored; 
	};
private:
	unsigned char *stored = new unsigned char(1);
	size_t *deleteReference = new size_t(0);
};

//dialogue strings helper class
class StoreTextHelper{
public:
	StoreTextHelper(){}
	StoreTextHelper(const StoreTextHelper &sh){
		Store(sh);
	}
	/*StoreTextHelper(const wxString &txt){
		StoreText(txt);
	}
	StoreTextHelper(const char *txt){
		StoreText(txt);
	}
	StoreTextHelper(const wchar_t *txt){
		StoreText(txt);
	}*/
	~StoreTextHelper(){
		if (*deleteReference < 1){
			delete stored; stored = NULL;
			delete deleteReference; deleteReference = NULL;
		}
		else{
			(*deleteReference)--;
		}
	};
	void Store(const StoreTextHelper &sh){
		if (*deleteReference < 1){
			delete stored; stored = NULL;
			delete deleteReference; deleteReference = NULL;
		}
		else{
			(*deleteReference)--;
		}
		stored = sh.stored;
		deleteReference = sh.deleteReference;
		(*deleteReference)++;
		
	};
	void StoreText(const wxString &txt){
		if (*deleteReference < 1){
			delete stored; stored = NULL;
			delete deleteReference; deleteReference = NULL;
		}
		else{
			(*deleteReference)--;
		}
		deleteReference = new size_t(0);
		stored = new wxString(txt);
	};
	StoreTextHelper &operator =(const StoreTextHelper &sh){
		Store(sh);
		return *this;
	}
	
	StoreTextHelper &operator =(const wxString &newString){
		StoreText(newString);
		return *this;
	}
	operator const wxString&(){ 
		return *stored; 
	}
	bool operator !=(const wxString &comptext) const{ return comptext != (*stored); };
	bool operator !=(const char *comptext) const{ return comptext != (*stored); };
	bool operator ==(const wxString &comptext) const{ return comptext == (*stored); };
	bool operator ==(const char *comptext) const{ return comptext == (*stored); };
	wxString &operator <<(wxString &text){
		if ((*deleteReference) > 0){
			StoreText(*stored + text);
		}
		else{ *stored << text; }
		return *stored;
	};
	wxString &operator <<(const char *text){
		if ((*deleteReference) > 0){
			StoreText(*stored + text);
		}
		else{ *stored << text; }
		return *stored;
	};
	//Operator kopiuje wska�nik gdy mamy dodatkowe u�ycia w przeciwnym razie to nie jest w og�le potrzebne.
	wxString *operator ->(){ 
		if ((*deleteReference) > 0){ StoreText(*stored); }
		return stored; 
	}
	wxString &CheckTlRef(StoreTextHelper &TextTl, bool condition){ 
		if (condition) { 
			return *TextTl.Copy();
		}
		else {
			return *Copy();
		}
	}
	wxString CheckTl(const StoreTextHelper &TextTl, bool condition){
		if (condition) {
			return *TextTl.stored;
		}
		else {
			return *stored;
		}
	}
	size_t Len() const{ 
		return stored->Len(); 
	}
	int CmpNoCase(const StoreTextHelper &TextTl) const{
		return stored->CmpNoCase(*TextTl.stored);
	}
	bool empty() const{
		return stored->empty();
	}
	wxString & Trim(bool fromRight = true){
		return stored->Trim(fromRight);
	}
	const wxScopedCharBuffer mb_str(const wxMBConv& conv = wxConvLibc) const{
		return stored->mb_str(conv);
	}
	wxString *Copy(){
		if ((*deleteReference) > 0){ StoreText(*stored); } 
		return stored;
	}
private:
	wxString *stored = new wxString();
	size_t *deleteReference = new size_t(0);
};

class TagData
{
public:
	TagData(const wxString &name, unsigned int startTextPos);
	void PutValue(const wxString &name, bool multiValue = false);
	wxString tagName;
	wxString value;
	bool multiValue;
	unsigned int startTextPos;
};

class ParseData
{
public:
	ParseData();
	~ParseData();
	void AddData(TagData *data);
	std::vector<TagData*> tags;
};

class Dialogue
{

public:
	StoreTextHelper Style, Actor, Effect, Text, TextTl;
	STime Start, End;
	int Layer;
	short MarginL, MarginR, MarginV;
	char State, Form;
	bool NonDialogue, IsComment;
	StoreHelper isVisible;
	ParseData *parseData;

	void SetRaw(const wxString &ldial);
	void GetRaw(wxString *txt,bool tl=false,const wxString &style="");
	wxString GetCols(int cols, bool tl=false,const wxString &style="");
	void Convert(char type,const wxString &pref="");
	Dialogue *Copy(bool keepstate=false, bool copyIsVisible = true);
	void ParseTags(wxString *tags, size_t n, bool plainText = false);
	void ChangeTimes(int start, int end);
	void ClearParse();
	Dialogue();
	Dialogue(const wxString &ldial,const wxString &txttl="");
	~Dialogue();
};

enum{
	NOT_VISIBLE = 0,
	VISIBLE,
	VISIBLE_BLOCK,
};