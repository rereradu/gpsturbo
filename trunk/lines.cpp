/*********************************************************************************/
/* GPSTurbo                                                                      */
/*                                                                               */
/* Programmed by Kevin Pickell                                                   */
/*                                                                               */
/* http://www.scale18.com/cgi-bin/page/gpsturbo.html                             */
/*                                                                               */
/*    GPSTurbo is free software; you can redistribute it and/or modify           */
/*    it under the terms of the GNU General Public License as published by       */
/*    the Free Software Foundation; version 2.                                   */
/*                                                                               */
/*    GPSTurbo is distributed in the hope that it will be useful,                 */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of             */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              */
/*    GNU General Public License for more details.                               */
/*                                                                               */
/*    http://www.gnu.org/licenses/gpl.txt                                        */
/*                                                                               */
/*    You should have received a copy of the GNU General Public License          */
/*    along with GPSTurbo; if not, write to the Free Software                    */
/*    Foundation, Inc->, 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */
/*                                                                               */
/*********************************************************************************/

#include "gpsturbo.h"

class GPXLineEntry
{
	friend class LinesPage;
public:
	kGUIString m_type;
	kGUIString m_value1;
	kGUIString m_value2;
	void Load(kGUIXMLItem *xml);
	void Save(kGUIXMLItem *xml);
};

class GPXLine
{
	friend class LinesPage;
public:
	GPXLine(kGUIString *name) {m_name.SetString(name);m_draw=true;m_numentries=0;m_entries.SetGrow(true);m_entries.Alloc(16);}
	~GPXLine();
	void SetName(kGUIString *name) {m_name.SetString(name);}
	const char *GetName(void) {return m_name.GetString();}
	void SetDraw(bool d) {m_draw=d;}
	bool GetDraw(void) {return m_draw;}
	unsigned int GetNumEntries(void) {return m_numentries;}
	void Load(kGUITableObj *table,kGUITickBoxObj *draw);
	bool Compare(kGUITableObj *table,kGUITickBoxObj *draw);
	void Save(kGUITableObj *table,kGUITickBoxObj *draw);
	void Load(kGUIXMLItem *xml);
	void Save(kGUIXMLItem *xml);
	bool LineRow(GPXRow *trow);
	GPXLineEntry *AddEntry(void) {GPXLineEntry *fe=new GPXLineEntry();m_entries.SetEntry(m_numentries++,fe);return fe;}
	GPXBounds *GetBoundsObj(void) {return &m_bounds;}
private:
	kGUIString m_name;
	bool m_draw;
	unsigned int m_numentries;
	Array<GPXLineEntry *>m_entries;
	GPXBounds m_bounds;
};

enum
{
LINECOL_TYPE,
LINECOL_VALUE1,
LINECOL_VALUE2,
LINECOL_NUMCOLUMNS};

enum
{
LINEPRIM_WAYPOINT,
LINEPRIM_POINT,
LINEPRIM_PROJECT,
LINEPRIM_HEADING,
LINEPRIM_ADDHEADING,
LINEPRIM_DISTANCE,
LINEPRIM_RADIUS,
LINEPRIM_COLOR,
LINEPRIM_THICKNESS,
LINEPRIM_LINE,
LINEPRIM_ARC,
LINEPRIM_CIRCLE,
LINEPRIM_SETWAYPOINT,
LINEPRIM_DEGREES,
LINEPRIM_NUM};

enum
{
ARC_DISTANCE,
ARC_DEGREES
};

const char *liprimnames[LINEPRIM_NUM]={
	"Waypoint","Point","Project","Heading","Add to Heading","Distance","Radius","Color","Thickness","Line","Arc","Circle","Set Waypoint","Degrees"};

const char *licolnames[LINECOL_NUMCOLUMNS]={
	"Type","Value1","Value2"};

const char *licoldesc[LINECOL_NUMCOLUMNS]={
	"Type","Value1","Value2"};
	
const int stcolwidths[LINECOL_NUMCOLUMNS]={150,250,250};

enum
{
USE_COMBO,
USE_INPUT,
USE_NONE
};

class GPXLineRow : public kGUITableRowObj
{
public:
	GPXLineRow();
	int GetNumObjects(void) {return LINECOL_NUMCOLUMNS;}
	kGUIObj **GetObjectList(void) {return m_objectlist;} 
	kGUIObj *m_objectlist[LINECOL_NUMCOLUMNS];
	CALLBACKGLUEPTR(GPXLineRow,TypeChangedEvent,kGUIEvent);
	void TypeChangedEvent(kGUIEvent *event) {if(event->GetEvent()==EVENT_AFTERUPDATE)TypeChanged();}
	void TypeChanged(void);
	int GetType(void) {return m_type.GetSelection();}
	const char *GetTypeString(void) {return m_type.GetSelectionString();}
	kGUIString *GetValue1(void) {return m_use1==USE_COMBO?m_combo1.GetSelectionStringObj():&m_input1;}
	kGUIString *GetValue2(void) {return m_use2==USE_COMBO?m_combo2.GetSelectionStringObj():&m_input2;}
	void SetType(int v) {m_type.SetSelection(v);TypeChanged();}
	void SetType(const char *v) {m_type.SetSelection(v);TypeChanged();}
	void SetValue1(const char *v) {if(m_use1==USE_COMBO)m_combo1.SetSelectionz(v);else m_input1.SetString(v);}
	void SetValue2(const char *v) {if(m_use2==USE_COMBO)m_combo2.SetSelectionz(v);else m_input2.SetString(v);}

	void SetPos(GPXCoord *c) {m_curpos.Set(c);}
	GPXCoord *GetPos(void) {return &m_curpos;}
	kGUIString *GetSettingsObj(void) {return &m_settings;}
private:
	kGUIComboBoxObj m_type;
	int m_use1;
	kGUIComboBoxObj m_combo1;
	kGUIInputBoxObj m_input1;
	kGUITextObj m_text1;
	int m_use2;
	kGUIInputBoxObj m_input2;
	kGUIComboBoxObj m_combo2;
	kGUITextObj m_text2;
	GPXCoord m_curpos;				/* current position at this point, used for double clicking on row */
	kGUIString m_settings;
};

LinesPage::LinesPage()
{
	m_numlines=0;
	m_lines.Init(8,2);
}

LinesPage::~LinesPage()
{
}

/* add gui items to their parent gui object */
void LinesPage::Init(kGUIContainerObj *obj)
{
	int i,y,bw,bh;
	kGUIText *t;

	bw=obj->GetChildZoneW();

	m_editlinecontrols.SetSize(bw,20);

	m_lineup.SetFontSize(11);
	m_lineup.SetSize(30,25);
	m_lineup.SetString("Up");
	m_lineup.SetEventHandler(this,CALLBACKNAME(ClickUp));
	m_editlinecontrols.AddObject(&m_lineup);

	m_linedown.SetFontSize(11);
	m_linedown.SetSize(30,25);
	m_linedown.SetString("Dn");
	m_linedown.SetEventHandler(this,CALLBACKNAME(ClickDown));
	m_editlinecontrols.AddObject(&m_linedown);

	m_editlinelist.SetSize(300,20);
	m_editlinelist.SetNumEntries(1);				/* list of Lines */
	m_editlinelist.SetEntry(0,"New Line",0);
	m_editlinelist.SetEventHandler(this,CALLBACKNAME(LoadEvent));
	m_editlinecontrols.AddObject(&m_editlinelist);

	m_saveline.SetSize(100,25);
	m_saveline.SetString("Save Lines");
	m_saveline.SetEventHandler(this,CALLBACKNAME(ClickSave));
	m_editlinecontrols.AddObject(&m_saveline);

	m_undoline.SetSize(100,25);
	m_undoline.SetString("Undo Changes");
	m_undoline.SetEventHandler(this,CALLBACKNAME(ClickUndo));
	m_editlinecontrols.AddObject(&m_undoline);

	m_deleteline.SetSize(100,25);
	m_deleteline.SetString("Delete Lines");
	m_deleteline.SetEventHandler(this,CALLBACKNAME(ClickDelete));
	m_editlinecontrols.AddObject(&m_deleteline);

	m_renameline.SetSize(100,25);
	m_renameline.SetString("Rename Lines");
	m_renameline.SetEventHandler(this,CALLBACKNAME(ClickRename));
	m_editlinecontrols.AddObject(&m_renameline);

	m_copyline.SetSize(100,25);
	m_copyline.SetString("Copy Lines");
	m_copyline.SetEventHandler(this,CALLBACKNAME(ClickCopy));
	m_editlinecontrols.AddObject(&m_copyline);

	m_deleteline.SetEnabled(false);
	m_renameline.SetEnabled(false);
	m_copyline.SetEnabled(false);

	m_editlinecontrols.NextLine();

	m_drawlinecaption.SetPos(0,0);
	m_drawlinecaption.SetFontSize(SMALLCAPTIONSIZE);
	m_drawlinecaption.SetFontID(SMALLCAPTIONFONT);
	m_drawlinecaption.SetString("Draw Lines on Map");
	m_drawline.SetPos(0,15);
	m_drawline.SetEventHandler(this,CALLBACKNAME(ChangedEvent));
	m_editlinecontrols.AddObjects(2,&m_drawlinecaption,&m_drawline);

	m_settingscaption.SetPos(0,0);
	m_settingscaption.SetFontSize(SMALLCAPTIONSIZE);
	m_settingscaption.SetFontID(SMALLCAPTIONFONT);
	m_settingscaption.SetString("Settings after cursor line");
	m_settings.SetLocked(true);
	m_settings.SetPos(0,15);
	m_settings.SetSize((m_editlinecontrols.GetChildZoneW()-m_editlinecontrols.GetCurrentX())-32,20);
	m_editlinecontrols.AddObjects(2,&m_settingscaption,&m_settings);

	obj->AddObject(&m_editlinecontrols);
	
	y=m_editlinecontrols.GetZoneH();
	bh=(obj->GetChildZoneH()-y);

	m_linetable.SetPos(0,y);
	m_linetable.SetSize(bw,bh);

	m_linetable.SetNumCols(LINECOL_NUMCOLUMNS);
	for(i=0;i<LINECOL_NUMCOLUMNS;++i)
	{
		t=m_linetable.GetColHeaderTextPtr(i);
		t->SetFontID(1);	/* bold */

		m_linetable.SetColTitle(i,licolnames[i]);
		m_linetable.SetColHint(i,licoldesc[i]);
		m_linetable.SetColWidth(i,stcolwidths[i]);
	}

	m_linetable.SetAllowAddNewRow(true);
	m_linetable.SetEventHandler(this,CALLBACKNAME(TableEvent));

	m_addmenu.SetNumEntries(2);
	m_addmenu.SetEntry(0,"Insert entry before row",0);
	m_addmenu.SetEntry(1,"Insert entry after row",1);
	m_addmenu.SetEventHandler(this,CALLBACKNAME(NewEntryMenuDone));

	obj->AddObject(&m_editlinecontrols);
	obj->AddObject(&m_linetable);
}

void LinesPage::Resize(int changey)
{
	m_linetable.SetZoneH(m_linetable.GetZoneH()+changey);
}

void LinesPage::TableEvent(kGUIEvent *event)
{
	switch(event->GetEvent())
	{
	case EVENT_AFTERUPDATE:
		Changed();
	break;
	case EVENT_ADDROW:
	{
		GPXLineRow *fr=new GPXLineRow();
		m_linetable.AddRow(fr);
	}
	break;
	case EVENT_ROW_RIGHTCLICK:
		m_addmenu.Activate(kGUI::GetMouseX(),kGUI::GetMouseY());
	break;
	case EVENT_ROW_LEFTDOUBLECLICK:
	{
		GPXCoord c;
		GPXLineRow *row;

		row=static_cast<GPXLineRow *>(m_linetable.GetChild(event->m_value[0].ui));
		c.Set(row->GetPos());
		gpx->SetScrollCenter(&c);
		gpx->SetCurrentTab(TAB_MAIN);
	}
	break;
	}

	/* if a row has been changed, added or deleted or cursor moved */
	switch(event->GetEvent())
	{
	case EVENT_AFTERUPDATE:
	case EVENT_ADDROW:
	case EVENT_DELETEROW:
	case EVENT_MOVED:
	{
		GPXLineRow *row;

		/* when the user changes the cursor position, then we update the "settings" box */
		/* to display the current values after the line is processed */

		if(m_linetable.GetCursorRow()<m_linetable.GetNumChildren())
		{
			row=static_cast<GPXLineRow *>(m_linetable.GetChild(m_linetable.GetCursorRow()));
			m_settings.SetString(row->GetSettingsObj());
		}
		else
			m_settings.Clear();
	}
	break;
	}
}


void LinesPage::Purge(void)
{
	unsigned int i;
	m_linetable.DeleteChildren();

	for(i=0;i<m_numlines;++i)
        delete m_lines.GetEntry(i);
}

void LinesPage::LoadPrefs(kGUIXMLItem *root)
{
	unsigned int i;
	kGUIXMLItem *item;

	item=root->Locate("linestable");
	if(item)
		m_linetable.LoadConfig(item);

	for(i=0;i<root->GetNumChildren();++i)
	{
		item=root->GetChild(i);
		if(!strcmp(item->GetName(),"line"))
		{
			GPXLine *s=new GPXLine(item->Locate("name")->GetValue());
			m_lines.SetEntry(m_numlines++,s);
			s->Load(item);
			UpdateBounds(s,s->GetBoundsObj());
		}
	}
	UpdateList();
}

void LinesPage::SavePrefs(kGUIXMLItem *root)
{
	unsigned int i;

	m_linetable.SaveConfig(root,"linestable");

	for(i=0;i<m_numlines;++i)
		m_lines.GetEntry(i)->Save(root);
}

void LinesPage::NewEntryMenuDone(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_SELECTED)
	{
		GPXLineRow *fr;

		switch(m_addmenu.GetSelection())
		{
		case 0:	/* insert before current row */
			fr=new GPXLineRow();
			m_linetable.AddRow(fr,-1);
		break;
		case 1:	/* insert after  before current row */
			fr=new GPXLineRow();
			m_linetable.AddRow(fr,1);
		break;
		}
	}
}

GPXLineRow::GPXLineRow()
{
	int e;

	m_objectlist[LINECOL_TYPE]=&m_type;
	m_type.SetSize(300,20);
	m_type.SetNumEntries(LINEPRIM_NUM);
	for(e=0;e<LINEPRIM_NUM;++e)
		m_type.SetEntry(e,liprimnames[e],e);

	m_type.SetEventHandler(this,CALLBACKNAME(TypeChangedEvent));

	m_combo1.SetSize(300,20);
	m_combo2.SetSize(300,20);

	/* place this in table for unused fields */
	m_text1.SetBGColor(DrawColor(128,128,128));
	m_text2.SetBGColor(DrawColor(128,128,128));

	TypeChanged();
}

void GPXLineRow::TypeChanged(void)
{
	/* switch the two table columns between input boxes and comboboxes */
	/* based on the type in the first column */

	m_use1=USE_NONE;
	m_use2=USE_NONE;

	switch(m_type.GetSelection())
	{
	case LINEPRIM_HEADING:
	case LINEPRIM_ADDHEADING:
		m_use1=USE_COMBO;
		m_use2=USE_INPUT;
		m_combo1.SetNumEntries(2);
		m_combo1.SetEntry(0,"Degrees",0);
		m_combo1.SetEntry(1,"Radians",1);
		m_combo1.SetHint("Heading type");
		m_input2.SetHint("Heading angle in degrees");
	break;
	case LINEPRIM_DISTANCE:
		m_use1=USE_COMBO;
		m_use2=USE_INPUT;
		GPX::InitDistCombo(&m_combo1);
		m_combo1.SetHint("Distance type");
		m_input2.SetHint("Distance in units selected");
	break;
	case LINEPRIM_RADIUS:
		m_use1=USE_COMBO;
		m_use2=USE_INPUT;
		GPX::InitDistCombo(&m_combo1);
		m_combo1.SetHint("Radius type");
		m_input2.SetHint("Radius in units selected");
	break;
	case LINEPRIM_DEGREES:
		m_use1=USE_INPUT;
		m_input1.SetLocked(false);
		m_input1.SetHint("Set arc distance in degrees");
	break;
	case LINEPRIM_COLOR:
		m_use1=USE_COMBO;
		GPX::InitColorCombo(&m_combo1);
		m_combo1.SetHint("Color");
	break;
	case LINEPRIM_WAYPOINT:
	case LINEPRIM_SETWAYPOINT:
		m_use1=USE_INPUT;
		m_input1.SetLocked(false);
		m_input1.SetHint("Waypoint Name");
	break;
	case LINEPRIM_POINT:
		m_use1=USE_INPUT;
		m_use2=USE_INPUT;
		m_input1.SetLocked(false);
		m_input1.SetHint("Latitude");
		m_input2.SetLocked(false);
		m_input2.SetHint("Longitude");
	break;
	case LINEPRIM_THICKNESS:
		m_use1=USE_INPUT;
		m_input1.SetLocked(false);
		m_input1.SetHint("Thickness for drawing");
	break;
	case LINEPRIM_ARC:
		m_use1=USE_COMBO;
		m_use2=USE_COMBO;
		m_combo1.SetNumEntries(2);
		m_combo1.SetEntry(0,"Clockwise",0);
		m_combo1.SetEntry(1,"Counterclockwise",1);
		m_combo1.SetHint("Direction");
		m_combo2.SetNumEntries(2);
		m_combo2.SetEntry(0,"Distance",ARC_DISTANCE);
		m_combo2.SetEntry(1,"Degrees",ARC_DEGREES);
		m_combo2.SetHint("Arc distance type");
	break;
	case LINEPRIM_LINE:
	case LINEPRIM_PROJECT:
	case LINEPRIM_CIRCLE:
	break;
	}

	switch(m_use1)
	{
	case USE_COMBO:
		m_objectlist[LINECOL_VALUE1]=&m_combo1;
	break;
	case USE_INPUT:
		m_objectlist[LINECOL_VALUE1]=&m_input1;
	break;
	case USE_NONE:
		m_objectlist[LINECOL_VALUE1]=&m_text1;
		m_input1.Clear();
	break;
	}

	switch(m_use2)
	{
	case USE_COMBO:
		m_objectlist[LINECOL_VALUE2]=&m_combo2;
	break;
	case USE_INPUT:
		m_objectlist[LINECOL_VALUE2]=&m_input2;
	break;
	case USE_NONE:
		m_objectlist[LINECOL_VALUE2]=&m_text2;
		m_input2.Clear();
	break;
	}
	
	//tell table code to re-position these cells
	CellChanged();
}

/* load from prefs file */
void GPXLineEntry::Load(class kGUIXMLItem *wpr)
{
	m_type.SetString(wpr->Locate("type")->GetValue());
	m_value1.SetString(wpr->Locate("value1")->GetValue());
	m_value2.SetString(wpr->Locate("value2")->GetValue());
}

/* save to prefs file */
void GPXLineEntry::Save(class kGUIXMLItem *wpr)
{
	wpr->AddParm("type",m_type.GetString());
	wpr->AddParm("value1",m_value1.GetString());
	wpr->AddParm("value2",m_value2.GetString());
}

void LinesPage::Load(void)
{
	int fnum;

	fnum=m_editlinelist.GetSelection();
	if(!fnum)
	{
		m_drawline.SetSelected(false);
		m_linetable.DeleteChildren();
	}
	else
		m_lines.GetEntry(fnum-1)->Load(&m_linetable,&m_drawline);
	Changed();
}

/* update the save/undo button status based on if the filter has changed */

void LinesPage::Changed(void)
{
	bool changed=false;

	UpdateBounds(0,&m_curbounds);

	if(!m_editlinelist.GetSelection())
	{
		/* should be empty, so any rows means changed */
		if(m_linetable.GetNumChildren(0))
			changed=true;
		m_deleteline.SetEnabled(false);
		m_renameline.SetEnabled(false);
		m_copyline.SetEnabled(false);
	}
	else
	{
		int i=GetIndex(m_editlinelist.GetSelectionString());
		changed=m_lines.GetEntry(i)->Compare(&m_linetable,&m_drawline);
		m_deleteline.SetEnabled(!changed);
		m_renameline.SetEnabled(!changed);
		m_copyline.SetEnabled(!changed);
	}

	m_saveline.SetEnabled(changed);	/* enabled after editing */
	m_undoline.SetEnabled(changed); /* enabled after editing */
	m_editlinelist.SetLocked(changed);
	gpx->MapDirty();
	
	{
		kGUIEvent e;

		/* trigger my own moved event so the current settings are updated */
		e.SetEvent(EVENT_MOVED);
		TableEvent(&e);
	}
}

int LinesPage::GetIndex(const char *name)
{
	unsigned int i;
	GPXLine *s;

	for(i=0;i<m_numlines;++i)
	{
		s=m_lines.GetEntry(i);
		if(!strcmp(s->GetName(),name))
			return(i);
	}
	return(-1);
}


void LinesPage::ClickSave(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		int fnum;
		kGUIInputBoxReq *box;

		fnum=m_editlinelist.GetSelection();
		if(!fnum)
			box=new kGUIInputBoxReq(this,CALLBACKNAME(ClickSave2),"Lines Name?");
		else
			Save(m_editlinelist.GetSelectionStringObj());
	}
}

void LinesPage::ClickSave2(kGUIString *result,int closebutton)
{
	kGUIMsgBoxReq *box;

	if(result->GetLen() && closebutton==MSGBOX_OK)
	{
		if(GetIndex(result->GetString())>=0)
			box=new kGUIMsgBoxReq(MSGBOX_OK,false,"Error: name already used, try again with a different name!");
		else
			Save(result);
	}
}

void LinesPage::Save(kGUIString *name)
{
	int i;
	GPXLine *s;

	/* is this an existing line? */
	i=GetIndex(name->GetString());
	if(i>=0)
	{
		s=m_lines.GetEntry(i);
		s->Save(&m_linetable,&m_drawline);
	}
	else
	{
		/* new line */
		s=new GPXLine(name);
		s->Save(&m_linetable,&m_drawline);
		m_lines.SetEntry(m_numlines,s);
		++m_numlines;
		UpdateList();
		m_editlinelist.SetSelection(m_numlines);
	}
	Load();
}

void LinesPage::ClickDelete(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		kGUIMsgBoxReq *box;
	
		box=new kGUIMsgBoxReq(MSGBOX_YES|MSGBOX_NO,this,CALLBACKNAME(ClickDelete2),true,"Delete Lines '%s'?",m_editlinelist.GetSelectionString());
	}
}

void LinesPage::ClickDelete2(int result)
{
	int i;
	GPXLine *f;

	if(result==MSGBOX_YES)
	{
		i=GetIndex(m_editlinelist.GetSelectionString());
		if(i>=0)
		{
			f=m_lines.GetEntry(i);
			delete f;
			m_lines.DeleteEntry(i);
			--m_numlines;
			m_editlinelist.SetSelection(0);

			UpdateList();
			Load();
		}
	}
}

void LinesPage::ClickRename(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		kGUIInputBoxReq *box;
	
		box=new kGUIInputBoxReq(this,CALLBACKNAME(ClickRename2),"New Line Name?");
	}
}

void LinesPage::ClickRename2(kGUIString *result,int closebutton)
{
	kGUIString newname;
	kGUIMsgBoxReq *box;

	if(result->GetLen() && closebutton==MSGBOX_OK)
	{
		newname.SetString(result);
		if(GetIndex(newname.GetString())>=0)
			box=new kGUIMsgBoxReq(MSGBOX_OK,false,"Error: name already used!");
		else
		{
			unsigned int i;
			GPXLine *s;

			i=GetIndex(m_editlinelist.GetSelectionString());
			if(i>=0)
			{
				s=m_lines.GetEntry(i);
				s->SetName(&newname);
				/* rename track name in select combo too! */
				m_editlinelist.SetEntry(i+1,newname.GetString(),i+1);
			}
		}
	}
}

void LinesPage::ClickCopy(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		kGUIInputBoxReq *box;
	
		box=new kGUIInputBoxReq(this,CALLBACKNAME(ClickCopy2),"Copied Line Name?");
	}
}

void LinesPage::ClickCopy2(kGUIString *result,int closebutton)
{
	kGUIString copyname;
	kGUIMsgBoxReq *box;

	if(result->GetLen() && closebutton==MSGBOX_OK)
	{
		copyname.SetString(result->GetString());
		if(GetIndex(copyname.GetString())>=0)
			box=new kGUIMsgBoxReq(MSGBOX_OK,false,"Error: name already used!");
		else
			Save(&copyname);
	}
}

void LinesPage::ClickUndo(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		kGUIMsgBoxReq *box;
	
		box=new kGUIMsgBoxReq(MSGBOX_YES|MSGBOX_NO,this,CALLBACKNAME(ClickUndo2),false,"Undo Changes?");
	}
}

void LinesPage::ClickUndo2(int result)
{
	if(result==MSGBOX_YES)
		Load();
}

void LinesPage::UpdateList(void)
{
	unsigned int e;
	kGUIString editLines;

	editLines.SetString(m_editlinelist.GetSelectionString());

	m_editlinelist.SetNumEntries(m_numlines+1);			/* list of select Liness */
	m_editlinelist.SetEntry(0,"New Line",0);
	for(e=0;e<m_numlines;++e)
		m_editlinelist.SetEntry(e+1,m_lines.GetEntry(e)->GetName(),e+1);
	m_editlinelist.SetSelection(editLines.GetString());
}

GPXLine::~GPXLine()
{
	unsigned int i;

	for(i=0;i<m_numentries;++i)
		delete m_entries.GetEntry(i);
}

void GPXLine::Load(kGUITableObj *table,kGUITickBoxObj *draw)
{
	unsigned int e;
	GPXLineRow *row;
	GPXLineEntry *re;

	draw->SetSelected(GetDraw());
	table->DeleteChildren();

	for(e=0;e<m_numentries;++e)
	{
		re=m_entries.GetEntry(e);	/* line record */
		row=new GPXLineRow();
		row->SetType(re->m_type.GetString());
		row->SetValue1(re->m_value1.GetString());
		row->SetValue2(re->m_value2.GetString());
		table->AddRow(row);
	}
}

/* true=changed, false=same */

bool GPXLine::Compare(kGUITableObj *table,kGUITickBoxObj *draw)
{
	unsigned int e;
	GPXLineRow *row;
	GPXLineEntry *re;

	if(table->GetNumChildren(0)!=m_numentries)
		return(true);

	/* draw flag has changed */
	if(draw->GetSelected()!=GetDraw())
		return(true);

	for(e=0;e<m_numentries;++e)
	{
		re=m_entries.GetEntry(e);	/* line record */
		row=static_cast<GPXLineRow *>(table->GetChild(e));

		if(strcmp(re->m_type.GetString(),row->GetTypeString()))
			return(true);	/* different */

		if(strcmp(re->m_value1.GetString(),row->GetValue1()->GetString()))
			return(true);	/* different */

		if(strcmp(re->m_value2.GetString(),row->GetValue2()->GetString()))
			return(true);	/* different */
	}
	return(false);	/* same! */
}

void GPXLine::Save(kGUITableObj *table,kGUITickBoxObj *draw)
{
	unsigned int e;
	unsigned int nr;
	kGUIObj *obj;
	GPXLineRow *row;
	GPXLineEntry *re;

	SetDraw(draw->GetSelected());

	/* delete old entries if there are any? */
	for(e=0;e<m_numentries;++e)
		delete m_entries.GetEntry(e);

	nr=table->GetNumChildren(0);
	/* number of valid entries */
	m_numentries=nr;
	m_entries.Alloc(nr);
	for(e=0;e<nr;++e)
	{
		obj=table->GetChild(e);
		row=static_cast<GPXLineRow *>(obj);
		re=new GPXLineEntry();
				
		re->m_type.SetString(row->GetTypeString());
		re->m_value1.SetString(row->GetValue1());
		re->m_value2.SetString(row->GetValue2());
		m_entries.SetEntry(e,re);
	}
}

/* Load from preferences file */

void GPXLine::Load(kGUIXMLItem *xml)
{
	int i,nc;
	GPXLineEntry *fe;
	kGUIXMLItem *flxml;

	flxml=xml->Locate("draw");
	if(flxml)
		SetDraw(flxml->GetValueInt()?true:false);

	nc=xml->GetNumChildren();
	for(i=0;i<nc;++i)
	{
		flxml=xml->GetChild(i);
		if(!strcmp(flxml->GetName(),"entry"))
		{
			fe=new GPXLineEntry();
			fe->Load(flxml);
			m_entries.SetEntry(m_numentries++,fe);
		}
	}
}

/* save to preferences file */

void GPXLine::Save(kGUIXMLItem *xml)
{
	unsigned int e;
	GPXLineEntry *re;
	kGUIXMLItem *fxml;
	kGUIXMLItem *flxml;

	/* if no entries in line then don't save it */
	if(m_numentries)
	{
		fxml=xml->AddChild("line");
		fxml->AddParm("name",m_name.GetString());
		fxml->AddParm("draw",GetDraw()==true?"1":"0");

		for(e=0;e<m_numentries;++e)
		{
			re=m_entries.GetEntry(e);	/* line record */
			flxml=fxml->AddChild("entry");
			re->Save(flxml);
		}
	}
}

void LinesPage::ClickUp(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		unsigned int line;

		line=m_linetable.GetCursorRow();
		if(line>0 && line<m_linetable.GetNumChildren(0))
		{
			m_linetable.SwapRow(-1);
			m_linetable.MoveRow(-1);
			Changed();
		}
	}
}

void LinesPage::ClickDown(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		int line;

		line=(int)m_linetable.GetCursorRow();
		if(line<((int)m_linetable.GetNumChildren(0)-1))
		{
			m_linetable.SwapRow(1);
			m_linetable.MoveRow(1);
			Changed();
		}
	}
}

void LinesPage::AddPointToLine(const char *name)
{
	GPXLineRow *lr=new GPXLineRow();

	lr->SetType(LINEPRIM_WAYPOINT);
	lr->SetValue1(name);

	m_linetable.AddRow(lr);
	Changed();
}

void LinesPage::AddPointToLine(double lat,double lon)
{
	GPXLineRow *lr=new GPXLineRow();
	GPXCoord c;
	kGUIString latstring,lonstring;

	lr->SetType(LINEPRIM_POINT);
	c.Set(lat,lon);
	c.Output(&latstring,&lonstring);
	lr->SetValue1(latstring.GetString());
	lr->SetValue2(lonstring.GetString());

	m_linetable.AddRow(lr);
	Changed();
}

/* draw all line groups that are flagged to draw */
void LinesPage::DrawMap(kGUICorners *c)
{
	unsigned int i;
	GPXLine *line;

	/* draw any lines tracks that are flagged to be drawn */
	for(i=0;i<m_numlines;++i)
	{
		/* skip current track, it is drawn below using the table */
		if(i!=((unsigned int)m_editlinelist.GetSelection()-1))
		{
			line=m_lines.GetEntry(i);
			if(line->GetDraw()==true)
				Draw(c,line);
		}
	}

	/* draw current track being edited if it is to be drawn */
	if(m_drawline.GetSelected())
		Draw(c,0);
}

void LinesPage::ProcessInit(void)
{
	m_pos1.Set(0.0f,0.0f);
	m_pos2.Set(0.0f,0.0f);
	m_col=DrawColor(0,0,0);
	m_heading=0.0f;
	m_degrees=45.0f;	/* only used for arc distance */
	m_disttype=0;
	m_distance=100.0f;
	m_radiustype=0;
	m_radius=100.0f;
	m_thickness=1;
}

/* calculate positions along line to generate a bounding box, also */
/* for a table calculate position at each table entry for double clicking */
/* on the row and going to that spot on the map */

void LinesPage::UpdateBounds(GPXLine *line,GPXBounds *bounds)
{
	unsigned int i;
	int type;
	kGUIString *v1;
	kGUIString *v2;

	bounds->Init();
	if(line)
	{
		GPXLineEntry *le;

		ProcessInit();
		for(i=0;i<line->GetNumEntries();++i)
		{
			le=line->m_entries.GetEntry(i);
			type=GPX::GetIndex(le->m_type.GetString(),sizeof(liprimnames)/sizeof(char *),liprimnames);
			v1=&le->m_value1;
			v2=&le->m_value2;
			ProcessLine(0,bounds,type,v1,v2);
		}
	}
	else
	{
		GPXLineRow *row;
		kGUIString *s;

		ProcessInit();
		for(i=0;i<m_linetable.GetNumChildren();++i)
		{
			row=static_cast<GPXLineRow *>(m_linetable.GetChild(i));
			type=row->GetType();
			v1=row->GetValue1();
			v2=row->GetValue2();
			ProcessLine(0,bounds,type,v1,v2);
			row->SetPos(&m_pos2);

			/* generate the current settings for this row */
			s=row->GetSettingsObj();
			s->Sprintf("lat=%f,lon=%f,heading=%f,distance=%f,radius=%f,degrees=%f,thickness=%d",m_pos2.GetLat(),m_pos2.GetLon(),m_heading,m_distance,m_radius,m_degrees,m_thickness);
		}
	}
}

/* if track is passed, then draw using it, else draw using current stuff in the table */
void LinesPage::Draw(kGUICorners *c,GPXLine *line)
{
	unsigned int i;
	int type;
	kGUIString *v1;
	kGUIString *v2;
	kGUICorners b;
	GPXCoord pos;

	if(line)
	{
		GPXLineEntry *le;
		GPXBounds *bounds=line->GetBoundsObj();

		/* first, project bounds for lines and see if it overlaps the draw area */
		pos.Set(bounds->GetMinLat(),bounds->GetMinLon());
		gpx->m_curmap->ToMap(&pos,&b.lx,&b.by);
		pos.Set(bounds->GetMaxLat(),bounds->GetMaxLon());
		gpx->m_curmap->ToMap(&pos,&b.rx,&b.ty);

		/* return if track does not overlap draw area */
		if(kGUI::Overlap(c,&b)==false)
			return;

		ProcessInit();
		for(i=0;i<line->GetNumEntries();++i)
		{
			le=line->m_entries.GetEntry(i);
			type=GPX::GetIndex(le->m_type.GetString(),sizeof(liprimnames)/sizeof(char *),liprimnames);
			ProcessLine(c,0,type,&le->m_value1,&le->m_value2);
		}
	}
	else
	{
		GPXLineRow *row;

		/* first, project bounds for lines and see if it overlaps the draw area */
		pos.Set(m_curbounds.GetMinLat(),m_curbounds.GetMinLon());
		gpx->m_curmap->ToMap(&pos,&b.lx,&b.by);
		pos.Set(m_curbounds.GetMaxLat(),m_curbounds.GetMaxLon());
		gpx->m_curmap->ToMap(&pos,&b.rx,&b.ty);

//		kGUI::DrawLine(b.lx-c->lx,b.ty-c->ty,b.rx-c->lx,b.ty-c->ty,DrawColor(255,255,0));
//		kGUI::DrawLine(b.lx-c->lx,b.by-c->ty,b.rx-c->lx,b.by-c->ty,DrawColor(255,255,0));
//		kGUI::DrawLine(b.lx-c->lx,b.ty-c->ty,b.lx-c->lx,b.by-c->ty,DrawColor(255,255,0));
//		kGUI::DrawLine(b.rx-c->lx,b.ty-c->ty,b.rx-c->lx,b.by-c->ty,DrawColor(255,255,0));

		/* return if track does not overlap draw area */
		if(kGUI::Overlap(c,&b)==false)
			return;

		ProcessInit();
		for(i=0;i<m_linetable.GetNumChildren();++i)
		{
			row=static_cast<GPXLineRow *>(m_linetable.GetChild(i));
			type=row->GetType();
			v1=row->GetValue1();
			v2=row->GetValue2();
			ProcessLine(c,0,type,v1,v2);
		}
	}
}

void LinesPage::ProcessLine(kGUICorners *c,GPXBounds *bounds,int type,kGUIString *v1,kGUIString *v2)
{
	switch(type)
	{
	case LINEPRIM_WAYPOINT:
	{
		GPXRow *gpxrow;

		gpxrow=gpx->Locate(v1);
		if(gpxrow)
		{
			m_pos1.Set(&m_pos2);
			gpxrow->GetPos(&m_pos2);
		}
	}
	break;
	case LINEPRIM_POINT:
		m_pos1.Set(&m_pos2);
		m_pos2.Set(v1->GetString(),v2->GetString());
	break;
	case LINEPRIM_PROJECT:
		m_pos1.Set(&m_pos2);
		GPX::Project(m_distance,m_disttype,0,m_heading,0,&m_pos1,&m_pos2);
	break;
	case LINEPRIM_HEADING:
		if(!strcmp("Degrees",v1->GetString()))
			m_heading=atof(v2->GetString());
		else
			m_heading=atof(v2->GetString())*(PI/180.0f);	/* radians */
	break;
	case LINEPRIM_ADDHEADING:
		if(!strcmp("Degrees",v1->GetString()))
			m_heading+=atof(v2->GetString());
		else
			m_heading+=atof(v2->GetString())*(PI/180.0f);	/* radians */
	break;
	case LINEPRIM_DISTANCE:
		m_disttype=GPX::GetDistIndex(v1->GetString());
		m_distance=atof(v2->GetString());
	break;
	case LINEPRIM_DEGREES:
		m_degrees=atof(v1->GetString());
	break;
	case LINEPRIM_RADIUS:
		m_radiustype=GPX::GetDistIndex(v1->GetString());
		m_radius=atof(v2->GetString());
	break;
	case LINEPRIM_COLOR:
		m_col=GPX::GetTableColor(GPX::GetTableColorIndex(v1->GetString()));
	break;
	case LINEPRIM_THICKNESS:
		m_thickness=atoi(v1->GetString());
	break;
	case LINEPRIM_LINE:
		if(bounds)
		{
			bounds->Add(&m_pos1);
			bounds->Add(&m_pos2);
		}
		if(c)
		{
			int x1,y1,x2,y2;

			gpx->m_curmap->ToMap(&m_pos1,&x1,&y1);
			gpx->m_curmap->ToMap(&m_pos2,&x2,&y2);
			x1-=c->lx;
			y1-=c->ty;
			x2-=c->lx;
			y2-=c->ty;
			if(m_thickness<=1)
				kGUI::DrawLine(x1,y1,x2,y2,m_col);
			else
				kGUI::DrawFatLine(x1,y1,x2,y2,m_col,m_thickness>>1);
		}
	break;
	case LINEPRIM_ARC:
	{
		int i,num;
		kGUIPoint2 points[360];
		GPXCoord edgepos;
		double movelat,movelon;
		double sweep,step;
		int x1,y1;
		double off,dir;

		if(m_radius>0.0f)
		{
			if(!strcmp("Clockwise",v1->GetString()))
			{
				off=90.0f;
				dir=1.0f;
			}
			else
			{
				off=270.0f;
				dir=-1.0f;
			}
			/* make arc start at last position so we need to translate it over */
			GPX::Project(m_radius,m_radiustype,0,m_heading-off,0,&m_pos2,&edgepos);
			movelat=edgepos.GetLat()-m_pos2.GetLat();
			movelon=edgepos.GetLon()-m_pos2.GetLon();

			/* use distance or degrees */
			if(!strcmp("Distance",v2->GetString()))
				sweep=(GPX::GetMeters(m_distance,m_disttype)*(180.0f/PI))/GPX::GetMeters(m_radius,m_radiustype)*dir;
			else
				sweep=m_degrees*dir;

			/* stop at full circle */
			if(sweep<-360.0f)
				sweep=-360.0f;
			else if(sweep>360.0f)
				sweep=360.0f;
			num=(int)abs((int)sweep);
			if(num)
			{
				step=sweep/num;

				for(i=0;i<num;++i)
				{
					GPX::Project(m_radius,m_radiustype,0,m_heading-off,0,&m_pos2,&edgepos);
					edgepos.SetLat(edgepos.GetLat()-movelat);
					edgepos.SetLon(edgepos.GetLon()-movelon);
					if(bounds)
						bounds->Add(&edgepos);
					if(c)
					{
						gpx->m_curmap->ToMap(&edgepos,&x1,&y1);
						points[i].x=x1-c->lx;
						points[i].y=y1-c->ty;
					}
					m_heading+=step;
				}

				/* set new current position to end of arc position */
				m_pos1.Set(&m_pos2);
				m_pos2.Set(&edgepos);		

				if(c)
				{
					if(m_thickness<=1)
						kGUI::DrawPolyLine(num,points,m_col);
					else
						kGUI::DrawFatPolyLine(num,points,m_col,m_thickness);
				}
			}
		}
	}
	break;
	case LINEPRIM_CIRCLE:
	{
		int i;
		kGUIPoint2 points[360+1];
		GPXCoord edgepos;
		int x1,y1;

		for(i=0;i<=360;++i)
		{
			GPX::Project(m_radius,m_radiustype,0,(double)i,0,&m_pos2,&edgepos);
			if(bounds)
				bounds->Add(&edgepos);
			if(c)
			{
				gpx->m_curmap->ToMap(&edgepos,&x1,&y1);
				points[i].x=x1-c->lx;
				points[i].y=y1-c->ty;
			}
		}
		
		if(c)
		{
			if(m_thickness<=1)
				kGUI::DrawPolyLine(360+1,points,m_col);
			else
				kGUI::DrawFatPolyLine(360+1,points,m_col,m_thickness);
		}
	}
	break;
	case LINEPRIM_SETWAYPOINT:
	{
		GPXRow *gpxrow;

		/* set the waypoints position to the current position */
		gpxrow=gpx->Locate(v1);
		if(gpxrow)
		{
			if((gpxrow->GetLat()!=m_pos2.GetLat()) || (gpxrow->GetLon()!=m_pos2.GetLon()))
			{
				kGUIEvent e;

				gpxrow->SetCoord(m_pos2.GetLat(),m_pos2.GetLon());

				/* map bsp needs to be rebuilt */
				e.SetEvent(EVENT_AFTERUPDATE);
				gpxrow->LocChanged(&e);
			}
		}
	}
	break;
	}
}
