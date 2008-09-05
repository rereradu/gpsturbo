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
/*    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */
/*                                                                               */
/*********************************************************************************/

#include "gpsturbo.h"

class GPXFilterEntry
{
public:
	int m_fieldnum;
	int m_opnum;
	/* only uses one of these values, dependant on the field num value */
	int m_value;
	kGUIString m_valuestr;
	GPXUnit m_valuepoint;
};

class GPXFilter
{
	friend class FiltersPage;
public:
	GPXFilter(kGUIString *name) {m_name.SetString(name);m_numentries=0;m_entries.SetGrow(true);m_entries.Alloc(16);}
	~GPXFilter();
	void SetName(kGUIString *name) {m_name.SetString(name);}
	const char *GetName(void) {return m_name.GetString();}
	void Load(kGUITableObj *table);
	bool Compare(kGUITableObj *table);
	void Save(kGUITableObj *table);
	void Load(kGUIXMLItem *xml);
	void Save(kGUIXMLItem *xml);
	bool FilterRow(GPXRow *trow);
	GPXFilterEntry *AddEntry(void) {GPXFilterEntry *fe=new GPXFilterEntry();m_entries.SetEntry(m_numentries++,fe);return fe;}
private:
	kGUIString m_name;
	unsigned int m_numentries;
	Array<GPXFilterEntry *>m_entries;
};

const char *wpcolnames[GPXCOL_NUMCOLUMNS]={
	"N/A","Fnd","Type","U1","U2","U3","U4","U5","U6","U7","U8","Name",
	"WptName","LogStat","Container","Diff","Terr","Dist","Corr",
	"Lat","Lon","Hint","User Notes","Owner","State","Country","Gen Date","Near"};

const char *wpcoldesc[GPXCOL_NUMCOLUMNS]={
	"Not Available","Found","Cache Type","UserTick1","UserTick2","UserTick3","UserTick4","UserTick5","UserTick6","UserTick7","UserTick8","Cache Name",
	"Waypoint Name","Log Status","Container Type","Difficulty","Terrain","Distance to Center","Corrected",
	"Latitude","Longitude","Hint","User Notes","Owner","State","Country","Generated Date","Near"};
	
int wpcolwidths[GPXCOL_NUMCOLUMNS]={30,30,32,30,30,30,30,30,30,30,30,270,65,80,80,40,40,80,30,80,80,300,300,150,150,150,75,30};

/* these are in gpsturbo.cpp */
extern const char *cachetypenames[CACHETYPE_NUM];
extern const char *containernames[CONTAINERTYPE_NUM];

enum
{
FILTERFIELD_FOUND,
FILTERFIELD_AVAILABLE,
FILTERFIELD_USERTICKED1,
FILTERFIELD_USERTICKED2,
FILTERFIELD_USERTICKED3,
FILTERFIELD_USERTICKED4,
FILTERFIELD_USERTICKED5,
FILTERFIELD_USERTICKED6,
FILTERFIELD_USERTICKED7,
FILTERFIELD_USERTICKED8,
FILTERFIELD_CORRECTED,
FILTERFIELD_OWNER,
FILTERFIELD_TYPE,
FILTERFIELD_CONTAINER,
FILTERFIELD_DIFFICULTY,
FILTERFIELD_TERRAIN,
FILTERFIELD_NAME,
FILTERFIELD_LAT,
FILTERFIELD_LON,
FILTERFIELD_STATE,
FILTERFIELD_COUNTRY,
FILTERFIELD_POSITION,
FILTERFIELD_NUMFILTERS};

enum
{
FILTEROP_EQ,
FILTEROP_NEQ,
FILTEROP_LT,
FILTEROP_LE,
FILTEROP_GT,
FILTEROP_GE,
FILTEROP_CONTAINS,
FILTEROP_DOESNTCONTAIN,
FILTEROP_INSIDE,
FILTEROP_OUTSIDE,
FILTEROP_WEST,
FILTEROP_EAST,
FILTEROP_NORTH,
FILTEROP_SOUTH,
FILTEROP_NUMOPS};

const char *filteropnames[FILTEROP_NUMOPS]={
	"equals",
	"doesn't equal",
	"less than",
	"less than or equal",
	"greater than",
	"greater than or equal",
	"contains",
	"doesn't contain",
	"is inside",
	"is outside",
	"is west of",
	"is east of",
	"is north of",
	"is south of"};

#define OP_EQ (1<<FILTEROP_EQ)
#define OP_COMBO (1<<FILTEROP_EQ)|(1<<FILTEROP_NEQ)
#define OP_STRING (1<<FILTEROP_EQ)|(1<<FILTEROP_NEQ)|(1<<FILTEROP_CONTAINS)|(1<<FILTEROP_DOESNTCONTAIN)
#define OP_NUM (1<<FILTEROP_EQ)|(1<<FILTEROP_NEQ)|(1<<FILTEROP_GT)|(1<<FILTEROP_GE)|(1<<FILTEROP_LT)|(1<<FILTEROP_LE)
#define OP_INOUT (1<<FILTEROP_INSIDE)|(1<<FILTEROP_OUTSIDE)
#define OP_LAT (1<<FILTEROP_EQ)|(1<<FILTEROP_NEQ)|(1<<FILTEROP_NORTH)|(1<<FILTEROP_SOUTH)
#define OP_LON (1<<FILTEROP_EQ)|(1<<FILTEROP_NEQ)|(1<<FILTEROP_EAST)|(1<<FILTEROP_WEST)

enum
{
CMPTYPE_COMBO,
CMPTYPE_STRING,
CMPTYPE_NUM,
CMPTYPE_TRACKCOMBO,
CMPTYPE_COORD
};

typedef struct
{
	const char *name;
	int cmptype;
	int ops;
	unsigned int numcomboentries;
	const char **combotext;
}FF_DEF;

const char *foundtext[2]={"haven't found","found"};
const char *availabletext[2]={"not available","available"};
const char *usertickedtext[2]={"not ticked","ticked"};
const char *corrtickedtext[2]={"not corrected","is corrected"};

FF_DEF ffslist[FILTERFIELD_NUMFILTERS]={
	{"Found",CMPTYPE_COMBO,OP_EQ,sizeof(foundtext)/sizeof(char *),foundtext},
	{"Available",CMPTYPE_COMBO,OP_EQ,sizeof(availabletext)/sizeof(char *),availabletext},
	{"User1 Ticked",CMPTYPE_COMBO,OP_EQ,sizeof(usertickedtext)/sizeof(char *),usertickedtext},
	{"User2 Ticked",CMPTYPE_COMBO,OP_EQ,sizeof(usertickedtext)/sizeof(char *),usertickedtext},
	{"User3 Ticked",CMPTYPE_COMBO,OP_EQ,sizeof(usertickedtext)/sizeof(char *),usertickedtext},
	{"User4 Ticked",CMPTYPE_COMBO,OP_EQ,sizeof(usertickedtext)/sizeof(char *),usertickedtext},
	{"User5 Ticked",CMPTYPE_COMBO,OP_EQ,sizeof(usertickedtext)/sizeof(char *),usertickedtext},
	{"User6 Ticked",CMPTYPE_COMBO,OP_EQ,sizeof(usertickedtext)/sizeof(char *),usertickedtext},
	{"User7 Ticked",CMPTYPE_COMBO,OP_EQ,sizeof(usertickedtext)/sizeof(char *),usertickedtext},
	{"User8 Ticked",CMPTYPE_COMBO,OP_EQ,sizeof(usertickedtext)/sizeof(char *),usertickedtext},
	{"Corrected",CMPTYPE_COMBO,OP_EQ,sizeof(corrtickedtext)/sizeof(char *),corrtickedtext},
	{"Owner",CMPTYPE_STRING,OP_STRING,0,0},
	{"Cache Type",CMPTYPE_COMBO,OP_COMBO,CACHETYPE_NUM,cachetypenames},
	{"Container",CMPTYPE_COMBO,OP_COMBO,CONTAINERTYPE_NUM/sizeof(char *),containernames},
	{"Difficulty",CMPTYPE_NUM,OP_NUM,0,0},
	{"Terrain",CMPTYPE_NUM,OP_NUM,0,0},
	{"Name",CMPTYPE_STRING,OP_STRING,0,0},
	{"Latitude",CMPTYPE_COORD,OP_LAT,0,0},
	{"Longitude",CMPTYPE_COORD,OP_LON,0,0},
	{"State",CMPTYPE_STRING,OP_STRING,0,0},
	{"Country",CMPTYPE_STRING,OP_STRING,0,0},
	{"Position",CMPTYPE_TRACKCOMBO,OP_INOUT,0,0}};

enum
{
FILTERCOL_FIELD,
FILTERCOL_OPERATOR,
FILTERCOL_VALUE,
FILTERCOL_NUMCOLUMNS};

const char *fcolnames[FILTERCOL_NUMCOLUMNS]={
	"Field","Operator","Value"};
const int fcolwidths[GPXCOL_NUMCOLUMNS]={250,250,400};

class GPXFilterRow : public kGUITableRowObj
{
public:
	GPXFilterRow();
	GPXFilterRow(class GPXFilterEntry *e);
	~GPXFilterRow();
	void Init(void);
	int GetNumObjects(void) {return FILTERCOL_NUMCOLUMNS;}
	kGUIObj **GetObjectList(void) {return m_objectlist;} 
	kGUIObj *m_objectlist[FILTERCOL_NUMCOLUMNS];
	void UseCombo(void) {m_objectlist[FILTERCOL_VALUE]=&m_valuecombo;CellChanged();}
	void UseInput(void) {m_objectlist[FILTERCOL_VALUE]=&m_valueinput;CellChanged();}
	void GenTrackCombo(void);
	
	CALLBACKGLUEPTR(GPXFilterRow,FieldChangedEvent,kGUIEvent)
	void FieldChanged(void);
	void FieldChangedEvent(kGUIEvent *event) {if(event->GetEvent()==EVENT_AFTERUPDATE)FieldChanged();}
	//private:
	kGUIComboBoxObj m_fieldcombo;
	kGUIComboBoxObj m_opcombo;
	/* it only uses one of these depending on the field */
	kGUIComboBoxObj m_valuecombo;
	kGUIInputBoxObj m_valueinput;
};

void FiltersPage::Purge(void)
{
	unsigned int i;

	m_filteredwptable.DeleteChildren(false);
	m_table.DeleteChildren();

	for(i=0;i<m_numfilters;++i)
        delete m_filters.GetEntry(i);
}

void FiltersPage::InitControls(kGUIControlBoxObj *obj)
{
	m_filterlistcaption.SetPos(0,0);
	m_filterlistcaption.SetFontSize(SMALLCAPTIONFONTSIZE);
	m_filterlistcaption.SetFontID(SMALLCAPTIONFONT);
	m_filterlistcaption.SetString(gpx->GetString(STRING_CURRENTFILTER));

	m_filterlist.SetFontSize(BUTTONFONTSIZE);
	m_filterlist.SetPos(0,15);
	m_filterlist.SetSize(200,20);
	m_filterlist.SetNumEntries(1);				/* list of filters */
	m_filterlist.SetEntry(0,gpx->GetString(STRING_SHOWALL),0);
	m_filterlist.SetEventHandler(this,CALLBACKNAME(ReFilterEvent));
	m_filterlist.SetHint(gpx->GetString(STRING_CURRENTFILTERHINT));

	m_currentdbcaption.SetPos(200+10,0);
	m_currentdbcaption.SetFontSize(SMALLCAPTIONFONTSIZE);
	m_currentdbcaption.SetFontID(SMALLCAPTIONFONT);
	m_currentdbcaption.SetString(gpx->GetString(STRING_CURRENTDATABASE));

	m_currentdb.SetFontSize(BUTTONFONTSIZE);
	m_currentdb.SetPos(200+10,15);
	m_currentdb.SetSize(200,20);
	m_currentdb.SetNumEntries(1);				/* list of databases */
	m_currentdb.SetEntry(0,gpx->GetString(STRING_ALLDATABASES),0);
	m_currentdb.SetEventHandler(this,CALLBACKNAME(ReFilterEvent));
	m_currentdb.SetHint(gpx->GetString(STRING_CURRENTDATABASEHINT));
	obj->AddObjects(4,&m_filterlistcaption,&m_currentdbcaption,&m_filterlist,&m_currentdb);

	/* reverse tickbox */
	m_revfiltercaption.SetPos(0,0);
	m_revfiltercaption.SetFontSize(SMALLCAPTIONFONTSIZE);
	m_revfiltercaption.SetFontID(SMALLCAPTIONFONT);
	m_revfiltercaption.SetString(gpx->GetString(STRING_REVERSE));

	m_revfilter.SetPos(0,15);
	m_revfilter.SetHint(gpx->GetString(STRING_REVERSEHINT));
	m_revfilter.SetEventHandler(this,CALLBACKNAME(ReFilterEvent));
	obj->AddObjects(2,&m_revfiltercaption,&m_revfilter);

	/* quick filter search string */
	m_quickfiltercaption.SetPos(0,0);
	m_quickfiltercaption.SetFontSize(SMALLCAPTIONFONTSIZE);
	m_quickfiltercaption.SetFontID(SMALLCAPTIONFONT);
	m_quickfiltercaption.SetString(gpx->GetString(STRING_SEARCHSTRING));

	m_quickfilter.SetFontSize(BUTTONFONTSIZE);
	m_quickfilter.SetPos(0,15);
	m_quickfilter.SetHint("Enter search text for comparison to waypoint names.");
	m_quickfilter.SetSize(200,20);
	m_quickfilter.SetEventHandler(this,CALLBACKNAME(ReFilterEvent));

	obj->AddObjects(2,&m_quickfiltercaption,&m_quickfilter);

	/* results of filter */
	m_resultsfiltercaption.SetPos(0,0);
	m_resultsfiltercaption.SetFontSize(SMALLCAPTIONFONTSIZE);
	m_resultsfiltercaption.SetFontID(SMALLCAPTIONFONT);
	m_resultsfiltercaption.SetString(gpx->GetString(STRING_FILTERRESULTS));

	m_resultsfilter.SetFontSize(BUTTONFONTSIZE);
	m_resultsfilter.SetPos(0,15);
	m_resultsfilter.SetHint("Number of waypoints that match current filter.");
	m_resultsfilter.SetSize(100,20);
	m_resultsfilter.SetLocked(true);

	obj->AddObjects(2,&m_resultsfiltercaption,&m_resultsfilter);
}

kGUITableObj *FiltersPage::InitTable(kGUIContainerObj *obj,int y,int h)
{
	unsigned int i;
	kGUIText *t;
	int bw;

	bw=obj->GetChildZoneW();
	m_filteredwptable.SetPos(0,y);
	m_filteredwptable.SetSize(bw,h);
	m_filteredwptable.SetNumCols(GPXCOL_NUMCOLUMNS);
	for(i=0;i<GPXCOL_NUMCOLUMNS;++i)
	{
		t=m_filteredwptable.GetColHeaderTextPtr(i);
		t->SetFontID(1);	/* bold */

		m_filteredwptable.SetColTitle(i,wpcolnames[i]);
		m_filteredwptable.SetColHint(i,wpcoldesc[i]);
		m_filteredwptable.SetColWidth(i,wpcolwidths[i]);
	}
	m_filteredwptable.SetEventHandler(this,CALLBACKNAME(MainTableEvent));

	obj->AddObject(&m_filteredwptable);
	return(&m_filteredwptable);
}

void FiltersPage::MainTableEvent(kGUIEvent *event)
{
	switch(event->GetEvent())
	{
	case EVENT_DELETEROW:
	{
		unsigned int e;
		kGUITableRowObj *obj;

		obj=static_cast<kGUITableRowObj *>(m_filteredwptable.GetChild(event->m_value[0].ui));

		/* user manually deleted a row in the */
		/* table so remove it from the full list! */
		for(e=0;e<gpx->m_numwpts;++e)
		{
			if(obj==gpx->m_wptlist.GetEntry(e))
			{
				/* remove it from the current route if it is there too. */
				gpx->m_routes.Remove(static_cast<GPXRow *>(obj)->GetWptName());

				gpx->m_wptlist.DeleteEntry(e);
				--gpx->m_numwpts;
				
				gpx->BSPDirty();
				UpdateDBList();
				return;
			}
		}

		assert(false,"Row not found error!");

	}
	break;
	case EVENT_COL_RIGHTCLICK:
		gpx->ColClick();
	break;
	case EVENT_ROW_LEFTDOUBLECLICK:
	{
		GPXRow *obj;

		obj=static_cast<GPXRow *>(m_filteredwptable.GetChild(event->m_value[0].ui));
		
		gpx->SetScrollCenter(obj->GetCoord());
	}
	break;
	}
}


FiltersPage::FiltersPage()
{
	m_numfilters=0;
	m_filters.Init(16,4);
}

FiltersPage::~FiltersPage()
{
}

/* init filters gui items */
void FiltersPage::Init(kGUIContainerObj *obj)
{
	unsigned int i;
	int y;
	int bw=obj->GetChildZoneW();
	int bh=obj->GetChildZoneH();

	m_editcontrols.SetPos(0,0);
	m_editcontrols.SetSize(bw,20);

	m_editfilterlist.SetFontSize(BUTTONFONTSIZE);
	m_editfilterlist.SetSize(300,20);
	m_editfilterlist.SetNumEntries(1);				/* list of filters */
	m_editfilterlist.SetEntry(0,gpx->GetString(STRING_NEWFILTER),0);
	m_editfilterlist.SetEventHandler(this,CALLBACKNAME(EditFilterChangedEvent));
	m_editcontrols.AddObject(&m_editfilterlist);

	m_save.SetFontSize(BUTTONFONTSIZE);
	m_save.SetString(gpx->GetString(STRING_SAVEFILTER));
	m_save.Contain();
	m_save.SetEventHandler(this,CALLBACKNAME(ClickSave));
	m_editcontrols.AddObject(&m_save);

	m_undo.SetFontSize(BUTTONFONTSIZE);
	m_undo.SetString(gpx->GetString(STRING_UNDOCHANGES));
	m_undo.Contain();
	m_undo.SetEventHandler(this,CALLBACKNAME(ClickUndo));
	m_editcontrols.AddObject(&m_undo);

	m_delete.SetFontSize(BUTTONFONTSIZE);
	m_delete.SetString(gpx->GetString(STRING_DELETEFILTER));
	m_delete.Contain();
	m_delete.SetEventHandler(this,CALLBACKNAME(ClickDelete));
	m_editcontrols.AddObject(&m_delete);

	m_rename.SetFontSize(BUTTONFONTSIZE);
	m_rename.SetString(gpx->GetString(STRING_RENAMEFILTER));
	m_rename.Contain();
	m_rename.SetEventHandler(this,CALLBACKNAME(ClickRename));
	m_editcontrols.AddObject(&m_rename);

	m_copy.SetFontSize(BUTTONFONTSIZE);
	m_copy.SetString(gpx->GetString(STRING_COPYFILTER));
	m_copy.Contain();
	m_copy.SetEventHandler(this,CALLBACKNAME(ClickCopy));
	m_editcontrols.AddObject(&m_copy);

	obj->AddObject(&m_editcontrols);

	y=m_editcontrols.GetZoneH();
	m_table.SetPos(0,y);
	m_table.SetSize(bw,bh-y);
	m_table.SetNumCols(FILTERCOL_NUMCOLUMNS);
	for(i=0;i<FILTERCOL_NUMCOLUMNS;++i)
	{
		m_table.SetColTitle(i,fcolnames[i]);
		m_table.SetColWidth(i,fcolwidths[i]);
	}
	m_table.SetAllowAddNewRow(true);
	m_table.SetEventHandler(this,CALLBACKNAME(TableEvent));
	obj->AddObject(&m_table);
}

void FiltersPage::Resize(int changey)
{
	m_table.SetZoneH(m_table.GetZoneH()+changey);
}

void FiltersPage::TableEvent(kGUIEvent *event)
{
	switch(event->GetEvent())
	{
	case EVENT_AFTERUPDATE:
		Changed();
	break;
	case EVENT_ADDROW:
	{
		GPXFilterRow *fr=new GPXFilterRow();
		m_table.AddRow(fr);
	}
	break;
	}
}

void FiltersPage::LoadPrefs(kGUIXMLItem *root)
{
	unsigned int i;
	kGUIXMLItem *item;

	item=root->Locate("table");
	if(item)
		m_filteredwptable.LoadConfig(item);

	item=root->Locate("ftable");
	if(item)
		m_table.LoadConfig(item);

	for(i=0;i<root->GetNumChildren();++i)
	{
		item=root->GetChild(i);
		if(!strcmp(item->GetName(),"filter"))
		{
			/* generate a new filter */
			GPXFilter *filter=new GPXFilter(item->Locate("name")->GetValue());
			m_filters.SetEntry(m_numfilters++,filter);
			filter->Load(item);
		}
	}

	UpdateFilterList();
	m_filterlist.SetSelectionz(root->Locate("curfilter")->GetValueString());
	item=root->Locate("curdb");
	if(item)
		m_currentdb.SetSelectionz(item->GetValueString());
	item=root->Locate("revfilter");
	if(item)
		m_revfilter.SetSelected(!strcmp(item->GetValueString(),"1")?true:false);
//	ReFilter();
}

void FiltersPage::SavePrefs(kGUIXMLItem *root)
{
	unsigned int i;

	m_filteredwptable.SaveConfig(root,"table");
	m_table.SaveConfig(root,"ftable");

	root->AddChild("curfilter",m_filterlist.GetSelectionString());
	root->AddChild("curdb",m_currentdb.GetSelectionString());
	root->AddChild("revfilter",m_revfilter.GetSelected()==true?"1":"0");

	for(i=0;i<m_numfilters;++i)
		m_filters.GetEntry(i)->Save(root);
}

/***********************************************************************/

int FiltersPage::GetFilter(const char *name)
{
	unsigned int i;
	GPXFilter *f;

	for(i=0;i<m_numfilters;++i)
	{
		f=m_filters.GetEntry(i);
		if(!strcmp(f->GetName(),name))
			return(i);
	}
	return(-1);
}


/* return pointer to filter if it exists, else return zero */
GPXFilter *FiltersPage::LocateFilterz(const char *name)
{
	unsigned int i;
	GPXFilter *f;

	for(i=0;i<m_numfilters;++i)
	{
		f=m_filters.GetEntry(i);
		if(!strcmp(f->GetName(),name))
			return(f);
	}
	return(0);
}

void FiltersPage::ClickSave(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		int fnum;
		kGUIInputBoxReq *box;

		fnum=m_editfilterlist.GetSelection();
		if(!fnum)
			box=new kGUIInputBoxReq(this,CALLBACKNAME(ClickSave2),"Filter Name?");
		else
			Save(m_editfilterlist.GetSelectionStringObj());
	}
}

void FiltersPage::ClickSave2(kGUIString *result,int closebutton)
{
	kGUIMsgBoxReq *box;

	if(result->GetLen() && closebutton==MSGBOX_OK)
	{
		m_newname.SetString(result->GetString());

		if(LocateFilterz(result->GetString()))
			box=new kGUIMsgBoxReq(MSGBOX_YES|MSGBOX_NO,this,CALLBACKNAME(ClickSave3),true,"A Filter named '%s' already exists, replace it?",result->GetString());
		else
			Save(result);
	}
}

/* this is only called if the name already exists */
void FiltersPage::ClickSave3(int  result)
{
	if(result==MSGBOX_YES)
		Save(&m_newname);
}

void FiltersPage::Save(kGUIString *name)
{
	unsigned int i;
	GPXFilter *fobj;

	for(i=0;i<m_numfilters;++i)
	{
		fobj=m_filters.GetEntry(i);
		if(!strcmp(fobj->GetName(),name->GetString()))
		{
			fobj->Save(&m_table);
			goto done;
		}
	}
	/* new filter */
	fobj=new GPXFilter(name);
	fobj->Save(&m_table);
	m_filters.SetEntry(m_numfilters,fobj);
	++m_numfilters;
	UpdateFilterList();
	m_editfilterlist.SetSelection(0);
done:;	
	EditFilterChanged();

	//if the filter changed is the current one end refilter the results to reflect the changed filter
	if(!strcmp(m_filterlist.GetSelectionString(),name->GetString()))
		gpx->ReFilter();
}

void FiltersPage::ClickDelete(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		kGUIMsgBoxReq *box;
	
		box=new kGUIMsgBoxReq(MSGBOX_YES|MSGBOX_NO,this,CALLBACKNAME(ClickDelete2),true,"Delete Filter '%s'?",m_editfilterlist.GetSelectionString());
	}
}

void FiltersPage::ClickDelete2(int result)
{
	int i;
	GPXFilter *f;

	if(result==MSGBOX_YES)
	{
		i=GetFilter(m_editfilterlist.GetSelectionString());
		if(i>=0)
		{
			f=m_filters.GetEntry(i);
			delete f;
			m_filters.DeleteEntry(i);
			--m_numfilters;
			UpdateFilterList();
			EditFilterChanged();
			//SavePrefs();
			return;
		}
		assert(false,"filter not found!");
	}
}

void FiltersPage::ClickRename(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		kGUIInputBoxReq *box;
	
		box=new kGUIInputBoxReq(this,CALLBACKNAME(Rename),"New Filter Name?");
	}
}

void FiltersPage::Rename(kGUIString *result,int closebutton)
{
	unsigned int i;
	GPXFilter *fobj;
	kGUIString oldname;
	GPXWPRenderRow *wpr;
	kGUIMsgBoxReq *box;

	if(result->GetLen() && closebutton==MSGBOX_OK)
	{
		/* make sure name doesn't already exist? */
		if(LocateFilterz(result->GetString()))
		{
			box=new kGUIMsgBoxReq(MSGBOX_OK,false,"Error: Name is already used!");
			return;
		}
		/* name is ok, so go ahead and rename it */
		oldname.SetString(m_editfilterlist.GetSelectionString());

		fobj=LocateFilterz(oldname.GetString());
		assert(fobj!=0,"Filter not found error!");
		fobj->SetName(result);

		/* rename it also in the waypoint label render color table too */
		for(i=0;i<gpx->m_labelcolourtable.GetNumChildren();++i)
		{
			wpr=static_cast<GPXWPRenderRow *>(gpx->m_labelcolourtable.GetChild(i));
			wpr->m_filtercombo.RenameEntry(oldname.GetString(),result->GetString());		/* list of select filters */
		}

		UpdateFilterList();
		EditFilterChanged();
		//SavePrefs();
	}
}

void FiltersPage::ClickCopy(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		kGUIInputBoxReq *box;
	
		box=new kGUIInputBoxReq(this,CALLBACKNAME(Copy),"Copy Filter Name?");
	}
}

void FiltersPage::Copy(kGUIString *result,int closebutton)
{
	kGUIMsgBoxReq *box;

	if(result->GetLen() && closebutton==MSGBOX_OK)
	{
		/* make sure name doesn't already exist? */
		if(LocateFilterz(result->GetString()))
		{
			box=new kGUIMsgBoxReq(MSGBOX_OK,false,"Error: Name is already used!");
			return;
		}
		/* name is ok, so go ahead and copy it */
		Save(result);
	}
}

void FiltersPage::ClickUndo(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		kGUIMsgBoxReq *box;
	
		box=new kGUIMsgBoxReq(MSGBOX_YES|MSGBOX_NO,this,CALLBACKNAME(ClickUndo2),false,"Undo Changes?");
	}
}

void FiltersPage::ClickUndo2(int result)
{
	if(result==MSGBOX_YES)
		EditFilterChanged();
}

void GPXFilterRow::Init(void)
{
	int i;

	m_objectlist[FILTERCOL_FIELD]=&m_fieldcombo;
	m_objectlist[FILTERCOL_OPERATOR]=&m_opcombo;
	m_objectlist[FILTERCOL_VALUE]=&m_valuecombo;

	m_fieldcombo.SetNumEntries(FILTERFIELD_NUMFILTERS);
	m_fieldcombo.SetSize(200,20);
	for(i=0;i<FILTERFIELD_NUMFILTERS;++i)
		m_fieldcombo.SetEntry(i,ffslist[i].name,i);

	m_fieldcombo.SetEventHandler(this,CALLBACKNAME(FieldChangedEvent));
	m_valueinput.SetParent(this);
	FieldChanged();
}

GPXFilterRow::GPXFilterRow()
{
	Init();
}

GPXFilterRow::GPXFilterRow(GPXFilterEntry *e)
{
	Init();
	m_fieldcombo.SetSelection(e->m_fieldnum);
	FieldChanged();
	m_opcombo.SetSelection(e->m_opnum);

	switch(ffslist[e->m_fieldnum].cmptype)
	{
	case CMPTYPE_COMBO:
	case CMPTYPE_TRACKCOMBO:
		m_valuecombo.SetSelection(e->m_value);
	break;
	case CMPTYPE_STRING:
	case CMPTYPE_NUM:
	case CMPTYPE_COORD:
		m_valueinput.SetString(e->m_valuestr.GetString());
	break;
	}
}

GPXFilterRow::~GPXFilterRow()
{
	/* the one not used needs to have this reset manually */
	m_valuecombo.SetParent(0);
	m_valueinput.SetParent(0);
}

void GPXFilterRow::FieldChanged(void)
{
	int i,nc;
	int fnum;
	int validopbits;
	int numvalidops;

	fnum=m_fieldcombo.GetSelection();
	validopbits=ffslist[fnum].ops;
	numvalidops=0;
	for(i=0;i<FILTEROP_NUMOPS;++i)
	{
		if((1<<i)&validopbits)
			++numvalidops;
	}
	m_opcombo.SetNumEntries(numvalidops);
	numvalidops=0;
	for(i=0;i<FILTEROP_NUMOPS;++i)
	{
		if((1<<i)&validopbits)
		{
			m_opcombo.SetEntry(numvalidops,filteropnames[i],i);
			++numvalidops;
		}
	}
	if(ffslist[fnum].cmptype==CMPTYPE_TRACKCOMBO)
	{
		UseCombo();
		GenTrackCombo();
	}
	else
	{
		nc=ffslist[fnum].numcomboentries;
		if(!nc)
			UseInput();
		else
		{
			UseCombo();
			m_valuecombo.SetNumEntries(nc);
			for(i=0;i<nc;++i)
				m_valuecombo.SetEntry(i,ffslist[fnum].combotext[i],i);
		}
	}
}

/*************************************************************************************************/

GPXFilter::~GPXFilter()
{
	unsigned int i;

	for(i=0;i<m_numentries;++i)
		delete m_entries.GetEntry(i);
}

void GPXFilter::Load(kGUITableObj *table)
{
	unsigned int e;
	GPXFilterRow *row;
	GPXFilterEntry *fe;

	table->DeleteChildren();

	for(e=0;e<m_numentries;++e)
	{
		fe=m_entries.GetEntry(e);	/* filter record */
		row=new GPXFilterRow();		/* m
									ake new table record */
		row->m_fieldcombo.SetSelection(fe->m_fieldnum);
		row->FieldChanged();	/* select proper combo box for this field and variable type */
		row->m_opcombo.SetSelectionz(fe->m_opnum);
		switch(ffslist[fe->m_fieldnum].cmptype)
		{
		case CMPTYPE_COMBO:
			row->UseCombo();
			row->m_valuecombo.SetSelection(fe->m_value);
		break;
		case CMPTYPE_TRACKCOMBO:
			row->UseCombo();
			row->m_valuecombo.SetSelectionz(fe->m_valuestr.GetString());
		break;
		case CMPTYPE_STRING:
		case CMPTYPE_NUM:
		case CMPTYPE_COORD:
			row->UseInput();
			row->m_valueinput.SetString(fe->m_valuestr.GetString());
		break;
		}
		table->AddRow(row);
	}
}

/* true=changed, false=same */

bool GPXFilter::Compare(kGUITableObj *table)
{
	unsigned int e;
	GPXFilterRow *row;
	GPXFilterEntry *fe;

	if(table->GetNumChildren(0)!=m_numentries)
		return(true);

	for(e=0;e<m_numentries;++e)
	{
		fe=m_entries.GetEntry(e);	/* filter record */
		row=static_cast<GPXFilterRow *>(table->GetChild(e));

		if(row->m_fieldcombo.GetSelection()!=fe->m_fieldnum)
			return(true);

		if(row->m_opcombo.GetSelection()!=fe->m_opnum)
			return(true);

		switch(ffslist[fe->m_fieldnum].cmptype)
		{
		case CMPTYPE_COMBO:
			if(row->m_valuecombo.GetSelection()!=fe->m_value)
				return(true);
		break;
		case CMPTYPE_TRACKCOMBO:
			if(strcmp(row->m_valuecombo.GetSelectionString(),fe->m_valuestr.GetString()))
				return(true);
		break;
		case CMPTYPE_STRING:
		case CMPTYPE_NUM:
		case CMPTYPE_COORD:
			if(strcmp(row->m_valueinput.GetString(),fe->m_valuestr.GetString()))
				return(true);
		break;
		}
	}
	return(false);	/* same! */
}


void GPXFilter::Save(kGUITableObj *table)
{
	unsigned int e;
	unsigned int nr;
	kGUIObj *obj;
	GPXFilterRow *row;
	GPXFilterEntry *fe;

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
		row=static_cast<GPXFilterRow *>(obj);
		fe=new GPXFilterEntry();
		fe->m_fieldnum=row->m_fieldcombo.GetSelection();
		fe->m_opnum=row->m_opcombo.GetSelection();
		switch(ffslist[fe->m_fieldnum].cmptype)
		{
		case CMPTYPE_COMBO:
			fe->m_value=row->m_valuecombo.GetSelection();
		break;
		case CMPTYPE_TRACKCOMBO:
			fe->m_valuestr.SetString(row->m_valuecombo.GetSelectionString());
		break;
		case CMPTYPE_STRING:
		case CMPTYPE_NUM:
			fe->m_valuestr.SetString(row->m_valueinput.GetString());
		break;
		case CMPTYPE_COORD:
			fe->m_valuestr.SetString(row->m_valueinput.GetString());
			fe->m_valuepoint.Set(fe->m_valuestr.GetString());
		break;
		}
		m_entries.SetEntry(e,fe);
	}
}

/* Load from preferences file */

void GPXFilter::Load(kGUIXMLItem *xml)
{
	unsigned int i;
	unsigned int nc;
	unsigned int j;
	GPXFilterEntry *fe;
	kGUIXMLItem *flxml;
	const char *fv;

	nc=xml->GetNumChildren();
	for(i=0;i<nc;++i)
	{
		flxml=xml->GetChild(i);
		if(!strcmp(flxml->GetName(),"line"))
		{
			fe=new GPXFilterEntry();
			m_entries.SetEntry(m_numentries++,fe);
			fv=flxml->Locate("field")->GetValueString();

			fe->m_fieldnum=-1;
			for(j=0;(j<sizeof(ffslist)/sizeof(FF_DEF));++j)
			{
				if(!strcmp(fv,ffslist[j].name))
				{
					fe->m_fieldnum=j;
					break;
				}
			}
			assert(fe->m_fieldnum>=0,"field not found!");
			fe->m_opnum=gpx->GetIndex(flxml->Locate("operator")->GetValueString(),FILTEROP_NUMOPS,filteropnames);
			switch(ffslist[fe->m_fieldnum].cmptype)
			{
			case CMPTYPE_COMBO:
				fe->m_value=-1;
				for(j=0;j<ffslist[fe->m_fieldnum].numcomboentries;++j)
				{
					if(!strcmp(flxml->Locate("value")->GetValueString(),ffslist[fe->m_fieldnum].combotext[j]))
					{
						fe->m_value=j;
						break;
					}
				}
				assert(fe->m_value>=0,"field not found!");
			break;
			case CMPTYPE_TRACKCOMBO:
			case CMPTYPE_STRING:
			case CMPTYPE_NUM:
				fe->m_valuestr.SetString(flxml->Locate("value")->GetValue());
			break;
			case CMPTYPE_COORD:
				fe->m_valuestr.SetString(flxml->Locate("value")->GetValue());
				fe->m_valuepoint.Set(fe->m_valuestr.GetString());
			break;
			}
		}
	}
}

/* save to preferences file */

void GPXFilter::Save(kGUIXMLItem *xml)
{
	unsigned int e;
	GPXFilterEntry *fe;
	kGUIXMLItem *fxml;
	kGUIXMLItem *flxml;

	fxml=xml->AddChild("filter");
	fxml->AddParm("name",m_name.GetString());

	for(e=0;e<m_numentries;++e)
	{
		fe=m_entries.GetEntry(e);	/* filter record */

		flxml=fxml->AddChild("line");
		flxml->AddChild("field",ffslist[fe->m_fieldnum].name);
		flxml->AddChild("operator",filteropnames[fe->m_opnum]);

		switch(ffslist[fe->m_fieldnum].cmptype)
		{
		case CMPTYPE_COMBO:
			flxml->AddChild("value",ffslist[fe->m_fieldnum].combotext[fe->m_value]);
		break;
		case CMPTYPE_TRACKCOMBO:
		case CMPTYPE_STRING:
		case CMPTYPE_NUM:
		case CMPTYPE_COORD:
			flxml->AddChild("value",fe->m_valuestr.GetString());
		break;
		}
	}
}

bool GPXFilter::FilterRow(GPXRow *trow)
{
	unsigned int e;
	GPXFilterEntry *fe;
	int op;
	bool res=0;
	bool neg;
	int cmptype;

	for(e=0;e<m_numentries;++e)
	{
		const char *tstring=0;
		int tval=0;
		const char *fstring=0;
		int fval=0;
		GPXUnit funit;
		GPXUnit tunit;
		GPXUnit tunit2;

		fe=m_entries.GetEntry(e);

		cmptype=ffslist[fe->m_fieldnum].cmptype;
		switch(cmptype)
		{
		case CMPTYPE_COMBO:
			fval=fe->m_value;
		break;
		case CMPTYPE_TRACKCOMBO:
		case CMPTYPE_STRING:
		case CMPTYPE_NUM:
			fstring=fe->m_valuestr.GetString();
		break;
		case CMPTYPE_COORD:
			funit.Set(fe->m_valuepoint.GetValue());
		break;
		}

		switch(fe->m_fieldnum)
		{
		case FILTERFIELD_FOUND:
			if(trow->m_found.GetSelected())
				tval=1;
			else
				tval=0;
		break;
		case FILTERFIELD_AVAILABLE:
			if(trow->m_na.GetSelected())
				tval=0;
			else
				tval=1;
		break;
		case FILTERFIELD_USERTICKED1:
		case FILTERFIELD_USERTICKED2:
		case FILTERFIELD_USERTICKED3:
		case FILTERFIELD_USERTICKED4:
		case FILTERFIELD_USERTICKED5:
		case FILTERFIELD_USERTICKED6:
		case FILTERFIELD_USERTICKED7:
		case FILTERFIELD_USERTICKED8:
			if(trow->m_user[fe->m_fieldnum-FILTERFIELD_USERTICKED1].GetSelected())
				tval=1;
			else
				tval=0;
		break;
		case FILTERFIELD_CORRECTED:
			if(trow->m_corrected.GetSelected())
				tval=1;
			else
				tval=0;
		break;
		case FILTERFIELD_OWNER:
			tstring=trow->m_owner.GetString();
		break;
		case FILTERFIELD_TYPE:
			tval=trow->m_xtype;
		break;
		case FILTERFIELD_CONTAINER:
			tval=trow->m_container.GetSelection();
		break;
		case FILTERFIELD_DIFFICULTY:
			tstring=trow->m_difficulty.GetString();
		break;
		case FILTERFIELD_TERRAIN:
			tstring=trow->m_terrain.GetString();
		break;
		case FILTERFIELD_NAME:
			tstring=trow->m_name.GetString();
		break;
		case FILTERFIELD_LAT:
			tunit.Set(trow->m_llcoord.GetLat());
		break;
		case FILTERFIELD_LON:
			tunit.Set(trow->m_llcoord.GetLon());
		break;
		case FILTERFIELD_STATE:
			tstring=trow->m_state.GetString();
		break;
		case FILTERFIELD_COUNTRY:
			tstring=trow->m_country.GetString();
		break;
		case FILTERFIELD_POSITION:
			tunit.Set(trow->m_llcoord.GetLat());
			tunit2.Set(trow->m_llcoord.GetLon());
		break;
		}

		/* reduce the number of compares in 1/2 by doing opposites and neging the results */
		op=fe->m_opnum;
		neg=false;
		switch(fe->m_opnum)
		{
		case FILTEROP_WEST:
			op=FILTEROP_LT;
			neg=false;
		break;
		case FILTEROP_EAST:
			op=FILTEROP_LT;
			neg=true;
		break;
		case FILTEROP_NORTH:
			op=FILTEROP_LT;
			neg=true;
		break;
		case FILTEROP_SOUTH:
			op=FILTEROP_LT;
			neg=false;
		break;
		case FILTEROP_NEQ:
			op=FILTEROP_EQ;
			neg=true;
		break;
		case FILTEROP_GE:
			op=FILTEROP_LT;
			neg=true;
		break;
		case FILTEROP_GT:
			op=FILTEROP_LE;
			neg=true;
		break;
		case FILTEROP_DOESNTCONTAIN:
			op=FILTEROP_CONTAINS;
			neg=true;
		break;
		case FILTEROP_OUTSIDE:
			op=FILTEROP_INSIDE;
			neg=true;
		break;
		}
		/* compares have been reduced down to 4 comparisions */
		switch(op)
		{
		case FILTEROP_EQ:
			switch(cmptype)
			{
			case CMPTYPE_COMBO:
				res=(fval==tval);
			break;
			case CMPTYPE_STRING:
				if(!strcmp(fstring,tstring))
					res=true;
				else
					res=false;
			case CMPTYPE_NUM:
				if(atof(fstring)==atof(tstring))
					res=true;
				else
					res=false;
			break;
			case CMPTYPE_COORD:
				if(funit.GetValue()==tunit.GetValue())
					res=true;
				else
					res=false;
			break;
			}
		break;
		case FILTEROP_LT:
			switch(cmptype)
			{
			case CMPTYPE_COMBO:
				res=(fval>tval);
			break;
			case CMPTYPE_STRING:
				assert(false,"not a valid compare");
			case CMPTYPE_NUM:
				if(atof(fstring)>atof(tstring))
					res=true;
				else
					res=false;
			break;
			case CMPTYPE_COORD:
				if(funit.GetValue()>tunit.GetValue())
					res=true;
				else
					res=false;
			break;
			}
		break;
		case FILTEROP_LE:
			switch(cmptype)
			{
			case CMPTYPE_COMBO:
				res=(fval>=tval);
			break;
			case CMPTYPE_STRING:
				assert(false,"not a valid compare");
			case CMPTYPE_NUM:
				if(atof(fstring)>=atof(tstring))
					res=true;
				else
					res=false;
			break;
			case CMPTYPE_COORD:
				if(funit.GetValue()>=tunit.GetValue())
					res=true;
				else
					res=false;
			break;
			}
		break;
		case FILTEROP_CONTAINS:
			switch(cmptype)
			{
			case CMPTYPE_NUM:
			case CMPTYPE_COMBO:
			case CMPTYPE_COORD:
				assert(false,"not a valid compare");
			break;
			case CMPTYPE_STRING:
				if(strstr(tstring,fstring))
					res=true;
				else
					res=false;
			break;
			}
		break;
		case FILTEROP_INSIDE:
			res=gpx->m_tracks.IsPointInside(fstring,trow->m_llcoord.GetLat(),trow->m_llcoord.GetLon());
		break;
		}
		if(neg==true)
			res=!res;
		if(!res)
			return(false);
	}
	return(true);
}

/* update the save/undo button status based on if the filter has changed */

void FiltersPage::Changed(void)
{
	bool changed=false;

	if(!m_editfilterlist.GetSelection())
	{
		/* should be empty, so any rows means changed */
		if(m_table.GetNumChildren(0))
			changed=true;
		m_delete.SetEnabled(false);
		m_rename.SetEnabled(false);
		m_copy.SetEnabled(false);
	}
	else
	{
		int i=GetFilter(m_editfilterlist.GetSelectionString());
		changed=m_filters.GetEntry(i)->Compare(&m_table);
		m_delete.SetEnabled(!changed);
		m_rename.SetEnabled(!changed);
		m_copy.SetEnabled(!changed);
	}

	m_save.SetEnabled(changed);	/* enabled after editing */
	m_undo.SetEnabled(changed); /* enabled after editing */
	m_editfilterlist.SetLocked(changed);
}

	/* default filters, default label colours */
	/* overwritten by user preferences file */

typedef struct
{
	const char *filtername;
	int field,foper,fvalue;
}DEFFILTERLIST_DEF;

DEFFILTERLIST_DEF deffilters[]={
	{"not found",		FILTERFIELD_FOUND,		FILTEROP_EQ,	0},
	{0,					FILTERFIELD_AVAILABLE,	FILTEROP_EQ,	1},
	{"found",			FILTERFIELD_FOUND,		FILTEROP_EQ,	1},
	{"not available",	FILTERFIELD_AVAILABLE,	FILTEROP_EQ,	0},
	{"mystery",			FILTERFIELD_TYPE,		FILTEROP_EQ,	CACHETYPE_UNKNOWN},
	{0,					FILTERFIELD_AVAILABLE,	FILTEROP_EQ,	1},
	{"multi",			FILTERFIELD_TYPE,		FILTEROP_EQ,	CACHETYPE_MULTI},
	{0,					FILTERFIELD_AVAILABLE,	FILTEROP_EQ,	1},
	{"user waypoints",	FILTERFIELD_TYPE,		FILTEROP_EQ,	CACHETYPE_UNDEFINED},
	{"user1 ticked",	FILTERFIELD_USERTICKED1,FILTEROP_EQ,	1}};

void FiltersPage::InitDefaultFilters(void)
{
	unsigned int i;
	DEFFILTERLIST_DEF *dfl;
	GPXFilterEntry *fe;
	GPXFilter *filter=0;
	kGUIString name;

	dfl=deffilters;
	for(i=0;i<(sizeof(deffilters)/sizeof(DEFFILTERLIST_DEF));++i)
	{
		if(dfl->filtername)
		{
			name.SetString(dfl->filtername);
			filter=new GPXFilter(&name);
			m_filters.SetEntry(m_numfilters++,filter);
		}
		fe=filter->AddEntry();
		fe->m_fieldnum=dfl->field;
		fe->m_opnum=dfl->foper;
		fe->m_value=dfl->fvalue;
		++dfl;
	}
	UpdateFilterList();
}

/* refilter a single row and flag only the map cells therein dirty */
void FiltersPage::ReFilter(GPXRow *row)
{
	int fnum=m_filterlist.GetSelection();
	const char *qtext=m_quickfilter.GetString();
	bool addme=true;
	GPXFilter *f;

	/* only check a specific database? */
	if(m_currentdb.GetSelection())	/* 0=all databases, 1->n = specific db */
	{
		if(strcmp(row->GetDB(),m_currentdb.GetSelectionString()))
			addme=false;
	}

	if(addme==true)
	{
		if(qtext[0])	/* quick search filter */
		{
			if(strstri(row->GetName(),qtext))
				addme=true;
			else
				addme=false;
		}

		if(addme==true)
		{
			if(fnum)
			{
				f=m_filters.GetEntry(fnum-1);
				addme=f->FilterRow(row);
			}
		}
	}

	if(m_revfilter.GetSelected()==true)
		addme=!addme;

	gpx->GetColour(row);		/* recalc color */
	row->SetLabelDraw(addme);

	if(addme==false)
		m_filteredwptable.DeleteRow(row,false);	/* don't purge */
	else
		m_filteredwptable.Sort(GPX::SortEntry);

	m_resultsfilter.Sprintf("%d / %d",m_filteredwptable.GetNumChildren(),gpx->m_numwpts);

	gpx->ReCalcNear();
	gpx->MapDirty();
}

/* the user just saved a track, if the current filter is referring to this */
/* track, then trigger a refilter */

void FiltersPage::TrackSaved(const char *trackname)
{
	int fnum=m_filterlist.GetSelection();
	GPXFilter *f;
	unsigned int e;
	GPXFilterEntry *fe;

	if(!fnum)
		return;
	else
		f=m_filters.GetEntry(fnum-1);

	for(e=0;e<f->m_numentries;++e)
	{
		fe=f->m_entries.GetEntry(e);
		if(ffslist[fe->m_fieldnum].cmptype==CMPTYPE_TRACKCOMBO)
		{
			if(!strcmp(fe->m_valuestr.GetString(),trackname))
			{
				/* yes it is referred to, trigger a refilter */
				ReFilter();
				return;
			}
		}
	}
	/* nope, this track is not referenced */
}

void FiltersPage::ReFilter(void)
{
	unsigned int e;
	int dbnum;
	GPXRow *row;
	int fnum=m_filterlist.GetSelection();
	const char *qtext=m_quickfilter.GetString();
	bool addme;
	GPXFilter *f;
	const char *dbsel=m_currentdb.GetSelectionString();

	kGUI::SetMouseCursor(MOUSECURSOR_BUSY);

	dbnum=m_currentdb.GetSelection();
	if(!fnum)
		f=0;
	else
		f=m_filters.GetEntry(fnum-1);

	/* remove all entries from the table without deleteing them */
	m_filteredwptable.DeleteChildren(false);	/* delete table entries but don't free them */

	for(e=0;e<gpx->m_numwpts;++e)
	{
		row=gpx->m_wptlist.GetEntry(e);

		addme=true;
		if(dbnum)	/* 0=all databases, 1->n = specific db */
		{
			if(strcmp(row->GetDB(),dbsel))
				addme=false;
		}

		if(addme==true)
		{
			if(qtext[0])	/* search filter string? */
			{
				if(strstri(row->GetName(),qtext))
					addme=true;
				else
					addme=false;
			}
			if(addme==true)
			{
				if(f)
					addme=f->FilterRow(row);
			}
		}

		if(m_revfilter.GetSelected()==true)
			addme=!addme;

		row->SetLabelDraw(addme);
		if(addme==true)
			m_filteredwptable.AddRow(row);
	}
	/* since the labels on the filter list has changed, the map needs to be updated */
	m_filteredwptable.Sort(GPX::SortEntry);

	/* only need to do this if overlap mode is turned on */
	if(gpx->m_movelabels.GetSelected()==true)
		gpx->BSPDirty();

	m_resultsfilter.Sprintf("%d / %d",m_filteredwptable.GetNumChildren(),gpx->m_numwpts);
	gpx->MapDirty();
	gpx->ReCalcNear();

	kGUI::SetMouseCursor(MOUSECURSOR_DEFAULT);
}

void FiltersPage::EditFilterChanged(void)
{
	int fnum;

	fnum=m_editfilterlist.GetSelection();
	if(!fnum)
	{
		m_table.DeleteChildren();
	}
	else
	{
		m_filters.GetEntry(fnum-1)->Load(&m_table);
	}
	Changed();
}

void FiltersPage::UpdateFilterList(void)
{
	unsigned int i;
	unsigned int e;
	kGUIString curfname;
	kGUIString editfname;
	GPXWPRenderRow *wpr;

	curfname.SetString(m_filterlist.GetSelectionString());
	editfname.SetString(m_editfilterlist.GetSelectionString());

	m_filterlist.SetNumEntries(m_numfilters+1);			/* list of select filters */
	m_editfilterlist.SetNumEntries(m_numfilters+1);		/* list of edit filters */
	m_filterlist.SetEntry(0,"Show All",0);
	m_editfilterlist.SetEntry(0,gpx->GetString(STRING_NEWFILTER),0);
	for(e=0;e<m_numfilters;++e)
	{
		m_filterlist.SetEntry(e+1,m_filters.GetEntry(e)->GetName(),e+1);
		m_editfilterlist.SetEntry(e+1,m_filters.GetEntry(e)->GetName(),e+1);
	}
	m_filterlist.SetSelectionz(curfname.GetString());
	m_editfilterlist.SetSelectionz(editfname.GetString());

	/* update combos on the wprender table too */
	for(i=0;i<gpx->m_labelcolourtable.GetNumChildren();++i)
	{
		wpr=static_cast<GPXWPRenderRow *>(gpx->m_labelcolourtable.GetChild(i));
		if(wpr->m_filtercombo.GetNumEntries())
			curfname.SetString(wpr->m_filtercombo.GetSelectionString());
		else
			curfname.Clear();

		wpr->m_filtercombo.SetNumEntries(m_numfilters);		/* list of select filters */
		for(e=0;e<m_numfilters;++e)
			wpr->m_filtercombo.SetEntry(e,m_filters.GetEntry(e)->GetName(),e);
		
		if(curfname.GetLen())
			wpr->m_filtercombo.SetSelection(curfname.GetString());
	}
}

/* scan waypoints and extract the list of unique databases */
void FiltersPage::UpdateDBList(void)
{
	unsigned int e;
	unsigned int num;
	kGUIString curdb;
	GPXRow *row;
	Hash names;			/* names are added to this list to check for duplicates */
	int def=0;
	HashEntry *he;

	curdb.SetString(m_currentdb.GetSelectionString());
	names.Init(8,sizeof(def));

	for(e=0;e<gpx->m_numwpts;++e)
	{
		row=gpx->m_wptlist.GetEntry(e);
		if(!names.Find(row->GetDB()))
			names.Add(row->GetDB(),&def);
	}

	num=names.GetNum();
	m_currentdb.SetNumEntries(num+1);				/* list of filters */
	m_currentdb.SetEntry(0,gpx->GetString(STRING_ALLDATABASES),0);
	he=names.GetFirst();
	for(e=0;e<num;++e)
	{
		m_currentdb.SetEntry(e+1,he->m_string,e+1);
		he=he->GetNext();
	}


	if(m_currentdb.SetSelectionz(curdb.GetString())==false)
	{
		/* since the current DB selection was not valid a re-filter is in order */
		ReFilter();
	}
}

#if 0
void FiltersPage::DeleteFRow(kGUITableRowObj *obj)
{
	unsigned int e;

	/* user manually deleted a row in the */
	/* table so remove it from the full list! */
	for(e=0;e<gpx->m_numwpts;++e)
	{
		if(obj==gpx->m_wptlist.GetEntry(e))
		{
			/* remove it from the current route if it is there too. */
			gpx->m_routes.Remove(static_cast<GPXRow *>(obj)->GetWptName());

			gpx->m_wptlist.DeleteEntry(e);
			--gpx->m_numwpts;
	
			
			gpx->BSPDirty();
			UpdateDBList();
			return;
		}
	}

	assert(false,"Row not found error!");
}
#endif

/* used by the VB code to filter the full list down to a subset */

int FiltersPage::Filter(const char *filtername,Array<GPXRow *>*list)
{
	unsigned int i;
	unsigned int num;
	int fi;
	GPXFilter *filter;
	GPXRow *row;

	fi=GetFilter(filtername);
	if(fi<0)
		return(0);	/* filter not found, so no matches, sorry! */
	
	num=0;
	filter=m_filters.GetEntry(fi);
	for(i=0;i<gpx->m_numwpts;++i)
	{
		row=gpx->m_wptlist.GetEntry(i);
		if(filter->FilterRow(row)==true)
			list->SetEntry(num++,row);
	}
	return(num);
}

bool FiltersPage::FilterRow(int index,GPXRow *row)
{
	return m_filters.GetEntry(index)->FilterRow(row);
}

const char *FiltersPage::GetFilterName(int index)
{
	return m_filters.GetEntry(index)->GetName();
}

void FiltersPage::UpdateTrackNames(const char *oldname,const char *newname)
{
	unsigned int i;
	unsigned int j;

	GPXFilter *filter;
	GPXFilterEntry *fe;
	GPXFilterRow *row;
	kGUIString was;

	/* rename any trackname references found in saved tracks */
	if(oldname)
	{
		for(i=0;i<m_numfilters;++i)
		{
			filter=m_filters.GetEntry(i);
			for(j=0;j<filter->m_numentries;++j)
			{
				fe=filter->m_entries.GetEntry(j);

				if(ffslist[fe->m_fieldnum].cmptype==CMPTYPE_TRACKCOMBO)
				{
					if(!strcmp(fe->m_valuestr.GetString(),oldname))
						fe->m_valuestr.SetString(newname);
				}
			}
		}
	}

	/* rebuild combos for current tracks and rename if applicable */
	for(i=0;i<m_table.GetNumChildren();++i)
	{
		row=static_cast<GPXFilterRow *>(m_table.GetChild(i));
		if(ffslist[row->m_fieldcombo.GetSelection()].cmptype==CMPTYPE_TRACKCOMBO)
		{
			was.SetString(row->m_valuecombo.GetSelectionString());
			if(oldname)
			{
				if(!strcmp(was.GetString(),oldname))
					was.SetString(newname);
			}
			/* rebuild the combo */
			row->GenTrackCombo();
			row->m_valuecombo.SetSelectionz(was.GetString());
		}
	}
}

void GPXFilterRow::GenTrackCombo(void)
{
	unsigned int i;
	unsigned int nc;

	nc=gpx->GetTrackObj()->GetNumTracks();
	if(!nc)
	{
		m_valuecombo.SetNumEntries(1);
		m_valuecombo.SetEntry(0,"No Tracks Defined",0);
	}
	else
	{
		m_valuecombo.SetNumEntries(nc);
		for(i=0;i<nc;++i)
			m_valuecombo.SetEntry(i,gpx->GetTrackObj()->GetTrackName(i),i);
	}
}

