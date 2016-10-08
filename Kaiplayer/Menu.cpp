
#include "Menu.h"
#include "Config.h"
#include "Hotkeys.h"

static wxFont font = wxFont(9,wxSWISS,wxFONTSTYLE_NORMAL,wxNORMAL,false,"Tahoma");
const static int height = 20;
const static int maxVisible = 30;

wxDEFINE_EVENT(EVT_MENU_OPENED, MenuEvent);

MenuItem::MenuItem(int _id, const wxString& _label, const wxString& _help, bool _enable, wxBitmap *_icon, Menu *Submenu, byte _type)
{
	icon=_icon; 
	label=_label; 
	id=_id; 
	enabled=_enable;
	type=_type;
	submenu=Submenu; 
	check=false;
	help = _help;
	accel=NULL;
}
MenuItem::~MenuItem()
{
	if(submenu){delete submenu;}
	if(icon){delete icon;}
}
bool MenuItem::Enable(bool enable)
{
	if(enabled!=enable){enabled=enable;return true;}
	return false;
}
wxBitmap MenuItem::GetBitmap()
{
	if(!icon){return wxBitmap();}
	if(!enabled){
		return wxBitmap(icon->ConvertToImage().ConvertToGreyscale());
	}
	return *icon;
}

void MenuItem::SetAccel(wxAcceleratorEntry *entry)
{
	accel = entry;
	if(label.find("\t")!=-1){label = label.BeforeFirst('\t');}
	label += "\t" + entry->ToString();
	//if(MenuBar::Menubar){MenuBar::Menubar->SetAccelerators();}
}

Menu::Menu()
	:dialog(NULL)
	,parentMenu(NULL)
{
}


int Menu::GetPopupMenuSelection(const wxPoint &pos, wxWindow *parent, int *accels, bool clientPos)
{
	wxPoint npos= pos;
	wxSize size;
	CalcPosAndSize(parent, &npos, &size, clientPos);
	//wxLogStatus("Pos & size %i, %i %i %i %i %i", npos.x, npos.y, size.x, size.y, pos.x, pos.y);
	//if(dialog){dialog->Destroy();}
	dialog = new MenuDialog(this, parent, npos, size, false);
	int ret = dialog->ShowPartialModal();
	if(accels){*accels = dialog->accel;}
	dialog->Destroy();
	return ret;
}

void Menu::PopupMenu(const wxPoint &pos, wxWindow *parent, bool clientPos)
{
	wxPoint npos= pos;
	wxSize size;
	CalcPosAndSize(parent, &npos, &size, clientPos);
	//wxLogStatus("Pos & size %i, %i %i %i %i %i", npos.x, npos.y, size.x, size.y, pos.x, pos.y);
	
	dialog = new MenuDialog(this, parent, npos, size);

	//dialog->Show();
	
	dialog->ShowWithEffect(wxSHOW_EFFECT_BLEND ,100);//wxSHOW_EFFECT_BLEND
	//dialog->SetFocus();
	dialog->Refresh(false);
	//MenuDialog::blockHideDialog=false;
}

void Menu::CalcPosAndSize(wxWindow *parent, wxPoint *pos, wxSize *size, bool clientPos)
{
	int tx=0, ty=0;
	if(clientPos){*pos = parent->ClientToScreen(*pos);}
	size_t isize = items.size();
	for(size_t i = 0; i < isize; i++){
		parent->GetTextExtent(items[i]->label, &tx, &ty, 0, 0, &font);
		if(tx > size->x){size->x = tx;}
	}
	
	size->x += 50;
	if(isize > maxVisible) {wxLogStatus("maxvisible");size->x += 20; isize=maxVisible;}
	size->y = height * isize + 4;
	int w,h;
	wxRect workArea = wxGetClientDisplayRect();
	w = workArea.width - workArea.x;
	h = workArea.height - workArea.y;
	if(size->y > h){ size->y = h; }
	if((pos->x + size->x) > w){
		pos->x -= size->x;
		//wxLogStatus("dialog %i %i", (int)parentMenu, (parentMenu)? (int)parentMenu->dialog : 0);
		if(parentMenu && parentMenu->dialog){
			wxSize size = parentMenu->dialog->GetClientSize();
			//wxLogStatus("posx %i", pos->x);
			pos->x -= size.x;
			//wxLogStatus("posx1 %i", pos->x);
		}
	}
	if((pos->y + size->y) > h){
		if(size->y > h/2){pos->y -= size->y;}
		else{pos->y = h - size->y;}
	}
	
}

MenuItem *Menu::Append(int _id, const wxString& _label, const wxString& _help, bool _enable, wxBitmap *_icon, Menu* Submenu, byte _type)
{
	if(Submenu){Submenu->parentMenu=this;}
	MenuItem *item = new MenuItem(_id, _label, _help, _enable, _icon, Submenu, _type);
	items.push_back(item);
	return item;
}
MenuItem *Menu::Append(int _id,const wxString& _label, Menu* Submenu, const wxString& _help, byte _type, bool _enable, wxBitmap *_icon)
{
	if(Submenu){Submenu->parentMenu=this;}
	MenuItem *item = new MenuItem(_id, _label, _help, _enable, _icon, Submenu, _type);
	items.push_back(item);
	return item;
}
	
MenuItem *Menu::Append(MenuItem *item)
{
	if(item->submenu){item->submenu->parentMenu=this;}
	items.push_back(item);
	return item;
}
	
MenuItem *Menu::Prepend(int _id, const wxString& _label, const wxString& _help, bool _enable, wxBitmap *_icon, Menu* Submenu, byte _type)
{
	if(Submenu){Submenu->parentMenu=this;}
	MenuItem *item = new MenuItem(_id, _label, _help, _enable, _icon, Submenu, _type);
	items.insert(items.begin(),item);
	return item;
}
	
MenuItem *Menu::Prepend(MenuItem *item)
{
	if(item->submenu){item->submenu->parentMenu=this;}
	items.insert(items.begin(),item);
	return item;
}
	
MenuItem *Menu::Insert(int position, int _id, const wxString& _label, const wxString& _help, bool _enable, wxBitmap *_icon, Menu* Submenu, byte _type)
{
	if(Submenu){Submenu->parentMenu=this;}
	MenuItem *item = new MenuItem(_id, _label, _help, _enable, _icon, Submenu, _type);
	items.insert(items.begin() + position,item);
	return item;
}
	
MenuItem *Menu::Insert(int position, MenuItem *item)
{
	if(item->submenu){item->submenu->parentMenu=this;}
	items.insert(items.begin() + position,item);
	return item;
}
	
bool Menu::Destroy(MenuItem *item)
{
	for(size_t i = 0; i < items.size(); i++ ){
		if(items[i] == item){
			delete item;
			items.erase(items.begin()+i);
			return true;
		}else if(items[i]->submenu){
			if(items[i]->submenu->Destroy(item)){return true;}
		}

	}
	delete item;
	return false;
}

void Menu::Delete(int position)
{
	delete items[position];
	items.erase(items.begin()+position);
}
	
int Menu::GetMenuItemCount()
{
	return items.size();
}
	
MenuItem *Menu::FindItem(int id)
{
	for(size_t i = 0; i < items.size(); i++ ){
		if(items[i]->id==id){
			return items[i];
		}else if(items[i]->submenu){
			MenuItem * item = items[i]->submenu->FindItem(id);
			if(item){return item;}
		}

	}
	return NULL;
}
	
MenuItem *Menu::FindItem(const wxString& label)
{
	for(size_t i = 0; i < items.size(); i++ ){
		if(items[i]->label==label){
			return items[i];
		}else if(items[i]->submenu){
			MenuItem * item = items[i]->submenu->FindItem(label);
			if(item){return item;}
		}

	}
	return NULL;
}

	
MenuItem *Menu::FindItemByPosition(int pos)
{
	if(pos>(int)items.size() || pos<0){return NULL;} 
	return items[pos];
}

void Menu::Check(int id, bool check)
{
	MenuItem * item = FindItem(id);
	if(item){
		item->check = check;
	}
}
	
void Menu::AppendSeparator()
{
	MenuItem *item = new MenuItem(-2, "", "", false, 0, 0, ITEM_SEPARATOR);
	items.push_back(item);
}

void Menu::DestroyDialog()
{
	if(dialog){
		dialog->Destroy(); dialog=NULL;
	}//if(dialog->IsModal()){dialog->EndModal(-3);}else{}
}

MenuItem *Menu::SetAccMenu(int id, const wxString &txt, const wxString &help, bool enable, int kind)
{
	wxString hkey=Hkeys.GetMenuH(id);
	wxString mtext=(hkey!="")? txt.BeforeFirst('\t')+"\t"+hkey : txt;
	if(hkey!="" && Hkeys.hkeys[id].Name==""){Hkeys.hkeys[id].Name=txt.BeforeFirst('\t');}
	return Append(id,mtext,help,true,0,0,kind);
}

MenuItem *Menu::SetAccMenu(MenuItem *menuitem, const wxString &name)
{
	int id=0;
	for(auto cur=Hkeys.hkeys.rbegin(); cur!=Hkeys.hkeys.rend(); cur++)
	{
		if(cur->first<30100){break;}
		if(cur->second.Name == name ){
			//hkey = cur->second.Accel;
			id = cur->first;	
		}
	}

	if(id){menuitem->SetAccel(&Hkeys.GetHKey(id));}
	return Append(menuitem);
}

void Menu::GetAccelerators(std::vector <wxAcceleratorEntry> *entries)
{
	for(auto item : items){
		if(item->submenu){
			item->submenu->GetAccelerators(entries);
		}else if(item->accel){
			entries->push_back(*item->accel);
		}

	}
}

MenuDialog* MenuDialog::ParentMenu=NULL;

MenuDialog::MenuDialog(Menu *_parent, wxWindow *DialogParent, const wxPoint &pos, const wxSize &size, bool sendEvent)
	:wxDialog(DialogParent,-1,"",pos, size, 0)
	,wxGUIEventLoop()
	,parent(_parent)
	,sel(-1)
	,scPos(0)
	,subMenuIsShown(false)
	,isPartialModal(false)
	,submenuShown(-1)
	,bmp(NULL)
	,id(-3)
	,accel(0)
{
	if(!ParentMenu){ParentMenu=this;}
	//show
	showSubmenuTimer.SetOwner(this, 13475);
	Bind(wxEVT_TIMER,[=](wxTimerEvent &evt){
		if(submenuShown == -1 || sel != submenuShown){return;}
		int scrollPos = submenuShown-scPos;
		wxSize size = GetSize();
		wxPoint pos = GetPosition();
		pos.x += size.x;
		pos.y += scrollPos * height;
		parent->items[submenuShown]->submenu->PopupMenu(pos, this->GetParent(), false);
		subMenuIsShown=true;
	},13475);
	//hide
	hideSubmenuTimer.SetOwner(this, 13476);
	Bind(wxEVT_TIMER,[=](wxTimerEvent &evt){
		if(submenuShown == -1){return;}
		MenuItem *olditem=parent->items[submenuShown];
		if(olditem->submenu->dialog){
			wxRect rc = olditem->submenu->dialog->GetScreenRect();
			wxPoint pos = wxGetMousePosition();
			if(pos.x >= rc.x && pos.y >= pos.y && pos.x <= rc.GetRight() && pos.y <= rc.GetBottom()){wxLogStatus("nie zamykamy okna");return;}
			//if(!HasFocus())SetFocus();
			olditem->submenu->dialog->HideWithEffect(wxSHOW_EFFECT_BLEND ,200);
			olditem->submenu->DestroyDialog();
		}
		submenuShown=-1;
	},13476);
	if(!ParentMenu->HasCapture()){CaptureMouse();}
}

MenuDialog::~MenuDialog()
{
	if(HasCapture()){ReleaseMouse();}
	if(IsRunning()){
		Exit(id);
	}
	wxDELETE(bmp);
}

void MenuDialog::OnMouseEvent(wxMouseEvent &evt)
{
	bool leftdown=evt.LeftDown();
	int x=evt.GetX();
	int y=evt.GetY();
	int w=0;
	int h=0;
	//GetSize (&w, &h);
	wxRect rc = GetClientRect();
	w=rc.x; h=rc.y;
	bool contains = rc.Contains(evt.GetPosition());
	if(!contains && leftdown){
		int subMenu=ParentMenu->submenuShown;
		//wxLogStatus("submenu");
		Menu *menu=ParentMenu->parent;
		//ParentMenu->GetParent()->SetFocus();
		//wxLogStatus("submenu1");
		if(ParentMenu->isPartialModal){/*wxLogStatus("partial");*/ParentMenu->EndPartialModal(id);}
		else{/*wxLogStatus("no partial");*/menu->dialog->HideWithEffect(wxSHOW_EFFECT_BLEND ,100);menu->DestroyDialog();}
		while(subMenu!= -1 && menu->items[subMenu]->submenu){
			menu = menu->items[subMenu]->submenu;
			//wxLogStatus("menu %i, %i", subMenu, (int)menu);
			if(menu->dialog){subMenu = menu->dialog->submenuShown;}
			else{subMenu = -1;}
			menu->dialog->HideWithEffect(wxSHOW_EFFECT_BLEND ,100);
			menu->DestroyDialog();
		}
		ParentMenu=NULL;
	}
	int elem = y/height;

	if(!contains){
		if(sel!=-1){sel=-1; Refresh(false);}
		return;
	}else{
		if(sel==-1){
			if(submenuShown != -1 && submenuShown != elem){hideSubmenuTimer.Start(100,true);}
			if( !((elem < (int)parent->items.size() && elem > 0 ) && 
				parent->items[elem]->submenu && submenuShown==-1)){
					sel=elem; Refresh(false);return;
			}
		}
	}
	
	//if(evt.Leaving()){
	//	//wxLogStatus("leaving");
	//	//gdy wychodzi resetuje zaznaczenie
	//	sel=-1; Refresh(false);return;
	//}else if(evt.Entering()){
	//	//wxLogStatus("entering");
	//	//trick, by naje�d�aj�c na element kt�ry ma submenu je od razu wy�wietli�, 
	//	//ale gdy jest wy�wietlone ma to ola�.
	//	//gdy wchodzi ustawia zaznaczenie i olewa dalsze akcje by zapobiec mrugozie submenu itp
	//	//if(parent->parentMenu ){parent->parentMenu->dialog->submenuShown = -1;}
	//	if(submenuShown != -1 && submenuShown != elem){hideSubmenuTimer.Start(100,true);}
	//	if( !((elem < (int)parent->items.size() && elem > 0 ) && 
	//		parent->items[elem]->submenu && submenuShown==-1)){
	//			sel=elem; Refresh(false);return;
	//	}
	//	
	//}
	
	elem+=scPos;
	if(elem>=(int)parent->items.size() || elem < 0){return;}
	if(elem!=sel){
		
		if(subMenuIsShown){
			hideSubmenuTimer.Start(400,true);
			subMenuIsShown=false;
		}
			
		MenuItem *item=parent->items[elem];
		sel=elem;
		if(item->submenu){
			submenuShown=elem;
			showSubmenuTimer.Start(400,true);
		}
		
		Refresh(false);
		if(item->help != ""){
			wxLogStatus(item->help);
		}
	}
	if (evt.GetWheelRotation() != 0) {
		int step = 3 * evt.GetWheelRotation() / evt.GetWheelDelta();
		scPos -=step;
		Refresh(false);
		return;
	}
	if(leftdown){
		MenuItem *item=parent->items[elem];
		if(!item->enabled){return;}
		if(item->type == ITEM_CHECK){
			item->check = !item->check;
			Refresh(false);
			return;
		}
		accel = evt.GetModifiers();
		if(!isPartialModal){
			wxCommandEvent *evt= new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, item->id);
			evt->SetClientData(item);
			evt->SetInt(accel);
			//AddPendingEvent(evt);
			wxQueueEvent(GetParent(),evt);
		}
		id = item->id;
		if(isPartialModal){EndPartialModal(id);}
		else{HideWithEffect(wxSHOW_EFFECT_BLEND ,200);parent->DestroyDialog();}//Destroy();}
	}
}
	
void MenuDialog::OnPaint(wxPaintEvent &event)
{
	int itemsize = parent->items.size();
	if(scPos>=itemsize-maxVisible){scPos=itemsize-maxVisible;}
	if(scPos<0){scPos=0;}
	int maxsize=itemsize;
	if(itemsize>maxVisible){
		maxsize=maxVisible;
		//wxLogStatus("setScrollbar %i %i %i", scPos, maxVisible, itemsize);
		SetScrollbar(wxVERTICAL, scPos, maxVisible, itemsize);
	}
	int w=0;
	int h=0;
	GetClientSize (&w, &h);
	if(w==0||h==0){return;}
	wxMemoryDC tdc;
	if (bmp && (bmp->GetWidth() < w || bmp->GetHeight() < h)) {
		delete bmp;
		bmp = NULL;
	}
	if(!bmp){bmp=new wxBitmap(w,h);}
	tdc.SelectObject(*bmp);
	wxBitmap checkbmp = wxBITMAP_PNG("check");
	wxBitmap dot = wxBITMAP_PNG("dot");
	wxColour highlight = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
	wxColour text = wxSystemSettings::GetColour(wxSYS_COLOUR_MENUTEXT);
	wxColour graytext = wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);
	tdc.SetFont(font);
	tdc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU)));
	tdc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_MENU)));
	tdc.DrawRectangle(0,0,w,h);
	tdc.SetTextForeground(text);
	//wxLogStatus("refresh");
	//int visible=25;//500/fh fh=20;
	
	bool noRadio=true;
	for(int i=0;i<maxsize; i++)
	{
		int scrollPos=i+scPos;
		MenuItem *item=parent->items[scrollPos];//+scPos
		if(item->type==ITEM_SEPARATOR){
			tdc.SetPen(wxPen("#FFFFFF"));
			tdc.SetBrush(wxBrush("#000000"));
			tdc.DrawRectangle(24,height*i+10,w-4,1);
			noRadio=true;
			continue;
		}
		if(scrollPos==sel){
			tdc.SetPen(wxPen("#000000"));
			tdc.SetBrush(wxBrush(highlight));
			tdc.DrawRectangle(0, height*i,w,height);
		}
		tdc.SetPen(wxPen("#497CB0",2));
		tdc.SetBrush(*wxTRANSPARENT_BRUSH);
		
		
		if(item->type==ITEM_CHECK && item->check){
			tdc.DrawBitmap(checkbmp,4,(height*i)+2);
		}else if(item->type==ITEM_RADIO && noRadio){
			tdc.DrawBitmap(dot,4,(height*i)+2);
			noRadio=false;
		}else if(item->icon){
			tdc.DrawBitmap(item->GetBitmap(),4,(height*i)+2);
		}
		wxString desc=item->GetLabel();
		tdc.SetTextForeground((item->enabled)? text : graytext);
		desc.Replace("&","");
		int find=desc.find("\t");
		tdc.SetPen(wxPen("#497CB0"));
		if (find!= -1 ){
			int fw, fhh;
			wxString accel=desc.AfterLast('\t');
			//wxLogStatus("accel "+accel);
			tdc.GetTextExtent(accel, &fw, &fhh);
			tdc.DrawText(accel,w-fw-20,(height*i)+2);
			desc=desc.BeforeLast('\t');
		}
		//wxLogStatus("desc1 "+label);
		tdc.DrawText(desc,24,(height*i)+2);
		
		if(item->submenu){
			wxPoint points[3];
			int pos = w-10;
			int pos1= (height*i)+12;
			points[0]=wxPoint(pos,pos1-6);
			points[1]=wxPoint(pos,pos1);
			points[2]=wxPoint(pos+4,pos1-3);
			tdc.SetBrush(wxBrush(text));
			tdc.SetPen(wxPen(graytext));
			tdc.DrawPolygon(3,points);
		}

	}
	wxPaintDC dc(this);
	dc.Blit(0,0,w,h,&tdc,0,0);
}
	
void MenuDialog::OnScroll(wxScrollWinEvent& event)
{
	int newPos=0;
	int tsize=parent->items.size();
	if(event.GetEventType()==wxEVT_SCROLLWIN_LINEUP)
	{
		newPos=scPos-1;
		if(newPos<0){newPos=0;return;}
	}
	else if(event.GetEventType()==wxEVT_SCROLLWIN_LINEDOWN)
	{
		newPos=scPos+1;
		//if(newPos>=tsize){newPos=tsize-1;return;}
	}
	else if(event.GetEventType()==wxEVT_SCROLLWIN_PAGEUP)
	{
		wxSize size=GetClientSize();
		newPos=scPos;
		newPos-=(size.y/height - 1);
		newPos=MAX(0,newPos);
	}
	else if(event.GetEventType()==wxEVT_SCROLLWIN_PAGEDOWN)
	{
		wxSize size=GetClientSize();
		newPos=scPos;
		newPos+=(size.y/height - 1);
		newPos=MIN(newPos,tsize-1);
	}
	else{newPos = event.GetPosition();}
	if (scPos != newPos) {
		scPos = newPos;
		Refresh(false);
	}
}

void MenuDialog::OnKillFocus(wxFocusEvent &evt)
{
	//wxLogStatus("KillFocus");
	//bool hasfocus = false;
	//if(blockHideDialog){blockHideDialog=false; return;}
	wxWindow * focused = FindFocus();
	if(focused && focused->IsKindOf(wxCLASSINFO(wxDialog))){return;}
	//submenuPos = -1;
	//wxLogStatus("no focus");

	int subMenu=ParentMenu->submenuShown;
	//wxLogStatus("submenu");
	Menu *menu=ParentMenu->parent;
	ParentMenu->GetParent()->SetFocus();
	//wxLogStatus("submenu1");
	if(ParentMenu->isPartialModal){/*wxLogStatus("partial");*/ParentMenu->EndPartialModal(id);}
	else{/*wxLogStatus("no partial");*/menu->dialog->HideWithEffect(wxSHOW_EFFECT_BLEND ,100);menu->DestroyDialog();}
	while(subMenu!= -1 && menu->items[subMenu]->submenu){
		menu = menu->items[subMenu]->submenu;
		//wxLogStatus("menu %i, %i", subMenu, (int)menu);
		if(menu->dialog){subMenu = menu->dialog->submenuShown;}
		else{subMenu = -1;}
		menu->dialog->HideWithEffect(wxSHOW_EFFECT_BLEND ,100);
		menu->DestroyDialog();
	}
	ParentMenu=NULL;
}
//void MenuDialog::OnLostCapture(wxMouseCaptureLostEvent &evt)
//{
//	//if(HasCapture()){ReleaseCapture();}
//	//if(IsModal()){}EndModal(-3);}
//	//else{Hide(); id=-3;}
//	if(isPartialModal){EndPartialModal(id);}
//}
void MenuDialog::OnLostCapture(wxMouseCaptureLostEvent &evt){
	if(HasCapture()){ReleaseMouse();}
	if(isPartialModal){EndPartialModal(id);}
	else{HideWithEffect(wxSHOW_EFFECT_BLEND ,200);parent->DestroyDialog();}
}

int MenuDialog::ShowPartialModal()
{
	isPartialModal=true;
	//Show();
	ShowWithEffect(wxSHOW_EFFECT_BLEND ,200);//wxSHOW_EFFECT_BLEND
	Refresh(false);
	//SetFocus();
	if(IsShown()){
		Run();
	}
	return id;
}
	
void MenuDialog::EndPartialModal(int ReturnId)
{
	Exit(ReturnId);
	HideWithEffect(wxSHOW_EFFECT_BLEND ,100);
}


BEGIN_EVENT_TABLE(MenuDialog, wxDialog)
	EVT_MOUSE_EVENTS(MenuDialog::OnMouseEvent)
	EVT_PAINT(MenuDialog::OnPaint)
	EVT_MOUSE_CAPTURE_LOST(MenuDialog::OnLostCapture)
	//EVT_KILL_FOCUS(MenuDialog::OnKillFocus)
	EVT_SCROLLWIN(MenuDialog::OnScroll)
END_EVENT_TABLE()
static int menuIndent = 20;
static int halfIndent = 10;
MenuBar *MenuBar::Menubar = NULL;

MenuBar::MenuBar(wxWindow *_parent)
	:wxWindow(_parent, -1, wxDefaultPosition, wxSize(-1,20))
	,parent(_parent)
	,bmp(NULL)
	,sel(-1)
	,clicked(false)
	,oldelem(-1)
	,shownMenu(-1)
{
	SetFont(font);
	Refresh(false);
	Bind(wxEVT_TIMER,[=](wxTimerEvent &event){
		if(shownMenu == -1){return;}
		MenuEvent evt(EVT_MENU_OPENED, GetId(), Menus[shownMenu]);
		ProcessEvent(evt);
		wxSize rc= GetClientSize();
		int posX=halfIndent;
		for(int i = 0; i < shownMenu; i++){
			Menu *menu = Menus[i];
			wxString desc=menu->GetTitle();
			desc.Replace("&","");
			wxSize te = GetTextExtent(desc);
			posX += te.x + menuIndent;
		}
		wxPoint pos1(posX, rc.y); 
		Menus[shownMenu]->PopupMenu(pos1, this);
	},56432);
	showMenuTimer.SetOwner(this, 56432);
	Menubar=this;
}

void MenuBar::Append(Menu *menu, const wxString &title)
{
	menu->SetTitle(title);
	Menus.push_back(menu);
}
	
void MenuBar::Prepend(Menu *menu, const wxString &title)
{
	menu->SetTitle(title);
	Menus.insert(Menus.begin(), menu);
}
	
void MenuBar::Insert(int position, Menu *menu, const wxString &title)
{
	menu->SetTitle(title);
	Menus.insert(Menus.begin()+position, menu);
}

void MenuBar::OnMouseEvent(wxMouseEvent &evt)
{
	if(evt.LeftUp()){clicked = false; Refresh(false);}
	
	wxPoint pos = evt.GetPosition();
	int elem = CalcMousePos(&pos);
	if(evt.Leaving()){
		sel=-1; Refresh(false); 
		return;
	}
	if(evt.Entering()){
		if(shownMenu!=-1 && Menus[shownMenu]->dialog==NULL){shownMenu=-1;}
		oldelem=-1;
		if(shownMenu == elem){oldelem=sel=elem;Refresh(false);return;}
	}
	if(elem != oldelem && elem != -1){
		oldelem=elem;
		sel=elem;
		Refresh(false);
		if(shownMenu!=-1){
			Menus[shownMenu]->DestroyDialog();
			/*wxSize rc= GetClientSize();
			wxPoint pos1(pos.x, rc.y); 
			Menus[elem]->PopupMenu(pos1, this);*/
			//
			shownMenu=elem;
			//menuIsShown=true;
			//wxLogStatus("wredna p�lta wesz�a");
			//oldelem=elem;
			//MenuEvent evt(EVT_MENU_OPENED, GetId(), Menus[shownMenu]);
			//ProcessEvent(evt);
			//ShowMenu();
			showMenuTimer.Start(500,true);
			//shownMenu=-1;
			
		}
		
	}
	
	if(evt.LeftDown() && elem>=0){
		clicked = true;
		Refresh(false);
		shownMenu=elem;
		//menuIsShown=true;
		//MenuEvent evt(EVT_MENU_OPENED, GetId(), Menus[shownMenu]);
		//ProcessEvent(evt);
		//ShowMenu();
		showMenuTimer.Start(10,true);
	}
	
}
	
void MenuBar::OnPaint(wxPaintEvent &event)
{
	int w=0;
	int h=0;
	GetClientSize (&w, &h);
	if(w==0||h==0){return;}
	//wxLogStatus("rysowanie ");
	wxMemoryDC tdc;
	if (bmp && (bmp->GetWidth() < w || bmp->GetHeight() < h)) {
		delete bmp;
		bmp = NULL;
	}
	if(!bmp){bmp=new wxBitmap(w,h);}
	tdc.SelectObject(*bmp);
	tdc.SetFont(font);
	tdc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_MENUBAR)));
	tdc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_MENUBAR)));
	tdc.DrawRectangle(0,0,w,h);
	tdc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_MENUTEXT));
	int posX=halfIndent;
	for(size_t i = 0; i < Menus.size(); i++){
		Menu *menu = Menus[i];
		wxString desc=menu->GetTitle();
		desc.Replace("&","");
		wxSize te = tdc.GetTextExtent(desc);
		//wxLogStatus("desc %i"+desc, posX);
		if(i == sel){
			tdc.SetBrush(wxBrush(wxSystemSettings::GetColour(clicked? wxSYS_COLOUR_BTNFACE : wxSYS_COLOUR_HIGHLIGHT)));
			tdc.SetPen(wxPen(wxSystemSettings::GetColour(clicked? wxSYS_COLOUR_BTNSHADOW : wxSYS_COLOUR_HIGHLIGHT)));
			tdc.DrawRoundedRectangle(posX-2, 2, te.x+4, h-4, 2.0);
			tdc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_MENUTEXT)));
		}
		tdc.DrawText(desc,posX,2);//(h-te.y)/2.0
		posX += te.x + menuIndent;
	}

	wxPaintDC dc(this);
	dc.Blit(0,0,w,h,&tdc,0,0);
}

int MenuBar::CalcMousePos(wxPoint *pos)
{
	int posX=0;
	for(size_t i = 0; i < Menus.size(); i++){
		Menu *menu = Menus[i];
		wxString desc=menu->GetTitle();
		desc.Replace("&","");
		wxSize te = GetTextExtent(desc);
		//posX += halfIndent;
		//wxLogStatus("te  %i %i %i %i, pos->x > posX %i ,%i", i, te.x, pos->x, posX, (int)(pos->x > posX), (int)(pos->x < (posX + te.x + halfIndent)));
		if(pos->x > posX && (posX + te.x + menuIndent) > pos->x){
			pos->x = posX, pos->y = posX + te.x + halfIndent;
			//wxLogStatus("wesz�o  %i %i %i %i", i, te.x, pos->x, posX);
			return i;
		}
		posX += te.x + menuIndent;
	}
	return -1;
}

MenuItem *MenuBar::FindItem(int id)
{
	MenuItem *item=NULL;
	for(auto menu : Menus){
		item = menu->FindItem(id);
		if (item) {return item;}
	}
	return NULL;
}
	
void MenuBar::Enable(int id, bool enable)
{
	MenuItem * item = FindItem(id);
	if(item){item->Enable(enable);}
	else{wxLogStatus("Cannot enable item with id %i", id);}
}

void MenuBar::AppendAccelerators(std::vector <wxAcceleratorEntry> *entries)
{
	for(size_t i = 0; i<Menus.size(); i++){
		Menus[i]->GetAccelerators(entries);
	}
}

BEGIN_EVENT_TABLE(MenuBar, wxWindow)
	EVT_MOUSE_EVENTS(MenuBar::OnMouseEvent)
	EVT_PAINT(MenuBar::OnPaint)
END_EVENT_TABLE()