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
#include "uploadxml.h"
#include "babelglue.h"

class GPXTrackEntry
{
	friend class TracksPage;
public:
	double m_lat;
	double m_lon;
	double m_elev;
	kGUIString m_time;
	kGUIString m_date;
	void Load(kGUIXMLItem *xml);
	void Save(kGUIXMLItem *xml);
};

class GPXTrack
{
	friend class TracksPage;
public:
	GPXTrack(const char *name) {m_draw=false;m_color=0;m_name.SetString(name);m_numentries=0;m_entries.SetGrow(true);m_entries.Alloc(16);m_numareapoints=0;}
	~GPXTrack();
	void SetName(const char *name) {m_name.SetString(name);}
	const char *GetName(void) {return m_name.GetString();}
	void SetDraw(bool d) {m_draw=d;}
	bool GetDraw(void) {return m_draw;}
	void SetColorIndex(unsigned int index) {m_color=index;}
	unsigned int GetColorIndex(void) {return m_color;}
	unsigned int GetNumPoints(void) {return m_numentries;}
	const char *Getm_starttime(void);
	void Load(kGUITableObj *table,kGUITickBoxObj *draw,kGUIComboBoxObj *color);
	bool Compare(kGUITableObj *table,kGUITickBoxObj *draw,kGUIComboBoxObj *color);
	void Save(kGUITableObj *table,kGUITickBoxObj *draw,kGUIComboBoxObj *color);
	unsigned int Changed(GPXTrack *t2);
	void Load(kGUIXMLItem *xml);
	void Save(kGUIXMLItem *xml);
	void UpdateBounds(void);
	GPXBounds *GetBounds(void) {return &m_bounds;}
	bool TrackRow(GPXRow *trow);
	GPXTrackEntry *AddEntry(void) {GPXTrackEntry *fe=new GPXTrackEntry();m_entries.SetEntry(m_numentries++,fe);return fe;}
	bool IsPointInside(double lat,double lon);
	void Draw(kGUICorners *c);
	private:
	kGUIString m_name;
	bool m_draw;
	unsigned int m_color;
	unsigned int m_numentries;
	Array<GPXTrackEntry *>m_entries;

	/* this array is populated if this track is used as an area with a filter */
	unsigned int m_numareapoints;
	Array<kGUIDPoint2> m_areapoints; 
	GPXBounds m_bounds;
};

enum
{
TRACKCOL_LAT,
TRACKCOL_LON,
TRACKCOL_ELEV,
TRACKCOL_TIME,
TRACKCOL_NUMCOLUMNS};

const char *trcolnames[TRACKCOL_NUMCOLUMNS]={
	"Lat","Lon","Elev","Time"};

const char *trcoldesc[TRACKCOL_NUMCOLUMNS]={
	"Lat","Lon","Elev","Time"};
	
const int trcolwidths[TRACKCOL_NUMCOLUMNS]={80,80,80,150};

class GPXTrackRow : public kGUITableRowObj
{
public:
	GPXTrackRow();
	int GetNumObjects(void) {return TRACKCOL_NUMCOLUMNS;}
	kGUIObj **GetObjectList(void) {return m_objectlist;} 
	kGUIObj *m_objectlist[TRACKCOL_NUMCOLUMNS];
	CALLBACKGLUEPTR(GPXTrackRow,LocChanged,kGUIEvent)
	//private:
	void LocChanged(kGUIEvent *event);
	GPXCoord m_llcoord;
	kGUIInputBoxObj m_lat;
	kGUIInputBoxObj m_lon;
	kGUIInputBoxObj m_elev;
	kGUIInputBoxObj m_time;
};

TracksPage::TracksPage()
{
	m_numtracks=0;
	m_tracks.Init(8,2);
}

TracksPage::~TracksPage()
{
}

/* if the hash table is passed, then only load tracks whose names are in it */
void TracksPage::LoadPrefs(kGUIXMLItem *root,Hash *hash)
{
	unsigned int i;
	bool add=true;
	kGUIXMLItem *item;
	kGUIString name;
	kGUIString trackname;
	int version;

	if(!hash)
	{
		item=root->Locate("tracktable");
		if(item)
			m_table.LoadConfig(item);
	}

	for(i=0;i<root->GetNumChildren();++i)
	{
		item=root->GetChild(i);
		if(!strcmp(item->GetName(),"trk"))
		{
			if(item->Locate("name"))		/* skip tracks without a name */
			{
				name.SetString(item->Locate("name")->GetValue());
				
				if(hash)
				{
					if(hash->Find(name.GetString()))
						add=true;
					else
						add=false;
				}

				if(add==true)
				{
					GPXTrack *s;
		
					/* if track name already exists, then generate a unique name */
					version=0;
					do
					{
						if(!version)
							trackname.SetString(&name);
						else
							trackname.Sprintf("%s.%d",name.GetString(),version);
						++version;

						/* loop until we get a name that isn't already used */
						if(GetIndex(trackname.GetString())<0)
							break;
					}while(1);

					s=new GPXTrack(trackname.GetString());
					m_tracks.SetEntry(m_numtracks++,s);
					s->Load(item);
				}
			}
		}
	}
	UpdateTracksList();
}

/* if the hash table is passed, then only save tracks whose names are in it */
int TracksPage::SavePrefs(kGUIXMLItem *root,Hash *hash,GPXBounds *bounds)
{
	unsigned int i;
	GPXTrack *track;
	int numsaved=0;
	bool add=true;

	if(!hash)
		m_table.SaveConfig(root,"tracktable");

	for(i=0;i<m_numtracks;++i)
	{
		track=m_tracks.GetEntry(i);
		if(hash)
		{
			if(hash->Find(track->GetName()))
				add=true;
			else
				add=false;
		}
		if(add==true)
		{
			track->Save(root);
			if(bounds)
				bounds->Add(track->GetBounds());
			++numsaved;
		}
	}
	return(numsaved);
}


/* add gui items to their parent gui object */
void TracksPage::Init(kGUIContainerObj *obj)
{
	int i,y,bw,bh;
	kGUIText *t;

	bw=obj->GetChildZoneW();

	m_editcontrols.SetSize(bw,20);

	m_edittracklist.SetSize(300,20);
	m_edittracklist.SetNumEntries(1);				/* list of Tracks */
	m_edittracklist.SetEntry(0,"New Track",0);
	m_edittracklist.SetEventHandler(this,CALLBACKNAME(LoadEvent));
	m_editcontrols.AddObject(&m_edittracklist);

	m_save.SetSize(100,25);
	m_save.SetString("Save Track");
	m_save.SetEventHandler(this,CALLBACKNAME(ClickSave));
	m_editcontrols.AddObject(&m_save);

	m_undo.SetSize(100,25);
	m_undo.SetString("Undo Changes");
	m_undo.SetEventHandler(this,CALLBACKNAME(ClickUndo));
	m_editcontrols.AddObject(&m_undo);

	m_delete.SetSize(100,25);
	m_delete.SetString("Delete Track");
	m_delete.SetEventHandler(this,CALLBACKNAME(ClickDelete));
	m_editcontrols.AddObject(&m_delete);

	m_rename.SetSize(100,25);
	m_rename.SetString("Rename Track");
	m_rename.SetEventHandler(this,CALLBACKNAME(ClickRename));
	m_editcontrols.AddObject(&m_rename);

	m_copy.SetSize(100,25);
	m_copy.SetString("Copy Track");
	m_copy.SetEventHandler(this,CALLBACKNAME(ClickCopy));
	m_editcontrols.AddObject(&m_copy);

	m_simplify.SetSize(100,25);
	m_simplify.SetString("Simplify Track");
	m_simplify.SetEventHandler(this,CALLBACKNAME(ClickSimplify));
	m_editcontrols.AddObject(&m_simplify);

	m_editcontrols.NextLine();

	m_drawcaption.SetPos(0,0);
	m_drawcaption.SetFontSize(SMALLCAPTIONSIZE);
	m_drawcaption.SetFontID(SMALLCAPTIONFONT);
	m_drawcaption.SetString("Draw Track");
	m_draw.SetPos(0,15);
	m_draw.SetEventHandler(this,CALLBACKNAME(ChangedEvent));
	m_editcontrols.AddObjects(2,&m_drawcaption,&m_draw);

	m_colorcaption.SetPos(0,0);
	m_colorcaption.SetFontSize(SMALLCAPTIONSIZE);
	m_colorcaption.SetFontID(SMALLCAPTIONFONT);
	m_colorcaption.SetString("Track color");

	m_color.SetPos(0,15);
	m_color.SetSize(275,20);
	GPX::InitColorCombo(&m_color);
	m_color.SetSelection("Magenta");
	m_color.SetEventHandler(this,CALLBACKNAME(ChangedEvent));
	m_editcontrols.AddObjects(2,&m_colorcaption,&m_color);

	m_editcontrols.NextLine();

	/* display info for track, not editable */
	m_numcaption.SetPos(0,0);
	m_numcaption.SetFontSize(SMALLCAPTIONSIZE);
	m_numcaption.SetFontID(SMALLCAPTIONFONT);
	m_numcaption.SetString("Total Track Points");

	m_num.SetPos(0,15);
	m_num.SetSize(80,20);
	m_num.SetLocked(true);
	m_num.SetHint("Number of points on the current track.");
	m_editcontrols.AddObjects(2,&m_numcaption,&m_num);

	m_distcaption.SetPos(0,0);
	m_distcaption.SetFontSize(SMALLCAPTIONSIZE);
	m_distcaption.SetFontID(SMALLCAPTIONFONT);
	m_distcaption.SetString("Total Track Distance");

	m_dist.SetPos(0,15);
	m_dist.SetSize(150,20);
	m_dist.SetLocked(true);
	m_dist.SetHint("Distance covered along track.");
	m_editcontrols.AddObjects(2,&m_distcaption,&m_dist);

	m_timecaption.SetPos(0,0);
	m_timecaption.SetFontSize(SMALLCAPTIONSIZE);
	m_timecaption.SetFontID(SMALLCAPTIONFONT);
	m_timecaption.SetString("Total Track Time");

	m_time.SetPos(0,15);
	m_time.SetSize(300,20);
	m_time.SetLocked(true);
	m_time.SetHint("Time used to generate track.");
	m_editcontrols.AddObjects(2,&m_timecaption,&m_time);

	m_delete.SetEnabled(false);
	m_rename.SetEnabled(false);
	m_copy.SetEnabled(false);
	m_simplify.SetEnabled(false);

	obj->AddObject(&m_editcontrols);
	
	y=m_editcontrols.GetZoneH();
	bh=(obj->GetChildZoneH()-y);

	m_table.SetPos(0,y);
	m_table.SetSize(bw,bh-y);

	m_table.SetNumCols(TRACKCOL_NUMCOLUMNS);
	for(i=0;i<TRACKCOL_NUMCOLUMNS;++i)
	{
		t=m_table.GetColHeaderTextPtr(i);
		t->SetFontID(1);	/* bold */

		m_table.SetColTitle(i,trcolnames[i]);
		m_table.SetColHint(i,trcoldesc[i]);
		m_table.SetColWidth(i,trcolwidths[i]);
	}

	m_table.SetAllowAddNewRow(true);
	m_table.SetEventHandler(this,CALLBACKNAME(TableEvent));

	obj->AddObject(&m_editcontrols);
	obj->AddObject(&m_table);
}

void TracksPage::TableEvent(kGUIEvent *event)
{
	switch(event->GetEvent())
	{
	case EVENT_AFTERUPDATE:
		Changed();
	break;
	case EVENT_ADDROW:
	{
		GPXTrackRow *fr=new GPXTrackRow();
		m_table.AddRow(fr);
	}
	break;
	case EVENT_ROW_LEFTDOUBLECLICK:
	{
		GPXCoord c;
		GPXTrackRow *row;
		unsigned int r=event->m_value[0].ui;

		row=static_cast<GPXTrackRow *>(m_table.GetChild(r));
		c.Set(&row->m_llcoord);
		gpx->SetScrollCenter(&c);
		gpx->SetCurrentTab(TAB_MAIN);
	}
	break;
	}
}

void TracksPage::Purge(void)
{
	unsigned int i;

	m_table.DeleteChildren();

	for(i=0;i<m_numtracks;++i)
        delete m_tracks.GetEntry(i);
}

void TracksPage::AddPointToTrack(double lat,double lon)
{
	GPXTrackRow *fr=new GPXTrackRow();

	m_table.AddRow(fr);

	m_draw.SetSelected(true);	/* turn on drawing for this track */
	fr->m_llcoord.Set(lat,lon);
	fr->m_llcoord.Output(&fr->m_lat,&fr->m_lon);
	Changed();
}


GPXTrackRow::GPXTrackRow()
{
	m_objectlist[TRACKCOL_LAT]=&m_lat;
	m_objectlist[TRACKCOL_LON]=&m_lon;
	m_objectlist[TRACKCOL_ELEV]=&m_elev;
	m_objectlist[TRACKCOL_TIME]=&m_time;
	SetRowHeight(20);

	m_lon.SetEventHandler(this,CALLBACKNAME(LocChanged));
	m_lat.SetEventHandler(this,CALLBACKNAME(LocChanged));
//	m_lat.SetDoubleClick(this,CALLBACKNAME(GotoOnMap));
//	m_lon.SetDoubleClick(this,CALLBACKNAME(GotoOnMap));
}

void GPXTrackRow::LocChanged(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_AFTERUPDATE)
	{
		/* convert the 2 strings into numeric lat/lon */
		m_llcoord.Set(m_lat.GetString(),m_lon.GetString());
	}
}

#if 0
/* center map on position */
void GPXTrackRow::GotoOnMap(void)
{
	gpx->SetScrollCenter(&m_llcoord);
	gpx->SetCurrentTab(TAB_MAIN);
}
#endif

/* load from prefs file */
void GPXTrackEntry::Load(class kGUIXMLItem *wpr)
{
	m_lat=wpr->Locate("lat")->GetValueDouble();
	m_lon=wpr->Locate("lon")->GetValueDouble();
	if(wpr->Locate("ele"))
		m_elev=wpr->Locate("ele")->GetValueDouble();
	else
		m_elev=0.0f;
	if(wpr->Locate("time"))
		m_time.SetString(wpr->Locate("time")->GetValue());
}

/* save to prefs file */
void GPXTrackEntry::Save(class kGUIXMLItem *wpr)
{
	wpr->AddParm("lat",m_lat);
	wpr->AddParm("lon",m_lon);
	wpr->AddChild("ele",m_elev);
	wpr->AddChild("time",m_time.GetString());
}

int TracksPage::GetIndex(const char *name)
{
	unsigned int i;
	GPXTrack *r;

	for(i=0;i<m_numtracks;++i)
	{
		r=m_tracks.GetEntry(i);
		if(!strcmp(r->GetName(),name))
			return(i);
	}
	return(-1);
}

void TracksPage::Load(void)
{
	int fnum;

	fnum=m_edittracklist.GetSelection();
	if(!fnum)
	{
		m_draw.SetSelected(false);
		m_color.SetSelection("Magenta");
		m_table.DeleteChildren();
		m_num.Clear();
		m_dist.Clear();
		m_time.Clear();
	}
	else
		m_tracks.GetEntry(fnum-1)->Load(&m_table,&m_draw,&m_color);

	Changed();
}

/* update bounds on current track, and regenerate the track length, time and elevation etc. */

void TracksPage::UpdateTrackInfo(void)
{
	unsigned int i;
	GPXTrackRow *row;
	GPXTrackRow *row2;
	double dist,ratio,eval1,eval2;

	/* generate results report */
	GPX::GetDistInfo(gpx->GetCurrentDist(),0,&ratio,&eval1,&eval2);

	m_num.Sprintf("%d",m_table.GetNumChildren(0));
	
	/* calc distance of track */
	dist=0.0f;
	m_curbounds.Init();
	if(m_table.GetNumChildren(0))
	{
		for(i=0;i<(m_table.GetNumChildren(0)-1);++i)
		{
			row=static_cast<GPXTrackRow *>(m_table.GetChild(i));
			row2=static_cast<GPXTrackRow *>(m_table.GetChild(i+1));
			dist+=row->m_llcoord.Dist(&row2->m_llcoord)*ratio;
			m_curbounds.Add(row->m_llcoord.GetLat(),row->m_llcoord.GetLon());
		}
	}
	m_dist.Sprintf("%.2f %s",dist,gpx->GetCurrentDistString());

	/* calc elapsed time of track */
	m_time.Clear();
	if(m_table.GetNumChildren(0)>1)
	{
		const char *t;
		kGUIDate m_starttime;
		kGUIDate endtime;
		int d,h,m,s;

		t=static_cast<GPXTrackRow *>(m_table.GetChild(0))->m_time.GetString();
		if(strlen(t)>10)
		{
			m_starttime.Set(t);
			t=static_cast<GPXTrackRow *>(m_table.GetChild(m_table.GetNumChildren(0)-1))->m_time.GetString();
			if(strlen(t)>10)
			{
				endtime.Set(t);

				/* difference in seconds */
				s=m_starttime.GetDiffSeconds(&endtime);
				d=0;
				while(s>=(60*60*24))
				{
					s-=60*60*24;
					++d;
				}
				h=0;
				while(s>=(60*60))
				{
					s-=60*60;
					++h;
				}
				m=0;
				while(s>=60)
				{
					s-=60;
					++m;
				}

				if(d)
				{
					if(d==1)
						m_time.ASprintf("1 day, ");
					else
						m_time.ASprintf("%d days, ",d);
				}
				if(d || h)
				{
					if(h==1)
						m_time.ASprintf("1 hour, ");
					else
						m_time.ASprintf("%d hours, ",h);
				}
				if(d || h || m)
				{
					if(m==1)
						m_time.ASprintf("1 minute, ");
					else
						m_time.ASprintf("%d minutes, ",m);
				}
				if(s==1)
					m_time.ASprintf("1 second");
				else
					m_time.ASprintf("%d seconds",s);
			}
		}
	}
}

/* update the save/undo button status based on if the filter has changed */

void TracksPage::Changed(void)
{
	bool changed=false;

	UpdateTrackInfo();

	/* is this a new track or an exisiting one? */
	if(!m_edittracklist.GetSelection())
	{
		/* should be empty, so any rows means changed */
		if(m_table.GetNumChildren(0))
			changed=true;
		m_delete.SetEnabled(false);
		m_rename.SetEnabled(false);
		m_copy.SetEnabled(false);
		m_simplify.SetEnabled(false);
	}
	else
	{
		int i=GetIndex(m_edittracklist.GetSelectionString());
		changed=m_tracks.GetEntry(i)->Compare(&m_table,&m_draw,&m_color);
		m_delete.SetEnabled(!changed);
		m_rename.SetEnabled(!changed);
		m_copy.SetEnabled(!changed);
		m_simplify.SetEnabled(!changed);
	}

	m_save.SetEnabled(changed);	/* enabled after editing */
	m_undo.SetEnabled(changed); /* enabled after editing */
	m_edittracklist.SetLocked(changed);

	/* redraw the tiles since the track selected has had changes made to it */
	gpx->MapDirty();
}

void TracksPage::ClickSave(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		int fnum;
		kGUIInputBoxReq *box;

		fnum=m_edittracklist.GetSelection();
		if(!fnum)
			box=new kGUIInputBoxReq(this,CALLBACKNAME(ClickSave2),"Track Name?");
		else
		{
			Save(m_edittracklist.GetSelectionString());

			/* if this track is in the currently used filter then trigger a refilter */
			gpx->GetFilterObj()->TrackSaved(m_edittracklist.GetSelectionString());
		}
	}
}

void TracksPage::ClickSave2(kGUIString *result,int closebutton)
{
	kGUIMsgBoxReq *box;

	if(result->GetLen() && closebutton==MSGBOX_OK)
	{
		if(GetIndex(result->GetString())>=0)
			box=new kGUIMsgBoxReq(MSGBOX_OK,false,"Error: name already used, try again with a different name!");
		else
			Save(result->GetString());
	}
}

void TracksPage::Save(const char *name)
{
	int t;
	GPXTrack *robj;

	/* is this an existing track? */
	t=GetIndex(name);
	if(t>=0)
	{
		robj=m_tracks.GetEntry(t);
		robj->Save(&m_table,&m_draw,&m_color);
	}
	else
	{
		/* new Track */
		robj=new GPXTrack(name);
		robj->Save(&m_table,&m_draw,&m_color);
		m_tracks.SetEntry(m_numtracks,robj);
		++m_numtracks;
		UpdateTracksList();
		m_edittracklist.SetSelection(m_numtracks);
	}
	Load();
}

void TracksPage::ClickDelete(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		kGUIMsgBoxReq *box;
	
		box=new kGUIMsgBoxReq(MSGBOX_YES|MSGBOX_NO,this,CALLBACKNAME(ClickDelete2),true,"Delete Track '%s'?",m_edittracklist.GetSelectionString());
	}
}

void TracksPage::ClickDelete2(int result)
{
	int i;
	GPXTrack *f;

	if(result==MSGBOX_YES)
	{
		i=GetIndex(m_edittracklist.GetSelectionString());
		if(i>=0)
		{
			f=m_tracks.GetEntry(i);
			delete f;
			m_tracks.DeleteEntry(i);
			--m_numtracks;
			m_edittracklist.SetSelection(0);

			UpdateTracksList();
			Load();
			//SavePrefs();
			return;
		}
		assert(false,"Tracks not found!");
	}
}

void TracksPage::ClickRename(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		kGUIInputBoxReq *box;
	
		box=new kGUIInputBoxReq(this,CALLBACKNAME(ClickRename2),"New Track Name?");
	}
}

void TracksPage::ClickRename2(kGUIString *result,int closebutton)
{
	const char *newname;
	kGUIMsgBoxReq *box;
	kGUIString oldname;

	if(result->GetLen() && closebutton==MSGBOX_OK)
	{
		newname=result->GetString();
		if(GetIndex(newname)>=0)
			box=new kGUIMsgBoxReq(MSGBOX_OK,false,"Error: name already used!");
		else
		{
			unsigned int i;
			GPXTrack *t;

			i=GetIndex(m_edittracklist.GetSelectionString());
			if(i>=0)
			{
				t=m_tracks.GetEntry(i);
				oldname.SetString(t->GetName());
				t->SetName(newname);

				/* rename track in any filters that reference tracks */
				gpx->GetFilterObj()->UpdateTrackNames(oldname.GetString(),newname);

				/* rename track name in select combo too! */
				m_edittracklist.SetEntry(i+1,newname,i+1);
			}
		}
	}
}

void TracksPage::ClickCopy(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		kGUIInputBoxReq *box;
	
		box=new kGUIInputBoxReq(this,CALLBACKNAME(ClickCopy2),"Copied Track Name?");
	}
}

void TracksPage::ClickCopy2(kGUIString *result,int closebutton)
{
	const char *copyname;
	kGUIMsgBoxReq *box;

	if(result->GetLen() && closebutton==MSGBOX_OK)
	{
		copyname=result->GetString();
		if(GetIndex(copyname)>=0)
			box=new kGUIMsgBoxReq(MSGBOX_OK,false,"Error: name already used!");
		else
			Save(copyname);
	}
}

void TracksPage::ClickUndo(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		kGUIMsgBoxReq *box;
	
		box=new kGUIMsgBoxReq(MSGBOX_YES|MSGBOX_NO,this,CALLBACKNAME(ClickUndo2),false,"Undo Changes?");
	}
}

void TracksPage::ClickUndo2(int result)
{
	if(result==MSGBOX_YES)
		Load();
}

void TracksPage::UpdateTracksList(void)
{
	unsigned int e;
	kGUIString editTracks;

	editTracks.SetString(m_edittracklist.GetSelectionString());

	m_edittracklist.SetNumEntries(m_numtracks+1);			/* list of select Trackss */
	m_edittracklist.SetEntry(0,"New Track",0);
	for(e=0;e<m_numtracks;++e)
		m_edittracklist.SetEntry(e+1,m_tracks.GetEntry(e)->GetName(),e+1);
	m_edittracklist.SetSelectionz(editTracks.GetString());

	/* if any filters use the isinside then they also need to regenerate their combo boxes too */
	gpx->GetFilterObj()->UpdateTrackNames(0,0);
}

void GPXTrack::Draw(kGUICorners *c)
{
	unsigned int e;
	unsigned int ne;
	GPXTrackEntry *te;
	kGUICorners b;
	kGUIColor col;
	kGUIColor col2;
	GPXCoord pos;
	int px,py,lpx=0,lpy=0;
	kGUIPoint2 lpoints[2];

	/* first, project bounds for track and see if it overlaps the draw area */
	pos.Set(m_bounds.GetMinLat(),m_bounds.GetMinLon());
	gpx->m_curmap->ToMap(&pos,&b.lx,&b.by);
	pos.Set(m_bounds.GetMaxLat(),m_bounds.GetMaxLon());
	gpx->m_curmap->ToMap(&pos,&b.rx,&b.ty);

	/* return if track does not overlap draw area */
	if(kGUI::Overlap(c,&b)==false)
		return;

	col=GPX::GetTableColor(GetColorIndex());
	col2=GPX::GetTableTColor(GetColorIndex());
	ne=GetNumPoints();
	for(e=0;e<ne;++e)
	{
		te=m_entries.GetEntry(e);

		pos.Set(te->m_lat,te->m_lon);
		gpx->m_curmap->ToMap(&pos,&px,&py);
		px-=c->lx;
		py-=c->ty;

		if(e)
		{
			lpoints[0].x=px;
			lpoints[0].y=py;
			lpoints[1].x=lpx;
			lpoints[1].y=lpy;
			kGUI::DrawFatPolyLine(2,lpoints,col,3,0.7f);
			kGUI::DrawPixel(lpx,lpy,col2);
			kGUI::DrawPixel(px,py,col2);
		}
		lpx=px;
		lpy=py;
	}
}

/* draw track line */
void TracksPage::DrawMap(kGUICorners *c)
{
	unsigned int e;
	unsigned int i;
	unsigned int ne;
	GPXTrack *track;
	GPXTrackRow *row;
	kGUIPoint2 lpoints[2];
	int px,py,lpx=0,lpy=0;
	kGUIColor col;
	kGUIColor col2;
	kGUICorners b;
	GPXCoord pos;

	/* draw any saved tracks that are flagged to be drawn */
	/* skipping current edited track as it will be drawn below */
	for(i=0;i<m_numtracks;++i)
	{
		/* skip current track, it is drawn below using the table */
		if(i!=((unsigned int)m_edittracklist.GetSelection()-1))
		{
			track=m_tracks.GetEntry(i);
			if(track->GetDraw()==true)
				track->Draw(c);
		}
	}

	/* draw current track being edited */
	if(m_draw.GetSelected())
	{
		/* first, project bounds for the current track and see if it overlaps the draw area */
		pos.Set(m_curbounds.GetMinLat(),m_curbounds.GetMinLon());
		gpx->m_curmap->ToMap(&pos,&b.lx,&b.by);
		pos.Set(m_curbounds.GetMaxLat(),m_curbounds.GetMaxLon());
		gpx->m_curmap->ToMap(&pos,&b.rx,&b.ty);

		/* only draw if track overlaps the draw area */
		if(kGUI::Overlap(c,&b)==true)
		{
			col=GPX::GetTableColor(m_color.GetSelection());
			col2=GPX::GetTableTColor(m_color.GetSelection());

			ne=m_table.GetNumChildren();
			for(e=0;e<ne;++e)
			{
				row=static_cast<GPXTrackRow *>(m_table.GetChild(e));

				gpx->m_curmap->ToMap(&row->m_llcoord,&px,&py);
				px-=c->lx;
				py-=c->ty;

				if(e)
				{
					lpoints[0].x=px;
					lpoints[0].y=py;
					lpoints[1].x=lpx;
					lpoints[1].y=lpy;
					kGUI::DrawFatPolyLine(2,lpoints,col,3,0.7f);
					kGUI::DrawPixel(lpx,lpy,col2);
					kGUI::DrawPixel(px,py,col2);
				}
				lpx=px;
				lpy=py;
			}
		}
	}
}

/*********************************************************/

const char *GPXTrack::Getm_starttime(void)
{
	if(!m_numentries)
		return(0);		/* N/A time as no entries */

	/* return pointer to time for first entry */
	return m_entries.GetEntry(0)->m_time.GetString();
}

GPXTrack::~GPXTrack()
{
	unsigned int i;

	for(i=0;i<m_numentries;++i)
		delete m_entries.GetEntry(i);
}

void GPXTrack::Load(kGUITableObj *table,kGUITickBoxObj *draw,kGUIComboBoxObj *color)
{
	unsigned int e;
	GPXTrackRow *row;
	GPXTrackEntry *re;

	if(draw)
		draw->SetSelected(GetDraw());
	if(color)
		color->SetSelection(GetColorIndex());

	table->DeleteChildren();
	for(e=0;e<m_numentries;++e)
	{
		re=m_entries.GetEntry(e);	/* track record */
		row=new GPXTrackRow();
	
		row->m_llcoord.Set(re->m_lat,re->m_lon);
		row->m_llcoord.Output(&row->m_lat,&row->m_lon);
		row->m_elev.Sprintf("%f",re->m_elev);
		row->m_time.SetString(re->m_time.GetString());
		table->AddRow(row);
	}
}

void GPXTrack::UpdateBounds(void)
{
	unsigned int e;
	GPXTrackEntry *re;

	m_bounds.Init();

	for(e=0;e<m_numentries;++e)
	{
		re=m_entries.GetEntry(e);	/* track record */
		m_bounds.Add(re->m_lat,re->m_lon);
	}
}

/* true=changed, false=same */

bool GPXTrack::Compare(kGUITableObj *table,kGUITickBoxObj *draw,kGUIComboBoxObj *color)
{
	unsigned int e;
	GPXTrackRow *row;
	GPXTrackEntry *re;

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
		re=m_entries.GetEntry(e);	/* track record */
		row=static_cast<GPXTrackRow *>(table->GetChild(e));

		if(re->m_lat!=row->m_llcoord.GetLat())
			return(true);	/* different */
		if(re->m_lon!=row->m_llcoord.GetLon())
			return(true);	/* different */
		if(re->m_elev!=row->m_elev.GetDouble())
			return(true);	/* different */

		if(strcmp(re->m_time.GetString(),row->m_time.GetString()))
			return(true);	/* different */
	}
	return(false);	/* same! */
}

void GPXTrack::Save(kGUITableObj *table,kGUITickBoxObj *draw,kGUIComboBoxObj *color)
{
	unsigned int e;
	unsigned int nr;
	kGUIObj *obj;
	GPXTrackRow *row;
	GPXTrackEntry *re;

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
		row=static_cast<GPXTrackRow *>(obj);
		re=new GPXTrackEntry();
				
		re->m_lat=row->m_llcoord.GetLat();
		re->m_lon=row->m_llcoord.GetLon();
		re->m_elev=row->m_elev.GetDouble();
		re->m_time.SetString(row->m_time.GetString());
		m_entries.SetEntry(e,re);
	}
	UpdateBounds();
	/* reset the area info so it will be regenerated if this track is used */
	/* in a isinside filter */
	m_numareapoints=0;
}

/* Load from preferences file */

void GPXTrack::Load(kGUIXMLItem *xml)
{
	int i,nc;
	GPXTrackEntry *fe;
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
		if(!strcmp(flxml->GetName(),"trkpt"))
		{
			fe=new GPXTrackEntry();
			fe->Load(flxml);
			m_entries.SetEntry(m_numentries++,fe);
		}
		else if(!strcmp(flxml->GetName(),"trkseg"))
			Load(flxml);
	}
	UpdateBounds();
	/* reset the area info so it will be regenerated if this track is used */
	/* in a isinside filter */
	m_numareapoints=0;
}

/* save to preferences file */

void GPXTrack::Save(kGUIXMLItem *xml)
{
	unsigned int e;
	GPXTrackEntry *re;
	kGUIXMLItem *fxml;
	kGUIXMLItem *flxml;
	kGUIXMLItem *fsegxml;

	/* if no entries in track then don't save it */
	if(m_numentries)
	{
		fxml=xml->AddChild("trk");
		fxml->AddChild("name",m_name.GetString());

		fxml->AddParm("draw",GetDraw()==true?"1":"0");
		fxml->AddParm("color",GPX::GetTableColorName(GetColorIndex()));

		fsegxml=fxml->AddChild("trkseg");
		for(e=0;e<m_numentries;++e)
		{
			re=m_entries.GetEntry(e);	/* track record */
			flxml=fsegxml->AddChild("trkpt");
			re->Save(flxml);
		}
	}
}

/**************************************************************/

enum
{
LOADTRACK_SELECTED,
LOADTRACK_NAME,
LOADTRACK_NUMPOINTS,
LOADTRACK_m_starttime,
LOADTRACK_NUMCOLUMNS
};

class SelectTracksRow : public kGUITableRowObj
{
public:
	SelectTracksRow(const char *name,int numpts,const char *starttime);
	~SelectTracksRow() {}
	int GetNumObjects(void) {return LOADTRACK_NUMCOLUMNS;}
	kGUIObj **GetObjectList(void) {return m_objectlist;} 
	kGUIObj *m_objectlist[LOADTRACK_NUMCOLUMNS];
	bool GetSelected(void) {return m_selected.GetSelected();}
	const char *GetName(void) {return m_name.GetString();}
private:
	kGUITickBoxObj m_selected;
	kGUIInputBoxObj m_name;
	kGUIInputBoxObj m_numpoints;
	kGUIInputBoxObj m_starttime;
};

SelectTracksRow::SelectTracksRow(const char *name,int numpts,const char *starttime)
{
	m_objectlist[LOADTRACK_SELECTED]=&m_selected;
	m_objectlist[LOADTRACK_NAME]=&m_name;
	m_objectlist[LOADTRACK_NUMPOINTS]=&m_numpoints;
	m_objectlist[LOADTRACK_m_starttime]=&m_starttime;

	m_name.SetString(name);
	m_numpoints.SetInt(numpts);
	if(starttime)
	{
		m_starttime.SetString(starttime);
		m_starttime.Replace("T"," ");
		m_starttime.Replace("Z","");
	}

	m_name.SetLocked(true);
	m_numpoints.SetLocked(true);
	m_starttime.SetLocked(true);
}

const char *ltcolnames[LOADTRACK_NUMCOLUMNS]={
	"Select","Name","Num Points","Start Time"};

const char *ltcoldesc[LOADTRACK_NUMCOLUMNS]={
	"Select Track","Track Name","Number of Points in Track","Track Start Time"};
	
const int ltcolwidths[LOADTRACK_NUMCOLUMNS]={60,200,75,125};

SelectTracks::SelectTracks(Hash *hash,void *codeobj,void (*code)(void *,int),kGUIXML *xml)
{
	unsigned int i;
	SelectTracksRow *tsr;
	GPXTrack *track;
	int w=600,h=300;
	TracksPage *tp=gpx->GetTrackObj();

	m_donecallback.Set(codeobj,code);
	m_pressed=MSGBOX_CANCEL;
	hash->Init(12,0);
	m_hash=hash;
	
	m_window.SetTitle("Select Tracks");
	m_window.SetEventHandler(this,CALLBACKNAME(WindowEvent));
	m_window.SetSize(w,h);
	m_window.SetPos( (kGUI::GetSurfaceWidth()-w)/2,(kGUI::GetSurfaceHeight()-h)/2);
	m_window.SetTop(true);
	kGUI::AddWindow(&m_window);

	m_list.SetSize(520,300);
	m_list.SetNumCols(LOADTRACK_NUMCOLUMNS);
	for(i=0;i<LOADTRACK_NUMCOLUMNS;++i)
	{
		m_list.SetColTitle(i,ltcolnames[i]);
		m_list.SetColHint(i,ltcoldesc[i]);
		m_list.SetColWidth(i,ltcolwidths[i]);
	}

	m_controls.AddObjects(1,&m_list);
	m_controls.NextLine();

	m_cancel.SetPos(0,0);
	m_cancel.SetSize(100,20);
	m_cancel.SetString("Cancel");
	m_cancel.SetEventHandler(this,CALLBACKNAME(PressCancel));

	m_ok.SetPos(125,0);
	m_ok.SetSize(100,20);
	m_ok.SetString("Done");
	m_ok.SetEventHandler(this,CALLBACKNAME(PressDone));

	m_controls.AddObjects(2,&m_cancel,&m_ok);
	m_controls.NextLine();

	m_window.AddObject(&m_controls);
	m_window.ExpandToFit();

	if(!xml)
	{
		/* populate with current track names */
		for(i=0;i<tp->GetNumTracks();++i)
		{
			track=tp->GetIndex(i);
			tsr=new SelectTracksRow(track->GetName(),track->GetNumPoints(),track->Getm_starttime());
			m_list.AddRow(tsr);
		}
	}
	else	/* populate with track names from the XML file passed */
	{
		kGUIXMLItem *xroot;
		kGUIXMLItem *xi;

		xroot=xml->GetRootItem()->Locate("gpx");
		if(xroot)
		{
			for(i=0;i<xroot->GetNumChildren();++i)
			{
				xi=xroot->GetChild(i);
				if(!strcmp(xi->GetName(),"trk"))
				{
					if(xi->Locate("name"))		/* ignore tracks with no name */
					{
						GPXTrack *s=new GPXTrack(xi->Locate("name")->GetValueString());
						s->Load(xi);
						tsr=new SelectTracksRow(s->GetName(),s->GetNumPoints(),s->Getm_starttime());
						m_list.AddRow(tsr);
						delete s;
					}
				}
			}
		}
	}
}

void SelectTracks::WindowEvent(kGUIEvent *event)
{
	switch(event->GetEvent())
	{
	case EVENT_CLOSE:
		m_donecallback.Call(m_pressed);
		delete this;
	break;
	}
}

void SelectTracks::PressCancel(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
		m_window.Close();
}

void SelectTracks::PressDone(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		unsigned int i;
		SelectTracksRow *rsr;

		/* add names of selected tracks to hash table */
		for(i=0;i<m_list.GetNumChildren();++i)
		{
			rsr=static_cast<SelectTracksRow *>(m_list.GetChild(i));
			if(rsr->GetSelected()==true)
				m_hash->Add(rsr->GetName(),0);
		}

		m_pressed=MSGBOX_OK;
		m_window.Close();
	}
}

/**********************************************************************************/

void TracksPage::ClickSimplify(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		kGUIInputBoxReq *box;

		box=new kGUIInputBoxReq(this,CALLBACKNAME(ClickSimplify2),"Number of points?");
	}
}

void TracksPage::ClickSimplify2(kGUIString *result,int closebutton)
{
	int n;
	BabelGlue babel;

	if(result->GetLen() && closebutton==MSGBOX_OK)
	{
		n=result->GetInt();
		if(n>0)
		{
			int j,np;
			kGUIString sc;

			babel.AddType(BABELTYPE_TRACKS);
			babel.SetInput("gpx","babel.gpx");
			babel.SimplifyTrack(n);
			babel.SetOutput("gpx","babel2.gpx");

#if 0
			char *simplifyparms[]={
				"GPSBabel",
				"-t",
				"-i",
				"gpx",
				"-f",
				"babel.gpx",
				"-x",
				"simplify,count=400",
				"-o",
				"gpx",
				"-F",
				"babel.gpx"};
#endif
			/* ok, save to gpx file, use gpx babel to simplify, then reload */
			UploadXML upload;
			GPXTrack *track;
			kGUIXML *xml;
			GPXTrackEntry *te;

#if 0
			sc.Sprintf("simplify,count=%d",n);
			simplifyparms[7]=(char *)sc.GetString();
#endif
			track=m_tracks.GetEntry(m_edittracklist.GetSelection()-1);
			track->Save(upload.GetXMLRoot());

			/* add points to file bounds */
			np=track->m_numentries;
			for(j=0;j<np;++j)
			{
				te=track->m_entries.GetEntry(j);
				upload.AddToBounds(te->m_lat,te->m_lon);
			}
			upload.Save("babel.gpx");
			kGUI::FileDelete("babel2.gpx");

			/* send to gpxbabel for processing */
			babel.Call(true,0);

			/* remove the original */
			kGUI::FileDelete("babel.gpx");

			xml=new kGUIXML();
			xml->SetNameCache(&gpx->m_xmlnamecache);
			if(xml->Load("babel2.gpx")==true)
			{
				kGUIXMLItem *xroot;
				kGUIXMLItem *x1;
				kGUIXMLItem *x2;

				xroot=xml->GetRootItem();
				x1=xroot->Locate("gpx");
				if(x1)
				{
					x2=x1->Locate("trk");
					if(x2)
					{
						/* make a new track, and load from xml into that track, then load that track into the table and delete it */
						track=new GPXTrack(track->GetName());
						track->Load(x2);
						track->Load(&m_table,0,0);
						delete track;
						Changed();
					}
				}
			}
			else
			{
				kGUIMsgBoxReq *box;

				box=new kGUIMsgBoxReq(MSGBOX_OK,false,"Error: GPSBabel could not simplify track!");
			}
			delete xml;
			kGUI::FileDelete("babel2.gpx");
		}
	}
}


/* used for comparing track uploaded to GPSR against one downloaded from GPSr */
unsigned int GPXTrack::Changed(GPXTrack *t2)
{
	unsigned int i;
	unsigned int nt;
	GPXTrackEntry *e1;
	GPXTrackEntry *e2;

	nt=min(m_numentries,t2->m_numentries);
	for(i=0;i<nt;++i)
	{
		e1=m_entries.GetEntry(i);
		e2=t2->m_entries.GetEntry(i);
		if(fabs(e1->m_lat-e2->m_lat)>=0.0001f || fabs(e1->m_lon-e2->m_lon)>=0.0001f)
			break;	/* error too big, match terminated at this spot */
	}
	return(i);
}

/* find closest point on current track and put cursor there */
void TracksPage::GotoTrackPosition(double lat,double lon)
{
	unsigned int i;
	unsigned int bestindex=0;
	double dist,bestdist;
	GPXCoord c;
	GPXTrackRow *row;

	c.Set(lat,lon);
	bestdist=999999.0f;
	for(i=0;i<m_table.GetNumChildren();++i)
	{
		row=static_cast<GPXTrackRow *>(m_table.GetChild(i));
		dist=c.Dist(&row->m_llcoord);
		if(dist<bestdist || !i)
		{
			bestdist=dist;
			bestindex=i;
		}
	}
	/* closest point is bestindex */
	m_table.GotoRow(bestindex);
	gpx->SetCurrentTab(TAB_TRACKS);
}

void TracksPage::PostDraw(int cx,int cy)
{
	/* draw cursor on track point if a track is being displayed */

	if(m_table.GetNumChildren())
	{
		unsigned int crow;
		GPXTrackRow *row;

		crow=m_table.GetCursorRow();
		/* check row since it could be in the a blank entry past end of table */
		if(crow<m_table.GetNumChildren())
		{
			int x1,y1;
			row=static_cast<GPXTrackRow *>(m_table.GetChild(crow));
			gpx->m_curmap->ToMap(&row->m_llcoord,&x1,&y1);

			x1-=cx;
			y1-=cy;

			/* draw cross here */
			kGUI::DrawLine(x1-10,y1+1,x1+10,y1+1,DrawColor(0,0,0));
			kGUI::DrawLine(x1+1,y1-10,x1+1,y1+10,DrawColor(0,0,0));
			kGUI::DrawLine(x1-10,y1,x1+10,y1,DrawColor(128,128,128));
			kGUI::DrawLine(x1,y1-10,x1,y1+10,DrawColor(128,128,128));
		}
	}
}

bool TracksPage::IsPointInside(const char *trackname,double lat,double lon)
{
	int i;
	GPXTrack *t;

	i=GetIndex(trackname);
	if(i<0)
		return(false);
	t=m_tracks.GetEntry(i);
	return(t->IsPointInside(lat,lon));
}

/* this is used by the filters for inside or outside of an track (area) */

bool GPXTrack::IsPointInside(double lat,double lon)
{
	unsigned int i;
	int added;
	kGUIDPoint2 px;
	GPXTrackEntry *e;
	GPXTrackEntry *e2;

	if(m_numentries<3)
		return(false);	/* must havee 3 or more points to make an area */

	/* if the track has changed then this will be zeroed and we need */
	/* to re-extract out the points */
	if(!m_numareapoints)
	{
		m_numareapoints=m_numentries;

		/* if the first and last point are different then close the polygon by */
		/* duplicating the first point at the end */

		e=m_entries.GetEntry(0);
		e2=m_entries.GetEntry(m_numentries-1);
		if((e->m_lat!=e2->m_lat) || (e->m_lon!=e2->m_lon))
			added=1;
		else
			added=0;

		m_areapoints.Alloc(m_numareapoints+added);
		for(i=0;i<m_numareapoints;++i)
		{
			e=m_entries.GetEntry(i);
			px.x=e->m_lat;
			px.y=e->m_lon;
			m_areapoints.SetEntry(i,px);
		}
		if(added)
		{
			/* duplcate the first point at the end */
			e=m_entries.GetEntry(0);
			px.x=e->m_lat;
			px.y=e->m_lon;
			m_areapoints.SetEntry(m_numareapoints,px);
			++m_numareapoints;
		}
	}
	return(kGUI::PointInsidePoly(lat,lon,m_numareapoints,m_areapoints.GetArrayPtr()));
}

void TracksPage::Verify(const char *filename,Hash *hash,kGUIString *results)
{
	unsigned int i;
	kGUIXML vxml;
	kGUIXMLItem *xroot;
	kGUIXMLItem *xi;
	unsigned int j;
	unsigned int numtracks;
	GPXTrack *track;
	Array<GPXTrack *>tracks;
	unsigned int match;
	unsigned int bestmatch;
	
	numtracks=0;
	tracks.Init(64,32);

	vxml.SetNameCache(&gpx->m_xmlnamecache);
	if(vxml.Load("babel.gpx")==true)
	{
		/* Load tracks temporarily into memory */
		xroot=vxml.GetRootItem()->Locate("gpx");
		if(xroot)
		{
			for(i=0;i<xroot->GetNumChildren();++i)
			{
				xi=xroot->GetChild(i);
				if(!strcmp(xi->GetName(),"trk"))
				{
					GPXTrack *s=new GPXTrack("TEMP");
					tracks.SetEntry(numtracks++,s);
					s->Load(xi);
				}
			}
		}
	}

	/* we have all tracks from the GPSr in memory now, Changed them against the */
	/* ones uploaded to the GPSr to see if they uploaded correctly */

	for(i=0;i<m_numtracks;++i)
	{
		track=m_tracks.GetEntry(i);
		if(hash->Find(track->GetName()))
		{
			/* find the best match */
			bestmatch=0;
			for(j=0;j<numtracks;++j)
			{
				match=track->Changed(tracks.GetEntry(j));
				if(match>bestmatch)
					bestmatch=match;
			}
			/* append results to results string */
			if(bestmatch==track->m_numentries)
				results->ASprintf("'%s' uploaded ok!\n",track->GetName());
			else if(bestmatch)
				results->ASprintf("'%s' only %d of %d points uploaded!\n",track->GetName(),bestmatch,track->m_numentries);
			else
				results->ASprintf("'%s' not found!\n",track->GetName());
		}
	}

	/* free temporarily loaded tracks */
	for(j=0;j<numtracks;++j)
		delete tracks.GetEntry(j);
}

const char *TracksPage::GetTrackName(int index)
{
	return m_tracks.GetEntry(index)->GetName();
}
