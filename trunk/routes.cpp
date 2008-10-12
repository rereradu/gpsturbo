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
#include "kguitsp.h"

class AutoOrderWindow
{
public:
	AutoOrderWindow(kGUITableObj *t);
	~AutoOrderWindow();
private:
	void WindowEvent(kGUIEvent *event);
	CALLBACKGLUEPTR(AutoOrderWindow,WindowEvent,kGUIEvent);
	void DoneEvent(kGUIEvent *event);
	CALLBACKGLUEPTR(AutoOrderWindow,DoneEvent,kGUIEvent);
	void Update(void);
	CALLBACKGLUE(AutoOrderWindow,Update);
	kGUITableObj *m_t;
	kGUITSP m_tsp;
	Array<GPXRow *>m_rows;
	kGUIWindowObj m_window;
	kGUIBusyRectObj m_busyrect;
	kGUITextObj m_tries;
	kGUITextObj m_best;
	kGUIButtonObj m_done;
	double m_startdist;
	unsigned int m_count;
	unsigned int m_curbest;
};

class GPXRouteEntry
{
public:
	kGUIString m_wptname;
};

class GPXRoute
{
public:
	GPXRoute(kGUIString *name) {m_draw=0;m_color=DrawColor(255,0,0);m_name.SetString(name);m_numentries=0;m_entries.SetGrow(true);m_entries.Alloc(16);}
	~GPXRoute();
	void SetName(kGUIString *name) {m_name.SetString(name);}
	const char *GetName(void) {return m_name.GetString();}
	void SetDraw(bool d) {m_draw=d;}
	bool GetDraw(void) {return m_draw;}
	void SetColorIndex(unsigned int index) {m_color=index;}
	unsigned int GetColorIndex(void) {return m_color;}
	void Load(kGUITableObj *table,kGUITickBoxObj *draw,kGUIComboBoxObj *color);
	bool Compare(kGUITableObj *table,kGUITickBoxObj *draw,kGUIComboBoxObj *color);
	void Save(kGUITableObj *table,kGUITickBoxObj *draw,kGUIComboBoxObj *color);
	void Load(kGUIXMLItem *xml);
	void Save(kGUIXMLItem *xml);
	bool RouteRow(GPXRow *trow);
	void Draw(kGUICorners *c);
	static void DrawLine(int index,int x1,int y1,int x2,int y2,kGUIColor color);
	unsigned int GetNumEntries(void) {return m_numentries;}
	GPXRouteEntry *GetEntry(unsigned int index) {return m_entries.GetEntry(index);}
private:
	kGUIString m_name;
	bool m_draw;
	unsigned int m_color;

	unsigned int m_numentries;
	Array<GPXRouteEntry *>m_entries;
};

/* these are in gpsturbo.cpp and also used for the filtered results table */
extern const char *wpcolnames[GPXCOL_NUMCOLUMNS];
extern const char *wpcoldesc[GPXCOL_NUMCOLUMNS];
extern int wpcolwidths[GPXCOL_NUMCOLUMNS];

RoutesPage::RoutesPage()
{
	m_numroutes=0;
	m_routes.Init(8,2);
}

RoutesPage::~RoutesPage()
{
}

void RoutesPage::Purge(void)
{
	unsigned int i;

	m_routewptable.DeleteChildren();

	for(i=0;i<m_numroutes;++i)
        delete m_routes.GetEntry(i);
}

void RoutesPage::LoadPrefs(kGUIXMLItem *root)
{
	unsigned int i;
	kGUIXMLItem *item;
	GPXRoute *r;

	item=root->Locate("rtable");
	if(item)
		m_routewptable.LoadConfig(item);

	for(i=0;i<root->GetNumChildren();++i)
	{
		item=root->GetChild(i);
		if(!strcmp(item->GetName(),"route"))
		{
			r=new GPXRoute(item->Locate("name")->GetValue());
			m_routes.SetEntry(m_numroutes++,r);
			r->Load(item);
		}
	}
	UpdateRouteList();
}

void RoutesPage::SavePrefs(kGUIXMLItem *root)
{
	unsigned int i;

	m_routewptable.SaveConfig(root,"rtable");

	for(i=0;i<m_numroutes;++i)
		m_routes.GetEntry(i)->Save(root);
}

/* init routes gui items */
void RoutesPage::Init(kGUIContainerObj *obj)
{
	unsigned int i;
	int y;
	kGUIText *t;
	int bw=obj->GetChildZoneW();
	//int bh=obj->GetChildZoneH();

	/* route table */
	m_editcontrols.SetSize(bw,20);

	m_pntup.SetFontSize(BUTTONFONTSIZE);
	m_pntup.SetString(gpx->GetString(STRING_UP));
	m_pntup.Contain();
	m_pntup.SetEventHandler(this,CALLBACKNAME(ClickWpntUp));
	m_editcontrols.AddObject(&m_pntup);

	m_pntdown.SetFontSize(BUTTONFONTSIZE);
	m_pntdown.SetString(gpx->GetString(STRING_DOWN));
	m_pntdown.Contain();
	m_pntdown.SetEventHandler(this,CALLBACKNAME(ClickWpntDown));
	m_editcontrols.AddObject(&m_pntdown);

	m_autoorder.SetFontSize(BUTTONFONTSIZE);
	m_autoorder.SetString(gpx->GetString(STRING_AUTOORDER));
	m_autoorder.Contain();
	m_autoorder.SetEventHandler(this,CALLBACKNAME(ClickAutoOrder));
	m_editcontrols.AddObject(&m_autoorder);

	m_editroutelist.SetFontSize(BUTTONFONTSIZE);
	m_editroutelist.SetSize(300,20);
	m_editroutelist.SetNumEntries(1);				/* list of routes */
	m_editroutelist.SetEntry(0,gpx->GetString(STRING_NEWROUTE),0);
	m_editroutelist.SetEventHandler(this,CALLBACKNAME(LoadEvent));
	m_editcontrols.AddObject(&m_editroutelist);

	m_save.SetFontSize(BUTTONFONTSIZE);
	m_save.SetString(gpx->GetString(STRING_SAVEROUTE));
	m_save.Contain();
	m_save.SetEventHandler(this,CALLBACKNAME(ClickSave));
	m_editcontrols.AddObject(&m_save);

	m_undo.SetFontSize(BUTTONFONTSIZE);
	m_undo.SetString(gpx->GetString(STRING_UNDOCHANGES));
	m_undo.Contain();
	m_undo.SetEventHandler(this,CALLBACKNAME(ClickUndo));
	m_editcontrols.AddObject(&m_undo);

	m_delete.SetFontSize(BUTTONFONTSIZE);
	m_delete.SetString(gpx->GetString(STRING_DELETEROUTE));
	m_delete.Contain();
	m_delete.SetEventHandler(this,CALLBACKNAME(ClickDelete));
	m_editcontrols.AddObject(&m_delete);

	m_rename.SetFontSize(BUTTONFONTSIZE);
	m_rename.SetString(gpx->GetString(STRING_RENAMEROUTE));
	m_rename.Contain();
	m_rename.SetEventHandler(this,CALLBACKNAME(ClickRename));
	m_editcontrols.AddObject(&m_rename);

	m_copy.SetFontSize(BUTTONFONTSIZE);
	m_copy.SetString(gpx->GetString(STRING_COPYROUTE));
	m_copy.Contain();
	m_copy.SetString("Copy Route");
	m_copy.SetEventHandler(this,CALLBACKNAME(ClickCopy));
	m_editcontrols.AddObject(&m_copy);

	m_delete.SetEnabled(false);
	m_rename.SetEnabled(false);
	m_copy.SetEnabled(false);

	m_editcontrols.NextLine();

	m_drawcaption.SetPos(0,0);
	m_drawcaption.SetFontSize(SMALLCAPTIONFONTSIZE);
	m_drawcaption.SetFontID(SMALLCAPTIONFONT);
	m_drawcaption.SetString("Draw ");
	m_draw.SetPos(0,15);
	m_draw.SetEventHandler(this,CALLBACKNAME(ChangedEvent));
	m_editcontrols.AddObjects(2,&m_drawcaption,&m_draw);

	m_colorcaption.SetPos(0,0);
	m_colorcaption.SetFontSize(SMALLCAPTIONFONTSIZE);
	m_colorcaption.SetFontID(SMALLCAPTIONFONT);
	m_colorcaption.SetString(" color");

	m_color.SetPos(0,15);
	m_color.SetSize(275,20);
	GPX::InitColorCombo(&m_color);
	m_color.SetSelection("Red");
	m_color.SetEventHandler(this,CALLBACKNAME(ChangedEvent));
	m_editcontrols.AddObjects(2,&m_colorcaption,&m_color);

	m_editcontrols.NextLine();

	/* display info for , not editable */
	m_numcaption.SetPos(0,0);
	m_numcaption.SetFontSize(SMALLCAPTIONFONTSIZE);
	m_numcaption.SetFontID(SMALLCAPTIONFONT);
	m_numcaption.SetString("Total  Points");

	m_num.SetPos(0,15);
	m_num.SetSize(80,20);
	m_num.SetLocked(true);
	m_num.SetHint("Number of points on the route.");
	m_editcontrols.AddObjects(2,&m_numcaption,&m_num);

	m_distcaption.SetPos(0,0);
	m_distcaption.SetFontSize(SMALLCAPTIONFONTSIZE);
	m_distcaption.SetFontID(SMALLCAPTIONFONT);
	m_distcaption.SetString("Total Route Distance");

	m_dist.SetPos(0,15);
	m_dist.SetSize(150,20);
	m_dist.SetLocked(true);
	m_dist.SetHint("Distance covered an the route.");
	m_editcontrols.AddObjects(2,&m_distcaption,&m_dist);

	obj->AddObject(&m_editcontrols);
	y=m_editcontrols.GetZoneH();
	m_routewptable.SetPos(0,y);
	m_routewptable.SetSize(bw,obj->GetChildZoneH()-y);
	m_routewptable.SetNumCols(GPXCOL_NUMCOLUMNS);
	for(i=0;i<GPXCOL_NUMCOLUMNS;++i)
	{
		t=m_routewptable.GetColHeaderTextPtr(i);
		t->SetFontID(1);	/* bold */

		m_routewptable.SetColTitle(i,wpcolnames[i]);
		m_routewptable.SetColHint(i,wpcoldesc[i]);
		m_routewptable.SetColWidth(i,wpcolwidths[i]);
	}

//	m_routewptable.SetColHeaderCallBack(gpx,CALLBACKCLASSNAME(GPX,ColClick));
	m_routewptable.SetEventHandler(this,CALLBACKNAME(TableEvent));

	obj->AddObject(&m_routewptable);
}

void RoutesPage::Resize(int changey)
{
	m_routewptable.SetZoneH(m_routewptable.GetZoneH()+changey);
}

void RoutesPage::TableEvent(kGUIEvent *event)
{
	switch(event->GetEvent())
	{
	case EVENT_AFTERUPDATE:
		Changed();
	break;
	case EVENT_COL_RIGHTCLICK:
		gpx->ColClick();
	break;
	}
}

void RoutesPage::Load(void)
{
	int fnum;

	fnum=m_editroutelist.GetSelection();
	if(!fnum)
	{
		m_routewptable.DeleteChildren();
		m_draw.SetSelected(false);
		m_color.SetSelection("Red");
	}
	else
		m_routes.GetEntry(fnum-1)->Load(&m_routewptable,&m_draw,&m_color);
	Changed();
}

double RoutesPage::UpdateInfo(void)
{
	unsigned int e;
	GPXRow *row;
	GPXRow *lrow=0;
	double dist=0.0f,d;
	double ratio,eval1,eval2;
	/* generate results report */
	GPX::GetDistInfo(gpx->GetCurrentDist(),0,&ratio,&eval1,&eval2);

	for(e=0;e<m_routewptable.GetNumChildren();++e)
	{
		row=static_cast<GPXRow *>(m_routewptable.GetChild(e));	/* wpt row to check */
		gpx->GetColour(row);
		if(e)
		{
			d=lrow->GetCoord()->Dist(row->GetCoord())*ratio;
			row->SetDist(d);
			dist+=d;
		}
		else
			row->SetDist(0.0f);
		lrow=row;
	}
	m_num.Sprintf("%d",m_routewptable.GetNumChildren());
	m_dist.Sprintf("%.2f %s",dist,gpx->GetCurrentDistString());
	return(dist);
}

void RoutesPage::Changed(void)
{
	bool changed=false;

	gpx->MapDirty();
	UpdateInfo();

	if(!m_editroutelist.GetSelection())
	{
		/* should be empty, so any rows means changed */
		if(m_routewptable.GetNumChildren(0))
			changed=true;
		m_delete.SetEnabled(false);
		m_rename.SetEnabled(false);
		m_copy.SetEnabled(false);
	}
	else
	{
		int i=GetIndex(m_editroutelist.GetSelectionString());
		changed=m_routes.GetEntry(i)->Compare(&m_routewptable,&m_draw,&m_color);
		m_delete.SetEnabled(!changed);
		m_rename.SetEnabled(!changed);
		m_copy.SetEnabled(!changed);
	}

	m_save.SetEnabled(changed);	/* enabled after editing */
	m_undo.SetEnabled(changed); /* enabled after editing */
	m_editroutelist.SetLocked(changed);
}

int RoutesPage::GetIndex(const char *name)
{
	unsigned int i;
	GPXRoute *r;

	for(i=0;i<m_numroutes;++i)
	{
		r=m_routes.GetEntry(i);
		if(!strcmp(r->GetName(),name))
			return(i);
	}
	return(-1);
}

void RoutesPage::ClickWpntUp(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		unsigned int line;

		line=m_routewptable.GetCursorRow();
		if(line>0 && line<m_routewptable.GetNumChildren(0))
		{
			m_routewptable.SwapRow(-1);
			m_routewptable.MoveRow(-1);
			Changed();
		}
	}
}

void RoutesPage::ClickAutoOrder(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED )
	{
		unsigned int nc=m_routewptable.GetNumChildren(0);

		if(nc>1)
		{
#if 1
			AutoOrderWindow *aw;

			aw=new AutoOrderWindow(&m_routewptable);
#else
			unsigned int f;
			unsigned int s;
			kGUITSP tsp;
			GPXCoord *cp;
			GPXRow *row;
			GPXRow *newrow;
			Array<GPXRow *>rows;
			double curdist;

			tsp.Init(nc);
			rows.Alloc(nc);

			/* get the current distance for the track */
			curdist=UpdateInfo();
			for(f=0;f<nc;++f)
			{
				row=static_cast<GPXRow *>(m_routewptable.GetChild(f));
				rows.SetEntry(f,row->GetCopiedFrom());
				cp=row->GetCoord();
				tsp.SetCoord(f,cp->GetLat(),cp->GetLon());
			}

			tsp.Calc();

			m_routewptable.DeleteChildren();

			/* find position of starting point */
			s=0;
			while(tsp.GetIndex(s))
				++s;

			for(f=0;f<nc;++f)
			{
				row=rows.GetEntry(tsp.GetIndex(s));
				if(++s==nc)
					s=0;
				newrow=new GPXRow();
				newrow->Copy(row);
				m_routewptable.AddRow(newrow);
			}

			/* is this better or worse than before? */
			if(UpdateInfo()>curdist)
			{
				/* this is worse!, revert back to original order */
				m_routewptable.DeleteChildren();
				for(f=0;f<nc;++f)
				{
					row=rows.GetEntry(f);
					newrow=new GPXRow();
					newrow->Copy(row);
					m_routewptable.AddRow(newrow);
				}
			}

			Changed();
			gpx->MapDirty();
#endif
		}
	}
}

void RoutesPage::ClickWpntDown(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		int line;

		line=(int)m_routewptable.GetCursorRow();
		if(line<((int)m_routewptable.GetNumChildren(0)-1))
		{
			m_routewptable.SwapRow(1);
			m_routewptable.MoveRow(1);
			Changed();
		}
	}
}

void RoutesPage::ClickSave(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		int fnum;
		kGUIInputBoxReq *box;

		fnum=m_editroutelist.GetSelection();
		if(!fnum)
			box=new kGUIInputBoxReq(this,CALLBACKNAME(ClickSave2),"Route Name?");
		else
			Save(m_editroutelist.GetSelectionStringObj());
	}
}

void RoutesPage::ClickSave2(kGUIString *result,int closebutton)
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

void RoutesPage::Save(kGUIString *name)
{
	int i;
	GPXRoute *r;

	i=GetIndex(name->GetString());
	if(i>=0)
	{
		r=m_routes.GetEntry(i);
		r->Save(&m_routewptable,&m_draw,&m_color);
	}
	else
	{
		/* new route */
		r=new GPXRoute(name);
		r->Save(&m_routewptable,&m_draw,&m_color);
		m_routes.SetEntry(m_numroutes,r);
		++m_numroutes;
		UpdateRouteList();
		m_editroutelist.SetSelection(m_numroutes);
	}
	Load();
}

void RoutesPage::ClickDelete(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		kGUIMsgBoxReq *box;

		box=new kGUIMsgBoxReq(MSGBOX_YES|MSGBOX_NO,this,CALLBACKNAME(ClickDelete2),true,"Delete Route '%s'?",m_editroutelist.GetSelectionString());
	}
}

void RoutesPage::ClickDelete2(int result)
{
	int i;
	GPXRoute *f;

	if(result==MSGBOX_YES)
	{
		i=GetIndex(m_editroutelist.GetSelectionString());
		if(i>=0)
		{
			f=m_routes.GetEntry(i);
			delete f;
			m_routes.DeleteEntry(i);
			--m_numroutes;
			m_editroutelist.SetSelection(0);

			UpdateRouteList();
			Load();
			//SavePrefs();
			return;
		}
		assert(false,"route not found!");
	}
}

void RoutesPage::ClickRename(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		kGUIInputBoxReq *box;

		box=new kGUIInputBoxReq(this,CALLBACKNAME(ClickRename2),"New Route Name?");
	}
}

void RoutesPage::ClickRename2(kGUIString *result,int closebutton)
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
			int i;
			GPXRoute *t;

			i=GetIndex(m_editroutelist.GetSelectionString());
			if(i>=0)
			{
				t=m_routes.GetEntry(i);
				t->SetName(&newname);
				/* rename route name in select combo too! */
				m_editroutelist.SetEntry(i+1,newname.GetString(),i+1);
			}
		}
	}
}

void RoutesPage::ClickCopy(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		kGUIInputBoxReq *box;

		box=new kGUIInputBoxReq(this,CALLBACKNAME(ClickCopy2),"Copied Route Name?");
	}
}

void RoutesPage::ClickCopy2(kGUIString *result,int closebutton)
{
	kGUIString copyname;
	kGUIMsgBoxReq *box;

	if(result->GetLen() && closebutton==MSGBOX_OK)
	{
		copyname.SetString(result);
		if(GetIndex(copyname.GetString())>=0)
			box=new kGUIMsgBoxReq(MSGBOX_OK,false,"Error: name already used!");
		else
			Save(&copyname);
	}
}

void RoutesPage::ClickUndo(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		kGUIMsgBoxReq *box;

		box=new kGUIMsgBoxReq(MSGBOX_YES|MSGBOX_NO,this,CALLBACKNAME(ClickUndo2),false,"Undo Changes?");
	}
}

void RoutesPage::ClickUndo2(int result)
{
	if(result==MSGBOX_YES)
		Load();
}

void RoutesPage::UpdateRouteList(void)
{
	unsigned int e;
	kGUIString editroute;

	editroute.SetString(m_editroutelist.GetSelectionString());

	m_editroutelist.SetNumEntries(m_numroutes+1);			/* list of select routes */
	m_editroutelist.SetEntry(0,gpx->GetString(STRING_NEWROUTE),0);
	for(e=0;e<m_numroutes;++e)
		m_editroutelist.SetEntry(e+1,m_routes.GetEntry(e)->GetName(),e+1);
	m_editroutelist.SetSelection(editroute.GetString());
}

/****************************************************************************/

GPXRoute::~GPXRoute()
{
	unsigned int i;

	for(i=0;i<m_numentries;++i)
		delete m_entries.GetEntry(i);
}

void GPXRoute::Load(kGUITableObj *table,kGUITickBoxObj *draw,kGUIComboBoxObj *color)
{
	unsigned int e;
	GPXRow *row;
	GPXRow *fullrow;
	GPXRouteEntry *re;

	draw->SetSelected(GetDraw());
	color->SetSelectionz(GetColorIndex());

	table->DeleteChildren();
	for(e=0;e<m_numentries;++e)
	{
		re=m_entries.GetEntry(e);	/* route record */
		fullrow=gpx->Locate(&re->m_wptname);
		if(fullrow)
		{
			row=new GPXRow();	/* copy record from full table */
			row->Copy(fullrow);
			table->AddRow(row);
		}
	}
}

/* true=changed, false=same */

bool GPXRoute::Compare(kGUITableObj *table,kGUITickBoxObj *draw,kGUIComboBoxObj *color)
{
	unsigned int e;
	kGUIObj *obj;
	GPXRow *row;
	GPXRouteEntry *re;

	if(table->GetNumChildren(0)!=m_numentries)
		return(true);

	/* draw flag has changed */
	if(draw->GetSelected()!=GetDraw())
		return(true);

	/* color has changed? */
	if((unsigned int)color->GetSelection()!=GetColorIndex())
		return(true);

	for(e=0;e<m_numentries;++e)
	{
		re=m_entries.GetEntry(e);	/* route record */
		obj=table->GetChild(e);
		row=static_cast<GPXRow *>(obj);

		if(strcmp(row->GetWptName(),re->m_wptname.GetString()))
			return(true);
	}
	return(false);	/* same! */
}

void GPXRoute::Save(kGUITableObj *table,kGUITickBoxObj *draw,kGUIComboBoxObj *color)
{
	unsigned int e;
	unsigned int nr;
	kGUIObj *obj;
	GPXRow *row;
	GPXRouteEntry *re;

	/* delete old entries if there are any? */
	for(e=0;e<m_numentries;++e)
		delete m_entries.GetEntry(e);

	SetDraw(draw->GetSelected());
	SetColorIndex(color->GetSelection());

	nr=table->GetNumChildren(0);
	/* number of valid entries */
	m_numentries=nr;
	m_entries.Alloc(nr);
	for(e=0;e<nr;++e)
	{
		obj=table->GetChild(e);
		row=static_cast<GPXRow *>(obj);
		re=new GPXRouteEntry();
		re->m_wptname.SetString(row->GetWptName());
		m_entries.SetEntry(e,re);
	}
}

/* Load from preferences file */

void GPXRoute::Load(kGUIXMLItem *xml)
{
	int i,nc;
	GPXRouteEntry *fe;
	kGUIXMLItem *flxml;

	flxml=xml->Locate("draw");
	if(flxml)
		SetDraw(flxml->GetValueInt()?true:false);
	flxml=xml->Locate("color");
	/* convert string to color index */
	if(flxml)
		SetColorIndex(GPX::GetTableColorIndex(flxml->GetValueString()));

	nc=xml->GetNumChildren();
	for(i=0;i<nc;++i)
	{
		flxml=xml->GetChild(i);
		if(!strcmp(flxml->GetName(),"wptname"))
		{
			fe=new GPXRouteEntry();
			m_entries.SetEntry(m_numentries++,fe);
			fe->m_wptname.SetString(flxml->GetValue());
		}
	}
}

/* save to preferences file */

void GPXRoute::Save(kGUIXMLItem *xml)
{
	unsigned int e;
	unsigned int nc;
	GPXRouteEntry *re;
	kGUIXMLItem *fxml;
	kGUIXMLItem *flxml;

	/* count number of points in the route */
	nc=0;
	for(e=0;e<m_numentries;++e)
	{
		re=m_entries.GetEntry(e);	/* route record */
		if(gpx->Locate(&re->m_wptname))
			++nc;
	}

	/* if no entries in route then don't save it */
	if(nc)
	{
		fxml=xml->AddChild("route");
		fxml->AddParm("name",m_name.GetString());

		fxml->AddParm("draw",GetDraw()==true?"1":"0");
		fxml->AddParm("color",GPX::GetTableColorName(GetColorIndex()));

		for(e=0;e<m_numentries;++e)
		{
			re=m_entries.GetEntry(e);	/* route record */
	
			/* only save if point is still in the main table */
			if(gpx->Locate(&re->m_wptname))
				flxml=fxml->AddChild("wptname",re->m_wptname.GetString());
		}
	}
}

/* return true if this waypoint name is already in the table */
GPXRow *RoutesPage::InTable(const char *name)
{
	unsigned int e;
	GPXRow *row;

	for(e=0;e<m_routewptable.GetNumChildren();++e)
	{
		row=static_cast<GPXRow *>(m_routewptable.GetChild(e));
		if(!strcmp(name,row->GetWptName()))
			return(row);
	}
	return(0);

}

void RoutesPage::AddRow(GPXRow *row)
{
	GPXRow *newrow;

	newrow=new GPXRow();
	newrow->Copy(row);
	m_routewptable.AddRow(newrow);
	m_draw.SetSelected(true);		/* turn on draw if it is not already on */
	Changed();
}

void RoutesPage::Remove(const char *name)
{
	GPXRow *row;

	/* make sure point is not already in the route */
	row=InTable(name);
	if(!row)
		return;

	/* hmmm, should this be true? */
	m_routewptable.DeleteRow(row,false);
	Changed();
}

void RoutesPage::CopyColors(void)
{
	unsigned int e;
	GPXRow *row;

	/* copy colors to the route table */
	for(e=0;e<m_routewptable.GetNumChildren();++e)
	{
		row=static_cast<GPXRow *>(m_routewptable.GetChild(e));	/* wpt row to check */
		row->CopyColours(row->GetCopiedFrom());
	}
}

void GPXRoute::DrawLine(int index,int x1,int y1,int x2,int y2,kGUIColor color)
{
	kGUIPoint2 lpoints[2];
	int ex,ey,tw,th;
	kGUIText t;

	lpoints[0].x=x1;
	lpoints[0].y=y1;
	lpoints[1].x=x2;
	lpoints[1].y=y2;

	kGUI::DrawFatPolyLine(2,lpoints,color,5,0.6f);

	/* draw box 1/2 way between line */
	ex=(x1+x2)>>1;
	ey=(y1+y2)>>1;
	t.SetFontSize(WPTNAMEFONTSIZE+2);
	t.Sprintf("%d",index);
	tw=(t.GetWidth()+8)>>1;
	th=(t.GetLineHeight()+8)>>1;

	kGUI::DrawRectFrame(ex-tw,ey-th,ex+tw,ey+th,color,DrawColor(0,0,0),0.8f);
	t.Draw(ex-tw+4,ey-th+4,0,0,DrawColor(255,255,255));
}

/* draw route on the map */
void GPXRoute::Draw(kGUICorners *c)
{
	unsigned int i;
	GPXRouteEntry *re;
	kGUIColor col;
	kGUIColor col2;
	int px,py,lpx=0,lpy=0;
	GPXRow *row;

	/* todo, bounding box for route, compare against the corners */

	col=GPX::GetTableColor(GetColorIndex());
	col2=GPX::GetTableTColor(GetColorIndex());

	for(i=0;i<m_numentries;++i)
	{
		re=m_entries.GetEntry(i);
		row=gpx->Locate(&re->m_wptname);
		if(!row)
			return;	/* error, missing waypoint in track!!! */
		gpx->m_curmap->ToMap(row->GetCoord(),&px,&py);
		px-=c->lx;
		py-=c->ty;
		if(i)
			GPXRoute::DrawLine(i,px,py,lpx,lpy,col);
		lpx=px;
		lpy=py;
	}
}

/* draw all routes flagged for draw */
void RoutesPage::DrawMap(kGUICorners *c)
{
	unsigned int i;
	GPXRoute *route;

	/* draw any saved tracks that are flagged to be drawn */
	/* skipping current edited track as it will be drawn below */
	for(i=0;i<m_numroutes;++i)
	{
		/* skip current track, it is drawn below using the table */
		if(i!=((unsigned int)m_editroutelist.GetSelection()-1))
		{
			route=m_routes.GetEntry(i);
			if(route->GetDraw()==true)
				route->Draw(c);
		}
	}
	/* then draw the currently shown one if desired */
	if(m_draw.GetSelected())
	{
		kGUIColor col;
		kGUIColor col2;
		int px,py,lpx=0,lpy=0;
		GPXRow *row;

		col=GPX::GetTableColor(m_color.GetSelection());
		col2=GPX::GetTableTColor(m_color.GetSelection());

		for(i=0;i<m_routewptable.GetNumChildren();++i)
		{
			row=static_cast<GPXRow *>(m_routewptable.GetChild(i));
			gpx->m_curmap->ToMap(row->GetCoord(),&px,&py);
			px-=c->lx;
			py-=c->ty;
			if(i)
				GPXRoute::DrawLine(i,px,py,lpx,lpy,col);
			lpx=px;
			lpy=py;
		}
	}
}

/* returns true if this row is in ANY (drawable) route and if so, then sets it's sort index */
/* this is used by the print code to print the route data ( in order ) on the right side */
bool RoutesPage::InAnyRoute(GPXRow *row)
{
	unsigned int i;
	unsigned int j;
	GPXRoute *r;
	GPXRouteEntry *re;
	GPXRow *trow;

	unsigned int index=0;

	for(i=0;i<m_numroutes;++i)
	{
		r=m_routes.GetEntry(i);
		if(r->GetDraw()==true)
		{
			for(j=0;j<r->GetNumEntries();++j)
			{
				re=r->GetEntry(j);
				if(gpx->Locate(&re->m_wptname)==row)
				{
					row->SetIndex(index);	/* set sort index */
					return(true);
				}
				++index;
			}
		}
	}
	/* check the current route under construction too */
	if(m_draw.GetSelected())
	{
		for(i=0;i<m_routewptable.GetNumChildren();++i)
		{
			trow=static_cast<GPXRow *>(m_routewptable.GetChild(i));
			if(row==trow)
			{
				row->SetIndex(index++);		/* set sort index */
				return(true);
			}
			++index;
		}
	}

	return(false);
}

AutoOrderWindow::AutoOrderWindow(kGUITableObj *t)
{
	unsigned int f;
	unsigned int nc;
	GPXCoord *cp;
	GPXRow *row;

	m_t=t;
	nc=t->GetNumChildren();
	m_tsp.Init(nc);
	m_rows.Alloc(nc);

	/* get the current distance for the track */
	m_startdist=gpx->m_routes.UpdateInfo();
	for(f=0;f<nc;++f)
	{
		row=static_cast<GPXRow *>(t->GetChild(f));
		m_rows.SetEntry(f,row->GetCopiedFrom());
		cp=row->GetCoord();
		m_tsp.SetCoord(f,cp->GetLat(),cp->GetLon());
	}

	m_tsp.AsyncCalc();

	m_tries.SetPos(2,20);
	m_tries.SetSize(100,20);
	m_window.AddObject(&m_tries);

	m_best.SetPos(100+2+20,20);
	m_best.SetSize(100,20);
	m_window.AddObject(&m_best);

	m_done.SetPos(200+2+20+20,20);
	m_done.SetString("Good Enough");
	m_done.SetSize(75,20);
	m_window.AddObject(&m_done);

	m_busyrect.SetPos(2,2);
	m_busyrect.SetSize(m_done.GetZoneRX(),16);
	m_busyrect.SetIsBar(false);
	m_window.AddObject(&m_busyrect);

	m_count=0;
	m_curbest=0;

	m_window.GetTitle()->SetString("Calculating...");
	m_window.SetSize(10,10);
	m_window.ExpandToFit();
	m_window.Center();
	m_window.SetTop(true);
	kGUI::AddWindow(&m_window);

	kGUI::AddEvent(this,CALLBACKNAME(Update));
	m_window.SetEventHandler(this,CALLBACKNAME(WindowEvent));
	m_done.SetEventHandler(this,CALLBACKNAME(DoneEvent));
}

void AutoOrderWindow::Update(void)
{
	unsigned int newcount;
	unsigned int newbest;

	m_busyrect.Animate();
	newcount=m_tsp.GetCount();
	if(newcount!=m_count)
	{
		m_count=newcount;
		m_tries.Sprintf("Tries: %d",m_count);

		newbest=m_tsp.GetNewBest();
		if(newbest!=m_curbest)
		{
			unsigned int f;
			unsigned int s;
			unsigned int nc=m_t->GetNumChildren();
			int *curlist=m_tsp.GetCurList();
			double dist=0.0f,ratio,eval1,eval2;
			GPXRow *lrow;
			GPXRow *row;

			/* generate results report */
			GPX::GetDistInfo(gpx->GetCurrentDist(),0,&ratio,&eval1,&eval2);

			s=0;
			while(curlist[s])
				++s;
			lrow=0;
			for(f=0;f<nc;++f)
			{
				row=m_rows.GetEntry(curlist[s]);
				if(lrow)
					dist+=(lrow->GetCoord()->Dist(row->GetCoord())*ratio);
				lrow=row;
				if(++s==nc)
					s=0;
			}
			if(dist<m_startdist)
				m_best.Sprintf("Distance: %.2f",dist);
		}
	}
}

void AutoOrderWindow::WindowEvent(kGUIEvent *event)
{
	switch(event->GetEvent())
	{
	case EVENT_CLOSE:
		delete this;
	break;
	}
}

void AutoOrderWindow::DoneEvent(kGUIEvent *event)
{
	switch(event->GetEvent())
	{
	case EVENT_PRESSED:
		m_window.Close();
	break;
	}
}

AutoOrderWindow::~AutoOrderWindow()
{
	unsigned int s;
	unsigned int nc=m_t->GetNumChildren();
	unsigned int f;
	GPXRow *newrow;
	GPXRow *row;

	/* wait for tsp to finish */
	m_tsp.Stop();
	while(m_tsp.GetActive());

	kGUI::DelEvent(this,CALLBACKNAME(Update));
	m_t->DeleteChildren();

	/* find position of starting point */
	s=0;
	while(m_tsp.GetIndex(s))
		++s;

	for(f=0;f<nc;++f)
	{
		row=m_rows.GetEntry(m_tsp.GetIndex(s));
		if(++s==nc)
			s=0;
		newrow=new GPXRow();
		newrow->Copy(row);
		m_t->AddRow(newrow);
	}

	/* is this better or worse than before? */
	if(gpx->m_routes.UpdateInfo()>m_startdist)
	{
		/* this is worse!, revert back to original order */
		m_t->DeleteChildren();
		for(f=0;f<nc;++f)
		{
			row=m_rows.GetEntry(f);
			newrow=new GPXRow();
			newrow->Copy(row);
			m_t->AddRow(newrow);
		}
	}

	gpx->m_routes.Changed();
	gpx->MapDirty();

	kGUI::DelWindow(&m_window);
}
