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
/*    GPSTurbo is distributed in the hope that it will be useful,                */
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
#include "defskin.h"
#include "kguibrowse.h"
#include "kguitsp.h"
#include "babelglue.h"

/*! @todo load gpx files from inside zip file automatically */
/*! @todo view tracks based on distance from a point, right click on map, view tracks ??? */


//zlib is used by the oziexplorer format and is only included here to get the version
//number for the credit screen.
#include "zlib/zlib.h"

BigFile *g_bf;
double GPX::m_adjust=1.0f;
int g_fullw;
int g_fullh;

#include "_gtext.cpp"

/* initial popup screen size */
#define DEFSCREENWIDTH 320
#define DEFSCREENHEIGHT 218
#define DEFMAXIMAGES 500

#define APPNAME "GPS Turbo"

#if defined(WIN32) || defined(MINGW)
#include "resource.h"
#endif

#include "kguisys.cpp"

#define PROFILEFILE "gpsturbo.txt"
#define PROFILEFILETEMP "gpsturbotemp.txt"
#define PROFILEFILEBACKUP "gpsturbo2.txt"

enum
{
MAPMENU_NAME,
MAPMENU_SELECT,
MAPMENU_VIEWPAGE,
MAPMENU_SETCENTER,
MAPMENU_ADDTOROUTE,
MAPMENU_REMOVEFROMROUTE,
MAPMENU_ADDNEARTOROUTE,
MAPMENU_TOGGLEFOUND,
MAPMENU_TOGGLEUSERTICKED,
MAPMENU_TOGGLENA,
MAPMENU_CLEARMAPSELECT,
MAPMENU_SELECTMAPUL,
MAPMENU_SELECTMAPLR,
MAPMENU_NEWPOINT,
MAPMENU_SELECTNEARESTTRACKPIECE,
MAPMENU_ADDTOTRACK,
MAPMENU_ADDTOLINE,
MAPMENU_NUM};

const char *mapmenutxt[]={
	"Name",								/* this is overwritten by the name of the waypoint */
	"Select",
	"View Webpage",
	"Set as Center",
	"Add to Route '%s'",
	"Remove from Route '%s'",
	"Add Nearby to Route '%s'",
	"%s Found",
	"%s User1 Ticked",
	"%s Not Available",
	"Clear Map Select area",
	"Set Upper Left Corner for Map Select",
	"Set Lower Right Corner for Map Select",
	"Add Point to Full Waypoint List",
	"Select Nearest Track Section",
	"Append Point to Track '%s'",
	"Append %s to Line '%s'"};

/* menu shown when clicked on a label on the map */
int mapmenunums[]={
	MAPMENU_NAME,
	MAPMENU_SETCENTER,
	MAPMENU_SELECT,
	MAPMENU_VIEWPAGE,
	MAPMENU_ADDTOROUTE,
	MAPMENU_ADDNEARTOROUTE,
	MAPMENU_REMOVEFROMROUTE,
	MAPMENU_TOGGLEFOUND,
	MAPMENU_TOGGLEUSERTICKED,
	MAPMENU_TOGGLENA,
	MAPMENU_SELECTNEARESTTRACKPIECE,
	MAPMENU_ADDTOTRACK,
	MAPMENU_ADDTOLINE};

/* menu shown when clicked on an empty area of the map */
int mapmenunums2[]={
	MAPMENU_SETCENTER,
	MAPMENU_CLEARMAPSELECT,
	MAPMENU_SELECTMAPUL,
	MAPMENU_SELECTMAPLR,
	MAPMENU_NEWPOINT,
	MAPMENU_SELECTNEARESTTRACKPIECE,
	MAPMENU_ADDTOTRACK,
	MAPMENU_ADDTOLINE
};

/* menu shown when clicked on waypoint name in filtered results table */
int mapmenunums3[]={
	MAPMENU_NAME,
	MAPMENU_SETCENTER,
	MAPMENU_VIEWPAGE,
	MAPMENU_ADDTOROUTE,
	MAPMENU_ADDNEARTOROUTE,
	MAPMENU_REMOVEFROMROUTE,
	MAPMENU_SELECTNEARESTTRACKPIECE,
	MAPMENU_ADDTOTRACK,
	MAPMENU_ADDTOLINE
};

static const char *logtypes[]={
	"Found it",
	"Didn't find it",
	"Write note",
	"Publish Listing",
	"Enable Listing",
	"Disable Listing",
	"Temporarily Disable Listing",
	"Owner Maintenance",
	"Post Reviewer Note",
	"Unarchive",
	"Needs Maintenance",
	"Needs Archived",
	"Attended",
	"Will Attend",
	"Webcam Photo Taken",
	"Update Coordinates",
	"Archive"};

static const char *logtypepics[]={
	 "icon_smile.gif",
	 "icon_sad.gif",
	 "icon_note.gif",
	 "icon_greenlight.gif",
	 "icon_enabled.gif",
	 "icon_disabled.gif",
	 "icon_disabled.gif",
	 "icon_maint.gif",
	 "big_smile.gif",
	 "traffic_cone.gif",
	 "icon_needsmaint.gif",
	 "icon_remove.gif",
	 "icon_attended.gif",
	 "icon_rsvp.gif",
	 "icon_camera.gif",
	 "coord_update.gif",
	 "traffic_cone.gif"};


/* update SHAPE_xxx table in header file too! */

const char *shapenames[]={
	 "pushpin.gif",
	 "trca.gif",
	 "muca.gif",
	 "vica.gif",
	 "evca.gif",
	 "meca.gif",
	 "unca.gif",
	 "rvca.gif",
	 "eaca.gif",
	 "ngca.gif",
	 "lbca.gif",
	 "wcca.gif",
	 "cica.gif",
	 "pape.gif",
	 "wigo.gif",
	 "maze.gif",
	 "othr.gif",
	 "marker.png",
	 "loadingimage.gif",
	 "brokenimage.gif"};

	 /* make sure to update defines list in main header file */
const char *cachetypenames[CACHETYPE_NUM]={
	"Undefined",
	"Traditional Cache",
	"Multi-cache",
	"Virtual Cache",
	"Event Cache",
	"Mega-Event Cache",
	"Unknown Cache",
	"Locationless (Reverse) Cache",
	"Earthcache",
	"Benchmark",
	"Letterbox Hybrid",
	"Webcam Cache",
	"Cache In Trash Out Event",
	"Project APE Cache",
	"Wherigo Cache",
	"GPS Adventures Exhibit",
	"Other"};

const char *containernames[CONTAINERTYPE_NUM]={
	"Micro",
	"Small",
	"Regular",
	"Large",
	"Other",
	"Virtual",
	"Not chosen"};


typedef struct
{
	const char *filtername;
	const char *colorname;
}DEFLABELLIST_DEF;

DEFLABELLIST_DEF defwptlabels[]={
	{"found","Dark-Gray"},
	{"not available","Black"},
	{"mystery","Medium-Orchid"},
	{"multi","Orange"},
	{"user waypoints","Red"},
	{"not found","Yellow"}};

bool g_isonline;


enum
{
REMOVESTALE_SELECTED,
REMOVESTALE_DATE,
REMOVESTALE_MATCHES,
REMOVESTALE_NUMCOLUMNS
};

class RemoveStaleRow : public kGUITableRowObj
{
public:
	RemoveStaleRow(const char *date,int matches);
	~RemoveStaleRow() {}
	int GetNumObjects(void) {return REMOVESTALE_NUMCOLUMNS;}
	kGUIObj **GetObjectList(void) {return m_objectlist;} 
	kGUIObj *m_objectlist[REMOVESTALE_NUMCOLUMNS];
	bool GetSelected(void) {return m_selected.GetSelected();}
	const char *GetDate(void) {return m_date.GetString();}
private:
	kGUITickBoxObj m_selected;
	kGUIInputBoxObj m_date;
	kGUIInputBoxObj m_matches;
};

RemoveStaleRow::RemoveStaleRow(const char *date,int matches)
{
	m_objectlist[REMOVESTALE_SELECTED]=&m_selected;
	m_objectlist[REMOVESTALE_DATE]=&m_date;
	m_objectlist[REMOVESTALE_MATCHES]=&m_matches;

	m_date.SetString(date);
	m_matches.SetInt(matches);
	m_date.SetLocked(true);
	m_matches.SetLocked(true);
}

class RemoveStale
{
public:
	RemoveStale();
	~RemoveStale() {m_list.DeleteChildren();}
private:
	CALLBACKGLUEPTR(RemoveStale,WindowEvent,kGUIEvent)
	CALLBACKGLUEPTR(RemoveStale,PressRadioAll,kGUIEvent)
	CALLBACKGLUEPTR(RemoveStale,PressRadioFilter,kGUIEvent)
	CALLBACKGLUEPTR(RemoveStale,PressRemove,kGUIEvent)
	CALLBACKGLUEPTR(RemoveStale,PressCancel,kGUIEvent)
	static int Sort(const void *o1,const void *o2);
	void CalcMatches(void);
	void WindowEvent(kGUIEvent *event);
	void PressRadioAll(kGUIEvent *event) {if(event->GetEvent()==EVENT_AFTERUPDATE){m_filter.SetSelected(!m_all.GetSelected());CalcMatches();}}
	void PressRadioFilter(kGUIEvent *event) {if(event->GetEvent()==EVENT_AFTERUPDATE){m_all.SetSelected(!m_filter.GetSelected());CalcMatches();}}
	void PressCancel(kGUIEvent *event);
	void PressRemove(kGUIEvent *event);
	
	kGUIWindowObj m_window;
	kGUIControlBoxObj m_controls;

	kGUITextObj m_filtercaption; 
	kGUIRadioObj m_filter;
	kGUITextObj m_allcaption; 
	kGUIRadioObj m_all;

	kGUITableObj m_list;

	kGUIButtonObj m_ok;
	kGUIButtonObj m_cancel;
};


class EditButtonWindowObj
{
	friend class EditButtonRowObj;
public:
	EditButtonWindowObj(unsigned int numsubs, ClassArray<kGUIString>*sublist);
private:
	CALLBACKGLUEPTR(EditButtonWindowObj,WindowEvent,kGUIEvent)
	CALLBACKGLUEPTR(EditButtonWindowObj,TableEvent,kGUIEvent)
	CALLBACKGLUEPTR(EditButtonWindowObj,PressUp,kGUIEvent)
	CALLBACKGLUEPTR(EditButtonWindowObj,PressDown,kGUIEvent)
	CALLBACKGLUEPTR(EditButtonWindowObj,PressCancel,kGUIEvent)
	CALLBACKGLUEPTR(EditButtonWindowObj,PressSave,kGUIEvent)
	void WindowEvent(kGUIEvent *event);
	void TableEvent(kGUIEvent *event);
	void PressUp(kGUIEvent *event);
	void PressDown(kGUIEvent *event);
	void PressCancel(kGUIEvent *event);
	void PressSave(kGUIEvent *event);

	unsigned int m_numsubs;
	ClassArray<kGUIString>m_sublist;

	kGUIWindowObj m_window;
	kGUIButtonObj m_up;
	kGUIButtonObj m_down;
	kGUIButtonObj m_cancel;
	kGUIButtonObj m_save;
	kGUITableObj m_table;	
};

enum
{
EBRCOL_FUNCNAME,
EBRCOL_BUTTONTEXT,
EBROL_USEIMAGE,
EBRCOL_IMAGE,
EBRCOL_BROWSE,
EBRCOL_NUMCOLS
};

class EditButtonRowObj : public kGUITableRowObj
{
public:
	EditButtonRowObj(class EditButtonWindowObj *w,MacroButton *bb);
	inline int GetNumObjects(void) {return EBRCOL_NUMCOLS;}
	kGUIObj **GetObjectList(void) {return m_objptrs;}

	kGUIString *GetFuncName(void) {return m_funclist.GetSelectionStringObj();}	
	kGUIString *GetButtonText(void) {return &m_buttontext;}	
	bool GetUseImage(void) {return m_useimage.GetSelected();}
	kGUIImage *GetImage(void) {return &m_image;}
private:
	CALLBACKGLUEPTRVAL(EditButtonRowObj,GotFilename,kGUIFileReq,int);
	void GotFilename(kGUIFileReq *fr,int pressed);
	CALLBACKGLUEPTR(EditButtonRowObj,Browse,kGUIEvent);
	void Browse(kGUIEvent *event);

	kGUIObj *m_objptrs[EBRCOL_NUMCOLS];

	kGUIComboBoxObj m_funclist;
	kGUIInputBoxObj m_buttontext;
	kGUITickBoxObj m_useimage;
	kGUIImageObj m_image;
	kGUIButtonObj m_browse;
};

GPX *gpx;

#define LISTICONSIZE 24
#define LISTICONSCALE 0.75f

void GPXRow::Init(void)
{
	int i;

	UpdateFontSize();
	m_numchildren=0;
	m_children.SetGrow(true);
	m_label.SetRow(this);
	m_inresults=false;

	m_copiedfrom=0;
	m_objectlist[GPXCOL_NA]=&m_na;
	m_objectlist[GPXCOL_FOUND]=&m_found;
	m_objectlist[GPXCOL_TYPE]=&m_typeshape;
	for(i=0;i<MAXUSERTICKS;++i)
		m_objectlist[GPXCOL_USER1+i]=m_user+i;
	m_objectlist[GPXCOL_NAME]=&m_name;
	m_objectlist[GPXCOL_WPTNAME]=&m_wptname;
	m_objectlist[GPXCOL_LOG]=&m_log;
	m_objectlist[GPXCOL_LAT]=&m_lat;
	m_objectlist[GPXCOL_LON]=&m_lon;
	m_objectlist[GPXCOL_DIST]=&m_dist;
	m_objectlist[GPXCOL_NUMNEAR]=&m_numnear;
	m_objectlist[GPXCOL_CORRECTED]=&m_corrected;
	m_objectlist[GPXCOL_HINT]=&m_hint;
	m_objectlist[GPXCOL_OWNER]=&m_owner;

	m_objectlist[GPXCOL_CONTAINER]=&m_container;
	m_objectlist[GPXCOL_DIFFICULTY]=&m_difficulty;
	m_objectlist[GPXCOL_TERRAIN]=&m_terrain;
	m_objectlist[GPXCOL_STATE]=&m_state;
	m_objectlist[GPXCOL_COUNTRY]=&m_country;
	m_objectlist[GPXCOL_USERNOTES]=&m_usernotes;
	m_objectlist[GPXCOL_GENDATE]=&m_gendate;

	m_lon.SetEventHandler(this,CALLBACKNAME(LocChanged));
	m_lat.SetEventHandler(this,CALLBACKNAME(LocChanged));

//	m_name.SetDoubleClick(this,CALLBACKNAME(GotoOnMap));
//	m_name.SetRightClick(this,CALLBACKNAME(SubMenu));
	
	m_wptname.SetEventHandler(this,CALLBACKNAME(NameChanged));
	m_name.SetEventHandler(this,CALLBACKNAME(NameChanged));
	m_difficulty.SetEventHandler(this,CALLBACKNAME(NameChanged));
	m_terrain.SetEventHandler(this,CALLBACKNAME(NameChanged));

	m_corrected.SetEventHandler(this,CALLBACKNAME(CorrChangedEvent));

#if USESHAREDCOMBOS
	m_container.SetSharedEntries(&gpx->m_sharedcontainer);
#else
	m_container.SetNumEntries(CONTAINERTYPE_NUM);
	for(i=0;i<CONTAINERTYPE_NUM;++i)
		m_container.SetEntry(i,containernames[i],i);
#endif

	SetType(CACHETYPE_UNDEFINED);
	m_typeshape.SetImage(gpx->GetShape(CACHETYPE_UNDEFINED));
	m_typeshape.SetSize(LISTICONSIZE,LISTICONSIZE);
	m_typeshape.SetScale(LISTICONSCALE,LISTICONSCALE);

	m_difficulty.SetString("N/A");
	m_terrain.SetString("N/A");
	m_container.SetSelection(CONTAINERTYPE_NOTCHOSEN);

	m_gendate.SetEventHandler(this,CALLBACKNAME(EditGenDate));

	m_label.SetFontSize(gpx->m_labelfontsize.GetInt());
}

void GPXRow::UpdateFontSize(void)
{
//	int i;
	int size=gpx->m_tablefontsize.GetInt();
//	kGUIText *t;

	m_name.SetFontSize(size);
	m_url.SetFontSize(size);
	m_wptname.SetFontSize(size);
	m_shortname.SetFontSize(size);

#if USESHAREDCOMBOS
#else
	m_container.SetFontSize(size);
	for(i=0;i<m_container.GetNumEntries();++i)
	{
		t=m_container.GetEntryTextPtr(i);
		t->SetFontSize(size);
	}
#endif

	m_numnear.SetFontSize(size);
	m_difficulty.SetFontSize(size);
	m_terrain.SetFontSize(size);
	m_dist.SetFontSize(size);
	m_lat.SetFontSize(size);
	m_lon.SetFontSize(size);
	m_state.SetFontSize(size);
	m_country.SetFontSize(size);
	m_hint.SetFontSize(size);
	m_owner.SetFontSize(size);

	m_log.SetFontSize(size);
	/* since this is a Rich Format string we need to set the font size this way too */
	m_log.SetRichFontSize(0,m_log.GetLen(),size);

	m_usernotes.SetFontSize(size);
	m_gendate.SetFontSize(size);
	m_placedby.SetFontSize(size);
	m_shortdesc.SetFontSize(size);
	m_desc.SetFontSize(size);
	CalcHeight();
}

void GPXRow::UpdateLabelName(void)
{
	gpx->ExpandLabel(this,&m_label,&gpx->m_wptname,gpx->m_wptnamenumentries,&gpx->m_wptnameentries);
}

void GPX::InitPopMenu(kGUIMenuColObj *menu,int numentries,int *entrylist)
{
	int i;
	int e;
	bool in;
	kGUIString s;

	menu->SetNumEntries(numentries);
	for(i=0;i<numentries;++i)
	{
		e=entrylist[i];
		/* handle special cases */
		switch(e)
		{
		case MAPMENU_NAME:
			menu->SetEntry(i,&m_currow->m_name,e);
			menu->SetBGColor(i,DrawColor(255,255,0));
			menu->SetEntryEnable(e,false,false);
		break;
		case MAPMENU_ADDTOROUTE:
		case MAPMENU_REMOVEFROMROUTE:
		case MAPMENU_ADDNEARTOROUTE:
			s.Sprintf(mapmenutxt[e],m_routes.GetCurrentName());
			menu->SetEntry(i,s.GetString(),e);
			menu->SetEntryEnable(e,true,true);

			if(e!=MAPMENU_ADDNEARTOROUTE)
			{
				/* is this point already in the current route? */
				if(m_routes.InTable(m_currow->m_wptname.GetString()))
					in=true;
				else
					in=false;
				if(e!=MAPMENU_REMOVEFROMROUTE)
					in=!in;
				menu->SetEntryEnable(e,in,true);
			}
		break;
		case MAPMENU_TOGGLEFOUND:
			s.Sprintf(mapmenutxt[e],m_currow->m_found.GetSelected()?"Clear":"Set");
			menu->SetEntry(i,s.GetString(),e);
			menu->SetEntryEnable(e,true,true);
		break;
		case MAPMENU_TOGGLEUSERTICKED:
			s.Sprintf(mapmenutxt[e],m_currow->m_user[0].GetSelected()?"Clear":"Set");
			menu->SetEntry(i,s.GetString(),e);
			menu->SetEntryEnable(e,true,true);
		break;
		case MAPMENU_TOGGLENA:
			s.Sprintf(mapmenutxt[e],m_currow->m_na.GetSelected()?"Clear":"Set");
			menu->SetEntry(i,s.GetString(),e);
			menu->SetEntryEnable(e,true,true);
		break;
		case MAPMENU_SELECTNEARESTTRACKPIECE:
			menu->SetEntry(i,mapmenutxt[e],e);
			/* if no entries in the track table then disable */
			menu->SetEntryEnable(e,m_tracks.GetNumTableEntries()!=0,true);
		break;
		case MAPMENU_ADDTOTRACK:
			s.Sprintf(mapmenutxt[e],m_tracks.GetCurrentName());
			menu->SetEntry(i,s.GetString(),e);
			menu->SetEntryEnable(e,true,true);
		break;
		case MAPMENU_ADDTOLINE:
			if(m_currow)
				s.Sprintf(mapmenutxt[e],"Waypoint",m_lines.GetCurrentName());
			else
				s.Sprintf(mapmenutxt[e],"Point",m_lines.GetCurrentName());
			menu->SetEntry(i,s.GetString(),e);
			menu->SetEntryEnable(e,true,true);
		break;
		default:
			menu->SetEntry(i,mapmenutxt[e],e);
			menu->SetEntryEnable(e,true,true);
		break;
		}
	}
}

void GPXRow::SubMenu(void)
{
	gpx->m_currow=this;
	if(gpx->m_rcmenu.IsActive()==false)
	{
		gpx->InitPopMenu(&gpx->m_rcmenu,sizeof(mapmenunums3)/sizeof(int),mapmenunums3);
		gpx->m_rcmenu.Activate(kGUI::GetMouseX(),kGUI::GetMouseY());
	}
}

GPXRow::GPXRow()
{
	Init();
}

GPXRow::GPXRow(kGUIString *ld,kGUIXMLItem *wp)
{
	Init();
	Load(ld,wp);
}

void GPXRow::AddChild(kGUIString *wptname,kGUIString *type,kGUIString *name,double lon,double lat)
{
	unsigned int i;
	bool found;
	GPXChild *c=0;

	found=false;
	for(i=0;(i<m_numchildren) && (found==false);++i)
	{
		c=m_children.GetEntry(i);
		if(!strcmp(c->GetWptName()->GetString(),wptname->GetString()) && !strcmp(c->GetType()->GetString(),type->GetString()))
			found=true;
	}
	if(!found)
	{
		c=new GPXChild();
		m_children.SetEntry(m_numchildren++,c);
	}
	c->SetWptName(wptname);
	c->SetType(type);
	c->SetName(name);
	c->m_label.SetString(name);
	c->SetPos(lat,lon);
	
	/* set waypoint name bold to indicate that there are child waypopints */
	m_name.SetFontID(1);
}

void GPXRow::Load(kGUIString *ld,kGUIXMLItem *wp)
{
	int i,nc;
	kGUIXMLItem *gc;
	kGUIString ll;
	kGUIXMLItem *logs;
	kGUIXMLItem *children;
	kGUIXMLItem *child;

	SetFound(false);
	if(wp->Locate("sym"))
	{
		if(!strcmp(wp->Locate("sym")->GetValueString(),"Geocache Found"))
			SetFound(true);
	}

	if(ld)
        SetGenDate(ld);
	else if(wp->Locate("gendate"))
		SetGenDate(wp->Locate("gendate")->GetValue());

	/* set db name for this row */	
	if(wp->Locate("db"))
		SetDB(wp->Locate("db")->GetValue());
	else
		SetDB(gpx->GetDefDB());

	if(wp->Locate("usernotes"))
		m_usernotes.SetString(wp->Locate("usernotes")->GetValue());

	/* things export by me ( not normally part of a gpx file */
	for(i=0;i<MAXUSERTICKS;++i)
	{
		kGUIString u;

		u.Sprintf("user%d",i+1);
		if(wp->Locate(u.GetString()))
			SetUser(i,true);		/* only exported if set */
		else
			SetUser(i,false);
	}

	if(wp->Locate("corrected"))
		SetCorrected(true);	/* only exported if set */

	if(wp->Locate("url"))
		SetURL(wp->Locate("url")->GetValue());
	SetWptName(wp->Locate("name")->GetValue());	/* GCXXXX */

	if(wp->Locate("time"))
	{
		m_placeddate.SetString(wp->Locate("time")->GetValue());
		if(m_placeddate.GetLen()>10)
			m_placeddate.Clip(10);
	}

	m_llcoord.Set(wp->Locate("lat")->GetValueDouble(),wp->Locate("lon")->GetValueDouble());
	m_llcoord.Output(&m_lat,&m_lon);

	gc=wp->Locate("groundspeak:cache");
	if(gc)
	{
		SetNA(!strcmp(gc->Locate("available")->GetValueString(),"False")?true:false);
		m_container.SetSelection(GPX::GetIndexz(gc->Locate("groundspeak:container")->GetValueString(),CONTAINERTYPE_NUM,containernames,CONTAINERTYPE_NOTCHOSEN));
		m_difficulty.SetString(gc->Locate("groundspeak:difficulty")->GetValue());
		m_terrain.SetString(gc->Locate("groundspeak:terrain")->GetValue());
		m_state.SetString(gc->Locate("groundspeak:state")->GetValue());
		m_country.SetString(gc->Locate("groundspeak:country")->GetValue());
		m_owner.SetString(gc->Locate("groundspeak:owner")->GetValue());
		m_placedby.SetString(gc->Locate("groundspeak:placed_by")->GetValue());
		SetName(gc->Locate("groundspeak:name")->GetValue());

		{
			int type;

			type=GPX::GetIndexz(gc->Locate("groundspeak:type")->GetValueString(),CACHETYPE_NUM,cachetypenames,-1);
			if(type==-1)
			{
				type=CACHETYPE_OTHER;
				gpx->m_badtype=true;
				gpx->m_badtypename.SetString(gc->Locate("groundspeak:type")->GetValue());
			}
			SetType(type);
		}
		if(gc->Locate("groundspeak:short_description"))
			SetShortDesc(gc->Locate("groundspeak:short_description")->GetValue());
		/* webpage text */
		SetDesc(gc->Locate("groundspeak:long_description")->GetValue());
		/* hint */
		SetHint(gc->Locate("groundspeak:encoded_hints")->GetValue());
		m_hint.Trim();
		m_hint.SetLocked(true);
	}
	else
	{
		if(wp->Locate("desc"))
			SetName(wp->Locate("desc")->GetValue());
	}

	/* if these were downloaded from the GPS, changes this is null */
	if(!m_name.GetLen())
		SetName(&m_wptname);

	/* load children */
	children=wp->Locate("children");
	if(children)
	{
		nc=children->GetNumChildren();
		for(i=0;i<nc;++i)
		{
			child=children->GetChild(i);
			AddChild(	child->Locate("wptname")->GetValue(),
						child->Locate("type")->GetValue(),
						child->Locate("name")->GetValue(),
						child->Locate("lon")->GetValueDouble(),
						child->Locate("lat")->GetValueDouble());
		}
	}

	if(gc)
	{
		m_log.Clear();
		logs=gc->Locate("groundspeak:logs");
		if(logs)		/* user point won't have logs */
		{
			int maxlogs;

			m_haslogs=true;
			nc=logs->GetNumChildren();

			/* clip logs ??? */

			if(GetFound()==true)
				maxlogs=gpx->GetClipFoundLogs();
			else
				maxlogs=gpx->GetClipNotFoundLogs();

			if(maxlogs>=0)	/* -1 == no clipping required */
			{
				while(nc>maxlogs)
				{
					logs->DelChild(logs->GetChild(nc-1));
					--nc;
				}
			}

			m_logs.Copy(logs);
			for(i=0;i<nc;++i)
			{
				const char *logstatus=logs->GetChild(i)->Locate("groundspeak:type")->GetValueString();
				/* count number of found it's */
				if(logstatus[0]=='F' || logstatus[0]=='D')
					m_log.Append(logstatus[0]);
			}
			m_log.SetLocked(true);
		}
		else
			m_haslogs=false;
		UpdateLogColors();
	}

	if(GetType()==CACHETYPE_UNDEFINED)	/* user point, allow editing of name */
		m_name.SetLocked(false);

	m_typeshape.SetImage(gpx->GetShape(GetType()));
	m_typeshape.SetSize(LISTICONSIZE,LISTICONSIZE);
	m_typeshape.SetScale(LISTICONSCALE,LISTICONSCALE);

	if(m_corrected.GetSelected())
		CorrChanged();

	CalcHeight();
	UpdateLabelName();
}

void GPXRow::Copy(GPXRow *copy,bool copyall)
{
	unsigned int i;

	/* copy logs too? */

	if(!copyall)
	{
		m_copiedfrom=copy;
	}
	else
	{
		/* copy logs and childen points too */
		GPXChild *c;

		m_copiedfrom=0;
		m_logs.Copy(&copy->m_logs);

		/* delete current children and replace */
		DelChildren();
		for(i=0;(i<copy->m_numchildren);++i)
		{
			c=copy->m_children.GetEntry(i);

			AddChild(	c->GetWptName(),
						c->GetType(),
						c->GetName(),
						c->GetLon(),
						c->GetLat());
		}
	}

	m_db.SetString(copy->m_db.GetString());
	SetType(copy->GetType());
	m_na.SetSelected(copy->m_na.GetSelected());
	m_found.SetSelected(copy->m_found.GetSelected());
	for(i=0;i<MAXUSERTICKS;++i)
		m_user[i].SetSelected(copy->m_user[i].GetSelected());
	m_typeshape.SetImage(gpx->GetShape(GetType()));
	m_typeshape.SetSize(LISTICONSIZE,LISTICONSIZE);
	m_typeshape.SetScale(LISTICONSCALE,LISTICONSCALE);

	SetName(&copy->m_name);
	m_url.SetString(copy->m_url.GetString());
	m_log.SetString(copy->m_log.GetString());
	UpdateLogColors();

	m_dist.SetString(copy->m_dist.GetString());
	m_numnear.SetString(copy->m_numnear.GetString());
	m_lat.SetString(copy->m_lat.GetString());
	m_lon.SetString(copy->m_lon.GetString());
	m_shortdesc.SetString(copy->m_shortdesc.GetString());
	m_desc.SetString(copy->m_desc.GetString());
	m_hint.SetString(copy->m_hint.GetString());
	
	m_placeddate.SetString(copy->m_placeddate.GetString());
	m_container.SetSelection(copy->m_container.GetSelection());
	m_difficulty.SetString(copy->m_difficulty.GetString());
	m_terrain.SetString(copy->m_terrain.GetString());
	m_state.SetString(copy->m_state.GetString());
	m_country.SetString(copy->m_country.GetString());
	m_usernotes.SetString(copy->m_usernotes.GetString());
	m_gendate.SetString(copy->m_gendate.GetString());

	m_owner.SetString(copy->m_owner.GetString());
	m_placedby.SetString(copy->m_placedby.GetString());
	m_llcoord.Set(&copy->m_llcoord);
	m_distval=copy->m_distval;
	m_nearval=copy->m_nearval;

	m_label.m_colour=copy->m_label.m_colour;
	m_label.m_tcolour=copy->m_label.m_tcolour;
	m_wptname.SetString(copy->m_wptname.GetString());
	m_wptname.SetColor(m_label.m_tcolour);
	m_wptname.SetBGColor(m_label.m_colour);
	CalcHeight();
	UpdateLabelName();
}

void GPX::SetUserHint(int usercol,kGUIString *hinttext)
{
	m_userhints.GetEntry(usercol)->SetString(hinttext);
	m_fwt->SetColHint(GPXCOL_USER1+usercol,hinttext);
}

/* search the current map for the text input */
void GPX::SearchMap(void)
{

}

void GPXRow::Save(kGUIXMLItem *wp,bool gpx)
{
	unsigned int i;
	kGUIXMLItem *gc;
	GPXChild *c;
	kGUIXMLItem *cp;
	kGUIXMLItem *cc;
	GPXCoord *o;

	if(m_corrected.GetSelected()==true)
	{
		GPXChild *c;
		
		c=LocateChild("Orig");
		o=c->GetPosPtr();
	}
	else
		o=&m_llcoord;

	wp->AddParm("lat",o->GetLat());
	wp->AddParm("lon",o->GetLon());

	wp->AddChild("sym",m_found.GetSelected()==true?"Geocache Found":"Geocache");
	wp->AddChild("gendate",&m_gendate);

	wp->AddChild("db",&m_db);

	if(m_usernotes.GetLen())
		wp->AddChild("usernotes",&m_usernotes);

	wp->AddChild("name",&m_wptname);
	wp->AddChild("url",&m_url);

	for(i=0;i<MAXUSERTICKS;++i)
	{
		kGUIString u;

		if(m_user[i].GetSelected())
		{
			u.Sprintf("user%d",i+1);
			wp->AddChild(u.GetString(),"1");
		}
	}

	if(m_corrected.GetSelected())
		wp->AddChild("corrected","1");

	wp->AddChild("time",&m_placeddate);

	/* add children */
	if(m_numchildren)
	{
		cp=wp->AddChild("children");
		for(i=0;i<m_numchildren;++i)
		{
			c=m_children.GetEntry(i);
			cc=cp->AddChild("child");
			cc->AddChild("wptname",c->GetWptName());
			cc->AddChild("name",c->GetName());
			cc->AddChild("type",c->GetType());
			cc->AddChild("lat",c->GetLat());
			cc->AddChild("lon",c->GetLon());
		}
	}

	gc=wp->AddChild("groundspeak:cache");

	gc->AddParm("available",m_na.GetSelected()==true?"False":"True");
	gc->AddChild("groundspeak:name",&m_name);
	gc->AddChild("groundspeak:container",containernames[m_container.GetSelection()]);
	gc->AddChild("groundspeak:difficulty",&m_difficulty);
	gc->AddChild("groundspeak:terrain",&m_terrain);
	gc->AddChild("groundspeak:state",&m_state);
	gc->AddChild("groundspeak:country",&m_country);
	gc->AddChild("groundspeak:owner",&m_owner);
	gc->AddChild("groundspeak:placed_by",&m_placedby);
	
	cc=gc->AddChild("groundspeak:short_description",&m_shortdesc);
	cc->AddParm("html","True");
	cc=gc->AddChild("groundspeak:long_description",&m_desc);
	cc->AddParm("html","True");
	
	gc->AddChild("groundspeak:encoded_hints",&m_hint);
	gc->AddChild("groundspeak:type",cachetypenames[GetType()]);
	if(m_haslogs)
		gc->CopyChild(&m_logs);
}

/* set the label draw flag for the label and child labels */
void GPXRow::SetLabelDraw(bool d)
{
	int i,nc;

	m_label.m_draw=d;
	nc=m_numchildren;
	for(i=0;i<nc;++i)
		m_children.GetEntry(i)->m_label.m_draw=d;
}

void GPXRow::DelChildren(void)
{
	unsigned int i;

	for(i=0;i<m_numchildren;++i)
		delete m_children.GetEntry(i);
	m_numchildren=0;
	/* put name back to regular (not bold) */
	m_name.SetFontID(0);
}

GPXRow::~GPXRow()
{
	DelChildren();
}

void GPXRow::GotoOnMap(void)
{
	gpx->SetScrollCenter(&m_llcoord);
}

GPXChild *GPXRow::LocateChild(const char *type,bool create)
{
	unsigned int i;
	GPXChild *c;

	for(i=0;i<m_numchildren;++i)
	{
		c=m_children.GetEntry(i);
		if(!strcmp(c->GetType()->GetString(),type))
			return(c);
	}
	if(create)
	{
		kGUIString t;

		t.SetString(type);
		c=new GPXChild;
		c->SetType(&t);
		c->SetPos(0.0f,0.0f);
		m_children.SetEntry(m_numchildren++,c);
	}
	else
		c=0;
	return(c);
}

/* user has changed either the name or difficulty etc */
void GPXRow::NameChanged(kGUIEvent *event)
{
	switch(event->GetEvent())
	{
	case EVENT_AFTERUPDATE:
		UpdateLabelName();
		gpx->BSPDirty();
	break;
	case EVENT_LEFTDOUBLECLICK:
		/* change to double click on selector instead */
		if(event->GetObj()==&m_name)
			GotoOnMap();
	break;
	case EVENT_RIGHTCLICK:
		if(event->GetObj()==&m_name)
			SubMenu();
	break;
	}
}

void GPXRow::EditGenDate(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_LEFTDOUBLECLICK)
	{
		kGUIDateReq *datereq;
		kGUIDate date;

		date.Setz(m_gendate.GetString());
		datereq=new kGUIDateReq(&date,this,CALLBACKNAME(EditGenDateDone));
	}
}

void GPXRow::EditGenDateDone(kGUIDate *date)
{
	date->ShortDate(&m_gendate);
}

/* user has changed either the lon or lat */
void GPXRow::LocChanged(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_AFTERUPDATE)
	{
		GPXChild *c;

		/* convert the 2 strings into numeric lat/lon */
		m_llcoord.Set(m_lat.GetString(),m_lon.GetString());

		/* update child if necessary */
		if(!m_corrected.GetSelected())
			c=LocateChild("Orig",false);
		else
			c=LocateChild("Corr");
		
		if(c)
			c->SetPos(&m_llcoord);
		gpx->BSPDirty();
	}
}

/* user has changed the corrected flag */
void GPXRow::CorrChangedEvent(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_AFTERUPDATE)
		CorrChanged();
}

void GPXRow::CorrChanged(void)
{
	GPXChild *c;
	kGUIString ll;

	if(m_corrected.GetSelected())
	{
		if(!LocateChild("Corr",false))	/* does a corrected coordinate for this wpt exist already? */
		{
			c=LocateChild("Corr");		/* no, so, generate both a corrected and original coordinate */
			c->SetPos(&m_llcoord);
			c=LocateChild("Orig");
			c->SetPos(&m_llcoord);
		}
		c=LocateChild("Corr");			/* load the corrected coord into the table row */
	}
	else
		c=LocateChild("Orig");			/* load the original coordinate into the table row */

	m_llcoord.Set(c->GetPosPtr());
	m_llcoord.Output(&m_lat,&m_lon);
}

void GPXRow::UpdateLogColors(void)
{
	int i,n;

	n=m_log.GetLen();
	if(n)
	{
		RICHINFO_DEF *ri;

		m_log.InitRichInfo();
		for(i=0;i<n;++i)
		{
			ri=m_log.GetRichInfoPtr(i);	
			if(m_log.GetChar(i)=='F')
			{
				ri->fcolor=DrawColor(0,0,0);
				ri->bgcolor=DrawColor(64,255,64);
			}
			else
			{
				ri->fcolor=DrawColor(0,0,0);
				ri->bgcolor=DrawColor(240,96,96);
			}
		}
	}
}

#define MAXRH 64
#define MINRH 16


void GPXRow::CalcHeight(void)
{
	int h;
	int h2;

	if(!gpx->m_fwt)
		return;

	h=m_name.CalcHeight(gpx->m_fwt->GetColWidth(GPXCOL_NAME)-8)+2;
	h2=m_hint.CalcHeight(gpx->m_fwt->GetColWidth(GPXCOL_HINT)-8)+2;
	if(h2>h)
		h=h2;
	
	h=MIN(h,MAXRH);
	h=MAX(h,MINRH);

	SetRowHeight(h);
}

/***********************************************************************/

class HtmlWindow : public kGUIWindowObj
{
public:
	HtmlWindow(int mode,kGUIString *source);
	~HtmlWindow();
	CALLBACKGLUE(HtmlWindow,PageChanged)
private:
	CALLBACKGLUEPTR(HtmlWindow,WindowEvent,kGUIEvent)
	void WindowEvent(kGUIEvent *event);
	void DirtyandCalcChildZone(void);
	void PageChanged(void) {SetTitle(m_page->GetTitle());}
	kGUIBrowseObj *m_page;
	int m_pagewidth;
	int m_pageheight;
};

HtmlWindow::HtmlWindow(int mode,kGUIString *source)
{
	int w=(int)(kGUI::GetScreenWidth()*0.85f),h=(int)(kGUI::GetScreenHeight()*0.85f);
	kGUIString url;

	m_page=0;
	SetAllowButtons(WINDOWBUTTON_CLOSE);
	SetSize(w,h);
	Center();
	
	m_page=new kGUIBrowseObj(&gpx->m_browsersettings,GetChildZoneW(),GetChildZoneH());

	m_pagewidth=GetChildZoneW();
	m_pageheight=GetChildZoneH();
	AddObject(m_page);
	SetEventHandler(this,CALLBACKNAME(WindowEvent));
	kGUI::AddWindow(this);
	m_page->SetPageChangedCallback(this,CALLBACKNAME(PageChanged));

	if(mode==BROWSE_DATA)
		m_page->SetSource(&url,source,0,0);	/* url, contents */
	else
		m_page->SetSource(source,0,0,0);
	
	/* set to last used printer */
	m_page->SetPID(kGUI::LocatePrinter(gpx->m_printbrowser.GetString(),true));
}

void HtmlWindow::WindowEvent(kGUIEvent *event)
{
	switch(event->GetEvent())
	{
	case EVENT_CLOSE:
		delete this;
	break;
	}
}

HtmlWindow::~HtmlWindow()
{
	/* save last used printer */
	gpx->m_printbrowser.SetString(kGUI::GetPrinterObj(m_page->GetPID())->GetName());
	delete m_page;
}

/* this is called when the window size is adjusted by the user */
void HtmlWindow::DirtyandCalcChildZone(void)
{
	int neww,newh;

	kGUIContainerObj::DirtyandCalcChildZone();
	if(m_page)
	{
		neww=GetChildZoneW();
		newh=GetChildZoneH();
		if(m_page->GetZoneW()!=neww || m_page->GetZoneH()!=newh)
		{
			m_page->SetSize(neww,newh);
			m_page->RePosition(false);
			m_pagewidth=neww;
			m_pageheight=newh;
		}
	}
}

typedef struct
{
	const char *colourname;
	kGUIColor color;
	kGUIColor tcolor;
}WPTCOLORS_DEF;

WPTCOLORS_DEF wptcolours[]={
{"Black",DrawColor(0,0,0),DrawColor(255,255,255)},
{"Dim-Gray",DrawColor(105,105,105),DrawColor(255,255,255)},
{"Gray",DrawColor(128,128,128),DrawColor(255,255,255)},
{"Dark-Gray",DrawColor(169,169,169),DrawColor(0,0,0)},
{"Silver",DrawColor(192,192,192),DrawColor(0,0,0)},
{"Light-Grey",DrawColor(211,211,211),DrawColor(0,0,0)},
{"Gainsboro",DrawColor(220,220,220),DrawColor(0,0,0)},
{"White",DrawColor(255,250,250),DrawColor(0,0,0)},
{"Rosybrown",DrawColor(188,143,143),DrawColor(0,0,0)},
{"Light-Coral",DrawColor(240,128,128),DrawColor(0,0,0)},
{"Indian-Red",DrawColor(205,92,92),DrawColor(255,255,255)},
{"Brown",DrawColor(165,42,42),DrawColor(255,255,255)},
{"Fire-Brick",DrawColor(178,34,34),DrawColor(255,255,255)},
{"Maroon",DrawColor(128,0,0),DrawColor(255,255,255)},
{"Darkred",DrawColor(139,0,0),DrawColor(255,255,255)},
{"Red",DrawColor(255,0,0),DrawColor(255,255,255)},
{"Misty-Rose",DrawColor(255,228,225),DrawColor(0,0,0)},
{"Salmon",DrawColor(250,128,114),DrawColor(0,0,0)},
{"Tomato",DrawColor(255,99,71),DrawColor(0,0,0)},
{"Dark-Salmon",DrawColor(233,150,122),DrawColor(0,0,0)},
{"Coral",DrawColor(255,127,80),DrawColor(0,0,0)},
{"Orangered",DrawColor(255,69,0),DrawColor(255,255,255)},
{"Orange-Red",DrawColor(255,69,0),DrawColor(255,255,255)},
{"Light-Salmon",DrawColor(255,160,122),DrawColor(0,0,0)},
{"Sienna",DrawColor(160,82,45),DrawColor(255,255,255)},
{"Chocolate",DrawColor(210,105,30),DrawColor(255,255,255)},
{"Saddlebrown",DrawColor(139,69,19),DrawColor(255,255,255)},
{"Sandy-Brown",DrawColor(244,164,96),DrawColor(0,0,0)},
{"Peach-Puff",DrawColor(255,218,185),DrawColor(0,0,0)},
{"Peru",DrawColor(205,133,63),DrawColor(0,0,0)},
{"Bisque",DrawColor(255,228,196),DrawColor(0,0,0)},
{"Peanut Butter/Caramel",DrawColor(140,102,56),DrawColor(255,255,255)},
{"Dark-Orange",DrawColor(255,140,0),DrawColor(255,255,255)},
{"Burlywood",DrawColor(222,184,135),DrawColor(0,0,0)},
{"Antique-White",DrawColor(250,235,215),DrawColor(0,0,0)},
{"Tan",DrawColor(210,180,140),DrawColor(0,0,0)},
{"Navajowhite",DrawColor(255,222,173),DrawColor(0,0,0)},
{"Blanchedalmond",DrawColor(255,235,205),DrawColor(0,0,0)},
{"Papaya-Whip",DrawColor(255,239,213),DrawColor(0,0,0)},
{"Moccasin",DrawColor(255,228,181),DrawColor(0,0,0)},
{"Orange",DrawColor(255,165,0),DrawColor(0,0,0)},
{"Wheat",DrawColor(245,222,179),DrawColor(0,0,0)},
{"Dark-Goldenrod",DrawColor(184,134,11),DrawColor(255,255,255)},
{"Goldenrod",DrawColor(218,165,32),DrawColor(0,0,0)},
{"Cornsilk",DrawColor(255,248,220),DrawColor(0,0,0)},
{"Gold",DrawColor(255,215,0),DrawColor(0,0,0)},
{"Lemon-Chiffon",DrawColor(255,250,205),DrawColor(0,0,0)},
{"Khaki",DrawColor(240,230,140),DrawColor(0,0,0)},
{"Pale-Goldenrod",DrawColor(238,232,170),DrawColor(0,0,0)},
{"Dark-Khaki",DrawColor(189,183,107),DrawColor(0,0,0)},
{"Beige",DrawColor(245,245,220),DrawColor(0,0,0)},
{"Light-Yellow",DrawColor(255,255,224),DrawColor(0,0,0)},
{"Light-Goldenrod-Yellow",DrawColor(250,250,210),DrawColor(0,0,0)},
{"Olive",DrawColor(128,128,0),DrawColor(255,255,255)},
{"Yellow",DrawColor(255,255,0),DrawColor(0,0,0)},
{"Olive-Drab",DrawColor(107,142,35),DrawColor(255,255,255)},
{"Yellow-Green",DrawColor(154,205,50),DrawColor(0,0,0)},
{"Dark-Olive-Green",DrawColor(85,107,47),DrawColor(255,255,255)},
{"Green-Yellow",DrawColor(173,255,47),DrawColor(0,0,0)},
{"Chartreuse",DrawColor(127,255,0),DrawColor(255,255,255)},
{"Lawn-Green",DrawColor(124,252,0),DrawColor(255,255,255)},
{"Dark-Seagreen",DrawColor(143,188,143),DrawColor(0,0,0)},
{"Pale-Green",DrawColor(152,251,152),DrawColor(0,0,0)},
{"Light-Green",DrawColor(144,238,144),DrawColor(0,0,0)},
{"Forest-Green",DrawColor(34,139,34),DrawColor(255,255,255)},
{"Lime-Green",DrawColor(50,205,50),DrawColor(255,255,255)},
{"Dark-Green",DrawColor(0,100,0),DrawColor(255,255,255)},
{"Green",DrawColor(0,128,0),DrawColor(255,255,255)},
{"Lime",DrawColor(0,255,0),DrawColor(255,255,255)},
{"Seagreen",DrawColor(46,139,87),DrawColor(255,255,255)},
{"Medium-Seagreen",DrawColor(60,179,113),DrawColor(255,255,255)},
{"Spring-Green",DrawColor(0,255,127),DrawColor(255,255,255)},
{"Medium-Spring-Green",DrawColor(0,250,154),DrawColor(0,0,0)},
{"Medium-Aquamarine",DrawColor(102,205,170),DrawColor(0,0,0)},
{"Aquamarine",DrawColor(127,255,212),DrawColor(0,0,0)},
{"Aquamarine",DrawColor(127,255,212),DrawColor(0,0,0)},
{"Turquoise",DrawColor(64,224,208),DrawColor(0,0,0)},
{"Light-Seagreen",DrawColor(32,178,170),DrawColor(255,255,255)},
{"Medium-Turquoise",DrawColor(72,209,204),DrawColor(0,0,0)},
{"Light-Cyan",DrawColor(224,255,255),DrawColor(0,0,0)},
{"Pale-Turquoise",DrawColor(175,238,238),DrawColor(0,0,0)},
{"Dark-Slate-Gray",DrawColor(47,79,79),DrawColor(255,255,255)},
{"Teal",DrawColor(0,128,128),DrawColor(255,255,255)},
{"Dark-Cyan",DrawColor(0,139,139),DrawColor(255,255,255)},
{"Aqua",DrawColor(0,255,255),DrawColor(0,0,0)},
{"Cyan",DrawColor(0,255,255),DrawColor(0,0,0)},
{"Dark-Turquoise",DrawColor(0,206,209),DrawColor(0,0,0)},
{"Cadet-Blue",DrawColor(95,158,160),DrawColor(0,0,0)},
{"Powder-Blue",DrawColor(176,224,230),DrawColor(0,0,0)},
{"Light-Blue",DrawColor(173,216,230),DrawColor(0,0,0)},
{"Deep-Skyblue",DrawColor(0,191,255),DrawColor(0,0,0)},
{"Sky Blue",DrawColor(135,206,235),DrawColor(0,0,0)},
{"Light-Sky-Blue",DrawColor(135,206,250),DrawColor(0,0,0)},
{"Steel-Blue",DrawColor(70,130,180),DrawColor(255,255,255)},
{"Dodger-Blue",DrawColor(30,144,255),DrawColor(0,0,0)},
{"Light-Slate-Gray",DrawColor(119,136,153),DrawColor(0,0,0)},
{"Slate-Gray",DrawColor(112,128,144),DrawColor(255,255,255)},
{"Light-Steel-Blue",DrawColor(176,196,222),DrawColor(0,0,0)},
{"Cornflower-Blue",DrawColor(100,149,237),DrawColor(0,0,0)},
{"Royal-Blue",DrawColor(65,105,225),DrawColor(255,255,255)},
{"Midnight-Blue",DrawColor(25,25,112),DrawColor(255,255,255)},
{"Navy",DrawColor(0,0,128),DrawColor(255,255,255)},
{"Dark-Blue",DrawColor(0,0,139),DrawColor(255,255,255)},
{"Medium-Blue",DrawColor(0,0,205),DrawColor(255,255,255)},
{"Blue",DrawColor(0,0,255),DrawColor(255,255,255)},
{"Slateblue",DrawColor(106,90,205),DrawColor(0,0,0)},
{"Dark-Slate-Blue",DrawColor(72,61,139),DrawColor(255,255,255)},
{"Medium-Slate-Blue",DrawColor(123,104,238),DrawColor(0,0,0)},
{"Medium-Purple",DrawColor(147,112,219),DrawColor(0,0,0)},
{"Blue-Violet",DrawColor(138,43,226),DrawColor(0,0,0)},
{"Indigo",DrawColor(75,0,130),DrawColor(255,255,255)},
{"Dark-Orchid",DrawColor(153,50,204),DrawColor(0,0,0)},
{"Dark-Violet",DrawColor(148,0,211),DrawColor(255,255,255)},
{"Medium-Orchid",DrawColor(186,85,211),DrawColor(0,0,0)},
{"Thistle",DrawColor(216,191,216),DrawColor(0,0,0)},
{"Plum",DrawColor(221,160,221),DrawColor(0,0,0)},
{"Violet",DrawColor(238,130,238),DrawColor(0,0,0)},
{"Purple",DrawColor(128,0,128),DrawColor(255,255,255)},
{"Dark-Magenta",DrawColor(139,0,139),DrawColor(255,255,255)},
{"Fuchsia",DrawColor(255,0,255),DrawColor(0,0,0)},
{"Magenta",DrawColor(255,0,255),DrawColor(0,0,0)},
{"Orchid",DrawColor(218,112,214),DrawColor(0,0,0)},
{"Medium-Violet-Red",DrawColor(199,21,133),DrawColor(255,255,255)},
{"Deep-Pink",DrawColor(255,20,147),DrawColor(0,0,0)},
{"Hotpink",DrawColor(255,105,180),DrawColor(0,0,0)},
{"Pale-Violet-Red",DrawColor(219,112,147),DrawColor(0,0,0)},
{"Crimson",DrawColor(220,20,60),DrawColor(255,255,255)},
{"Pink",DrawColor(255,192,203),DrawColor(0,0,0)},
{"Light-Pink",DrawColor(255,182,193),DrawColor(0,0,0)}};

#define WPTCOLOURS_NUMCOLOURS (sizeof(wptcolours)/sizeof(WPTCOLORS_DEF))

void GPX::InitColorCombo(kGUIComboBoxObj *box)
{
	unsigned int e;

	box->SetNumEntries(0);
	box->SetColorMode(96);
	box->SetNumEntries(WPTCOLOURS_NUMCOLOURS);		/* list of select filters */
	for(e=0;e<WPTCOLOURS_NUMCOLOURS;++e)
	{
		box->SetColorBox(e,wptcolours[e].color);
		box->SetEntry(e,wptcolours[e].colourname,e);
	}
	box->SetAllowTyping(true);
}

typedef struct
{
	const char *text;
	int alpha;
}ALPHA_DEF;

const static ALPHA_DEF alphalist[]={
	{"100% - opaque",100},
	{"90%",90},
	{"80%",80},
	{"70%",70},
	{"60%",60},
	{"50%",50},
	{"40%",40},
	{"30%",30},
	{"20%",20},
	{"10%",10}};

void GPX::InitAlphaCombo(kGUIComboBoxObj *box)
{
	unsigned int i;

	box->SetNumEntries(sizeof(alphalist)/sizeof(ALPHA_DEF));
	for(i=0;i<(sizeof(alphalist)/sizeof(ALPHA_DEF));++i)
		box->SetEntry(i,alphalist[i].text,alphalist[i].alpha);
	box->SetSize(box->GetWidest(),20);

}

kGUIColor GPX::GetTableColor(int index)
{
	return(wptcolours[index].color);
}

const char *GPX::GetTableColorName(int index)
{
	return(wptcolours[index].colourname);
}

kGUIColor GPX::GetTableTColor(int index)
{
	return(wptcolours[index].tcolor);
}

/* loop through wptcolours and return index to color name */
unsigned int GPX::GetTableColorIndex(const char *c)
{
	unsigned int e;

	for(e=0;e<WPTCOLOURS_NUMCOLOURS;++e)
	{
		if(!strcmp(wptcolours[e].colourname,c))
			return(e);
	}
	// color name not found, return 0
	return(0);
}

const int tabnames[]={
STRING_FILTEREDANDMAP,
STRING_ROUTES,
STRING_TRACKS,
STRING_LINES,
STRING_FILTERS,
STRING_DRAWSETTINGS,
STRING_GPSRS,
STRING_MAPDOWNLOAD,
STRING_SOLVER,
STRING_STICKERS,
STRING_NOTES,
STRING_SCRIPTS,
STRING_DEBUG};

enum
{
MAINMENU_LOAD,
MAINMENU_SAVE,
MAINMENU_LOADOTHER,
MAINMENU_SAVEOTHER,
MAINMENU_SAVECHANGES,
MAINMENU_RENAMEDATABASE,
MAINMENU_VIEWGENERATED,
MAINMENU_PRINTTABLE,
MAINMENU_PRINTMAP,
MAINMENU_SAVEMAPBITMAP,
MAINMENU_SAVEMAPKML,
MAINMENU_VIEWPAGES,
MAINMENU_CREDITS,
MAINMENU_HELP,
MAINMENU_QUIT,
MAINMENU_NUM};

static const int menutext[]={
STRING_LOADGPX,
STRING_SAVEGPX,
STRING_LOADAS,
STRING_SAVEAS,
STRING_SAVECHANGES,
STRING_RENAMEDATABASE,
STRING_REMOVESTALEWAYPOINTS,
STRING_PRINTTABLE,
STRING_PRINTMAP,
STRING_SAVESELECTEDSHAPE,
STRING_SAVESELECTEDSHAPEKML,
STRING_VIEWFILTEREDCACHEPAGES,
STRING_CREDITS,
STRING_VIEWHELP,
STRING_QUIT};

static const char *menuicon[]={
	"micon_load.gif",
	"micon_save.gif",
	"micon_load.gif",
	"micon_save.gif",
	"micon_save.gif",
	"micon_rename.gif",			//rename database
	"micon_delete.gif",			//remove stale waypoints
	"micon_print.gif",			//print map
	"micon_print.gif",			//print table
	"micon_save.gif",			//save map as jpg
	"micon_save.gif",			//save map as kml
	"micon_page.gif",			//view cache pages in browser
	"micon_credits.gif",		//credits
	"micon_help.gif",			//help
	"micon_exit.gif"};			//quit

int filemenu[]={
	MAINMENU_LOAD,
	MAINMENU_SAVE,
	MAINMENU_LOADOTHER,
	MAINMENU_SAVEOTHER,
	-1,
	MAINMENU_SAVECHANGES,
	-1,
	MAINMENU_PRINTTABLE,
	MAINMENU_PRINTMAP,
	-1,
	MAINMENU_RENAMEDATABASE,
	MAINMENU_VIEWGENERATED,
	MAINMENU_SAVEMAPBITMAP,
	MAINMENU_SAVEMAPKML,
	MAINMENU_VIEWPAGES,
	MAINMENU_QUIT};

int helpmenu[]={
	MAINMENU_CREDITS,
	MAINMENU_HELP};

enum
{
COLMENU_SORTASC,
COLMENU_SORTDESC,
COLMENU_HIDE,
COLMENU_ADDSORTASC,
COLMENU_ADDSORTDESC,
COLMENU_NUM,
COLMENU_CLEARALL=COLMENU_NUM,
COLMENU_SETALL,
COLMENU_SETHINT,
COLMENU_NUM2
};

const char *colmenutxt[]={
	"Sort Ascending","Sort Descending","Hide Column","Secondary Sort Ascending","Secondary Sort Descending","Clear All","Set All","Set Column Hint"};

GPX::GPX()
{

}

class SplashScreen : public kGUIContainerObj
{
public:
	SplashScreen();
	~SplashScreen(){}
	void Draw(void);
	bool UpdateInput(void);
	void CalcChildZone(void) {SetChildZone(0,0,GetZoneW(),GetZoneH());}
	void SetDoing(const char *doing) {m_doing.SetString(doing);kGUI::ReDraw();}
private:
	kGUIImage m_image;
	kGUITextObj m_doing;
};

SplashScreen::SplashScreen()
{
	SetNumGroups(1);	/* only 1 container list */

	m_image.SetFilename("splash.jpg");
	m_doing.SetFontSize(11);
	m_doing.SetPos(0,200);
	m_doing.SetSize(320,30);
	AddObject(&m_doing);
}

void SplashScreen::Draw(void)
{
	kGUICorners c;

	GetCorners(&c);
	m_image.Draw(0,c.lx,c.ty);
	kGUI::DrawRect(c.lx,c.ty+m_image.GetImageHeight(),c.rx,c.by,DrawColor(255,255,255));
	DrawC(0);
}

bool SplashScreen::UpdateInput(void)
{
	return(true);
}

SplashScreen *ss;

class mykGUIXML : public kGUIXML
{
public:
	void ChildLoaded(kGUIXMLItem *child,kGUIXMLItem *parent);
};

/* to conserve memory we will load these directly as they are streamed in and */
/* return the items to the pool right away */

void mykGUIXML::ChildLoaded(kGUIXMLItem *child,kGUIXMLItem *parent)
{
	if(!strcmp(child->GetName(),"wpt"))
	{
		/* load a waypoint row directly */
		GPXRow *row;

		row=new GPXRow(0,child);
		gpx->m_wptlist.SetEntry(gpx->m_numwpts++,row);

		/* remove this child from the parent since we have processed it, don't purge though */
		parent->DelChild(child,false);
		/* add item and it's children back to the available pool */
		PoolAdd(child);
	}
}

void GPX::PreInit(void)
{
	unsigned int i;
	mykGUIXML xml;
	bool xmlstatus;
	DefSkin *skin;
	kGUIDate endtime;
	int language=0;		/* default to english */

	m_starttime.SetToday();

	InitArt();

	//kgui does allow a totally custom skin, alternatively you can just
	//change textures and colors in the default skin, it's much easier
	//then re-writing all the draw code.
	skin=static_cast<DefSkin *>(kGUI::GetSkin());

	skin->SetWindowIcon("skin_wicon.gif");

	/* colors */

	skin->SetColor(SKINCOLOR_BACKGROUND,DrawColor(236,233,206));
	skin->SetColor(SKINCOLOR_WINDOWBACKGROUND,DrawColor(236,233,216));
	skin->SetColor(SKINCOLOR_CONTAINERBACKGROUND,DrawColor(255,255,216));

	skin->SetShape(SKIN_WINDOWTOPLEFT,"skin_tl.gif");
	skin->SetShape(SKIN_WINDOWTOPMIDDLE,"skin_tc.gif");
	skin->SetShape(SKIN_WINDOWTOPRIGHT,"skin_tr.gif");

	skin->SetShape(SKIN_WINDOWLEFTSIDE,"skin_ls.gif");
	skin->SetShape(SKIN_WINDOWRIGHTSIDE,"skin_rs.gif");

	skin->SetShape(SKIN_WINDOWBOTTOMLEFT,"skin_bl.gif");
	skin->SetShape(SKIN_WINDOWBOTTOMMIDDLE,"skin_bc.gif");
	skin->SetShape(SKIN_WINDOWBOTTOMRIGHT,"skin_br.gif");

	skin->SetShape(SKIN_WINDOWCLOSEBUTTON,"skin_wclose.gif");
	skin->SetShape(SKIN_WINDOWFULLBUTTON,"skin_wfull.gif");
	skin->SetShape(SKIN_WINDOWMINIMIZEBUTTON,"skin_wmin.gif");

	skin->SetShape(SKIN_WINDOWCLOSEBUTTONOVER,"skin_wclose2.gif");
	skin->SetShape(SKIN_WINDOWFULLBUTTONOVER,"skin_wfull2.gif");
	skin->SetShape(SKIN_WINDOWMINIMIZEBUTTONOVER,"skin_wmin2.gif");

	skin->SetShape(SKIN_MINIMIZEDWINDOW,"skin_mini.gif");

	skin->SetShape(SKIN_TICK,"skin_tick.gif");

	skin->SetShape(SKIN_TABLEFT,"skin_tabl.gif");
	skin->SetShape(SKIN_TABCENTER,"skin_tabc.gif");
	skin->SetShape(SKIN_TABRIGHT,"skin_tabr.gif");

	skin->SetShape(SKIN_TABSELECTEDLEFT,"skin_tabsl.gif");
	skin->SetShape(SKIN_TABSELECTEDCENTER,"skin_tabsc.gif");
	skin->SetShape(SKIN_TABSELECTEDRIGHT,"skin_tabsr.gif");

	skin->SetShape(SKIN_SCROLLBARVERTTOP,"skin_scrvt.gif");
	skin->SetShape(SKIN_SCROLLBARVERTCENTER,"skin_scrvc.gif");
	skin->SetShape(SKIN_SCROLLBARVERTBOTTOM,"skin_scrvb.gif");

	skin->SetShape(SKIN_SCROLLBARVERTSLIDERTOP,"skin_scrvbt.gif");
	skin->SetShape(SKIN_SCROLLBARVERTSLIDERCENTER,"skin_scrvbc.gif");
	skin->SetShape(SKIN_SCROLLBARVERTSLIDERLINE,"skin_scrvbl.gif");
	skin->SetShape(SKIN_SCROLLBARVERTSLIDERBOTTOM,"skin_scrvbb.gif");

	skin->SetShape(SKIN_SCROLLBARHORIZLEFT,"skin_scrhl.gif");
	skin->SetShape(SKIN_SCROLLBARHORIZCENTER,"skin_scrhc.gif");
	skin->SetShape(SKIN_SCROLLBARHORIZRIGHT,"skin_scrhr.gif");

	skin->SetShape(SKIN_SCROLLBARHORIZSLIDERLEFT,"skin_scrhbl.gif");
	skin->SetShape(SKIN_SCROLLBARHORIZSLIDERCENTER,"skin_scrhbc.gif");
	skin->SetShape(SKIN_SCROLLBARHORIZSLIDERLINE,"skin_scrhbf.gif");
	skin->SetShape(SKIN_SCROLLBARHORIZSLIDERRIGHT,"skin_scrhbr.gif");

	skin->SetShape(SKIN_TABLEROWMARKER,"skin_tabler.gif");
	skin->SetShape(SKIN_TABLEROWMARKERSELECTED,"skin_tablers.gif");
	skin->SetShape(SKIN_TABLEROWNEW,"skin_tabnew.gif");

	skin->SetShape(SKIN_COMBODOWNARROW,"skin_cdown.gif");

	skin->SetShape(SKIN_RADIOUNSELECTED,"skin_radu.gif");
	skin->SetShape(SKIN_RADIOSELECTED,"skin_rads.gif");

#if 0
	skin->SetShape(SKIN_WINDOWTOPLEFT,"skin_tl.gif");
	skin->SetShape(SKIN_WINDOWTOPMIDDLE,"skin_tc.gif");
	skin->SetShape(SKIN_WINDOWTOPRIGHT,"skin_tr.gif");

	skin->SetShape(SKIN_WINDOWLEFTSIDE,"skin_ls.gif");
	skin->SetShape(SKIN_WINDOWRIGHTSIDE,"skin_rs.gif");

	skin->SetShape(SKIN_WINDOWBOTTOMLEFT,"skin_bl.gif");
	skin->SetShape(SKIN_WINDOWBOTTOMMIDDLE,"skin_bc.gif");
	skin->SetShape(SKIN_WINDOWBOTTOMRIGHT,"skin_br.gif");

	skin->SetShape(SKIN_WINDOWCLOSEBUTTON,"skin_wclose.gif");
	skin->SetShape(SKIN_WINDOWFULLBUTTON,"skin_wfull.gif");
	skin->SetShape(SKIN_WINDOWMINIMIZEBUTTON,"skin_wmin.gif");

	skin->SetShape(SKIN_WINDOWCLOSEBUTTONOVER,"skin_wclose2.gif");
	skin->SetShape(SKIN_WINDOWFULLBUTTONOVER,"skin_wfull2.gif");
	skin->SetShape(SKIN_WINDOWMINIMIZEBUTTONOVER,"skin_wmin2.gif");
#endif

	endtime.SetToday();
	m_debug.ASprintf("Start-UpdateSkin seconds=%d\n",m_starttime.GetDiffSeconds(&endtime));

	ss=new 	SplashScreen();
	ss->SetSize(DEFSCREENWIDTH,DEFSCREENHEIGHT);
	kGUI::AddWindow(ss);
	kGUI::ReDraw();

	kGUI::LoadFont("arial.ttf");
	ss->SetDoing("Loading fonts.");
	kGUI::LoadFont("arialbd.ttf");
	kGUI::LoadFont("courier.ttf");

	endtime.SetToday();
	m_debug.ASprintf("Start-Load Fonts seconds=%d\n",m_starttime.GetDiffSeconds(&endtime));

	ss->SetDoing("Initializing maps.");
	MSGPXMap::Init();

	//create maps and tiles directories
	ss->SetDoing("Creating directories.");
	kGUI::MakeDirectory("tiles");
	kGUI::MakeDirectory("geoart");	/* extract files from data.big to it */

	/* extract art for geocaching to geocaching directory */

	ss->SetDoing("Extracting Geocaching images.");
	for(i=0;i<sizeof(shapenames)/sizeof(char *);++i)
	{
		kGUIString exname;

		exname.Sprintf("geoart" DIRCHAR "%s",shapenames[i]);
		g_bf->Extract(shapenames[i],exname.GetString());
	}

	for(i=0;i<sizeof(logtypepics)/sizeof(char *);++i)
	{
		kGUIString exname;

		exname.Sprintf("geoart" DIRCHAR "%s",logtypepics[i]);
		g_bf->Extract(logtypepics[i],exname.GetString());
	}

	/* html images for help */
	g_bf->Extract("tsl.gif","geoart" DIRCHAR "tsl.gif");
	g_bf->Extract("tsb.gif","geoart" DIRCHAR "tsb.gif");
	g_bf->Extract("tsr.gif","geoart" DIRCHAR "tsr.gif");
	g_bf->Extract("fb.gif","geoart" DIRCHAR "fb.gif");

	endtime.SetToday();
	m_debug.ASprintf("Start-ExtractArt seconds=%d\n",m_starttime.GetDiffSeconds(&endtime));

	m_logo.SetFilename("logo.gif");
	m_zin.SetFilename("zoomin.gif");
	m_zout.SetFilename("zoomout.gif");

	for(i=0;i<SHAPE_NUMSHAPES;++i)
		m_shapes[i].SetFilename(shapenames[i]);

	/* default label name */
	m_wptname.SetString(".Name(.Diff/.Terr),.Wptname");
	m_wptnameentries.Alloc(10);
	m_wptnameentries.SetGrow(true);
	CompileLabelDefinition(&m_wptname,&m_wptnamenumentries,&m_wptnameentries);

	m_nummaps=0;
	ss->SetDoing("Loading Database/Prefs.");

	m_xmlnamecache.Init(10,0);
	xml.SetNameCache(&m_xmlnamecache);

#if USESHAREDCOMBOS
	m_sharedcontainer.SetNumEntries(CONTAINERTYPE_NUM);
	for(i=0;i<CONTAINERTYPE_NUM;++i)
		m_sharedcontainer.SetEntry(i,containernames[i],i);
#endif

	m_mapdirty=false;
	m_fwt=0;
	m_numwpts=0;
	m_wptlist.Alloc(1024);
	m_wptlist.SetGrow(true);
	m_wptlist.SetGrowSize(256);
	m_wptpool.SetBlockSize(2048);

	SetClipFoundLogs(-1);
	SetClipNotFoundLogs(-1);

	xmlstatus=xml.StreamLoad(PROFILEFILE);
	endtime.SetToday();
	m_debug.ASprintf("Start-XML Loaded seconds=%d\n",m_starttime.GetDiffSeconds(&endtime));

	/* get language now before we build the gui */
	if(xmlstatus==true)
	{
		kGUIXMLItem *root;
		kGUIXMLItem *item;

		root=xml.GetRootItem()->Locate("gtp");
		item=root->Locate("language");
		if(item)
			language=item->GetValueInt();
	}

	/* extract map paths first */
	LoadMapPaths(&xml,xmlstatus);

	//set default versions for google map tiles
	GGPXMap::InitVersions();
#if 0
	if(xmlstatus==true)
		GGPXMap::LoadVersions(xml.GetRootItem()->Locate("gtp"));
#endif

	/* traverse the map directory and add all maps found as well */

	m_mapinfo.Init(32,4);
	{
		GPXMAPInfo *mi;
		kGUIDir dir;
		unsigned int p;
		kGUIString mname;

		mi=new GPXMAPInfo(MAPTYPE_GOOGLESAT,"Google Satellite","");
		m_mapinfo.SetEntry(0,mi);
		mi=new GPXMAPInfo(MAPTYPE_GOOGLEHYBRID,"Google Hybrid","");
		m_mapinfo.SetEntry(1,mi);
		mi=new GPXMAPInfo(MAPTYPE_GOOGLEMAP,"Google Map","");
		m_mapinfo.SetEntry(2,mi);
		mi=new GPXMAPInfo(MAPTYPE_GOOGLETERRAIN,"Google Terrain","");
		m_mapinfo.SetEntry(3,mi);
#if 0
		mi=new GPXMAPInfo(MAPTYPE_TOPOCANADAMAP,"Topographic Canada","");
		m_mapinfo.SetEntry(4,mi);
#endif
#if 0
		mi=new GPXMAPInfo(MAPTYPE_TERRASERVSAT,"Terraserver USA Sattllite","");
		m_mapinfo.SetEntry(5,mi);
		mi=new GPXMAPInfo(MAPTYPE_TERRASERVTOPO,"Terraserver USA Topo","");
		m_mapinfo.SetEntry(6,mi);
		m_nummaps=7;
#else
		m_nummaps=4;
#endif
		/* scan 2 directories */
		ss->SetDoing("Detecting IMG Files.");
		for(p=0;p<GetNumMapPaths();++p)
			AddMaps(GetMapPath(p)->GetString());
	}
	endtime.SetToday();
	m_debug.ASprintf("Start-Maps Generated seconds=%d\n",m_starttime.GetDiffSeconds(&endtime));

	/* now go full screen */
	kGUI::HideWindow();
	kGUI::GetBackground()->SetPos(0,0);
	/* leave a few pixels at the bottom so they can access their taskbar */
	kGUI::GetBackground()->SetSize(g_fullw,g_fullh-35);
	kGUI::GetBackground()->SetTitle("GPS Turbo");
	ss->SetPos((g_fullw-ss->GetZoneW())/2,(g_fullh-ss->GetZoneH())/2);
	ss->SetDoing("Initializing GUI.");
	kGUI::ReDraw();
	kGUI::ShowWindow();

	/* add gui items to full screen window */
	Init(language);

	/* rebuild combo box */
	UpdateMapMenu();
	m_maptypes.SetSelection(MAPTYPE_GOOGLEMAP);

	m_browseritemcache.SetDirectory("bcache");
	m_browsersettings.SetItemCache(&m_browseritemcache);
	m_browsersettings.SetVisitedCache(&m_browservisitedcache);

	ss->SetDoing("Parsing Database/Prefs.");
	LoadPrefs(&xml,xmlstatus);
	UpdateMacroButtons();

	kGUI::DelWindow(ss);
	delete ss;
	kGUI::ReDraw();

	endtime.SetToday();
	m_debug.ASprintf("Start seconds=%d\n",m_starttime.GetDiffSeconds(&endtime));

#if 0
	//trigger async thread
	GGPXMap::CheckVersions();
#endif

	/* call this shutdown func if a fatal error occurs */
	kGUI::SetPanic(this,CALLBACKNAME(Panic));
}

/* add maps found in directory to the map list */

void GPX::AddMaps(const char *path)
{
	unsigned int e;
	GPXMAPInfo *mi;
	kGUIDir dir;
	kGUIString iname;
	kGUIString mname;

	MSGPXFName::Load(path);
	/* check for any other maps */
	dir.LoadDir(path,false,true);
	for(e=0;e<dir.GetNumFiles();++e)
	{
		if(strstri(dir.GetFilename(e),".map"))
		{
			mname.SetString(dir.GetFilename(e));
			mname.Replace(".map","");
			mname.Replace(path,"");
			
			/* check if already is in list */
			if(FindMapz(mname.GetString())<0)
			{
				mi=new GPXMAPInfo(MAPTYPE_OZF2,mname.GetString(),dir.GetFilename(e));
				m_mapinfo.SetEntry(m_nummaps++,mi);
			}
		}
		else if(strstri(dir.GetFilename(e),".osb"))
		{
			mname.SetString(dir.GetFilename(e));
			mname.Replace(".osb","");
			mname.Replace(path,"");
			
			/* check if already is in list */
			if(FindMapz(mname.GetString())<0)
			{
				mi=new GPXMAPInfo(MAPTYPE_OPENSTREETMAP,mname.GetString(),dir.GetFilename(e));
				m_mapinfo.SetEntry(m_nummaps++,mi);
			}
		}
	}

	/* look for tdb and img with same name */
	for(e=0;e<dir.GetNumFiles();++e)
	{
		if(strstri(dir.GetFilename(e),".tdb"))
		{
			iname.SetString(dir.GetFilename(e));
			iname.Replace(".tdb",".img",0,1);
			if(kGUI::FileExists(iname.GetString()))
			{
				MSGPXMap::GetLongName(iname.GetString(),&mname);
				if(!mname.GetLen())
				{
					mname.SetString(iname.GetString());
					mname.Replace(".img","");
					mname.Replace(path,"");
				}
				/* check if already is in list */
				if(FindMapz(mname.GetString())<0)
				{
					mi=new GPXMAPInfo(MAPTYPE_MS,mname.GetString(),iname.GetString());
					m_mapinfo.SetEntry(m_nummaps++,mi);
				}
			}
		}
	}
}

/* return index to map */
int GPX::FindMapz(const char *name)
{
	int i;
	GPXMAPInfo *mi;
 
	for(i=0;i<(int)m_nummaps;++i)
	{
		mi=m_mapinfo.GetEntry(i);
		if(!strcmp(mi->m_name.GetString(),name))
			return(i);
	}
	return(-1);
}

/* rebuild the map pulldown menu with newly added maps */
void GPX::UpdateMapMenu(void)
{
	kGUIString name;

	if(m_maptypes.GetNumEntries())
		name.SetString(m_maptypes.GetSelectionString());

	m_maptypes.SetNumEntries(m_nummaps);
	for(unsigned int i=0;i<m_nummaps;++i)
	{
		GPXMAPInfo *mi=m_mapinfo.GetEntry(i);
		m_maptypes.SetEntry(i,mi->m_name.GetString(),i);
	}
	m_maptypes.SetSelectionz(name.GetString());
}

#ifdef INCLUDEWIG
const int scripttabnames[SCRIPTTAB_NUMTABS]={
	STRING_BASIC,
	STRING_LUA,
	STRING_WIG};
#endif

void GPX::Init(int language)
{
	unsigned int i;
	int y;
	int bw,bh,tw;
	kGUIString s;
	kGUIText *t;
	kGUITabObj *ptab;

	m_locstrings.Init(&STRING_DEF);	/* generated data in _text.cpp */

	kGUI::SetLanguage(language);
	m_locstrings.SetLanguage(language);

	m_nummacrobuttons=0;
	m_macrobuttons.Init(16,16);

	m_realtime=new BabelGlue();
	m_clipfoundlogs=-1;
	m_clipnotfoundlogs=6;

	bw=kGUI::GetBackground()->GetChildZoneW();
	bh=kGUI::GetBackground()->GetChildZoneH();
	m_rebuildbsp=true;
	m_mapsel=0;
	m_mapasync=true;

	m_curmap=new GGPXMap(MAPTYPE_GOOGLEHYBRID);

	m_logo.SetPos(0,0);
	if(bh<800)
	{
		m_logo.SetScale(0.75f,0.75f);
		m_logo.SetSize((int)(m_logo.GetImageWidth()*0.75f),(int)(m_logo.GetImageHeight()*0.75f));
	}
	else
		m_logo.SetSize(m_logo.GetImageWidth(),m_logo.GetImageHeight());
	m_logo.SetHint("Click to visit geocaching.com");
	m_logo.SetEventHandler(this,CALLBACKNAME(ShowGeocaching));
	kGUI::GetBackground()->AddObject(&m_logo);

	m_maincontrols.SetPos(m_logo.GetZoneW()+8,0);
	m_maincontrols.SetMaxWidth(bw);

	m_mainmenu.SetFontSize(20);
	m_mainmenu.SetFontID(1);
	m_mainmenu.SetNumEntries(2);
	m_mainmenu.GetTitle(0)->SetString("File");
	m_mainmenu.GetTitle(1)->SetString("Help");
	m_mainmenu.SetEntry(0,&m_filemenu);
	m_mainmenu.SetEntry(1,&m_helpmenu);
	m_mainmenu.SetEventHandler(this,CALLBACKNAME(DoMainMenu));

	m_filemenu.SetIconWidth(22);
	m_helpmenu.SetIconWidth(22);

	m_maincontrols.SetDrawBG(false);
	m_maincontrols.SetDrawFrame(false);
	m_maincontrols.AddObject(&m_mainmenu);
	m_maincontrols.NextLine();

	m_filemenu.SetFontSize(16);
	m_filemenu.Init(sizeof(filemenu)/sizeof(int));
	for(i=0;i<sizeof(filemenu)/sizeof(int);++i)
	{
		kGUIImageObj *icon;
		const char *iconfn;

		if(filemenu[i]<0)
			m_filemenu.GetEntry(i)->SetIsBar(true);
		else
		{
			m_filemenu.SetEntry(i,GetString(menutext[filemenu[i]]),filemenu[i]);
			m_filemenu.SetEntryEnable(filemenu[i],true);
			
			iconfn=menuicon[filemenu[i]];
			if(strlen(iconfn))
			{
				icon=m_filemenu.GetEntry(i)->GetIconObj();
				icon->SetFilename(iconfn);
				icon->SetSize(16+2,16);
				icon->SetXOffset(-2);	/* move over 2 pix to the right */
			}
		}
	}

	m_helpmenu.SetFontSize(16);
	m_helpmenu.Init(sizeof(helpmenu)/sizeof(int));
	for(i=0;i<sizeof(helpmenu)/sizeof(int);++i)
	{
		kGUIImageObj *icon;
		const char *iconfn;

		m_helpmenu.SetEntry(i,GetString(menutext[helpmenu[i]]),helpmenu[i]);
		m_helpmenu.SetEntryEnable(helpmenu[i],true);

		iconfn=menuicon[helpmenu[i]];
		if(strlen(iconfn))
		{
			icon=m_helpmenu.GetEntry(i)->GetIconObj();
			icon->SetFilename(iconfn);
			icon->SetSize(16+2,16);
			icon->SetXOffset(-2);	/* move over 2 pix to the right */
		}
	}

	m_colmenu.SetFontSize(14);
	m_colmenu.SetEventHandler(this,CALLBACKNAME(DoColMenu));

	kGUI::GetBackground()->AddObject(&m_maincontrols);

	//macro buttons will be added here!
	m_macrocontrols.SetDrawBG(false);
	m_macrocontrols.SetPos(m_maincontrols.GetZoneX(),m_maincontrols.GetZoneBY());
	m_macrocontrols.SetMaxWidth(kGUI::GetBackground()->GetChildZoneW()-m_macrocontrols.GetZoneX());
	m_macrocontrols.SetZoneH(24);
	kGUI::GetBackground()->AddObject(&m_macrocontrols);

	y=m_logo.GetZoneH()-m_tabs.GetTabRowHeight();
	/* font size on tabs? */
	m_tabs.SetPos(0,y);
	m_tabs.SetStartTabX(m_logo.GetZoneW()+10);
	m_tabs.SetSize(bw,bh-y-5);
	m_tabs.SetNumTabs(TAB_NUMTABS);
	assert((sizeof(tabnames)/sizeof(int))==TAB_NUMTABS,"Not enough strings in the tabname array!");
	for(i=0;i<TAB_NUMTABS;++i)
	{
		t=m_tabs.GetTabTextPtr(i);

		t->SetFontSize(BUTTONFONTSIZE);
		t->SetFontID(1);	/* bold */
		m_tabs.SetTabName(i,GetString(tabnames[i]));
	}
	m_tabs.SetEventHandler(this,CALLBACKNAME(TabChanged));
	kGUI::GetBackground()->AddObject(&m_tabs);

	m_rcmenu.SetFontSize(14);
	m_rcmenu.SetEventHandler(this,CALLBACKNAME(DoMapMenu));

	m_wpmenu.SetFontSize(14);
	m_wpmenu.SetEventHandler(this,CALLBACKNAME(DoWPMenu));

	m_controls.SetPos(0,0);
	m_controls.SetMaxWidth(bw);
	m_controls.SetBorderGap(0);

	m_mapcontrols.SetMaxWidth(bw);

	m_maptype=0;	/* change to load from prefs file */
	m_maptypecaption.SetPos(0,0);
	m_maptypecaption.SetFontSize(SMALLCAPTIONFONTSIZE);
	m_maptypecaption.SetFontID(SMALLCAPTIONFONT);
	m_maptypecaption.SetString(gpx->GetString(STRING_CURRENTMAP));

	m_maptypes.SetFontSize(BUTTONFONTSIZE);
	m_maptypes.SetPos(0,15);
	m_maptypes.SetSize(GetAdjust(250),20);
	m_maptypes.SetNumEntries(m_nummaps);
	m_maptypes.SetHint(gpx->GetString(STRING_CURRENTMAPHINT));
	m_maptypes.SetEventHandler(this,CALLBACKNAME(ChangeMapTypeEvent));
	m_mapcontrols.AddObjects(2,&m_maptypecaption,&m_maptypes);

	m_zoomcaption.SetPos(0,0);
	m_zoomcaption.SetFontSize(SMALLCAPTIONFONTSIZE);
	m_zoomcaption.SetFontID(SMALLCAPTIONFONT);
	m_zoomcaption.SetString(gpx->GetString(STRING_ZOOM));

	m_zoomgoto.SetFontSize(BUTTONFONTSIZE);
	m_zoomgoto.SetPos(0,15);
	m_zoomgoto.SetSize(GetAdjust(200),20);
	m_zoomgoto.SetEventHandler(this,CALLBACKNAME(ZoomGoto));
	m_zoomgoto.SetHint(gpx->GetString(STRING_ZOOMGOTOHINT));

	m_zoomin.SetPos(GetAdjust(200)+10,15);
	m_zoomin.SetHint(gpx->GetString(STRING_ZOOMINHINT));
	m_zoomin.SetImage(&m_zin);
	m_zoomin.Contain();
	m_zoomin.SetEventHandler(this,CALLBACKNAME(ClickZoomIn));

	m_zoomout.SetPos(GetAdjust(200)+40,15);
	m_zoomout.SetHint(gpx->GetString(STRING_ZOOMOUTHINT));
	m_zoomout.SetImage(&m_zout);
	m_zoomout.Contain();
	m_zoomout.SetEventHandler(this,CALLBACKNAME(ClickZoomOut));
	m_mapcontrols.AddObjects(4,&m_zoomcaption,&m_zoomgoto,&m_zoomin,&m_zoomout);

	m_onlinecaption.SetPos(0,0);
	m_onlinecaption.SetFontSize(SMALLCAPTIONFONTSIZE);
	m_onlinecaption.SetFontID(SMALLCAPTIONFONT);
	m_onlinecaption.SetString("Online");

	g_isonline=true;
	m_online.SetPos(0,15);
	m_online.SetHint("Enable / Disable Online downloading of map data.");
	m_online.SetSelected(true);
	m_online.SetEventHandler(this,CALLBACKNAME(Online));
	m_mapcontrols.AddObjects(2,&m_onlinecaption,&m_online);

	/* filter controls */
	m_filters.InitControls(&m_mapcontrols);

#if 0
	// not implemented yet
	/* search map for text input */
	m_quicksearchmapcaption.SetPos(0,0);
	m_quicksearchmapcaption.SetFontSize(SMALLCAPTIONFONTSIZE);
	m_quicksearchmapcaption.SetFontID(SMALLCAPTIONFONT);
	m_quicksearchmapcaption.SetString("Search Map");

	m_quicksearchmap.SetPos(0,15);
	m_quicksearchmap.SetHint("Search map.");
	m_quicksearchmap.SetSize(200,20);
	m_quicksearchmap.SetEventHandler(this,CALLBACKNAME(SearchMap));

	m_mapcontrols.AddObjects(2,&m_quicksearchmapcaption,&m_quicksearchmap);
#endif

	/* position under mouse pointer */
	m_ucaption.SetPos(0,0);
	m_ucaption.SetFontSize(SMALLCAPTIONFONTSIZE);
	m_ucaption.SetFontID(SMALLCAPTIONFONT);
	m_ucaption.SetString("Map Position under Mouse");

	/* map position under the mouse pointer */

	m_ulat.SetPos(0,15);
	m_ulat.SetHint("Latitude on map under Mouse.");
	m_ulat.SetFontSize(BUTTONFONTSIZE);
	m_ulat.SetSize(GetAdjust(100),20);
	m_ulat.SetLocked(true);

	m_ulon.SetPos(GetAdjust(100)+10,15);
	m_ulon.SetHint("Longitude on map under Mouse.");
	m_ulon.SetFontSize(BUTTONFONTSIZE);
	m_ulon.SetSize(GetAdjust(100),20);
	m_ulon.SetLocked(true);
	m_mapcontrols.AddObjects(3,&m_ucaption,&m_ulat,&m_ulon);

	/* Map controls */

	m_cposcaption.SetPos(0,0);
	m_cposcaption.SetFontSize(SMALLCAPTIONFONTSIZE);
	m_cposcaption.SetFontID(SMALLCAPTIONFONT);
	m_cposcaption.SetString("Current Center Position");

	m_clat.SetPos(0,15);
	m_clat.SetHint("Current center latitude.");
	m_clat.SetFontSize(BUTTONFONTSIZE);
	m_clat.SetSize(GetAdjust(100),20);
	m_clat.SetEventHandler(this,CALLBACKNAME(UpdateCenter));

	m_clon.SetPos(GetAdjust(100)+10,15);
	m_clon.SetHint("Current center longitude.");
	m_clon.SetFontSize(BUTTONFONTSIZE);
	m_clon.SetSize(GetAdjust(100),20);
	m_clon.SetEventHandler(this,CALLBACKNAME(UpdateCenter));

	m_mapcontrols.AddObjects(3,&m_cposcaption,&m_clat,&m_clon);

	m_nearcaption.SetPos(0,0);
	m_nearcaption.SetFontSize(SMALLCAPTIONFONTSIZE);
	m_nearcaption.SetFontID(SMALLCAPTIONFONT);
	m_nearcaption.SetString("Near Distance");

	m_neardist.Sprintf("%d",2);
	m_neardist.SetPos(0,15);
	m_neardist.SetHint("Distance for Near calculation using current draw units.");
	m_neardist.SetFontSize(BUTTONFONTSIZE);
	m_neardist.SetSize(GetAdjust(100),20);
	m_neardist.SetEventHandler(this,CALLBACKNAME(ReCalcNearEvent));

	m_mapcontrols.AddObjects(2,&m_nearcaption,&m_neardist);

	m_gpscaption.SetPos(0,0);
	m_gpscaption.SetFontSize(SMALLCAPTIONFONTSIZE);
	m_gpscaption.SetFontID(SMALLCAPTIONFONT);
	m_gpscaption.SetString("Realtime Tracking: GPSr Name / Connect / Track on Map / Position");

	m_realtimegps.SetFontSize(BUTTONFONTSIZE);
	m_realtimegps.SetPos(0,15);
	m_realtimegps.SetHint("Select GPSr for realtime tracking.");
	m_realtimegps.SetNumEntries(1);
	m_realtimegps.SetEntry(0,"No GPSrs defined",-1);
	m_realtimegps.SetSelection(-1);
	m_realtimegps.SetSize(GetAdjust(200),20);

	m_gpsconnect.SetPos(GetAdjust(200)+10,15);
	m_gpsconnect.SetHint("Connect/Disconnect to Tracking GPSr.");
	m_gpsconnect.SetEventHandler(this,CALLBACKNAME(GPSConnectChanged));
	m_gpsfollow.SetPos(210+15,15);
	m_gpsfollow.SetHint("Center map on GSP position.");

	m_gpslat.SetPos(GetAdjust(200)+10+15+15,15);
	m_gpslat.SetHint("GPS latitude.");
	m_gpslat.SetFontSize(BUTTONFONTSIZE);
	m_gpslat.SetSize(GetAdjust(100),20);
	m_gpslat.SetLocked(true);

	m_gpslon.SetPos(GetAdjust(200)+10+GetAdjust(100)+10+15+15,15);
	m_gpslon.SetHint("GPS longitude.");
	m_gpslon.SetFontSize(BUTTONFONTSIZE);
	m_gpslon.SetSize(GetAdjust(100),20);
	m_gpslon.SetLocked(true);
	m_mapcontrols.AddObjects(6,&m_gpscaption,&m_realtimegps,&m_gpsconnect,&m_gpsfollow,&m_gpslat,&m_gpslon);

	m_controls.AddObject(&m_mapcontrols);

	m_sortcols.SetGrow(true);
	m_sortcols.Alloc(3);
	m_sortrevs.SetGrow(true);
	m_sortrevs.Alloc(3);
	m_numsortcols=1;
	m_sortcols.SetEntry(0,GPXCOL_NAME);
	m_sortrevs.SetEntry(0,false);

	m_tabs.AddObject(&m_controls);
	y=m_controls.GetZoneH();
	bh=((m_tabs.GetChildZoneH())-15-m_controls.GetZoneH())>>2;
	m_fwt=m_filters.InitTable(&m_tabs,y,bh);

	m_divider.SetPos(0,y+bh);
	m_divider.SetSize(bw,10);
	m_divider.SetEventHandler(this,CALLBACKNAME(MoveDividerEvent));
	m_tabs.AddObject(&m_divider);

	SetMapAsync(true);	/* async tile loading is turned off for printing */
	m_grid.SetPos(0,y+bh+10);
	m_grid.Init(bw,m_tabs.GetChildZoneH()-(y+bh+10),256,256);
	m_grid.SetPreDrawCallBack(this,CALLBACKNAME(PreDrawMap));
	m_grid.SetDrawCallBack(this,CALLBACKNAME(DrawMapCell));
	m_grid.SetOverCallBack(this,CALLBACKNAME(OverMap));
	m_grid.SetPostDrawCallBack(this,CALLBACKNAME(PostDrawMap));
	m_tabs.AddObject(&m_grid);

	m_tabs.SetCurrentTab(TAB_ROUTE);
	m_routes.Init(&m_tabs);

	/* lines page */
	m_tabs.SetCurrentTab(TAB_LINES);
	m_lines.Init(&m_tabs);

	m_tabs.SetCurrentTab(TAB_FILTERS);
	m_filters.Init(&m_tabs);

	/* waypoint render preferences table */

	m_tabs.SetCurrentTab(TAB_SETTINGS);
	InitSettings();

	m_tabs.SetCurrentTab(TAB_GPSES);
	m_gpsr.Init(&m_tabs);

	/* solver page */
	m_tabs.SetCurrentTab(TAB_SOLVER);
	InitSolvers();		/* todo: needs to be made into a solverspage */

	/* stickers page */
	m_tabs.SetCurrentTab(TAB_STICKERS);
	m_stickers.Init(&m_tabs);

	/* tracks page */
	m_tabs.SetCurrentTab(TAB_TRACKS);
	m_tracks.Init(&m_tabs);

	/* notes page */
	m_tabs.SetCurrentTab(TAB_NOTES);
	m_notes.Init(&m_tabs);

	m_tabs.SetCurrentTab(TAB_DOWNLOAD);
	m_download.Init(&m_tabs);

	/***************************************************************/
	m_tabs.SetCurrentTab(TAB_SCRIPTS);

	bh=m_tabs.GetChildZoneH();

#ifdef INCLUDEWIG
	m_scripttabs.SetPos(0,0);
	m_scripttabs.SetSize(bw,bh-5);
	m_scripttabs.SetNumTabs(SCRIPTTAB_NUMTABS);
	for(i=0;i<SCRIPTTAB_NUMTABS;++i)
	{
		t=m_scripttabs.GetTabTextPtr(i);

		t->SetFontSize(16);
		t->SetFontID(1);	/* bold */

		m_scripttabs.SetTabName(i,GetString(scripttabnames[i]));
	}
	m_tabs.AddObject(&m_scripttabs);

	m_scripttabs.SetCurrentTab(SCRIPTTAB_BASIC);
	ptab=&m_scripttabs;
#else
	ptab=&m_tabs;
#endif
	tw=ptab->GetChildZoneW()-(m_basiccontrol.GetBorderGap()<<1);
	m_basiccontrol.SetPos(0,0);
	m_basiccontrol.SetSize(tw,20);

	m_basicstart.SetFontSize(BUTTONFONTSIZE);
	m_basicstart.SetString(gpx->GetString(STRING_START));
	m_basicstart.Contain();
	m_basicstart.SetEventHandler(this,CALLBACKNAME(StartBasic));
	m_basiccontrol.AddObject(&m_basicstart);

	m_basiccancel.SetFontSize(BUTTONFONTSIZE);
	m_basiccancel.SetString(gpx->GetString(STRING_ABORT));
	m_basiccancel.Contain();
	m_basiccancel.SetEventHandler(this,CALLBACKNAME(BasicCancel));
	m_basiccancel.SetEnabled(false);
	m_basiccontrol.AddObject(&m_basiccancel);

	m_basicaddbutton.SetFontSize(BUTTONFONTSIZE);
	m_basicaddbutton.SetString(gpx->GetString(STRING_ADDEDITMACROBUTTONS));
	m_basicaddbutton.Contain();
	m_basicaddbutton.SetEventHandler(this,CALLBACKNAME(BasicAddButton));
	m_basicaddbutton.SetEnabled(true);
	m_basiccontrol.AddObject(&m_basicaddbutton);
	m_basiccontrol.NextLine();

	m_basic.SetDoneCallback(this,CALLBACKNAME(BasicDone));
	m_basic.SetErrorCallback(this,CALLBACKNAME(BasicError));
	m_basicstartmenu.SetEventHandler(this,CALLBACKNAME(StartBasicMenuDone));

	bh=ptab->GetChildZoneH()-m_basiccontrol.GetZoneH();
	m_basicsource.SetFontID(2);		/* courier */
	m_basicsource.SetFontSize(20);
	m_basicsource.SetSize(tw,bh>>1);
	m_basicsource.SetWrap(false);		/* don't wrap lines, add scrolbar to buttom if off of right */
	m_basicsource.SetAllowEnter(true);
	m_basicsource.SetAllowTab(true);
	m_basicsource.SetLeaveSelection(true);

	/* calc the width of 4 spaces ( using current font etc ) for the tab size */
	m_basicsource.SetString("    ");
	m_basicsource.SetFixedTabs(true);
	m_basicsource.SetTab(0,m_basicsource.GetWidth());
	m_basicsource.Clear();
	m_basiccontrol.AddObject(&m_basicsource);

	m_basicdivider.SetSize(tw,10);
	m_basicdivider.SetEventHandler(this,CALLBACKNAME(MoveBasicDivider));
	m_basiccontrol.AddObject(&m_basicdivider);

	m_basicoutput.SetSize(tw,ptab->GetChildZoneH()-m_basiccontrol.GetCurrentY()-45);
	m_basiccontrol.AddObject(&m_basicoutput);

	m_basic.SetAddAppObjectsCallback(this,CALLBACKNAME(AddCClasses));
	ptab->AddObject(&m_basiccontrol);

#ifdef INCLUDEWIG
	m_scripttabs.AddObject(&m_basiccontrol);

	m_scripttabs.SetCurrentTab(SCRIPTTAB_LUA);

	m_luacontrol.SetPos(0,0);
	m_luacontrol.SetSize(tw,20);

	m_luaload.SetFontSize(BUTTONFONTSIZE);
	m_luaload.SetString(gpx->GetString(STRING_LOAD));
	m_luaload.Contain();
	m_luaload.SetEventHandler(this,CALLBACKNAME(ClickLoadLua));
	m_luacontrol.AddObject(&m_luaload);

	m_luastart.SetFontSize(BUTTONFONTSIZE);
	m_luastart.SetString(gpx->GetString(STRING_START));
	m_luastart.Contain();
	m_luastart.SetEventHandler(this,CALLBACKNAME(StartLua));
	m_luacontrol.AddObject(&m_luastart);

	m_luacancel.SetFontSize(BUTTONFONTSIZE);
	m_luacancel.SetString(gpx->GetString(STRING_ABORT));
	m_luacancel.Contain();
	m_luacancel.SetEventHandler(this,CALLBACKNAME(LuaCancel));
	m_luacancel.SetEnabled(false);
	m_luacontrol.AddObject(&m_luacancel);
	m_luacontrol.NextLine();

	bh=m_scripttabs.GetChildZoneH()-m_luacontrol.GetZoneH();
	m_luasource.SetFontID(2);		/* courier */
	m_luasource.SetFontSize(20);
	m_luasource.SetSize(tw,bh>>1);
	m_luasource.SetWrap(false);		/* don't wrap lines, add scrolbar to buttom if off of right */
	m_luasource.SetAllowEnter(true);
	m_luasource.SetAllowTab(true);
	m_luasource.SetLeaveSelection(true);

	/* calc the width of 4 spaces ( using current font etc ) for the tab size */
	m_luasource.SetString("    ");
	m_luasource.SetFixedTabs(true);
	m_luasource.SetTab(0,m_luasource.GetWidth());
	m_luasource.Clear();
	m_luacontrol.AddObject(&m_luasource);

	m_luadivider.SetSize(tw,10);
	m_luadivider.SetEventHandler(this,CALLBACKNAME(MoveLuaDivider));
	m_luacontrol.AddObject(&m_luadivider);
	m_luacontrol.NextLine();

	m_luaoutput.SetSize(tw,m_scripttabs.GetChildZoneH()-m_luacontrol.GetCurrentY()-45);
	m_luacontrol.AddObject(&m_luaoutput);
	m_scripttabs.AddObject(&m_luacontrol);

	/******************************************/
	m_scripttabs.SetCurrentTab(SCRIPTTAB_WIG);

	m_wigcontrol.SetPos(0,0);
	m_wigcontrol.SetSize(tw,20);

	m_loadwig.SetFontSize(BUTTONFONTSIZE);
	m_loadwig.SetString("Load Cartridge");
	m_loadwig.Contain();
	m_loadwig.SetEventHandler(this,CALLBACKNAME(ClickLoadWig));
	m_loadwig.SetEnabled(true);
	m_wigcontrol.AddObject(&m_loadwig);

	m_playwig.SetFontSize(BUTTONFONTSIZE);
	m_playwig.SetString("Play Cartridge");
	m_playwig.Contain();
	m_playwig.SetEventHandler(this,CALLBACKNAME(ClickPlayWig));
	m_playwig.SetEnabled(true);
	m_wigcontrol.AddObject(&m_playwig);
	m_wigcontrol.NextLine();

	m_wiginfo.SetSize(MIN(tw>>1,300),bh-50);
	m_wiginfo.SetLocked(true);
	m_wigcontrol.AddObject(&m_wiginfo);

	m_scripttabs.AddObject(&m_wigcontrol);

	m_scripttabs.SetCurrentTab(SCRIPTTAB_BASIC);
#endif
	/***************************************************************/
	m_tabs.SetCurrentTab(TAB_DEBUG);
	m_debug.SetPos(0,0);
	m_debug.SetSize(bw,m_tabs.GetChildZoneH());
	m_tabs.AddObject(&m_debug);

	m_tabs.SetCurrentTab(0);

	kGUI::SetInputCallback(this,CALLBACKNAME(CheckSpecialKeys));

	m_zoom=10;	/* default zoom */
	m_curmap->SetZoom(m_zoom);

	m_grid.SetBounds(0,0,m_curmap->GetWidth(),m_curmap->GetHeight());
	m_grid.ReSetCellSize(m_curmap->GetTileWidth(),m_curmap->GetTileHeight());
	
	{
		/* default map position */
		GPXCoord c;
		int sx,sy;

		c.SetLat(49.361803f);
		c.SetLon(-123.581085);
		m_curmap->ToMap(&c,&sx,&sy);
		m_grid.SetScrollCorner(sx,sy);
	}

	m_userhints.Alloc(MAXUSERTICKS);
	for(i=0;i<MAXUSERTICKS;++i)
		m_userhints.SetEntry(i,new kGUIString());

	/* defaults for printmap, overwritten by preferences */
	m_split=0;
	m_splitfontsize=12;
	m_splitsort=0;
	m_splitwidth.SetString("2.5");
	m_splitdesc.SetString(".Name, .Container, .Hint");
}

void GPX::MoveBasicDivider(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_AFTERUPDATE)
	{
		int move=event->m_value[0].i;
		int currenty=m_basicdivider.GetZoneY();
		int newy;

		newy=currenty+move;
		if(newy<100)
			newy=100;
		else if(newy>(m_tabs.GetChildZoneH()-100))
			newy=m_tabs.GetChildZoneH()-100;

		if(newy!=currenty)
		{
			int changey=currenty-newy;

			/* adjust size of top box */
			m_basicsource.SetZoneH(m_basicsource.GetZoneH()-changey);
			/* adjust position of divider */
			m_basicdivider.SetZoneY(m_basicdivider.GetZoneY()-changey);
			/* adjust position and size of bottom box */
			m_basicoutput.SetZoneY(m_basicoutput.GetZoneY()-changey);
			m_basicoutput.SetZoneH(m_basicoutput.GetZoneH()+changey);
		}
	}
}

void GPX::StartBasic(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		if(m_basic.Compile(&m_basicsource,&m_basicoutput)==true)
		{
			unsigned int i,numsubs;
			ClassArray<kGUIString>sublist;
			kGUIMsgBoxReq *msg;

			numsubs=m_basic.GetPublicSubs(&sublist);

			if(!numsubs)
				msg=new kGUIMsgBoxReq(MSGBOX_OK,false,"No public subroutines defined!");
			else
			{
				/* populate a popup menu with all public subroutine names*/
				m_basicstartmenu.SetNumEntries(numsubs);
				for(i=0;i<numsubs;++i)
					m_basicstartmenu.SetEntry(i,sublist.GetEntryPtr(i),i);
				m_basicstartmenu.Activate(kGUI::GetMouseX(),kGUI::GetMouseY());
			}
		}
	}
}

void GPX::StartBasicMenuDone(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_SELECTED)
	{
		int s;

		/* neg is invalid */
		s=m_basicstartmenu.GetSelection();
		if(s>=0)
		{
			m_basicoutput.Clear();
			m_basicsource.SetLocked(true);
			m_basicstart.SetEnabled(false);
			m_basiccancel.SetEnabled(true);
			m_basic.Start(m_basicstartmenu.GetSelectionString(),true);
		}
	}
}

void GPX::BasicCancel(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
		m_basic.Cancel();
}

MacroButtonEdit::MacroButtonEdit()
{
	Init();
}

void MacroButtonEdit::Init(void)
{
	unsigned int i;
	unsigned int numsubs;
	ClassArray<kGUIString>sublist;

	numsubs=gpx->m_basic.GetPublicSubs(&sublist);

	m_window.SetAllowButtons(WINDOWBUTTON_CLOSE);
	m_window.SetTitle("Edit Button");
	m_window.SetPos(kGUI::GetMouseX(),kGUI::GetMouseY());
	m_window.SetSize(800,200);
	kGUI::AddWindow(&m_window);
	m_window.SetEventHandler(this,CALLBACKNAME(WindowEvent));

	m_controls.SetPos(0,0);

	m_subcaption.SetPos(0,0);
	m_subcaption.SetFontSize(SMALLCAPTIONFONTSIZE);
	m_subcaption.SetFontID(SMALLCAPTIONFONT);
	m_subcaption.SetString("Subroutine");

	m_sublist.SetPos(0,15);
	m_sublist.SetSize(250,20);
	m_sublist.SetHint("Select old database name.");

	/* populate a popup menu with all public subroutine names*/
	m_sublist.SetNumEntries(numsubs);
	for(i=0;i<numsubs;++i)
		m_sublist.SetEntry(i,sublist.GetEntryPtr(i),i);

	m_controls.AddObjects(2,&m_subcaption,&m_sublist);

	m_buttoncaption.SetPos(0,0);
	m_buttoncaption.SetFontSize(SMALLCAPTIONFONTSIZE);
	m_buttoncaption.SetFontID(SMALLCAPTIONFONT);
	m_buttoncaption.SetString("Button Text");

	m_button.SetPos(0,15);
	m_button.SetSize(250,20);
	m_button.SetHint("Button Text");

	m_controls.AddObjects(2,&m_buttoncaption,&m_button);
	m_controls.NextLine();

	m_delete.SetFontSize(11);
	m_delete.SetPos(0,15);
	m_delete.SetSize(70,20);
	m_delete.SetString("Delete");
	m_delete.SetEventHandler(this,CALLBACKNAME(Delete));
	m_controls.AddObjects(1,&m_delete);

	m_cancel.SetFontSize(11);
	m_cancel.SetPos(0,15);
	m_cancel.SetSize(70,20);
	m_cancel.SetString("Cancel");
	m_cancel.SetEventHandler(this,CALLBACKNAME(Cancel));
	m_controls.AddObjects(1,&m_cancel);

	m_save.SetFontSize(11);
	m_save.SetPos(0,15);
	m_save.SetSize(70,20);
	m_save.SetString("Save");
	m_save.SetEventHandler(this,CALLBACKNAME(Save));
	m_controls.AddObjects(1,&m_save);

	m_window.AddObject(&m_controls);
	m_window.Shrink();
}

void MacroButtonEdit::WindowEvent(kGUIEvent *event)
{
	switch(event->GetEvent())
	{
	case EVENT_CLOSE:
		delete this;
	break;
	}
}

void GPX::BasicAddButton(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		if(m_basic.Compile(&m_basicsource,&m_basicoutput)==true)
		{
			unsigned int numsubs;
			ClassArray<kGUIString>sublist;
			kGUIMsgBoxReq *msg;

			numsubs=m_basic.GetPublicSubs(&sublist);

			if(!numsubs)
				msg=new kGUIMsgBoxReq(MSGBOX_OK,false,"No public subroutines defined!");
			else
			{
#if 1
				EditButtonWindowObj *ebw;

				ebw=new EditButtonWindowObj(numsubs,&sublist);
#else
				MacroButtonEdit *bbe;
				/* add a new button */
				bbe=new MacroButtonEdit();
#endif
			}
		}
	}
}

void GPX::BasicDone(void)
{
	m_basicsource.SetLocked(false);
	m_basicstart.SetEnabled(true);
	m_basiccancel.SetEnabled(false);
}

void GPX::ChangeMapType(void)
{
	GPXMAPInfo *mi;
	int sx,sy;
	GPXCoord c;

	m_maptype=m_maptypes.GetSelection();
	mi=m_mapinfo.GetEntry(m_maptype);

	m_grid.GetScrollCenter(&sx,&sy);
	m_curmap->FromMap(sx,sy,&c);	/* get l/l of map center */
	delete m_curmap;

	switch(mi->m_type)
	{
	case MAPTYPE_GOOGLESAT:
		m_curmap=new GGPXMap(MAPTYPE_GOOGLESAT);
	break;
	case MAPTYPE_GOOGLEHYBRID:
		m_curmap=new GGPXMap(MAPTYPE_GOOGLEHYBRID);
	break;
	case MAPTYPE_GOOGLEMAP:
		m_curmap=new GGPXMap(MAPTYPE_GOOGLEMAP);
	break;
	case MAPTYPE_GOOGLETERRAIN:
		m_curmap=new GGPXMap(MAPTYPE_GOOGLETERRAIN);
	break;
//	case MAPTYPE_TOPOCANADAMAP:
//		m_curmap=new ACGPXMap();
//	break;
	case MAPTYPE_OPENSTREETMAP:
		m_curmap=new OSMMap(mi->m_filename.GetString());
	break;
	case MAPTYPE_OZF2:
		m_curmap=new OZF2GPXMap(mi->m_filename.GetString());
	break;
	case MAPTYPE_MS:
		m_curmap=new MSGPXMap(mi->m_filename.GetString());
	break;
	case MAPTYPE_TERRASERVSAT:
		m_curmap=new TSGPXMap(MAPTYPE_TERRASERVSAT);
	break;
	case MAPTYPE_TERRASERVTOPO:
		m_curmap=new TSGPXMap(MAPTYPE_TERRASERVTOPO);
	break;
	default:
		assert(false,"Unhandled map type");
	break;
	}

	if(m_zoom>=(m_curmap->GetMaxZoom()))
		m_zoom=m_curmap->GetMaxZoom()-1;

	if(m_zoom<m_curmap->GetMinZoom())
		m_zoom=m_curmap->GetMinZoom();

	m_curmap->SetZoom(m_zoom);
	m_grid.Flush();
	m_grid.SetBounds(0,0,m_curmap->GetWidth(),m_curmap->GetHeight());
	m_grid.ReSetCellSize(m_curmap->GetTileWidth(),m_curmap->GetTileHeight());

	m_curmap->ToMap(&c,&sx,&sy);	/* convert l/l to map coords */
	m_grid.SetScrollCenter(sx,sy);

	UpdateZoomButtons();

	BSPDirty();	/* since projection has changed */
	MapDirty();
}

void GPX::GPSConnectChanged(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_AFTERUPDATE)
	{
		unsigned int i;

		if(m_gpsconnect.GetSelected())
		{
			m_gpsdelay=0;

			for(i=0;i<(sizeof(m_gpsdeltas)/sizeof(kGUIDPoint2))-1;++i)
			{
				m_gpsdeltas[i].x=0;
				m_gpsdeltas[i].y=0;
			}

			m_gpsr.SetInput(m_realtimegps.GetSelection(),m_realtime);
			m_realtime->StartTracking();
			m_realtimegps.SetLocked(true);		/* don't allow it to change if tracking */
		}
		else
		{
			if(m_realtime->IsTracking())
			{
				m_realtime->StopTracking();
				m_realtimegps.SetLocked(false);		/* allow it to change again */
			}
			m_gpslat.Clear();
			m_gpslon.Clear();
		}
	}
}

void GPX::ShowGeocaching(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_LEFTCLICK)
	{
		//todo, optionally open in built-in browser
		if(m_usebrowser.GetSelected())
		{
			HtmlWindow *win;
			kGUIString s;

			s.SetString("http://www.geocaching.com/");
			win=new HtmlWindow(BROWSE_URL,&s);
		}
		else
			kGUI::FileShow("http://www.geocaching.com/");
	}
}

void GPX::ShowFileMenu(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_LEFTCLICK)
	{
		kGUITableObj *table;

		/* are both corners set? */
		m_filemenu.SetEntryEnable(MAINMENU_SAVEMAPBITMAP,m_mapsel==3);
		m_filemenu.SetEntryEnable(MAINMENU_SAVEMAPKML,m_mapsel==3);

		/* if numfiltered < 100 then enable show */
		if(m_tabs.CurrentGroup()==TAB_ROUTE)
			table=m_routes.GetTable();
		else
			table=m_fwt;

		m_filemenu.SetEntryEnable(MAINMENU_VIEWPAGES,table->GetNumChildren()<100);
		
		m_filemenu.Activate(kGUI::GetMouseX(),kGUI::GetMouseY());
	}
}

void GPX::ShowHelpMenu(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_LEFTCLICK)
	{
		/* are both corners set? */
		m_helpmenu.Activate(kGUI::GetMouseX(),kGUI::GetMouseY());
	}
}

class Credits
{
public:
	Credits();
	static void GetCreditString(kGUIString *s);	/* also used in the help page */
private:
	CALLBACKGLUEPTR(Credits,WindowEvent,kGUIEvent)
	void WindowEvent(kGUIEvent *event);
	kGUIWindowObj m_window;
	kGUITextObj m_name;
	kGUITextObj m_desc;
};

Credits::Credits()
{
	int dh;
	int warea;

	m_window.SetAllowButtons(WINDOWBUTTON_CLOSE);
	m_window.SetSize(600,100);
	m_name.SetPos(0,0);
	m_name.SetFontSize(20);
	m_name.SetString("GPSTurbo v0.98");
	m_name.SetColor(DrawColor(255,0,0));
	m_window.AddObject(&m_name);
	
	warea=m_window.GetChildZoneW()-10;
	m_desc.SetFontSize(14);
	m_desc.SetPos(0,m_name.GetLineHeight());
	GetCreditString(&m_desc);

	/* append current goole map tile info */
	m_desc.ASprintf("\nCurrent Google Tiles (Map=%s,Sat=%s,Overlay=%s,Ter=%s)",
		GGPXMap::m_mapver.GetString(),
		GGPXMap::m_satmapver.GetString(),
		GGPXMap::m_overlayver.GetString(),
		GGPXMap::m_terver.GetString());

	m_desc.CalcLineList(warea);
	dh=m_desc.CalcHeight(warea);
	m_desc.SetSize(warea,dh);
	m_window.AddObject(&m_desc);

	m_window.SetTitle("Credits");
	m_window.SetEventHandler(this,CALLBACKNAME(WindowEvent));
	m_window.SetTop(true);
	m_window.ExpandToFit();
	m_window.Center();

	kGUI::AddWindow(&m_window);
}

void Credits::WindowEvent(kGUIEvent *event)
{
	switch(event->GetEvent())
	{
	case EVENT_CLOSE:
		delete this;
	break;
	}
}

void Credits::GetCreditString(kGUIString *s)
{
	s->SetString("Programmed by Kevin Pickell\r\n");
	s->ASprintf("Started: April 10, 2006, Current Build: %s\r\n\r\n",__DATE__);

	s->ASprintf("The Groundspeak Geocaching 4-box square logo is a registered Trademark of Groundspeak Inc.  Used with Permission.  All Rights Reserved.\r\n");
	s->ASprintf("Cache icon graphics (c) 2005 Groundspeak Inc. Used with permission.\r\n\r\n");

	s->ASprintf("Libraries/Code included in this program:\r\n\r\n");
	s->ASprintf("kGUI: %s\r\n",kGUI::GetVersion());

	s->ASprintf("FreeType: %s\r\n",kGUI::GetFTVersion());
	s->ASprintf("JpegLib: %d%c\r\n",kGUI::GetJpegVersion()/10,(char)(kGUI::GetJpegVersion()%10)+'a'-1);
	s->ASprintf("PNGLib: %s\r\n",kGUI::GetPngVersion());
	
	//	s->ASprintf("MYSQL: %s\r\n",__DATE__);
	s->ASprintf("ZLIB: %s\r\n",ZLIB_VERSION);
//	s->ASprintf("ffmpeg: %s\r\n",kGUI::GetFFMpegVersion());

	//run GPSBabel and grab it's version number
	{
		kGUIString v;
		BabelGlue b;

		b.GetVersion(&v);
		
		if(v.GetLen())
			s->ASprintf("\nExternally referenced code:\r\n\r\n%s\r\n",v.GetString());
	}


	s->ASprintf("\r\nSpecial thanks to:\r\n\r\n");
	s->ASprintf("Robert Lipe and the rest of the GPSBabel team.\r\n");
	s->ASprintf("John Mechalas for his Garmin IMG format documentation.\r\n");
	s->ASprintf("Google Maps\r\n");
	s->ASprintf("Center for Topographic Information Canada\r\n");
	s->ASprintf("National Geophysical Data Center USA. geomagc 2005\r\n");
	s->ASprintf("Ed Williams, Great Circle Calculator\r\n");
	s->ASprintf("Keld Helsgaun, LKH - traveling salesman heuristic.\r\n");
	s->ASprintf("Clive, author of GSAK.\r\n");
}

void GPX::DoMainMenu(kGUIEvent *event)
{
	switch(event->GetEvent())
	{
	case EVENT_ENTER:
	{
		kGUITableObj *table;

		/* are both corners set? */
		m_filemenu.SetEntryEnable(MAINMENU_SAVEMAPBITMAP,m_mapsel==3);

		/* if numfiltered < 100 then enable show */
		if(m_tabs.CurrentGroup()==TAB_ROUTE)
			table=m_routes.GetTable();
		else
			table=m_fwt;

		m_filemenu.SetEntryEnable(MAINMENU_VIEWPAGES,table->GetNumChildren()<100);
	}
	break;
	case EVENT_SELECTED:
		DoMenu(event->m_value[0].i);
	break;
	}
}

void GPX::DoMenu(int selection)
{
	switch(selection)
	{
	case MAINMENU_LOAD:
	{
		kGUIFileReq *req;
		
		req=new kGUIFileReq(FILEREQ_OPEN,m_defpath.GetString(),".gpx",this,CALLBACKNAME(GetLoadSettings));
	}
	break;
	case MAINMENU_SAVE:
	{
		kGUIFileReq *req;
		
		req=new kGUIFileReq(FILEREQ_SAVE,m_defpath.GetString(),".gpx",this,CALLBACKNAME(DoSave));
	}
	break;
	case MAINMENU_LOADOTHER:
	{
		kGUIFileReq *req;

		/* todo: make sure babel is available */

		req=new kGUIFileReq(FILEREQ_OPEN,m_defpath.GetString(),0,this,CALLBACKNAME(DoLoadOther));
	}
	break;
	case MAINMENU_SAVEOTHER:
	{
		kGUIFileReq *req;

		/* todo: make sure babel is available */
		
		req=new kGUIFileReq(FILEREQ_SAVE,m_defpath.GetString(),0,this,CALLBACKNAME(DoSaveAs));
	}
	break;
	case MAINMENU_SAVECHANGES:
		SavePrefs(true);
	break;
	case MAINMENU_RENAMEDATABASE:
	{
		kGUIRenameDBReq *req;
		
		req=new kGUIRenameDBReq();
	}
	break;
	case MAINMENU_VIEWGENERATED:
	{
		RemoveStale *rs;
		
		rs=new RemoveStale();
	}
	break;
	case MAINMENU_PRINTTABLE:
	{
		kGUITableObj *table;

		if(m_tabs.CurrentGroup()==TAB_ROUTE)
			table=m_routes.GetTable();
		else
			table=m_fwt;

		DoPrintTable(table);
	}
	break;
	case MAINMENU_PRINTMAP:
		DoPrintMap();
	break;
	case MAINMENU_SAVEMAPBITMAP:
	{
		kGUIFileReq *req;

		req=new kGUIFileReq(FILEREQ_SAVE,m_defpath.GetString(),".jpg",this,CALLBACKNAME(DoSaveMapShape));
	}
	break;
	case MAINMENU_SAVEMAPKML:
	{
		kGUIFileReq *req;

		req=new kGUIFileReq(FILEREQ_SAVE,m_defpath.GetString(),".kml",this,CALLBACKNAME(DoSaveMapKML));
	}
	break;
	case MAINMENU_VIEWPAGES:
		if(m_usebrowser.GetSelected())
		{
			int e,ne;
			kGUITableObj *table;
			GPXRow *row;
			DataHandle dh;
			kGUIString s;
			HtmlWindow *win;

			if(m_tabs.CurrentGroup()==TAB_ROUTE)
				table=m_routes.GetTable();
			else
				table=m_fwt;
		
			dh.SetMemory();
			dh.OpenWrite("wb");
			ne=table->GetNumChildren();
			for(e=0;e<ne;++e)
			{
				row=static_cast<GPXRow *>(table->GetChild(e));
				if(row->m_copiedfrom)
					row=row->m_copiedfrom;
				BuildPage(&dh,row,e==0,e==(ne-1));
			}
			dh.Close();

			dh.Open();
			dh.Read(&s,dh.GetSize());
			dh.Close();

			win=new HtmlWindow(BROWSE_DATA,&s);
		}
		else
		{
			int e,ne;
			DataHandle dh;
			kGUITableObj *table;
			GPXRow *row;

			if(m_tabs.CurrentGroup()==TAB_ROUTE)
				table=m_routes.GetTable();
			else
				table=m_fwt;

			dh.SetFilename("temp.html");
			dh.OpenWrite("wb");
			ne=table->GetNumChildren();
			for(e=0;e<ne;++e)
			{
				row=static_cast<GPXRow *>(table->GetChild(e));
				if(row->m_copiedfrom)
					row=row->m_copiedfrom;
				BuildPage(&dh,row,e==0,e==(ne-1));
			}
			dh.Close();
			kGUI::FileShow("temp.html");
		}
	break;
	case MAINMENU_CREDITS:
	{
		Credits *credits;
		credits=new Credits();
	}
	break;
	case MAINMENU_HELP:
		if(m_usebrowser.GetSelected())
		{
			kGUIString h;
			kGUIString credits;
			HtmlWindow *win;

extern char helpstart[];
extern char help[];
extern char helpend[];

extern char xtabstart[];
extern char xtabend[];
extern char xgroupstart[];
extern char xgroupend[];
extern char xcolstart[];
extern char xcolend[];

			Credits::GetCreditString(&credits);
			credits.Replace("\r\n","<BR>");

			h.SetString(helpstart);
			h.Append(help);
			h.Append(kGUIBasic::GetInstructions());
			GetAppBasicInstructions(&h);
			h.Append(helpend);

			h.Replace("<xTABSTART>",xtabstart);
			h.Replace("<xTABEND>",xtabend);
			h.Replace("<xGROUPSTART>",xgroupstart);
			h.Replace("<xGROUPEND>",xgroupend);
			h.Replace("<xCOLSTART>",xcolstart);
			h.Replace("<xCOLEND>",xcolend);
			h.Replace("<CREDITS>",credits.GetString());

			win=new HtmlWindow(BROWSE_DATA,&h);
		}
		else
		{
			FILE *f;
			kGUIString h;
			kGUIString credits;

extern char helpstart[];
extern char help[];
extern char helpend[];

extern char xtabstart[];
extern char xtabend[];
extern char xgroupstart[];
extern char xgroupend[];
extern char xcolstart[];
extern char xcolend[];

			Credits::GetCreditString(&credits);
			credits.Replace("\r\n","<BR>");

			h.SetString(helpstart);
			h.Append(help);
			h.Append(helpend);
			h.Append(kGUIBasic::GetInstructions());

			GetAppBasicInstructions(&h);

			h.Replace("<xTABSTART>",xtabstart);
			h.Replace("<xTABEND>",xtabend);
			h.Replace("<xGROUPSTART>",xgroupstart);
			h.Replace("<xGROUPEND>",xgroupend);
			h.Replace("<xCOLSTART>",xcolstart);
			h.Replace("<xCOLEND>",xcolend);
			h.Replace("<CREDITS>",credits.GetString());

			f=fopen("temp.html","wb");
			fwrite(h.GetString(),h.GetLen(),1,f);
			fclose(f);
			kGUI::FileShow("temp.html");
		}
	break;
	case MAINMENU_QUIT:
		kGUI::CloseApp();
	break;
	}
}

class LoadWPTSettings
{
public:
	LoadWPTSettings(kGUIXML *xml,const char *fn,const char *defdb);
	void SetDoneCallBack(void *codeobj,void (*code)(void *)) {m_donecallback.Set(codeobj,code);}
	void SetLoadTracksToo(void) {m_loadtrackstoo=true;}
private:
	CALLBACKGLUEPTR(LoadWPTSettings,WindowEvent,kGUIEvent)
	CALLBACKGLUEPTR(LoadWPTSettings,PressRadioLoad,kGUIEvent)
	CALLBACKGLUEPTR(LoadWPTSettings,PressRadioUser,kGUIEvent)
	CALLBACKGLUEPTR(LoadWPTSettings,PressOK,kGUIEvent)
	CALLBACKGLUEPTR(LoadWPTSettings,PressCancel,kGUIEvent)
	void WindowEvent(kGUIEvent *event);
	void PressRadioLoad(kGUIEvent *event) {if(event->GetEvent()==EVENT_AFTERUPDATE){m_user.SetSelected(!m_load.GetSelected());}}
	void PressRadioUser(kGUIEvent *event) {if(event->GetEvent()==EVENT_AFTERUPDATE){m_load.SetSelected(!m_user.GetSelected());}}
	void PressCancel(kGUIEvent *event);
	void PressOK(kGUIEvent *event);
	kGUIWindowObj m_window;

	kGUIControlBoxObj m_controls;
	bool m_loadtrackstoo;
	kGUITextObj m_loadcaption; 
	kGUIRadioObj m_load;
	kGUITextObj m_dbnamecaption; 
	kGUIInputBoxObj m_dbname;
	kGUITextObj m_clipfoundlogscaption; 
	kGUIInputBoxObj m_clipfoundlogs;
	kGUITextObj m_clipnotfoundlogscaption; 
	kGUIInputBoxObj m_clipnotfoundlogs;

	kGUITextObj m_usercaption; 
	kGUIRadioObj m_user;
	kGUITextObj m_userslotcaption; 
	kGUIComboBoxObj m_userslot;
	kGUITextObj m_userhintcaption; 
	kGUIInputBoxObj m_userhint;

	kGUIButtonObj m_ok;
	kGUIButtonObj m_cancel;
	kGUIXML *m_xml;
	kGUIString m_filename;
	kGUICallBack m_donecallback;
};

LoadWPTSettings::LoadWPTSettings(kGUIXML *xml,const char *fn,const char *defdb)
{
	int i;
	int w=320,h=200;
	kGUIXMLItem *xmlitem;

	m_filename.SetString(fn);
	m_xml=xml;
	xmlitem=xml->GetRootItem()->Locate("gpx");
	if(xmlitem)
	{
		if(xmlitem->Locate("name"))
			m_dbname.SetString(xmlitem->Locate("name")->GetValue());
		else
		{
			const char *cp;
			unsigned int n;

			m_dbname.SetString(defdb);
			cp=m_dbname.GetString();
			while(strstr(cp,DIRCHAR))
				cp=strstr(cp,DIRCHAR)+1;
			n=(int)(cp-m_dbname.GetString());
			if(n)
				m_dbname.Delete(0,n);
			for(n=0;n<m_dbname.GetLen();++n)
			{
				if(m_dbname.GetChar(n)=='.')
				{
					m_dbname.Clip(n);
					break;
				}
			}
		}
		m_userhint.SetString(m_dbname.GetString());
	}

	m_window.SetTitle("Load Settings");
	m_window.SetEventHandler(this,CALLBACKNAME(WindowEvent));
	m_window.SetSize(w,h);
	m_window.SetPos( (kGUI::GetSurfaceWidth()-w)/2,(kGUI::GetSurfaceHeight()-h)/2);
	m_window.SetTop(true);
	kGUI::AddWindow(&m_window);

	m_load.SetSelected(true);
	m_load.SetEventHandler(this,CALLBACKNAME(PressRadioLoad));
	m_load.SetPos(0,10);

	m_loadcaption.SetFontID(1);
	m_loadcaption.SetPos(40,10);
	m_loadcaption.SetString("Load Waypoints");
	m_controls.AddObjects(2,&m_load,&m_loadcaption);
	m_controls.NextLine();

	m_dbnamecaption.SetPos(0,0);
	m_dbnamecaption.SetString("Database Name");
	m_dbname.SetPos(100,0);
	m_dbname.SetSize(200,20);
	m_controls.AddObjects(2,&m_dbnamecaption,&m_dbname);
	m_controls.NextLine();

	m_clipfoundlogscaption.SetPos(0,0);
	m_clipfoundlogscaption.SetString("Maximum # of Found Logs:");
	m_clipfoundlogs.SetPos(200,0);
	m_clipfoundlogs.SetSize(100,20);
	if(gpx->GetClipFoundLogs()>=0)
		m_clipfoundlogs.SetInt(gpx->GetClipFoundLogs());

	m_controls.AddObjects(2,&m_clipfoundlogscaption,&m_clipfoundlogs);
	m_controls.NextLine();

	m_clipnotfoundlogscaption.SetPos(0,0);
	m_clipnotfoundlogscaption.SetString("Maximum # of Not Found Logs:");
	m_clipnotfoundlogs.SetPos(200,0);
	m_clipnotfoundlogs.SetSize(100,20);
	if(gpx->GetClipNotFoundLogs()>=0)
		m_clipnotfoundlogs.SetInt(gpx->GetClipNotFoundLogs());

	m_controls.AddObjects(2,&m_clipnotfoundlogscaption,&m_clipnotfoundlogs);
	m_controls.NextLine();

	m_user.SetPos(0,10);
	m_user.SetEventHandler(this,CALLBACKNAME(PressRadioUser));

	m_usercaption.SetFontID(1);
	m_usercaption.SetPos(40,10);
	m_usercaption.SetString("Load Found into UserTick");
	m_controls.AddObjects(2,&m_user,&m_usercaption);
	m_controls.NextLine();

	m_userslotcaption.SetPos(0,0);
	m_userslotcaption.SetString("Load Found into:");
	m_userslot.SetPos(100,0);
	m_userslot.SetNumEntries(MAXUSERTICKS);
	for(i=0;i<MAXUSERTICKS;++i)
	{
		kGUIString uname;
		uname.Sprintf("UserTick #%d",i+1);
		m_userslot.SetEntry(i,uname.GetString(),i);
	}
	m_userslot.SetSize(100,20);

	m_controls.AddObjects(2,&m_userslotcaption,&m_userslot);
	m_controls.NextLine();

	m_userhintcaption.SetPos(0,0);
	m_userhintcaption.SetString("User Column Hint");
	m_userhint.SetPos(100,0);
	m_userhint.SetSize(200,20);
	m_controls.AddObjects(2,&m_userhintcaption,&m_userhint);
	m_controls.NextLine();

	m_cancel.SetPos(0,0);
	m_cancel.SetSize(100,20);
	m_cancel.SetString("Cancel");
	m_cancel.SetEventHandler(this,CALLBACKNAME(PressCancel));

	m_ok.SetPos(125,0);
	m_ok.SetSize(100,20);
	m_ok.SetString("Load");
	m_ok.SetEventHandler(this,CALLBACKNAME(PressOK));

	m_controls.AddObjects(2,&m_cancel,&m_ok);
	m_controls.NextLine();

	m_window.AddObject(&m_controls);
	m_window.ExpandToFit();
}

void LoadWPTSettings::WindowEvent(kGUIEvent *event)
{
	switch(event->GetEvent())
	{
	case EVENT_CLOSE:
		if(m_donecallback.IsValid())
			m_donecallback.Call();
		else
			delete m_xml;
		delete this;
	break;
	}
}

void LoadWPTSettings::PressCancel(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
		m_window.Close();
}

void LoadWPTSettings::PressOK(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		if(m_load.GetSelected()==true)
		{
			if(m_clipfoundlogs.IsNull())
				gpx->SetClipFoundLogs(-1);
			else
				gpx->SetClipFoundLogs(m_clipfoundlogs.GetInt());

			if(m_clipnotfoundlogs.IsNull())
				gpx->SetClipNotFoundLogs(-1);
			else
				gpx->SetClipNotFoundLogs(m_clipnotfoundlogs.GetInt());

			gpx->DoLoad(m_xml,m_dbname.GetString(),m_filename.GetString());
		}
		else
		{
			gpx->SetUserHint(m_userslot.GetSelection(),&m_userhint);
			gpx->DoLoadUserFound(m_xml,m_userslot.GetSelection());
		}
		m_window.Close();
	}
}

/************************************************************************/

typedef struct
{
const char *name;
const char *desc;
const char *ext;
bool show;
}BABELTYPES_DEF;

enum
{
LOADAS_LOAD,
LOADAS_SAVE};

class LoadAs
{
public:
	LoadAs(int type,const char *fn);
private:
	CALLBACKGLUEPTR(LoadAs,WindowEvent,kGUIEvent)
	CALLBACKGLUEPTR(LoadAs,PressRadioSuggested,kGUIEvent)
	CALLBACKGLUEPTR(LoadAs,PressRadioAll,kGUIEvent)
	CALLBACKGLUEPTR(LoadAs,PressOK,kGUIEvent)
	CALLBACKGLUEPTR(LoadAs,PressCancel,kGUIEvent)
	void PressRadioAll(kGUIEvent *event);
	void PressRadioSuggested(kGUIEvent *event);
	void WindowEvent(kGUIEvent *event);
	void ShowTypes(void);
	void PressCancel(kGUIEvent *event);
	void PressOK(kGUIEvent *event);

	kGUIWindowObj m_window;

	kGUIControlBoxObj m_controls;
	kGUITextObj m_filenamecaption; 
	kGUIInputBoxObj m_filename;

	kGUITextObj m_showsuggestedcaption; 
	kGUIRadioObj m_showsuggested;
	kGUITextObj m_showallcaption; 
	kGUIRadioObj m_showall;

	kGUITextObj m_filetypecaption; 
	kGUIComboBoxObj m_filetype;

	kGUIButtonObj m_ok;
	kGUIButtonObj m_cancel;

	int m_type;
	int m_numbabeltypes;
	BabelGlue m_babel;
	Array<BABELTYPES_DEF>m_babeltypes;
};

LoadAs::LoadAs(int type,const char *fn)
{
	int w=420,h=200;
	BABELTYPES_DEF bt;

	m_type=type;
	/* build array for valid input types */
	m_numbabeltypes=0;
	m_babeltypes.Init(64,32);
	do
	{
		if(m_babel.GetInputFormats(m_numbabeltypes,&bt.name,&bt.desc,&bt.ext)==0)
			break;
		m_babeltypes.SetEntry(m_numbabeltypes++,bt);
	}while(1);

	m_filename.SetString(fn);
	if(type==LOADAS_LOAD)
		m_window.SetTitle("Load As");
	else
		m_window.SetTitle("Save As");
	m_window.SetEventHandler(this,CALLBACKNAME(WindowEvent));
	m_window.SetSize(w,h);
	m_window.SetPos( (kGUI::GetSurfaceWidth()-w)/2,(kGUI::GetSurfaceHeight()-h)/2);
	m_window.SetTop(true);
	kGUI::AddWindow(&m_window);

	m_filenamecaption.SetFontID(1);
	m_filenamecaption.SetPos(10,10);
	m_filenamecaption.SetString("Filename");
	m_filename.SetPos(70,0);
	m_filename.SetSize(350,20);
	m_filename.SetString(fn);
	m_controls.AddObjects(2,&m_filenamecaption,&m_filename);
	m_controls.NextLine();

	m_showsuggested.SetSelected(true);
	m_showsuggested.SetPos(0,10);
	m_showsuggested.SetEventHandler(this,CALLBACKNAME(PressRadioSuggested));

	m_showsuggestedcaption.SetFontID(1);
	m_showsuggestedcaption.SetPos(40,10);
	m_showsuggestedcaption.SetString("Show suggested file types");
	m_controls.AddObjects(2,&m_showsuggested,&m_showsuggestedcaption);
	m_controls.NextLine();

	m_showall.SetEventHandler(this,CALLBACKNAME(PressRadioAll));
	m_showall.SetPos(0,10);

	m_showallcaption.SetFontID(1);
	m_showallcaption.SetPos(40,10);
	m_showallcaption.SetString("Show all file types");
	m_controls.AddObjects(2,&m_showall,&m_showallcaption);
	m_controls.NextLine();

	m_filetypecaption.SetFontID(1);
	m_filetypecaption.SetPos(10,10);
	m_filetypecaption.SetString("File type");

	m_filetype.SetPos(70,0);
	m_filetype.SetSize(350,20);
	m_controls.AddObjects(2,&m_filetypecaption,&m_filetype);
	m_controls.NextLine();

	m_cancel.SetPos(0,0);
	m_cancel.SetSize(100,20);
	m_cancel.SetString("Cancel");
	m_cancel.SetEventHandler(this,CALLBACKNAME(PressCancel));

	m_ok.SetPos(125,0);
	m_ok.SetSize(100,20);
	if(type==LOADAS_LOAD)
		m_ok.SetString("Load");
	else
		m_ok.SetString("Save");
	m_ok.SetEventHandler(this,CALLBACKNAME(PressOK));

	m_controls.AddObjects(2,&m_cancel,&m_ok);
	m_controls.NextLine();

	ShowTypes();

	m_window.AddObject(&m_controls);
	m_window.ExpandToFit();
}

void LoadAs::PressRadioSuggested(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_AFTERUPDATE)
	{
		m_showall.SetSelected(!m_showsuggested.GetSelected());
		ShowTypes();
	}
}

void LoadAs::PressRadioAll(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_AFTERUPDATE)
	{
		m_showsuggested.SetSelected(!m_showall.GetSelected());
		ShowTypes();
	}
}

/* show the available types in a combobox */
void LoadAs::ShowTypes(void)
{
	int i,numshow;
	BABELTYPES_DEF bt;
	kGUIString ext;

	/* count number that match current settings */
	do
	{
		numshow=0;
		for(i=0;i<m_numbabeltypes;++i)
		{
			bt=m_babeltypes.GetEntry(i);
			bt.show=false;
			if(m_showall.GetSelected()==true)
			{
				bt.show=true;
				++numshow;
			}
			else
			{
				if(bt.ext)
				{
					/* some have '.' at beginning already, others don't */
					ext.SetString(bt.ext);
					if(ext.GetChar(0)!='.')
						ext.Insert(0,".");

					if(!stricmp(m_filename.GetString()+m_filename.GetLen()-ext.GetLen(),ext.GetString()))
					{
						bt.show=true;
						++numshow;
					}
				}
			}
			m_babeltypes.SetEntry(i,bt);
		}

		/* if none to show, then force back to show all */
		if(!numshow)
		{
			m_showall.SetSelected(true);
			m_showsuggested.SetSelected(false);
		}
	}while(!numshow);

	m_filetype.SetNumEntries(numshow);
	numshow=0;
	for(i=0;i<m_numbabeltypes;++i)
	{
		bt=m_babeltypes.GetEntry(i);
		if(bt.show)
			m_filetype.SetEntry(numshow++,bt.desc,bt.name);
	}
	/* select the first one in the list */
	m_filetype.SetSelection(0);
	//if(numshow>1)
	//	m_filetype.Activate();
}

void LoadAs::WindowEvent(kGUIEvent *event)
{
	switch(event->GetEvent())
	{
	case EVENT_CLOSE:
		delete this;
	break;
	}
}

void LoadAs::PressCancel(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
		m_window.Close();
}

/* use GPSBabel to convert from other format to GPX format, then load */

void LoadAs::PressOK(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		/* trigger gpsbabel and see if we can convert this file to gpx format */
		kGUIString fn;

		if(m_type==LOADAS_LOAD)
		{
			m_babel.SetInput(m_filetype.GetSelectionString(),m_filename.GetString());
			m_babel.AddType(BABELTYPE_WAYPOINTS);
			m_babel.AddType(BABELTYPE_TRACKS);
			m_babel.SetOutput("gpx","babel.gpx");

			/* send to gpxbabel for processing */
			if(m_babel.Call(true,0)==false)
				return;
			else
			{
				/* ok, now load file */
				gpx->OpenLoadSettings("babel.gpx",m_filename.GetString());
				m_window.Close();
			}
		}
		else
		{
			/* save as */
			gpx->DoSaveAs2(m_filename.GetString(),m_filetype.GetSelectionString());
			m_window.Close();
		}
	}
}

void GPX::DoLoadOther(kGUIFileReq *result,int pressed)
{
	LoadAs *la;
	kGUIString fn;

	if(pressed==MSGBOX_OK)
	{
		fn.SetString(result->GetFilename());
		la=new LoadAs(LOADAS_LOAD,fn.GetString());
	}
}

const char *rscolnames[REMOVESTALE_NUMCOLUMNS]={
	"Delete","Date","Number"};

const char *rscoldesc[REMOVESTALE_NUMCOLUMNS]={
	"Delete Selected Waypoints","Generated Date","Number"};
	
const int rscolwidths[REMOVESTALE_NUMCOLUMNS]={60,125,75};

RemoveStale::RemoveStale()
{
	int i;

	m_all.SetSelected(true);
	m_all.SetPos(0,10);
	m_all.SetEventHandler(this,CALLBACKNAME(PressRadioAll));

	m_allcaption.SetFontID(1);
	m_allcaption.SetPos(40,10);
	m_allcaption.SetString("Show using all waypoints");
	m_controls.AddObjects(2,&m_all,&m_allcaption);
	m_controls.NextLine();

	m_filter.SetEventHandler(this,CALLBACKNAME(PressRadioFilter));
	m_filter.SetPos(0,10);

	m_filtercaption.SetFontID(1);
	m_filtercaption.SetPos(40,10);
	m_filtercaption.SetString("Show using filtered waypoints");
	m_controls.AddObjects(2,&m_filter,&m_filtercaption);
	m_controls.NextLine();

	m_list.SetSize(320,200);
	m_list.SetNumCols(REMOVESTALE_NUMCOLUMNS);
	for(i=0;i<REMOVESTALE_NUMCOLUMNS;++i)
	{
		m_list.SetColTitle(i,rscolnames[i]);
		m_list.SetColHint(i,rscoldesc[i]);
		m_list.SetColWidth(i,rscolwidths[i]);
	}
	m_controls.AddObjects(1,&m_list);
	m_controls.NextLine();

	CalcMatches();

	m_cancel.SetPos(0,0);
	m_cancel.SetSize(100,20);
	m_cancel.SetString("Cancel");
	m_cancel.SetEventHandler(this,CALLBACKNAME(PressCancel));

	m_ok.SetPos(125,0);
	m_ok.SetSize(100,20);
	m_ok.SetString("Remove Selected");
	m_ok.SetEventHandler(this,CALLBACKNAME(PressRemove));

	m_controls.AddObjects(2,&m_cancel,&m_ok);
	m_controls.NextLine();

	m_window.AddObject(&m_controls);
	m_window.SetTitle("Remove Stale Waypoints");
	m_window.SetEventHandler(this,CALLBACKNAME(WindowEvent));
	m_window.SetSize(100,100);
	m_window.ExpandToFit();
	m_window.SetPos( (kGUI::GetSurfaceWidth()-m_window.GetZoneW())/2,(kGUI::GetSurfaceHeight()-m_window.GetZoneH())/2);
	m_window.SetTop(true);
	kGUI::AddWindow(&m_window);
}

void RemoveStale::WindowEvent(kGUIEvent *event)
{
	switch(event->GetEvent())
	{
	case EVENT_CLOSE:
		delete this;
	break;
	}
}

int RemoveStale::Sort(const void *o1,const void *o2)
{
	RemoveStaleRow *r1=*(static_cast<RemoveStaleRow **>((void *)o1));
	RemoveStaleRow *r2=*(static_cast<RemoveStaleRow **>((void *)o2));

	return(strcmp(r1->GetDate(),r2->GetDate()));
}

void RemoveStale::CalcMatches(void)
{
	unsigned int i;
	GPXRow *row;
	int *countp;
	int start=1;
	unsigned int num;
	HashEntry *he;
	RemoveStaleRow *rsr;
	Hash hash;

	hash.Init(12,sizeof(int));

	/* use whole list */
	if(m_all.GetSelected())
	{
		for(i=0;i<gpx->m_numwpts;++i)
		{
			row=gpx->m_wptlist.GetEntry(i);
			countp=(int *)hash.Find(row->GetGenDate());
			if(countp)
				*(countp)=*(countp)+1;
			else
				hash.Add(row->GetGenDate(),&start);
		}
	}
	else	/* use current filtered results list */
	{
		kGUITableObj *ft;

		ft=gpx->m_fwt;

		for(i=0;i<ft->GetNumChildren();++i)
		{
			row=static_cast<GPXRow *>(ft->GetChild(i));
			countp=(int *)hash.Find(row->GetGenDate());
			if(countp)
				*(countp)=*(countp)+1;
			else
				hash.Add(row->GetGenDate(),&start);
		}
	}
	/* rebuild the list table */
	m_list.DeleteChildren();

	num=hash.GetNum();
	he=hash.GetFirst();
	for(i=0;i<num;++i)
	{
		rsr=new RemoveStaleRow(he->m_string,*((int *)(he->m_data)));
		m_list.AddRow(rsr);
		he=he->GetNext();
	}

	/* sort oldest to newest */
	m_list.Sort(Sort);
}

void RemoveStale::PressCancel(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
		m_window.Close();
}

void RemoveStale::PressRemove(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		unsigned int i;
		Hash hash;
		RemoveStaleRow *rsr;
		GPXRow *row;
		int rflag;
		int *dp;
		kGUITableObj *ft;
		bool all=m_all.GetSelected();

		ft=gpx->m_fwt;

		/* remove any selected waypoints */
		hash.Init(12,sizeof(int));

		for(i=0;i<m_list.GetNumChildren();++i)
		{
			rsr=static_cast<RemoveStaleRow *>(m_list.GetChild(i));
			rflag=rsr->GetSelected()==true?1:0;
			hash.Add(rsr->GetDate(),&rflag);
		}

		i=0;
		while(i<gpx->m_numwpts)
		{
			row=gpx->m_wptlist.GetEntry(i);

			if((all==true) || (row->GetInResults()))
			{
				dp=(int *)hash.Find(row->GetGenDate());
				if(dp)
				{
					if(*(dp)==1)
					{
						/* if this row is in the filtered results table */
						/* then remove it but don't free it */
						if(row->GetInResults())
							ft->DeleteRow(row,false);

						gpx->m_wptlist.DeleteEntry(i);
						--gpx->m_numwpts;
						delete row;
						--i;
					}
				}
			}
			++i;
		}
		gpx->UpdateFilterCount();

		/* rebuild database list */
		gpx->UpdateDBList();
		gpx->BSPDirty();
		m_window.Close();
	}
}

void GPX::GetLoadSettings(kGUIFileReq *result,int pressed)
{
	if(pressed==MSGBOX_OK)
	{
		/* save default path for next time */
		m_defpath.SetString(result->GetPath());

		OpenLoadSettings(result->GetFilename(),result->GetFilename());
	}
}

void GPX::OpenLoadSettings(const char *fn,const char *defdb)
{
	kGUIXML *xml;
	kGUIXMLItem *xroot;
	kGUIXMLItem *xi;
	kGUIMsgBoxReq *box;
	int numwp,numtrk;

	m_busy=0;
	xml=new kGUIXML();
	xml->SetNameCache(&m_xmlnamecache);
	xml->SetLoadingCallback(this,CALLBACKNAME(PreLoadXML));

	if(xml->Load(fn)==false)
	{
		box=new kGUIMsgBoxReq(MSGBOX_OK,true,"Error: cannot opening file '%s'!",fn);
		delete xml;
		delete m_busy;
		return;
	}
	delete m_busy;

	/* is this a waypoint gpx file or a tracklog gpx file */

	numwp=0;
	numtrk=0;
	xroot=xml->GetRootItem()->Locate("gpx");
	if(xroot)
	{
		unsigned int i;

		for(i=0;i<xroot->GetNumChildren();++i)
		{
			xi=xroot->GetChild(i);
			if(!strcmp(xi->GetName(),"wpt"))
				++numwp;
			else if(!strcmp(xi->GetName(),"trk"))
				++numtrk;
		}
	}

	m_tempxml=xml;
	if(numwp)
	{
		LoadWPTSettings *ls;
		ls=new LoadWPTSettings(xml,fn,defdb);
		if(numtrk)
			ls->SetDoneCallBack(this,CALLBACKNAME(SelectLoadTracks));
	}
	else if(numtrk)
	{
		/* load track settings */
		SelectTracks *st;

		st=new SelectTracks(&m_temphash,this,CALLBACKNAME(LoadTracks),xml);
	}
	else
	{
		box=new kGUIMsgBoxReq(MSGBOX_OK,false,"Error: GPX file doesn't contain any waypoints or tracks!");
		delete xml;
	}
}

void GPX::SelectLoadTracks(void)
{
	SelectTracks *st;

	st=new SelectTracks(&m_temphash,this,CALLBACKNAME(LoadTracks),m_tempxml);
}

void GPX::LoadTracks(int pressed)
{
	if(pressed==MSGBOX_OK)
	{
		kGUIXMLItem *xroot;
		HashEntry *he;
		unsigned int i,num;

		xroot=m_tempxml->GetRootItem()->Locate("gpx");
		if(xroot)
		{
			/* iterate through the hash table and load all the selected tracks */
			num=m_temphash.GetNum();
			he=m_temphash.GetFirst();
			for(i=0;i<num;++i)
			{
				m_tracks.LoadTrack(he->m_string,*((kGUIXMLItem **)(he->m_data)));
				he=he->GetNext();
			}
		}
	}
	delete m_tempxml;
	m_tempxml=0;
}

/* only set user flag */
void GPX::DoLoadUserFound(kGUIXML *xml,int userslot)
{
	unsigned int i;
	int notdb,found,notfound;
	kGUIXMLItem *xmlitem;
	kGUIXMLItem *wp;
	GPXRow *row;
	kGUIBusy *busy;
	kGUIMsgBoxReq *box;

	busy=new kGUIBusy(kGUI::GetScreenWidth()>>1);
	busy->GetTitle()->SetString("Scanning GPX File");

	notdb=0;
	found=0;
	notfound=0;

	xmlitem=xml->GetRootItem()->Locate("gpx");
	if(xmlitem)
	{
		busy->SetMax(xmlitem->GetNumChildren());

		/* set all usercol to false */
		for(i=0;i<m_numwpts;++i)
		{
			row=m_wptlist.GetEntry(i);
			row->SetUser(userslot,false);
		}

		for(i=0;i<xmlitem->GetNumChildren();++i)
		{
			busy->SetCur(i);
			wp=xmlitem->GetChild(i);
			if(!strcmp(wp->GetName(),"wpt"))
			{
				/* check to see if this wptname already exists in the table */
				row=Locate(wp->Locate("name")->GetValue());

				if(row)
				{
					if(!strcmp(wp->Locate("sym")->GetValueString(),"Geocache Found"))
					{
						row->SetUser(userslot,true);
						++found;
					}
					else 
						++notfound;
				}
				else
					++notdb;
			}
		}
	}
	delete busy;
	BSPDirty();
	UpdateDBList();
	UpdateWPRender();
	ReFilter();
	box=new kGUIMsgBoxReq(MSGBOX_OK,true,"%d Found of %d Waypoints loaded, %d not in Database",found,found+notfound,notdb);
}

void GPX::DoLoad(kGUIXML *xml,const char *dbname,const char *filename)
{
	unsigned int i;
	int numwp,numwploaded;
	kGUIXMLItem *xmlitem;
	kGUIXMLItem *wp;
	kGUIString gendate;
	GPXRow *row;
	GPXRow *rrow;
	kGUIBusy *busy;
	kGUIMsgBoxReq *box;
	bool older;

	SetDefDB(dbname);	/* default database to use */

	busy=new kGUIBusy(kGUI::GetScreenWidth()>>1);
	busy->GetTitle()->SetString("Importing GPX File");

	m_badtype=false;
	numwp=0;
	numwploaded=0;
	xmlitem=xml->GetRootItem()->Locate("gpx");
	if(xmlitem)
	{
		busy->SetMax(xmlitem->GetNumChildren());
		/* date file was generated */
		if(xmlitem->Locate("time"))
		{
			gendate.SetString(xmlitem->Locate("time")->GetValue());
			if(gendate.GetLen()>10)
				gendate.Clip(10);
		}
		for(i=0;i<xmlitem->GetNumChildren();++i)
		{
			busy->SetCur(i);
			wp=xmlitem->GetChild(i);
			if(!strcmp(wp->GetName(),"wpt"))
			{
				older=false;
				++numwp;
				/* check to see if this wptname already exists in the table */
				row=Locate(wp->Locate("name")->GetValue());
				if(row)
				{
					/* is this one older than the one we already have? */
					if(strcmp(gendate.GetString(),row->m_gendate.GetString())<0)
						older=true;

					if(older==false)
					{
						row->Load(&gendate,wp);
						++numwploaded;
					}
				}
				else
				{
					//todo, check to see if this matches a child waypoint????
					row=new GPXRow(&gendate,wp);
					m_wptlist.SetEntry(m_numwpts++,row);
					++numwploaded;
				}

				/* if this id in the route table, then update it */
				if(older==false)
				{
					rrow=m_routes.InTable(row->m_wptname.GetString());
					if(rrow)
						rrow->Load(&gendate,wp);
				}
			}
		}
	}

	/* is there a child waypoint list too? */

	{
		kGUIString fn;
		kGUIString pname;
		kGUIXML xml2;

		fn.SetString(filename);
		fn.Replace(".gpx","-wpts.gpx");

		if(xml2.Load(fn.GetString())==true)
		{
			xmlitem=xml2.GetRootItem()->Locate("gpx");
			if(xmlitem)
			{
				busy->GetTitle()->SetString("Loading Additional Waypoints");
				busy->SetMax(xmlitem->GetNumChildren());
				for(i=0;i<xmlitem->GetNumChildren();++i)
				{
					busy->SetCur(i);
					wp=xmlitem->GetChild(i);
					if(!strcmp(wp->GetName(),"wpt"))
					{
						pname.SetString(wp->Locate("name")->GetValue());
						pname.SetChar(0,'G');
						pname.SetChar(1,'C');
						/* check to see if this wptname already exists in the table */
						row=Locate(&pname);
						if(row)
						{
							row->AddChild(	wp->Locate("name")->GetValue(),
											wp->Locate("sym")->GetValue(),
											wp->Locate("desc")->GetValue(),
											atof(wp->Locate("lon")->GetValueString()),
											atof(wp->Locate("lat")->GetValueString()));
						}
					}
				}
			}
		}
	}
	delete busy;
	CalcDists();
	BSPDirty();
	UpdateDBList();
	UpdateWPRender();
	ReFilter();

	if(m_badtype)
	{
		box=new kGUIMsgBoxReq(MSGBOX_OK,true,"A Cache with the unknown type '%s' was loaded.\n This type is not known by GPSTurbo so you probably need to get a newer version.",m_badtypename.GetString());
	}
	else
	{
		if(numwploaded==numwp)
			box=new kGUIMsgBoxReq(MSGBOX_OK,true,"%d Waypoints loaded",numwploaded);
		else
			box=new kGUIMsgBoxReq(MSGBOX_OK,true,"%d of %d Waypoints loaded ( older Waypoints ignored )",numwploaded,numwp);
	}
}

/* save gpx file */

void GPX::DoSave(kGUIFileReq *result,int pressed)
{
	SelectTracks *st;
	const char *fn;

	if(pressed==MSGBOX_OK)
	{
		fn=result->GetFilename();
		/* save default path for next time */
		m_defpath.SetString(result->GetPath());

		m_tempfilename.SetString(fn);	/* save filename */
		if(m_tracks.GetNumTracks())
			st=new SelectTracks(&m_temphash,this,CALLBACKNAME(DoSave2),0);
		else
			DoSave2(MSGBOX_OK);
	}
}

void GPX::DoSaveAs(kGUIFileReq *result,int pressed)
{
	LoadAs *la;

	if(pressed==MSGBOX_OK)
	{
		/* save default path for next time */
		m_defpath.SetString(result->GetPath());
		la = new LoadAs(LOADAS_SAVE,result->GetFilename());
	}
}

void GPX::DoSaveAs2(const char *fn,const char *ft)
{
	SelectTracks *st;

	m_tempfilename.SetString(fn);	/* save filename */
	m_tempfiletype.SetString(ft);	/* type */
	if(m_tracks.GetNumTracks())
		st=new SelectTracks(&m_temphash,this,CALLBACKNAME(DoSaveAs3),0);
	else
		DoSaveAs3(MSGBOX_OK);
}

void GPX::DoSaveAs3(int pressed)
{
	if(pressed==MSGBOX_OK)
	{
		BabelGlue babel;

		// first save as gpx, then convert using gpsbabel
		SaveXML("babel.gpx");

		babel.SetInput("gpx","babel.gpx");
		babel.AddType(BABELTYPE_WAYPOINTS);
		babel.AddType(BABELTYPE_TRACKS);
		babel.SetOutput(m_tempfiletype.GetString(),m_tempfilename.GetString());
		babel.Call(true,0);
		kGUI::FileDelete("babel.gpx");
	}
}

void GPX::DoSave2(int pressed)
{
	if(pressed==MSGBOX_OK)
		SaveXML(m_tempfilename.GetString());
}

void GPX::SaveXML(const char *fn)
{
	unsigned int i;
	kGUIXML xml;
	kGUIXMLItem *root;
	kGUIMsgBoxReq *box;

	root=xml.GetRootItem()->AddChild("gpx");
	root->AddParm("xmlns:xsd","http://www.w3.org/2001/XMLSchema");
	root->AddParm("xmlns:xsi","http://www.w3.org/2001/XMLSchema-instance");
	root->AddParm("version","1.0");
	root->AddParm("creator","GPSTurbo");
	root->AddParm("xsi:schemaLocation","http://www.topografix.com/GPX/1/0 http://www.topografix.com/GPX/1/0/gpx.xsd http://www.groundspeak.com/cache/1/0 http://www.groundspeak.com/cache/1/0/cache.xsd");
	root->AddParm("xmlns","http://www.topografix.com/GPX/1/0");
	
	for(i=0;i<m_fwt->GetNumChildren();++i)
	{
		GPXRow *row;

		row=static_cast<GPXRow *>(m_fwt->GetChild(i));
		row->Save(root->AddChild("wpt"),true);
	}

	/* only save tracks that are in the hash table */
	m_tracks.SavePrefs(root,&m_temphash);

	if(xml.Save(fn)==false)
		box=new kGUIMsgBoxReq(MSGBOX_OK,false,"Error saving file!");

}

/* save selected area of the map as a KML/JPG */

void GPX::DoSaveMapKML(kGUIFileReq *result,int pressed)
{
	int x1,x2,y1,y2,temp,shapew,shapeh,centerx,centery;
	int lx,ty;
	int shapenum;
	kGUIDrawSurface tempsurface;
	kGUIDrawSurface *savesurface;
	kGUIImage image;
	kGUIMsgBoxReq *box;
	DataHandle outdh;
	kGUIXML xml;
	kGUIXMLItem *root;
	kGUIXMLItem *gitem;
	kGUIXMLItem *xitem;
	kGUIString tempstring;
	kGUIString fn;
	kGUIString path;
	kGUIString shortfn;
	GPXCoord tl;
	GPXCoord br;

#if 0
<kml xmlns=....>
<GroundOverlay>
	<Icon>
		<href>files/CentralParkRunningMap.jpg</href>
		<DrawOrder>0</DrawOrder>
	</Icon>
	<LatLonBox>
		<north>40.80531332719471</north>
		<south>40.7601597052842</south>
		<east>-73.95274155944784</east>
		<west>-73.97364085195952</west>
		<rotation>-28.77996170942711</rotation>
	</LatLonBox>
</GroundOverlay>
</kml>
	
<?xml version="1.0" encoding="UTF-8"?>
<kml xmlns="http://www.opengis.net/kml/2.2">
  <Folder>
    <name>Ground Overlays</name>
    <description>Examples of ground overlays</description>
    <GroundOverlay>
      <name>Large-scale overlay on terrain</name>
      <description>Overlay shows Mount Etna erupting 
          on July 13th, 2001.</description>
      <Icon>
        <href>http://code.google.com/apis/kml/documentation/etna.jpg</href>
      </Icon>
      <LatLonBox>
        <north>37.91904192681665</north>
        <south>37.46543388598137</south>
        <east>15.35832653742206</east>
        <west>14.60128369746704</west>
        <rotation>-0.1556640799496235</rotation>
      </LatLonBox>
    </GroundOverlay>
  </Folder>
</kml>
#endif

	if(pressed==MSGBOX_OK)
	{
		if(strlen(result->GetFilename()))
		{
			m_curmap->ToMap(&m_mapselul,&x1,&y1);
			m_curmap->ToMap(&m_mapsellr,&x2,&y2);

			/* force x1<x2, y1<y2 */
			if(x1>x2)
			{
				temp=x1;
				x1=x2;
				x2=temp;
			}
			if(y1>y2)
			{
				temp=y1;
				y1=y2;
				y2=temp;
			}

			/* max 1024 x 1024 */
			shapenum=0;
			root=xml.GetRootItem();
			root=root->AddChild("kml");
			root->AddParm("xmlns","http://www.opengis.net/kml/2.2");

			for(lx=x1;lx<x2;lx+=1024)
			{
				for(ty=y1;ty<y2;ty+=1024)
				{
					shapew=abs(x2-x1);
					shapeh=abs(y2-y1);

					shapew=MIN(x2-lx,1024);
					shapeh=MIN(y2-ty,1024);

					centerx=lx+(shapew/2);
					centery=ty+(shapeh/2);

					tempsurface.Init(shapew,shapeh);

					kGUI::PushClip();
					savesurface=kGUI::GetCurrentSurface();
					kGUI::SetCurrentSurface(&tempsurface);
					kGUI::PushClip();
					kGUI::ResetClip();	/* set clip to full surface on stack */

					kGUI::SetCurrentSurface(&tempsurface);
					
					DrawGrid(shapew,shapeh,centerx,centery,0,(double)1.0f,1);

					kGUI::PopClip();
					/* put surface back */
					kGUI::SetCurrentSurface(savesurface);

					/* make image point to the temp surface we drew the map info */
					image.SetMemImage(0,GUISHAPE_SURFACE,shapew,shapeh,tempsurface.GetBPP(),(const unsigned char *)tempsurface.GetSurfacePtr(0,0));

					/* save the image */
					tempstring.SetString(result->GetFilename());
					tempstring.Clip(".");
					fn.Sprintf("%s%d.jpg",tempstring.GetString(),shapenum);
					outdh.SetFilename(fn.GetString());
					if(image.SaveJPGImage(&outdh,100)==false)
					{
						box=new kGUIMsgBoxReq(MSGBOX_OK,false,"Error saving file!");
						return;
					}
					kGUI::SplitFilename(&fn,&path,&shortfn);
					gitem=root->AddChild("GroundOverlay");
					xitem=gitem->AddChild("Icon");
					xitem->AddChild("href",shortfn.GetString());
					xitem->AddChild("draworder","0");

					m_curmap->FromMap(lx,ty,&tl);
					m_curmap->FromMap(lx+shapew,ty+shapeh,&br);

					xitem=gitem->AddChild("LatLonBox");
					xitem->AddChild("north",tl.GetLat());
					xitem->AddChild("south",br.GetLat());
					xitem->AddChild("east",br.GetLon());
					xitem->AddChild("west",tl.GetLon());
					xitem->AddChild("rotation",0.0f);
					++shapenum;
				}
			}

			/* save default path for next time */
			m_defpath.SetString(result->GetPath());
		}
	}
	tempstring.SetString(result->GetFilename());
	tempstring.Clip(".");
	fn.Sprintf("%s.kml",tempstring.GetString());
	xml.Save(fn.GetString());
//	box=new kGUIMsgBoxReq(MSGBOX_OK,true,"Jpeg file: width=%d, height=%d '%s' saved!",shapew,shapeh,result->GetFilename());
}

/* save selected area of the map as a JPG */

void GPX::DoSaveMapShape(kGUIFileReq *result,int pressed)
{
	int x1,x2,y1,y2,shapew,shapeh,centerx,centery;
	kGUIDrawSurface tempsurface;
	kGUIDrawSurface *savesurface;
	kGUIImage image;
	kGUIMsgBoxReq *box;
	DataHandle outdh;

	if(pressed==MSGBOX_OK)
	{
		if(strlen(result->GetFilename()))
		{
			m_curmap->ToMap(&m_mapselul,&x1,&y1);
			m_curmap->ToMap(&m_mapsellr,&x2,&y2);

			shapew=abs(x2-x1);
			shapeh=abs(y2-y1);
			centerx=(x1+x2)/2;
			centery=(y1+y2)/2;

			tempsurface.Init(shapew,shapeh);

			kGUI::PushClip();
			savesurface=kGUI::GetCurrentSurface();
			kGUI::SetCurrentSurface(&tempsurface);
			kGUI::PushClip();
			kGUI::ResetClip();	/* set clip to full surface on stack */

			kGUI::SetCurrentSurface(&tempsurface);
			
			DrawGrid(shapew,shapeh,centerx,centery,0,(double)1.0f,1);

			kGUI::PopClip();
			/* put surface back */
			kGUI::SetCurrentSurface(savesurface);

			/* make image point to the temp surface we drew the map info */
			image.SetMemImage(0,GUISHAPE_SURFACE,shapew,shapeh,tempsurface.GetBPP(),(const unsigned char *)tempsurface.GetSurfacePtr(0,0));

			/* save the image */
			outdh.SetFilename(result->GetFilename());
			if(image.SaveJPGImage(&outdh,100)==false)
				box=new kGUIMsgBoxReq(MSGBOX_OK,false,"Error saving file!");
			else
				box=new kGUIMsgBoxReq(MSGBOX_OK,true,"Jpeg file: width=%d, height=%d '%s' saved!",shapew,shapeh,result->GetFilename());

			/* save default path for next time */
			m_defpath.SetString(result->GetPath());
		}
	}
}

GPXRow *GPX::Locate(kGUIString *wptname)
{
	unsigned int e;
	GPXRow *row;

	for(e=0;e<m_numwpts;++e)
	{
		row=m_wptlist.GetEntry(e);
		if(!strcmp(wptname->GetString(),row->m_wptname.GetString()))
			return(row);
	}
	return(0);
}

GPXRow *GPX::InTable(kGUITableObj *table,const char *wptname)
{
	int e,nr;
	kGUIObj *obj;
	GPXRow *row;

	nr=table->GetNumChildren(0);
	for(e=0;e<nr;++e)
	{
		obj=table->GetChild(e);
		row=static_cast<GPXRow *>(obj);
		if(!strcmp(wptname,row->m_wptname.GetString()))
			return(row);
	}
	return(0);
}

GPX::~GPX()
{
	unsigned int x;

	if(m_realtime->IsTracking())
		m_realtime->StopTracking();
	delete m_realtime;
	kGUI::SetInputCallback(0,0);

	for(x=0;x<m_nummaps;++x)
		delete m_mapinfo.GetEntry(x);

	for(x=0;x<SHAPE_NUMSHAPES;++x)
		m_shapes[x].Purge();
	m_logo.Purge();

	m_labelcolourtable.DeleteChildren();
	m_mapdirstable.DeleteChildren();
	m_macrocontrols.DeleteChildren();

	m_routes.Purge();
	m_tracks.Purge();
	m_lines.Purge();
	m_notes.Purge();
	m_filters.Purge();
	m_gpsr.Purge();
	m_stickers.Purge();
	m_download.Purge();

	for(x=0;x<m_numwpts;++x)
		delete m_wptlist.GetEntry(x);
	m_numwpts=0;

	delete m_curmap;
	m_zin.Purge();
	m_zout.Purge();

	for(x=0;x<MAXUSERTICKS;++x)
		delete m_userhints.GetEntry(x);
}

int GPX::GetIndexz(const char *t,int num,const char **strings,int nomatchval)
{
	int i;

	for(i=0;i<num;++i)
	{
		if(!stricmp(t,strings[i]))
			return(i);
	}
	return(nomatchval);
}


int GPX::GetIndex(const char *t,int num,const char **strings)
{
	int index;
	
	index=GetIndexz(t,num,strings,-1);
	if(index==-1)
		passert(false,"string '%s' not in list!",t);
	return(index);
}



void GPX::TableFontSizeChanged(void)
{
	unsigned int i;
	int size;
	GPXRow *row;
	kGUIText *t;

	size=m_tablefontsize.GetInt();
	if(size<5)
		size=5;
	else if(size>50)
		size=50;

	for(i=0;i<GPXCOL_NUMCOLUMNS;++i)
	{
		t=m_fwt->GetColHeaderTextPtr(i);
		t->SetFontSize(size);
		t=m_routes.GetTable()->GetColHeaderTextPtr(i);
		t->SetFontSize(size);
	}

#if USESHAREDCOMBOS
	m_sharedcontainer.SetFontSize(size);
	for(i=0;i<m_sharedcontainer.GetNumEntries();++i)
		m_sharedcontainer.GetEntryTextPtr(i)->SetFontSize(size);
#endif

	for(i=0;i<m_numwpts;++i)
	{
		row=m_wptlist.GetEntry(i);
		row->UpdateFontSize();
	}
}

void GPX::LabelFontSizeChanged(void)
{
	int fs=m_labelfontsize.GetInt();
	unsigned int e;
	int c,nc;
	GPXRow *row;

	for(e=0;e<m_numwpts;++e)
	{
		row=m_wptlist.GetEntry(e);
		row->m_label.SetFontSize(fs);
		nc=row->m_numchildren;
		for(c=0;c<nc;++c)
			row->m_children.GetEntry(c)->m_label.SetFontSize(fs);
	}

	/* since the labels on the filter list has changed, the map needs to be updated */
	MapDirty();
	BSPDirty();
}

void GPX::CompileLabelDefinition(kGUIString *in,int *numptr,Array<WNENT_DEF>*arrayptr)
{
	/* parse the user input string and calculate the */
	/* offsets to the column names */
	int i,i2,l,num;
	WNENT_DEF we;
	char c;
	kGUIString fn;

	l=in->GetLen();
	num=0;
	i=0;
	while(i<l)
	{
		if(in->GetChar(i)=='.')
		{
			++i;
			fn.Clear();
			while(i<l)
			{
				c=in->GetChar(i);
				if((c>='A' && c<='Z') || (c>='a' && c<='z') || (c>='0' && c<='9'))
				{
					fn.Append(c);
					++i;
				}
				else
					break;
			}

			we.isfield=true;
			we.index=GetColNumz(fn.GetString());
			we.len=0;	/* not used */
			arrayptr->SetEntry(num++,we);
		}
		else
		{
			i2=i;
			while(i<l)
			{
				if(in->GetChar(i)=='.')
					break;
				++i;
			}
			/* from i to i2 is static characters */
			we.isfield=false;
			we.index=i2;
			we.len=i-i2;
			arrayptr->SetEntry(num++,we);
		}
	}
	*(numptr)=num;
}

/* generate a expanded string from a compile list and a particular row */
void GPX::ExpandLabel(GPXRow *row,kGUIString *in,kGUIString *def,int numptr,Array<WNENT_DEF>*arrayptr)
{
	int i;
	WNENT_DEF we;

	in->Clear();
	/* default the encoding to the waypoint name encoding */
	in->SetEncoding(row->m_wptname.GetEncoding());

	for(i=0;i<numptr;++i)
	{
		we=arrayptr->GetEntry(i);

		if(we.isfield==true)
		{
			switch(we.index)
			{
			case GPXCOL_NA:
			break;
			case GPXCOL_FOUND:
			break;
			case GPXCOL_TYPE:
				in->Append(cachetypenames[row->GetType()]);
			break;
			case GPXCOL_USER1:
			break;
			case GPXCOL_USER2:
			break;
			case GPXCOL_USER3:
			break;
			case GPXCOL_USER4:
			break;
			case GPXCOL_USER5:
			break;
			case GPXCOL_USER6:
			break;
			case GPXCOL_USER7:
			break;
			case GPXCOL_USER8:
			break;
			case GPXCOL_NAME:
				in->Append(row->m_name.GetString());
			break;
			case GPXCOL_WPTNAME:
				in->Append(row->m_wptname.GetString());
			break;
			case GPXCOL_LOG:
				in->Append(row->m_log.GetString());
			break;
			case GPXCOL_CONTAINER:
				in->Append(containernames[row->m_container.GetSelection()]);				
			break;
			case GPXCOL_DIFFICULTY:
				in->Append(row->m_difficulty.GetString());
			break;
			case GPXCOL_TERRAIN:
				in->Append(row->m_terrain.GetString());
			break;
			case GPXCOL_DIST:
			break;
			case GPXCOL_CORRECTED:
			break;
			case GPXCOL_LAT:
				in->Append(row->m_lat.GetString());
			break;
			case GPXCOL_LON:
				in->Append(row->m_lon.GetString());
			break;
			case GPXCOL_HINT:
				in->Append(row->m_hint.GetString());
			break;
			case GPXCOL_USERNOTES:
				in->Append(row->m_usernotes.GetString());
			break;
			case GPXCOL_OWNER:
				in->Append(row->m_owner.GetString());
			break;
			case GPXCOL_STATE:
				in->Append(row->m_state.GetString());
			break;
			case GPXCOL_COUNTRY:
				in->Append(row->m_country.GetString());
			break;
			case GPXCOL_GENDATE:
				in->Append(row->m_gendate.GetString());
			break;
			case -1:
			break;
			}
		}
		else
			in->Append(def->GetString()+we.index,we.len);
	}
}


/* toggle between name and "name, wptname" */

void GPX::LabelNamesChanged(void)
{
	unsigned int e;
	GPXRow *row;

	CompileLabelDefinition(&m_wptname,&m_wptnamenumentries,&m_wptnameentries);
	for(e=0;e<m_numwpts;++e)
	{
		row=m_wptlist.GetEntry(e);
		row->UpdateLabelName();
	}

	/* since the labels on the filter list has changed, the map needs to be updated */
	MapDirty();
}


void GPX::SetScrollCenter(GPXCoord *c)
{
	int sx,sy;
	m_curmap->ToMap(c,&sx,&sy);
	m_grid.SetScrollCenter(sx,sy);
}

void GPX::ZoomGoto(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_AFTERUPDATE)
	{
		m_zoom=m_zoomgoto.GetSelection();
		ZoomChanged();
	}
}

void GPX::ClickZoomIn(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
		ZoomIn();
}

void GPX::ZoomIn(void)
{
	if((m_zoom+1)<m_curmap->GetMaxZoom())
	{
		++m_zoom;
		ZoomChanged();
	}
}


void GPX::ClickZoomOut(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
		ZoomOut();
}

void GPX::ZoomOut(void)
{
	if(m_zoom>m_curmap->GetMinZoom())
	{
		--m_zoom;
		ZoomChanged();
	}
}

void GPX::ZoomChanged(void)
{
	int tx,ty;
	GPXCoord c;

	m_grid.GetScrollCenter(&tx,&ty);
	m_curmap->FromMap(tx,ty,&c);
	UpdateZoomButtons();

	m_curmap->SetZoom(m_zoom);
	m_grid.SetBounds(0,0,m_curmap->GetWidth(),m_curmap->GetHeight());

	m_curmap->ToMap(&c,&tx,&ty);
	m_grid.SetScrollCenter(tx,ty);
	BSPDirty();	/* since projection has changed */
	MapDirty();
}

#define LEGWIDTH 161

void GPX::UpdateZoomButtons(void)
{
	int z,maxz,minz;
	kGUIString zs;
	int cx,cy;
	int savezoom=m_zoom;
	double dist,ratio,eval1,eval2;
	GPXCoord pos1;
	GPXCoord pos2;
	int savesx,savesy;
	GPXCoord c;

	GetDistInfo(m_disttype.GetSelection(),0,&ratio,&eval1,&eval2);

	m_zoomout.SetEnabled(m_zoom>m_curmap->GetMinZoom());
	m_zoomin.SetEnabled(m_zoom<(m_curmap->GetMaxZoom()-1));

	minz=m_curmap->GetMinZoom();
	maxz=m_curmap->GetMaxZoom();

	m_zoomgoto.SetNumEntries(maxz-minz);

	/* get and save current scroll values for center of map */
	m_grid.GetScrollCenter(&savesx,&savesy);
	/* convert to a lat/lon point */
	m_curmap->FromMap(savesx,savesy,&c);

	for(z=minz;z<maxz;++z)
	{
		/* calc scale for current map using each zoom level */
		m_zoom=z;
		m_curmap->SetZoom(z);
		/* center map on lat/lon */
		m_curmap->ToMap(&c,&cx,&cy);
		m_grid.SetScrollCenter(cx,cy);

		/* calc distance from screen pixels in LEGWIDTH */
		m_curmap->FromMap(cx-(LEGWIDTH/2),cy,&pos1);
		m_curmap->FromMap(cx+LEGWIDTH-(LEGWIDTH/2),cy,&pos2);
		dist=pos1.Dist(&pos2)*ratio;

		zs.Sprintf("%d - %.2f %s",z,dist,m_disttype.GetSelectionString());

		m_zoomgoto.SetEntry(z-minz,zs.GetString(),z);
	}
	m_zoom=savezoom;
	m_curmap->SetZoom(savezoom);
	m_grid.SetScrollCenter(savesx,savesy);

	m_zoomgoto.SetSelection(m_zoom);
}

/* coords are in map space */
int GPX::MapEntryUnderMouse(int mx,int my)
{
	int e,nr;
	int sx,sy,sx2,sy2;
	GPXRow *row;

	if(m_onmap==true)
	{
		nr=m_fwt->GetNumChildren(0);
		for(e=0;e<nr;++e)
		{
			row=static_cast<GPXRow *>(m_fwt->GetChild(e));

			m_curmap->ToMap(&(row->m_llcoord),&sx,&sy);
			sx+=16;	/* name is right of type shape */
			sx2=sx+row->m_label.m_lw;
			sy2=sy+row->m_label.m_lh;

			if((mx>=sx) && (mx<=sx2) && (my>=sy) && (my<=sy2))
				return(e);
		}
	}
	return(-1);
}

/* return the row number for this label */

int GPX::GetRow(GPXLabel *l)
{
	unsigned int e;
	GPXRow *row;

	for(e=0;e<m_fwt->GetNumChildren();++e)
	{
        row=static_cast<GPXRow *>(m_fwt->GetChild(e));
		if(&(row->m_label)==l)
			return(e);
	}
	return(-1);
}

void GPX::OverMap(int mx,int my)
{
	int i;
	kGUIString ll;
	int sx,sy;
	int numunder=0;	/* num labels under the mouse */
	GPXCoord upos;	
	int scroll=kGUI::GetMouseWheelDelta();

	m_grid.GetScrollCorner(&sx,&sy);
	mx+=sx;
	my+=sy;

	m_curmap->FromMap(mx,my,&upos);

	/* only set if it is different as setting triggers screen draws */
	if( (upos.GetLat()!=m_upos.GetLat()) || (upos.GetLon()!=m_upos.GetLon()))
	{
		m_upos.Set(&upos);
		m_upos.Output(&m_ulat,&m_ulon);
	}

	if(scroll)
	{
		if(scroll>0)
			ZoomIn();
		else
			ZoomOut();
		kGUI::ClearMouseWheelDelta();
		return;
	}

	if((kGUI::GetMouseDoubleClickLeft()==true) || (kGUI::GetMouseClickRight()==true) )
	{
		/* only look if labels are drawn at this zoom level */
		if(m_onmap==true)
		{
			kGUICorners c;
			kGUIBSPRectEntry *bspe;

			/* final all labels that overlap the mouse pointer */
			
			c.lx=mx-2;
			c.rx=mx+2;
			c.ty=my-2;
			c.by=my+2;
			m_bsp.Select(&c);
			++m_wasdrawn;
			do
			{
				bspe=m_bsp.GetEntry();
				if(bspe)
				{
					GPXLabel *label=static_cast<GPXLabel *>(bspe);
					
					//DebugPrint("%s\n",label->GetString());

					if(label->m_draw==true)
					{
						if(label->m_wasdrawn!=m_wasdrawn)
						{
							if(kGUI::Overlap(&c,&(label->m_c)))
							{
								m_llp[numunder++]=label;
								label->m_wasdrawn=m_wasdrawn;
							}
						}
					}
				}
			}while(bspe && (numunder<MAXLL));
		}

		if(numunder==1 && kGUI::GetMouseDoubleClickLeft()==true)
		{
			int e;
			
			e=GetRow(m_llp[0]);
			if(e>=0)
				m_fwt->GotoRow(e);
		}
		else if(kGUI::GetMouseClickRight()==true)
		{
			if(!numunder)
			{
				m_currow=0;
				gpx->InitPopMenu(&m_rcmenu,sizeof(mapmenunums2)/sizeof(int),mapmenunums2);
				m_rcmenu.Activate(kGUI::GetMouseX(),kGUI::GetMouseY());
			}
			else if(numunder==1)
				DoRCLabel(m_llp[0]);
			else
			{
				/* bring up select point menu */
				
				m_wpmenu.SetNumEntries(numunder);
				for(i=0;i<numunder;++i)
					m_wpmenu.SetEntry(i,m_llp[i]->GetString(),i);
				m_wpmenu.Activate(kGUI::GetMouseX(),kGUI::GetMouseY());
			}
		}
	}
}

void GPX::DoWPMenu(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_SELECTED)
	{
		int s=m_wpmenu.GetSelection();

		if(s>=0)
			DoRCLabel(m_llp[s]);
	}
}

void GPX::DoRCLabel(GPXLabel *label)
{
	int e;
	kGUIObj *obj;
	kGUIString addtorname;
	kGUIString removefromrname;
	kGUIString addneartorname;
	kGUIString toggle;

	e=GetRow(label);
	if(e<0)
		return;		/* must be a child point */

	obj=m_fwt->GetChild(e);
	m_currowindex=e;
	m_currow=static_cast<GPXRow *>(obj);

	/* is this wpt alreaday on the route? */

	InitPopMenu(&m_rcmenu,sizeof(mapmenunums)/sizeof(int),mapmenunums);
	m_rcmenu.Activate(kGUI::GetMouseX(),kGUI::GetMouseY());
}

void GPX::BuildPage(DataHandle *dh,GPXRow *page,bool head,bool tail)
{
	int i,numlogs;
	int logtype;
	kGUIString date;
	kGUIString type;
	kGUIString icon;

	icon.SetString(shapenames[page->GetType()]);

	/* force fields to UTF-8 if not already */
	page->m_wptname.ChangeEncoding(ENCODING_UTF8);
	page->m_name.ChangeEncoding(ENCODING_UTF8);
	page->m_shortdesc.ChangeEncoding(ENCODING_UTF8);
	page->m_desc.ChangeEncoding(ENCODING_UTF8);

	if(head)
	{
		dh->Sprintf("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">");
		dh->Sprintf("<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n");
		dh->Sprintf("<title>%s - %s</title></head><body bgcolor=\"#ffffff\" text=\"#000000\">",page->m_wptname.GetString(),page->m_name.GetString());
	}
	dh->Sprintf("<table width=\"95%%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\"><tr><td bgcolor=\"#ffffff\">");
	{
		int lr,lg,lb;
		int tr,tg,tb;

		DrawColorToRGB(page->m_label.m_colour,lr,lg,lb);
		DrawColorToRGB(page->m_label.m_tcolour,tr,tg,tb);
		dh->Sprintf("<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0><TR BGCOLOR=\"#%02x%02x%02x\"><TD><FONT COLOR=\"#%02x%02x%02x\">",lr,lg,lb,tr,tg,tb);
	}

	dh->Sprintf("<h2><img src=\"geoart/%s\"><b>%s - %s</b></h2>",icon.GetString(),page->m_wptname.GetString(),page->m_name.GetString());
	dh->Sprintf("</FONT></TD></TR></TABLE>");

	dh->Sprintf("<B>Page Generated on:</B> %s, <B>View Current:</B> <a href=\"%s\">Online Webpage</a><br>",page->m_gendate.GetString(),page->m_url.GetString());
	if(!(page->m_corrected.GetSelected()))
		dh->Sprintf("<B>%s %s</B> (WGS84)<br>",page->m_lon.GetString(),page->m_lat.GetString());
	else
	{
		GPXChild *orig=page->LocateChild("Orig");
		kGUIString olat;
		kGUIString olon;

		orig->GetPosPtr()->Output(&olat,&olon);
		dh->Sprintf("<B>%s %s</B> <FONT COLOR=\"red\">(Corrected)</FONT> (WGS84) <B>Original:</B> %s %s<br>",page->m_lon.GetString(),page->m_lat.GetString(),olat.GetString(),olon.GetString());
	}
	dh->Sprintf("<B>Size:</B> %s <B>Hidden on:</B> %s <B>By:</B> %s<br>",page->m_container.GetSelectionString(),page->m_placeddate.GetString(),page->m_owner.GetString());
	dh->Sprintf("<B>In:</B> %s, %s ",page->m_state.GetString(),page->m_country.GetString());
	dh->Sprintf("<B>Difficulty:</B> %s, <B>Terrain:</B> %s<br>",page->m_difficulty.GetString(),page->m_terrain.GetString());
	dh->Sprintf("<br><hr>");
	dh->Sprintf("%s<br>",page->m_shortdesc.GetString());
	dh->Sprintf("%s<br>",page->m_desc.GetString());
	dh->Sprintf("<br><hr>");
	dh->Sprintf("<B>Hint:</B> %s<br>",page->m_hint.GetString());
	dh->Sprintf("<br><hr>");

	dh->Sprintf("</td></tr>");
	numlogs=page->m_logs.GetNumChildren();
	for(i=0;i<numlogs;++i)
	{
		kGUIXMLItem *log=page->m_logs.GetChild(i);

		dh->Sprintf("<tr><td bgcolor=\"#%s\">",(i&1)?"e0eaef":"f1f5f8");
		date.SetString(log->Locate("groundspeak:date")->GetValueString());
		if(date.GetLen()>10)
			date.Clip(10);
		type.SetString(log->Locate("groundspeak:type")->GetValueString());
		logtype=GetIndex(log->Locate("groundspeak:type")->GetValueString(),sizeof(logtypes)/sizeof(char *),logtypes);			
		dh->Sprintf("<img src=\"geoart/%s\">",logtypepics[logtype]);			
		dh->Sprintf(" %s by <B>%s</B> <br>",date.GetString(),log->Locate("groundspeak:finder")->GetValueString());
		dh->Sprintf("%s<br>",log->Locate("groundspeak:text")->GetValueString());
		dh->Sprintf("<br><hr>");
		dh->Sprintf("</td></tr>");
	}
	dh->Sprintf("</table>");

	if(tail)
		dh->Sprintf("</body></html>");
}

void GPX::DoMapMenu(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_SELECTED)
	{
		switch(m_rcmenu.GetSelection())
		{
		case MAPMENU_SELECT:
			m_fwt->GotoRow(m_currowindex);
		break;
		case MAPMENU_VIEWPAGE:
			if(m_usebrowser.GetSelected())
			{
				DataHandle dh;
				kGUIString s;
				HtmlWindow *win;

				dh.SetMemory();
				dh.OpenWrite("wb");
				BuildPage(&dh,m_currow,true,true);
				dh.Close();

				dh.Open();
				dh.Read(&s,dh.GetSize());
				dh.Close();

				win=new HtmlWindow(BROWSE_DATA,&s);
			}
			else
			{
				DataHandle dh;

				dh.SetFilename("temp.html");
				dh.OpenWrite("wb");
				BuildPage(&dh,m_currow,true,true);
				dh.Close();

				kGUI::FileShow("temp.html");
			}
		break;
		case MAPMENU_SETCENTER:
			if(m_currow)
			{
				m_clat.SetString(m_currow->m_lat.GetString());
				m_clon.SetString(m_currow->m_lon.GetString());
			}
			else
			{
				m_clat.SetString(m_ulat.GetString());
				m_clon.SetString(m_ulon.GetString());
			}
			m_center.Set(m_clat.GetString(),m_clon.GetString());
			ReCalcDists();
			MapDirty();		/* draws marker on map for center point so force redraw*/
			SetScrollCenter(&m_center);
		break;
		case MAPMENU_ADDTOROUTE:
		{
			/* make sure point is not already in the route */
			if(m_routes.InTable(m_currow->m_wptname.GetString()))
				return;

			m_routes.AddRow(m_currow);
			MapDirty();
		}
		break;
		case MAPMENU_REMOVEFROMROUTE:
			m_routes.Remove(m_currow->m_wptname.GetString());
		break;
		case MAPMENU_ADDNEARTOROUTE:
		{
			unsigned int f;
			unsigned int numnear;
			double neardist;
			double distance;
			Array<GPXRow *>nearlist;
			GPXCoord testcenter;
			GPXCoord testup;
			GPXCoord testdown;
			GPXCoord testleft;
			GPXCoord testright;
			GPXCoord *tp;
			GPXCoordBox testbox;
			unsigned int	nr=m_fwt->GetNumChildren(0);
			GPXRow *row;
			double ratio,eval1,eval2;

			/* find near points, remove ones already in route, then sort based on */
			/* shortest total distance */
			
			nearlist.Init(32,32);
			testcenter.Set(&(m_currow->m_llcoord));
			GetDistInfo(m_disttype.GetSelection(),0,&ratio,&eval1,&eval2);

			neardist=m_neardist.GetDouble();
			testcenter.Project(neardist,  0.0f,eval1,eval2,&testdown);
			testcenter.Project(neardist, 90.0f,eval1,eval2,&testright);
			testcenter.Project(neardist,180.0f,eval1,eval2,&testup);
			testcenter.Project(neardist,270.0f,eval1,eval2,&testleft);

			testbox.Set(testup.GetLat(),testdown.GetLat(),testleft.GetLon(),testright.GetLon());

			numnear=0;
			for(f=0;f<nr;++f)
			{
				row=static_cast<GPXRow *>(m_fwt->GetChild(f));
				tp=&row->m_llcoord;
				if(testbox.Inside(tp)==true)
				{
					distance=testcenter.Dist(tp);	/* in km */
					distance*=ratio;				/* convert to user distance */
					if(distance<neardist)
					{
						/* make sure point is not already in the route */
						if(!m_routes.InTable(row->m_wptname.GetString()))
							nearlist.SetEntry(numnear++,row);
					}
				}
			}
			if(numnear)
			{
				/* solve the travelling salesman problem */
				kGUITSP tsp;
				GPXCoord *cp;

				tsp.Init(numnear);
				for(f=0;f<numnear;++f)
				{
					cp=&(nearlist.GetEntry(f)->m_llcoord);
					tsp.SetCoord(f,cp->GetLat(),cp->GetLon());
				}
				tsp.Calc();

				for(f=0;f<numnear;++f)
				{
					row=nearlist.GetEntry(tsp.GetIndex(f));
					m_routes.AddRow(row);
				}
				MapDirty();
			}
		}
		break;
		case MAPMENU_TOGGLEFOUND:
			m_currow->m_found.SetSelected(!m_currow->m_found.GetSelected());
			ReFilter(m_currow);
		break;
		case MAPMENU_TOGGLEUSERTICKED:
			m_currow->m_user[0].SetSelected(!m_currow->m_user[0].GetSelected());
			ReFilter(m_currow);
		break;
		case MAPMENU_TOGGLENA:
			m_currow->m_na.SetSelected(!m_currow->m_na.GetSelected());
			ReFilter(m_currow);
		break;
		case MAPMENU_CLEARMAPSELECT:
			m_mapsel=0;
			MapDirty();
		break;
		case MAPMENU_SELECTMAPUL:
			m_mapsel|=1;
			m_mapselul.Set(&m_upos);
			if(m_mapsel==3)
				MapDirty();
		break;
		case MAPMENU_SELECTMAPLR:
			m_mapsel|=2;
			m_mapsellr.Set(&m_upos);
			if(m_mapsel==3)
				MapDirty();
		break;
		case MAPMENU_NEWPOINT:
		{
			int id=0;
			/* add user point to the map */
			GPXRow *userrow;
			kGUIString username;
			kGUIString ll;

			/* look for unused USER_XX */
			do
			{
				++id;
				username.Sprintf("USER_%d",id);
				if(!Locate(&username))
					break;
			}while(1);

			userrow=new GPXRow();

			userrow->SetName(&username);
			userrow->SetWptName(&username);

			userrow->m_llcoord.Set(&m_upos);
			userrow->m_llcoord.Output(&userrow->m_lat,&userrow->m_lon);
			userrow->UpdateLabelName();

			m_wptlist.SetEntry(m_numwpts++,userrow);
			BSPDirty();
			GetColour(userrow);		/* calc label colour */

			/* requery, so it will show up as a label */
			ReFilter();
		}
		break;
		case MAPMENU_SELECTNEARESTTRACKPIECE:
			/* find nearest track piece to the mouse pointer position */
			/* and then put the cursor at that place in the track table */
			m_tracks.GotoTrackPosition(m_upos.GetLat(),m_upos.GetLon());
		break;
		case MAPMENU_ADDTOTRACK:
			m_tracks.AddPointToTrack(m_upos.GetLat(),m_upos.GetLon());
		break;
		case MAPMENU_ADDTOLINE:
			if(m_currow)
				m_lines.AddPointToLine(m_currow->m_wptname.GetString());
			else
				m_lines.AddPointToLine(m_upos.GetLat(),m_upos.GetLon());
		break;
		}
	}
}

void GPX::MoveDividerEvent(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_AFTERUPDATE)
		MoveDivider(event->m_value[0].i);
}

void GPX::MoveDivider(int delta)
{
	int currenty=m_divider.GetZoneY();
	int newy;

	newy=currenty+delta;
	if(newy<150)
		newy=150;
	else if(newy>(m_tabs.GetZoneH()-50))
		newy=m_tabs.GetZoneH()-50;

	if(newy!=currenty)
	{
		int changey=currenty-newy;

		/* adjust size of table */
		m_fwt->SetZoneH(m_fwt->GetZoneH()-changey);
		/* adjust position of divider */
		m_divider.SetZoneY(m_divider.GetZoneY()-changey);
		/* adjust position and size of grid */
		m_grid.SetZoneY(m_grid.GetZoneY()-changey);
		m_grid.SetZoneH(m_grid.GetZoneH()+changey);
	}
}

void GPX::Online(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_AFTERUPDATE)
	{
		g_isonline=m_online.GetSelected();
		if(m_online.GetSelected()==true)
		{
			m_curmap->ResetOnline();
			MapDirty();
		}
	}
}

void GPX::UpdateCenter(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_AFTERUPDATE)
	{
		m_center.Set(m_clat.GetString(),m_clon.GetString());
		ReCalcDists();
		SetScrollCenter(&m_center);
	}
}

class NearBSP : public kGUIBSPPointEntry
{
public:
	GPXRow *m_row;
	unsigned int m_e;
};

#define USEBSP 1

void GPX::ReCalcNear(void)
{
	unsigned int e;
	GPXRow *trow;
	GPXRow *row;
	double distance;
	double neardist;
	GPXCoord *tp;
	GPXCoord testcenter;
	GPXCoord testup;
	GPXCoord testdown;
	GPXCoord testleft;
	GPXCoord testright;
	unsigned int nr=m_fwt->GetNumChildren(0);
	double ratio,eval1,eval2;
	kGUIDate starttime;
	kGUIDate endtime;
	GPXCoordBox testbox;
#if USEBSP
	int numnear;
	kGUIBSPPoint bsp;
	NearBSP *nearbsp;
	NearBSP *nbp;
	kGUICorners c;
#else
	unsigned int f;
	Array<int>numnear;
#endif

	starttime.SetToday();
	GetDistInfo(m_disttype.GetSelection(),0,&ratio,&eval1,&eval2);

	/* calc #near */

	neardist=m_neardist.GetDouble()/ratio;

#if USEBSP
	bsp.Alloc(nr,8);
	nearbsp=new NearBSP[nr];
	nbp=nearbsp;
	for(e=0;e<nr;++e)
	{
		row=static_cast<GPXRow *>(m_fwt->GetChild(e));

		nbp->m_c[0]=(int)(row->m_llcoord.GetLon()*100000);
		nbp->m_c[1]=(int)(row->m_llcoord.GetLat()*100000);

		nbp->m_row=row;
		nbp->m_e=e;
		bsp.AddEntry(nbp);
		++nbp;
	}
	bsp.Cut();
	endtime.SetToday();
	m_debug.ASprintf("CalcNumNear-BspCut, numrecords=%d seconds=%d\n",nr,starttime.GetDiffSeconds(&endtime));
#else
	numnear.Alloc(nr);
	for(e=0;e<nr;++e)
		numnear.SetEntry(e,0);
#endif

	for(e=0;e<nr;++e)
	{
		row=static_cast<GPXRow *>(m_fwt->GetChild(e));
		testcenter.Set(&(row->m_llcoord));

		testcenter.Project(neardist,  0.0f,eval1,eval2,&testdown);
		testcenter.Project(neardist, 90.0f,eval1,eval2,&testright);
		testcenter.Project(neardist,180.0f,eval1,eval2,&testup);
		testcenter.Project(neardist,270.0f,eval1,eval2,&testleft);
		testbox.Set(testup.GetLat(),testdown.GetLat(),testleft.GetLon(),testright.GetLon());

#if USEBSP
		numnear=0;
		c.lx=(int)(testleft.GetLon()*100000);
		c.rx=(int)(testright.GetLon()*100000);
		c.ty=(int)(testup.GetLat()*100000);
		c.by=(int)(testdown.GetLat()*100000);
		bsp.Select(&c);
		do
		{
			nbp=static_cast<NearBSP *>(bsp.GetEntry());
			if(!nbp)
				break;

			trow=nbp->m_row;
			if(nbp->m_e!=e)
			{
				tp=&trow->m_llcoord;
				if(testbox.Inside(tp)==true)
				{
					distance=testcenter.Dist(tp);	/* in km */
//					distance*=ratio;
					if(distance<neardist)
						++numnear;
				}
			}
		}while(1);
		row->SetNumNear(numnear);
#else
		testbox.Set(testup.GetLat(),testdown.GetLat(),testleft.GetLon(),testright.GetLon());

		for(f=e+1;f<nr;++f)
		{
			row=static_cast<GPXRow *>(m_fwt->GetChild(f));
			tp=&row->m_llcoord;
			if(testbox.Inside(tp)==true)
			{
				distance=testcenter.Dist(tp);	/* in km */
//				distance*=ratio;
				if(distance<neardist)
				{
					numnear.SetEntry(e,numnear.GetEntry(e)+1);
					numnear.SetEntry(f,numnear.GetEntry(f)+1);
				}
			}
		}
#endif
	}
#if USEBSP
	delete []nearbsp;
#else
	for(e=0;e<nr;++e)
	{
		trow=static_cast<GPXRow *>(m_fwt->GetChild(e));
		trow->SetNumNear(numnear.GetEntry(e));
	}
#endif
	endtime.SetToday();
	m_debug.ASprintf("CalcNumNear, numrecords=%d seconds=%d\n",nr,starttime.GetDiffSeconds(&endtime));
}

void GPX::CalcDists(void)
{
	unsigned int e;
	GPXRow *row;
	double ratio,eval1,eval2;
	double distance;

	GetDistInfo(m_disttype.GetSelection(),0,&ratio,&eval1,&eval2);
	for(e=0;e<m_numwpts;++e)
	{
		row=m_wptlist.GetEntry(e);
		distance=m_center.Dist(&row->m_llcoord);	/* in km */
		distance*=ratio;
		row->SetDist(distance);
	}
}

void GPX::ReCalcDists(void)
{
	UpdateZoomButtons();
	CalcDists();
	ReCalcNear();
	gpx->m_routes.UpdateInfo();
}

/* callback when user clicks on a column header in the list */

void GPX::ColClick(void)
{
	int col;
	kGUITableObj *table=0;

	switch(m_tabs.CurrentGroup())
	{
	case TAB_MAIN:
		table=m_fwt;
	break;
	case TAB_ROUTE:
		table=m_routes.GetTable();
	break;
	default:
		assert(false,"unimplemented table sort error!");
	break;
	}

	col=table->GetCursorCol();

	if(col>=GPXCOL_USER1 && col<(GPXCOL_USER1+MAXUSERTICKS))
		m_colmenu.Init(COLMENU_NUM2,colmenutxt);
	else
		m_colmenu.Init(COLMENU_NUM,colmenutxt);
	m_colmenu.Activate(kGUI::GetMouseX(),kGUI::GetMouseY());
}

void GPX::DoColMenu(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_SELECTED)
	{
		int e,nr,ucol;
		kGUITableObj *table=0;
		kGUIInputBoxReq *box;

		switch(m_tabs.CurrentGroup())
		{
		case TAB_MAIN:
			table=m_fwt;
		break;
		case TAB_ROUTE:
			table=m_routes.GetTable();
		break;
		default:
			assert(false,"unimplemented table sort error!");
		break;
		}

		switch(m_colmenu.GetSelection())
		{
		case COLMENU_SORTASC:
			m_numsortcols=1;
			m_sortcols.SetEntry(0,table->GetColOrder(table->GetCursorCol()));
			m_sortrevs.SetEntry(0,false);
			table->Sort(SortEntry);
		break;
		case COLMENU_SORTDESC:
			m_numsortcols=1;
			m_sortcols.SetEntry(0,table->GetColOrder(table->GetCursorCol()));
			m_sortrevs.SetEntry(0,true);
			table->Sort(SortEntry);
		break;
		case COLMENU_HIDE:
			table->SetColShow(table->GetColOrder(table->GetCursorCol()),false);
		break;
		case COLMENU_ADDSORTASC:
			m_sortcols.SetEntry(m_numsortcols,table->GetColOrder(table->GetCursorCol()));
			m_sortrevs.SetEntry(m_numsortcols,false);
			++m_numsortcols;
			table->Sort(SortEntry);
		break;
		case COLMENU_ADDSORTDESC:
			m_sortcols.SetEntry(m_numsortcols,table->GetColOrder(table->GetCursorCol()));
			m_sortrevs.SetEntry(m_numsortcols,true);
			++m_numsortcols;
			table->Sort(SortEntry);
		break;
		case COLMENU_CLEARALL:
			ucol=table->GetCursorCol()-GPXCOL_USER1;
			assert(ucol>=0 && ucol<MAXUSERTICKS,"Internal error!");

			nr=table->GetNumChildren(0);
			for(e=0;e<nr;++e)
			{
				GPXRow *row=static_cast<GPXRow *>(table->GetChild(e));
				row->m_user[ucol].SetSelected(false);
			}
		break;
		case COLMENU_SETALL:
			ucol=table->GetCursorCol()-GPXCOL_USER1;
			assert(ucol>=0 && ucol<MAXUSERTICKS,"Internal error!");

			nr=table->GetNumChildren(0);
			for(e=0;e<nr;++e)
			{
				GPXRow *row=static_cast<GPXRow *>(table->GetChild(e));
				row->m_user[ucol].SetSelected(true);
			}
		break;
		case COLMENU_SETHINT:
			m_ucol=table->GetCursorCol()-GPXCOL_USER1;
			assert(m_ucol>=0 && m_ucol<MAXUSERTICKS,"Internal error!");

			box=new kGUIInputBoxReq(this,CALLBACKNAME(SetHintDone),"Column Hint?");
		break;
		}

		switch(m_tabs.CurrentGroup())
		{
		case TAB_ROUTE:
			m_routes.Changed();
		break;
		}
	}
}

void GPX::SetHintDone(kGUIString *result,int closebutton)
{
	if(closebutton==MSGBOX_OK)
		SetUserHint(m_ucol,result);
}

int GPX::SortEntry(const void *o1,const void *o2)
{
	unsigned int s;
	int sc;
	int res;
	bool sr;
	GPXRow *r1=*(static_cast<GPXRow **>((void *)o1));
	GPXRow *r2=*(static_cast<GPXRow **>((void *)o2));

	for(s=0;s<gpx->m_numsortcols;++s)
	{
		sc=gpx->m_sortcols.GetEntry(s);
		sr=gpx->m_sortrevs.GetEntry(s);

		switch(sc)
		{
		case GPXCOL_NA:
			if((r1->m_na.GetSelected()==false) && (r2->m_na.GetSelected()==true))
				res=1;
			else if(r1->m_na.GetSelected()!=r2->m_na.GetSelected())
				res=-1;
			else
				res=0;
		break;
		case GPXCOL_FOUND:
			if((r1->m_found.GetSelected()==false) && (r2->m_found.GetSelected()==true))
				res=1;
			else if(r1->m_found.GetSelected()!=r2->m_found.GetSelected())
				res=-1;
			else
				res=0;
		break;
		case GPXCOL_NAME:
			res=strcmp(r1->m_name.GetString(),r2->m_name.GetString());
		break;
		case GPXCOL_LOG:
			res=strcmp(r1->m_log.GetString(),r2->m_log.GetString());
		break;
		case GPXCOL_WPTNAME:
			res=strcmp(r1->m_wptname.GetString(),r2->m_wptname.GetString());
		break;
		case GPXCOL_TYPE:
			res=r1->m_xtype-r2->m_xtype;
		break;
		case GPXCOL_LAT:
			res=sgnd(r1->m_llcoord.GetLat()-r2->m_llcoord.GetLat());
		break;
		case GPXCOL_LON:
			res=sgnd(r1->m_llcoord.GetLon()-r2->m_llcoord.GetLon());
		break;
		case GPXCOL_DIST:
			res=sgnd(r1->m_distval-r2->m_distval);
		break;
		case GPXCOL_NUMNEAR:
			res=sgn(r1->m_nearval-r2->m_nearval);
		break;
		case GPXCOL_CONTAINER:
			res=r1->m_container.GetSelection()-r2->m_container.GetSelection();
		break;
		case GPXCOL_TERRAIN:
			res=sgnd(r1->m_terrain.GetDouble()-r2->m_terrain.GetDouble());
		break;
		case GPXCOL_DIFFICULTY:
			res=sgnd(r1->m_difficulty.GetDouble()-r2->m_difficulty.GetDouble());
		break;
		case GPXCOL_OWNER:
			res=strcmp(r1->m_owner.GetString(),r2->m_owner.GetString());
		break;
		case GPXCOL_STATE:
			res=strcmp(r1->m_state.GetString(),r2->m_state.GetString());
		break;
		case GPXCOL_COUNTRY:
			res=strcmp(r1->m_country.GetString(),r2->m_country.GetString());
		break;
		case GPXCOL_USERNOTES:
			res=strcmp(r1->m_usernotes.GetString(),r2->m_usernotes.GetString());
		break;
		case GPXCOL_GENDATE:
			res=strcmp(r1->m_gendate.GetString(),r2->m_gendate.GetString());
		break;
		default:
			res=0;
		break;
		}
		if(res)
		{
			if(sr==false)
				return(res);
			return(-res);
		}

	}
	/* tie */
	return(0);
}

GPXLabel::GPXLabel()
{
	m_row=0;
	m_draw=false;
	m_wasdrawn=-1;
	SetFontSize(WPTNAMEFONTSIZE);
}

void GPXLabel::Changed(void)
{
	m_lw=GetWidth()+4;
	m_lh=GetLineHeight()+4;
}

void GPXLabel::Draw(int cxpix,int cypix)
{
	int i,nt;
	int px,py;
	kGUICorners c;
	float alpha=(float)gpx->GetLabelAlpha();

	px=m_c.lx-cxpix;
	py=m_c.ty-cypix;

	/* if label has moved, then draw a line between the old position and the new location */
	if((m_origx!=m_c.lx) || (m_origy!=m_c.ty))
	{
		kGUI::DrawFatLine((float)(m_origx-cxpix),(float)(m_origy-cypix),(float)px+16,(float)py,DrawColor(0,0,0),3.2f,alpha);
		kGUI::DrawFatLine((float)(m_origx-cxpix),(float)(m_origy-cypix),(float)px+16,(float)py,m_colour,2.0f,alpha);
	}

	if(m_icon<0)
		kGUI::DrawRectBevel(px+2,py+2,px+12,py+12,false);
	else
		gpx->DrawIcon(m_icon,1.0f/2.0f,px,py);

	c.lx=px+16;
	c.ty=py;

	nt=gpx->GetNumShowUserTicks();
	if(m_row && nt)
	{
		kGUITickBoxObj tick;

		for(i=0;i<nt;++i)
		{
			tick.SetPos(c.lx,c.ty);
			tick.SetSelected(m_row->GetUser(i));
			tick.Draw();
			c.lx+=14;
		}
	}

	c.rx=c.lx+m_lw;
	c.by=c.ty+m_lh;

	kGUI::DrawRectFrame(c.lx,c.ty,c.rx,c.by,m_colour,DrawColor(32,32,32),alpha);
	kGUIText::Draw(c.lx+2,c.ty+2,0,0,m_tcolour);
}

void GPX::DrawBSPMapCellPoints(int cxpix,int cypix)
{
	kGUIBSPRectEntry *bspe;

	/* each row has 4 points in the tree for the 4 corners so we need to */
	/* check and not draw labels 4 times if all corners are on the same square */
	do
	{
		bspe=m_bsp.GetEntry();
		if(!bspe)
			return;
		GPXLabel *label=static_cast<GPXLabel *>(bspe);
		if(label->m_wasdrawn!=m_wasdrawn)
		{
			label->m_wasdrawn=m_wasdrawn;
			if(label->m_draw==true && label->GetLen())
				label->Draw(cxpix,cypix);
		}
	}while(1);
}

void GPX::MapDirty(void)
{
	if(m_mapdirty==false)
	{
		if(m_tabs.CurrentGroup()==TAB_MAIN)
			m_grid.Dirty();
	}
	m_mapdirty=true;	/* cells have changed */
}

/* sort recent arrivals into order */
int qs_sortlabels(const void *v1,const void *v2)
{
	ROWY_DEF *i1;
	ROWY_DEF *i2;

	i1=(ROWY_DEF *)v1;
	i2=(ROWY_DEF *)v2;

	return(i1->y-i2->y);
}

void GPX::PreDrawMap(void)
{
	if(m_mapdirty==true)
	{
		m_mapdirty=false;
		m_grid.Flush();
	}

	if(m_rebuildbsp==true)
	{
		unsigned int e;
		int px,py;
		unsigned int c;
		GPXRow *row;
		bool showchildren;
		int offx;

		showchildren=m_showchildren.GetSelected();
		m_wasdrawn=0;
		m_rebuildbsp=false;
		m_bsp.Alloc(m_numwpts<<3,8);	/* allocate 8* actual num incase lots of children */

		m_numshowticks=m_shownumticks.GetInt();
		offx=16+(14*m_numshowticks);

		for(e=0;e<m_numwpts;++e)
		{
			row=m_wptlist.GetEntry(e);

			/* convert from lat/lon to pixel values */
			m_curmap->ToMap(&row->m_llcoord,&px,&py);
			
			/* point to record in full list */
			row->m_label.m_origx=px;
			row->m_label.m_origy=py;

			row->m_label.m_c.lx=px;
			row->m_label.m_c.ty=py;
			row->m_label.m_c.rx=px+offx+row->m_label.m_lw;
			row->m_label.m_c.by=py+row->m_label.m_lh;
			row->m_label.m_wasdrawn=-1;

			if(showchildren)
			{
				for(c=0;c<row->m_numchildren;++c)
				{
					GPXChild *child;

					child=row->m_children.GetEntry(c);

					/* convert from lat/lon to pixel values */
					m_curmap->ToMap(child->GetPosPtr(),&px,&py);

					child->m_label.m_origx=px;
					child->m_label.m_origy=py;

					child->m_label.m_c.lx=px;
					child->m_label.m_c.ty=py;
					child->m_label.m_c.rx=px+offx+child->m_label.m_lw;
					child->m_label.m_c.by=py+child->m_label.m_lh;
					child->m_label.m_wasdrawn=-1;
					/* copy colors from parent */
					child->m_label.m_colour=row->m_label.m_colour;
					child->m_label.m_tcolour=row->m_label.m_tcolour;

				}
			}
		}

		if(m_movelabels.GetSelected()==true)
		{
			unsigned int le;
			int maxmovepixels=m_movelabelsmax.GetInt();
			ROWY_DEF ry;
			unsigned int numsorted;
			unsigned int unsorted;
			bool ignore;

			/* put labels to be sorted against each other at beginning and labels */
			/* that are not to be sorted at the end of the list */

			m_swptlist.Alloc(m_numwpts);
			numsorted=0;
			unsorted=m_numwpts;
			for(e=0;e<m_numwpts;++e)
			{
				row=m_wptlist.GetEntry(e);
				ry.row=row;
				ry.y=row->m_label.m_c.ty;
				ignore=!row->m_label.m_draw;

				if(ignore)
					m_swptlist.SetEntry(--unsorted,ry);
				else
					m_swptlist.SetEntry(numsorted++,ry);
			}
			qsort(m_swptlist.GetArrayPtr(),numsorted,sizeof(ROWY_DEF),qs_sortlabels);

			le=0;
			for(e=0;e<numsorted;++e)
			{
				int o;
				int maxmoved;
				int movedist;
				bool moved;
				bool collided;
				ROWY_DEF lry;
				GPXRow *lrow;
				kGUICorners newpos;

				ry=m_swptlist.GetEntry(e);
				row=ry.row;

				while(le<e)
				{
					lry=m_swptlist.GetEntry(le);
					lrow=lry.row;
					if(lrow->m_label.m_c.by<=row->m_label.m_c.ty)
						++le;
					else
						break;
				}

				/* copy current position */
				newpos.lx=row->m_label.m_c.lx;
				newpos.rx=row->m_label.m_c.rx;
				newpos.ty=row->m_label.m_c.ty;
				newpos.by=row->m_label.m_c.by;

				moved=false;

				maxmoved=maxmovepixels;	/* maximum allowable move distance */

				do
				{
					collided=false;
					o=0;
					while((le+o)<e)
					{
						lry=m_swptlist.GetEntry(le+o);
						lrow=lry.row;
						/* if we overlap this label then move down till we clear it */
						if(kGUI::Overlap(&newpos,&(lrow->m_label.m_c)))
						{
							collided=true;
							moved=true;
							/* calc # pixels to skip down to clear this label */
							movedist=(lrow->m_label.m_c.by-newpos.ty)+1;
							maxmoved-=movedist;
							if(maxmoved<=0)
								break;
							newpos.ty+=movedist;
							newpos.by+=movedist;
						}
						++o;
					}
				}while(collided && (maxmoved>0));
				
				/* did it find a new place to move it to? */
				if(collided==false && moved==true)
				{
					row->m_label.m_c.lx=newpos.lx;
					row->m_label.m_c.rx=newpos.rx;
					row->m_label.m_c.ty=newpos.ty;
					row->m_label.m_c.by=newpos.by;
				}
			}
		}

		/* add them all to the BSP */
		for(e=0;e<m_numwpts;++e)
		{
			if(m_movelabels.GetSelected()==true)
			{
				ROWY_DEF ry;

				ry=m_swptlist.GetEntry(e);
				row=ry.row;
			}
			else
				row=m_wptlist.GetEntry(e);

			m_bsp.AddEntry(&row->m_label);
			if(showchildren)
			{
				for(c=0;c<row->m_numchildren;++c)
				{
					GPXChild *child;

					child=row->m_children.GetEntry(c);
					m_bsp.AddEntry(&child->m_label);
				}
			}
		}
		m_bsp.Cut();
	}
}

kGUIPoint2 arrowpoly[]={
	{0,-20},
	{-10,10},
	{0,4},
	{10,10}};

#define LEGOFF 25

void GPX::PostDrawMap(void)
{
	int cx,cy;
	kGUICorners c;
	GPXCoord pos1;
	GPXCoord pos2;
	double dist;
	kGUIText stext;
	int h,w;
	int l,nc,y;

	m_grid.GetCorners(&c);

	if(m_realtime->IsTracking())
	{
		int sx,sy;
		kGUIPoint2 rot_arrowpoly[sizeof(arrowpoly)/sizeof(kGUIPoint2)];
		kGUIPoint2 rot;

		m_grid.GetScrollCorner(&cx,&cy);
		m_curmap->ToMap(&m_gpspos,&sx,&sy);
		sx-=cx;
		sy-=cy;
		sx+=c.lx;
		sy+=c.ty;

		kGUI::RotatePoints(sizeof(arrowpoly)/sizeof(kGUIPoint2),arrowpoly,rot_arrowpoly,m_gpsangle);
		rot.x=sx;
		rot.y=sy;
		kGUI::TranslatePoints(sizeof(arrowpoly)/sizeof(kGUIPoint2),rot_arrowpoly,rot_arrowpoly,&rot);
		kGUI::DrawPoly(sizeof(arrowpoly)/sizeof(kGUIPoint2),rot_arrowpoly,DrawColor(255,0,0));
	}
#if 0
	else
	{
		/* tried to connect but it must have been aborted! */
		if(m_gpsconnect.GetSelected())
		{
			m_gpsconnect.SetSelected(false);
			m_realtimegps.SetLocked(false);		/* allow it to change again */
			m_gpslat.Clear();
			m_gpslon.Clear();
		}
	}
#endif

	/* draw status line in top left corner if it exists */

	stext.SetFontSize(13);

	if(m_curmap->GetStatusLine())
	{
		stext.SetString(m_curmap->GetStatusLine());
		if(stext.GetLen())
		{
			w=stext.GetWidth()+10;
			h=stext.GetLineHeight();
			kGUI::DrawRect(c.lx+5,c.ty+10,c.lx+w,c.ty+10+h,DrawColor(255,255,255),0.65f);
			stext.Draw(c.lx+10,c.ty+11,0,0,DrawColor(0,0,0));
		}
	}

	/* draw copyright lines on bottom left of window */

	nc=m_curmap->GetNumCopyrightLines();
	y=c.by-10-(nc*stext.GetLineHeight());
	for(l=0;l<nc;++l)
	{
		stext.SetString(m_curmap->GetCopyrightLine(l));
		w=stext.GetWidth()+10;
		h=stext.GetLineHeight();
		kGUI::DrawRect(c.lx+5,y-1,c.lx+w,y-1+h,DrawColor(255,255,255),0.65f);
		stext.Draw(c.lx+10,y,0,0,DrawColor(0,0,0));
		y+=stext.GetLineHeight();
	}

	/* draw map scale on bottom right corner */
	
	m_grid.GetScrollCenter(&cx,&cy);
	m_curmap->FromMap(cx-(LEGWIDTH/2),cy,&pos1);
	m_curmap->FromMap(cx+LEGWIDTH-(LEGWIDTH/2),cy,&pos2);
	dist=pos1.Dist(&pos2);

	{
		double ratio,eval1,eval2;
		GetDistInfo(m_disttype.GetSelection(),0,&ratio,&eval1,&eval2);

		dist*=ratio;
	}

	stext.SetFontSize(13);
	stext.Sprintf("%.2f %s",dist,m_disttype.GetSelectionString());

	w=stext.GetWidth();
	h=stext.GetLineHeight();
	kGUI::DrawRect(c.rx-(LEGWIDTH>>1)-LEGOFF-(w>>1),c.by-LEGOFF-h,c.rx-(LEGWIDTH>>1)-LEGOFF+(w>>1),c.by-LEGOFF,DrawColor(255,255,255),0.65f);
	stext.Draw(c.rx-(LEGWIDTH>>1)-LEGOFF-(w>>1),c.by-LEGOFF-h,w,h,DrawColor(0,0,0));

	/* black edges */
	kGUI::DrawLine(c.rx-(LEGWIDTH+LEGOFF)-1,c.by-LEGOFF-15,c.rx-(LEGWIDTH+LEGOFF)-1,c.by-LEGOFF,DrawColor(0,0,0));	
	kGUI::DrawLine(c.rx-LEGOFF-1,c.by-LEGOFF-15,c.rx-LEGOFF-1,c.by-LEGOFF,DrawColor(0,0,0));	
	kGUI::DrawLine(c.rx-(LEGWIDTH+LEGOFF),c.by-LEGOFF-1,c.rx-LEGOFF,c.by-LEGOFF-1,DrawColor(0,0,0));	

	/* white edges */
	kGUI::DrawLine(c.rx-(LEGWIDTH+LEGOFF),c.by-LEGOFF-15,c.rx-(LEGWIDTH+LEGOFF),c.by-LEGOFF,DrawColor(255,255,255));	
	kGUI::DrawLine(c.rx-LEGOFF,c.by-LEGOFF-15,c.rx-LEGOFF,c.by-LEGOFF,DrawColor(255,255,255));	
	kGUI::DrawLine(c.rx-(LEGWIDTH+LEGOFF),c.by-LEGOFF,c.rx-LEGOFF,c.by-LEGOFF,DrawColor(255,255,255));	

	m_grid.GetScrollCorner(&cx,&cy);
	cx-=c.lx;
	cy-=c.ty;

	/* if both corners are set, then draw the map selected rectangle */
	/* don't draw when printing or saving */
	if(m_mapsel==3 && m_mapasync==true)
	{
		int x1,y1,x2,y2;

		m_curmap->ToMap(&m_mapselul,&x1,&y1);
		m_curmap->ToMap(&m_mapsellr,&x2,&y2);
		x1-=cx;
		x2-=cx;
		y1-=cy;
		y2-=cy;
		kGUI::DrawCurrentFrame(x1,y1,x2,y2);
	}
	m_tracks.PostDraw(cx,cy);
}

void GPX::DrawMapCell(kGUICellObj *cell)
{
	int cx,cy;
	int drawn;
	int cxpix,cypix;
	kGUICorners c;

	cx=cell->GetX();
	cy=cell->GetY();
	cxpix=cx*m_grid.GetCellWidth();
	cypix=cy*m_grid.GetCellHeight();

	c.lx=cxpix;
	c.rx=cxpix+m_grid.GetCellWidth();
	c.ty=cypix;
	c.by=cypix+m_grid.GetCellHeight();

	kGUI::SetMouseCursor(MOUSECURSOR_BUSY);

	if(m_curmap->m_bad)	/* error loading this map? */
		drawn=TILE_ERROR;
	else
		drawn=m_curmap->DrawTile(cx,cy);

	switch(drawn)
	{
	case TILE_WAITING:
		cell->Flush();
	case TILE_LOADING:
		kGUI::DrawRect(0,0,m_grid.GetCellWidth(),m_grid.GetCellHeight(),(cx+cy)&1?DrawColor(255,255,255):DrawColor(32,32,255));
		m_shapes[SHAPE_LOADINGIMAGE].ScaleTo(m_grid.GetCellWidth()/3,m_grid.GetCellHeight()/3);
		m_shapes[SHAPE_LOADINGIMAGE].Draw(0,0,0);
	break;
	case TILE_ERROR:
		kGUI::DrawRect(0,0,m_grid.GetCellWidth(),m_grid.GetCellHeight(),(cx+cy)&1?DrawColor(255,255,255):DrawColor(32,32,255));
		m_shapes[SHAPE_BROKENIMAGE].ScaleTo(m_grid.GetCellWidth()/3,m_grid.GetCellHeight()/3);
		m_shapes[SHAPE_BROKENIMAGE].Draw(0,0,0);
	break;
	}

	m_routes.DrawMap(&c);
	if(m_zoom>=m_labelzoomsize.GetInt())
	{
		kGUIText t;
		m_bsp.Select(&c);
		++m_wasdrawn;

		m_onmap=true;

		DrawBSPMapCellPoints(cxpix,cypix);
	}
	else
		m_onmap=false;

	m_tracks.DrawMap(&c);

	/* draw the current position as a marker */
	{
		int px,py;
		m_curmap->ToMap(&m_center,&px,&py);
		px-=cxpix;
		py-=cypix;
		m_shapes[SHAPE_MARKER].Draw(0,px-(m_shapes[SHAPE_MARKER].GetImageWidth()>>1),py-m_shapes[SHAPE_MARKER].GetImageHeight());
	}

	m_lines.DrawMap(&c);

	kGUI::SetMouseCursor(MOUSECURSOR_DEFAULT);
}


/* global app input callback */
void GPX::CheckSpecialKeys(void)
{
	int key=kGUI::GetKey();

	unsigned int i;

	if(m_realtime->IsTracking())
	{
		if(m_realtime->GetPos(&m_gpspos))
		{
			double dx,dy;

			m_gpsdelay=0;	/* reset counter */
//			m_gpspos.Set(pvt->lat*(180.0f/3.141592654),pvt->lon*(180.0f/3.141592654));
			dx=m_gpspos.GetLat()-m_lastgpspos.GetLat();
			dy=m_gpspos.GetLon()-m_lastgpspos.GetLon();
			if((dx!=0.0f) || (dy!=0.0f))
			{
				m_lastgpspos.Set(&m_gpspos);
				memmove(m_gpsdeltas+1,m_gpsdeltas,sizeof(m_gpsdeltas)-sizeof(kGUIDPoint2));
				m_gpsdeltas[0].x=dx;
				m_gpsdeltas[0].y=dy;
			}

			if(m_tabs.CurrentGroup()==TAB_MAIN)
			{
				double tdx=0.0f,tdy=0.0f;
				/* keep a buffer of the last n positions */
				
				/* calculate the average angle over the last n points */
				for(i=0;i<(sizeof(m_gpsdeltas)/sizeof(kGUIDPoint2))-1;++i)
				{
					tdx+=m_gpsdeltas[i].x;
					tdy+=m_gpsdeltas[i].y;
				}
				m_gpsangle=kGUI::CalcAngle(tdx,tdy);

				m_gpspos.Output(&m_gpslat,&m_gpslon);

				if(m_gpsfollow.GetSelected()==true)
					SetScrollCenter(&m_gpspos);
			}
		}
		else
		{
			if(++m_gpsdelay==120)
			{
				m_gpslat.SetString("no signal");
				m_gpslon.Clear();
			}
		}
	}
	else
	{
		/* tried to connect but it must have been aborted! */
		if(m_gpsconnect.GetSelected())
		{
			m_gpsconnect.SetSelected(false);
			m_realtimegps.SetLocked(false);		/* allow it to change again */
			m_gpslat.Clear();
			m_gpslon.Clear();

			if(m_realtime->GetOutputString()->GetLen())
			{
				kGUIMsgBoxReq *box;

				box=new kGUIMsgBoxReq(MSGBOX_OK,false,m_realtime->GetOutputString()->GetString());
			}
		}
	}

	switch(key)
	{
	case GUIKEY_CTRL_PLUS:	/* ctrl '+' */
		ZoomIn();
		kGUI::ClearKey();	/* clear all control keys */
	break;
	case GUIKEY_CTRL_MINUS:	/* ctrl '-' */
		ZoomOut();
		kGUI::ClearKey();	/* clear all control keys */
	break;
	}
}

void GPX::TabChanged(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_MOVED)
	{
		switch(m_tabs.CurrentGroup())
		{
		case TAB_GPSES:
			//this is called here since two buttons on this page are enabled/disabled based on if there are any
			//valid tracks and valid waypoints in the filtered results list
			m_gpsr.UpdateGPSList();
		break;
		}
	}
}

int GPX::GetColNum(const char *colname)
{
	int cnum;

	cnum=GetColNumz(colname);
	assert(cnum>=0,"Cannot find column!");
	return(cnum);
}

extern const char *wpcolnames[GPXCOL_NUMCOLUMNS];

int GPX::GetColNumz(const char *colname)
{
	int xcol;

	/* todo, change to use a hash table */
	for(xcol=0;xcol<GPXCOL_NUMCOLUMNS;++xcol)
	{
		if(!stricmp(wpcolnames[xcol],colname))
			return(xcol);
	}
	return(-1);
}

void GPX::LoadMapPaths(kGUIXML *xml,bool xmlstatus)
{
	unsigned int i;
	kGUIXMLItem *root;
	kGUIXMLItem *item;

	if(xmlstatus==true)
	{
		root=xml->GetRootItem()->Locate("gtp");

		/* get map paths */
		for(i=0;i<root->GetNumChildren();++i)
		{
			item=root->GetChild(i);
			if(!strcmp(item->GetName(),"mappath"))
			{
				GPXMapPathRow *row;
				row=new GPXMapPathRow();
				row->Load(item);
				m_mapdirstable.AddRow(row);
			}
		}
	}
}

void GPX::LoadPrefs(kGUIXML *xml,bool xmlstatus)
{
	unsigned int i;
	kGUIXMLItem *root;
	kGUIXMLItem *group;
	kGUIXMLItem *item;
	kGUIDate endtime;

	if(xmlstatus==true)
	{
		root=xml->GetRootItem()->Locate("gtp");
		if(root)
		{
			Get(root,"clipfoundlogs",&m_clipfoundlogs);
			Get(root,"clipnotfoundlogs",&m_clipnotfoundlogs);
			Get(root,"wptname",&m_wptname);
			CompileLabelDefinition(&m_wptname,&m_wptnamenumentries,&m_wptnameentries);

			/* load default sort */
			{
				int slot;
				kGUIXMLItem *entry;

				item=root->Locate("wptsort");

				m_numsortcols=item->GetNumChildren();
				m_sortcols.Alloc(m_numsortcols);
				m_sortrevs.Alloc(m_numsortcols);
				for(i=0;i<m_numsortcols;++i)
				{
					entry=item->GetChild(i);
					slot=entry->Locate("col")->GetValueInt();
					m_sortcols.SetEntry(slot,GetColNum(entry->Locate("name")->GetValueString()));
					m_sortrevs.SetEntry(slot,entry->Locate("rev")->GetValueInt()==1?true:false);
				}
			}

			/* load user hints if any */
			{
				unsigned int jj;
				kGUIXMLItem *entry;

				item=root->Locate("userhints");
				if(item)
				{
					jj=item->GetNumChildren();
					for(i=0;i<jj;++i)
					{
						entry=item->GetChild(i);
						SetUserHint(entry->Locate("col")->GetValueInt(),entry->Locate("hint")->GetValue());
					}
				}
			}

			endtime.SetToday();
			m_debug.ASprintf("LoadPrefs 1 seconds=%d\n",m_starttime.GetDiffSeconds(&endtime));

			m_gpsr.LoadPrefs(root);

			endtime.SetToday();
			m_debug.ASprintf("LoadPrefs 2(gpsrs) seconds=%d\n",m_starttime.GetDiffSeconds(&endtime));

			/* since we pre-loaded these during the xmlstream load they don't */
			/* save the correct settings / colors etc so we update them all now */
			for(i=0;i<m_numwpts;++i)
			{
				GPXRow *row;

				row=m_wptlist.GetEntry(i);
				row->CalcHeight();
				row->UpdateLabelName();
			}
			UpdateDBList();
			endtime.SetToday();
			m_debug.ASprintf("LoadPrefs 3(cache entries) seconds=%d\n",m_starttime.GetDiffSeconds(&endtime));

			/* get lines */
			m_lines.LoadPrefs(root);

			endtime.SetToday();
			m_debug.ASprintf("LoadPrefs 4(lines) seconds=%d\n",m_starttime.GetDiffSeconds(&endtime));

			/* get tracks */
			m_tracks.LoadPrefs(root);

			endtime.SetToday();
			m_debug.ASprintf("LoadPrefs 5(tracks) seconds=%d\n",m_starttime.GetDiffSeconds(&endtime));

			/* get routes */
			m_routes.LoadPrefs(root);

			endtime.SetToday();
			m_debug.ASprintf("LoadPrefs 6(routes) seconds=%d\n",m_starttime.GetDiffSeconds(&endtime));

			/* get notes */
			m_notes.LoadPrefs(root);

			/* get downloads */
			m_download.LoadPrefs(root);

			endtime.SetToday();
			m_debug.ASprintf("LoadPrefs 7(notes) seconds=%d\n",m_starttime.GetDiffSeconds(&endtime));

			Get(root,"neardistance",&m_neardist);
			/* load filters */
			m_filters.LoadPrefs(root);

			endtime.SetToday();
			m_debug.ASprintf("LoadPrefs 8(filters) seconds=%d\n",m_starttime.GetDiffSeconds(&endtime));

			/* get stickers */
			m_stickers.LoadPrefs(root);

			endtime.SetToday();
			m_debug.ASprintf("LoadPrefs 9(stickers) seconds=%d\n",m_starttime.GetDiffSeconds(&endtime));

			/* these must be loaded after the filters since they reference the current filters */
			for(i=0;i<root->GetNumChildren();++i)
			{
				item=root->GetChild(i);
				if(!strcmp(item->GetName(),"wptrender"))
				{
					GPXWPRenderRow *row;
					row=new GPXWPRenderRow();
					row->Load(item);
					m_labelcolourtable.AddRow(row);
				}
			}

			endtime.SetToday();
			m_debug.ASprintf("LoadPrefs 10(labelcolors) seconds=%d\n",m_starttime.GetDiffSeconds(&endtime));

			MoveDivider(root->Locate("dividery")->GetValueInt()-m_divider.GetZoneY());

			Get(root,"centerlon",&m_clon);
			Get(root,"centerlat",&m_clat);
			m_center.Set(m_clat.GetString(),m_clon.GetString());

			Get(root,"printgrid",&m_printgrid);
			Get(root,"printmap",&m_printmap);
			Get(root,"printbrowser",&m_printbrowser);
			Get(root,"defpath",&m_defpath);
#ifdef INCLUDEWIG
			Get(root,"luapath",&m_luapath);
			Get(root,"wigpath",&m_wigpath);
#endif
			Get(root,"realtimegps",&m_realtimegps);
			Get(root,"basic",&m_basicsource);

			m_nummacrobuttons=0;
			group=root->Locate("macrobuttons");
			if(group)
			{
				for(i=0;i<group->GetNumChildren();++i)
					m_macrobuttons.GetEntryPtr(m_nummacrobuttons++)->Load(group->GetChild(i));
			}

			Get(root,"showchildwpts",&m_showchildren);

			Get(root,"split",&m_split);
			Get(root,"splitfontsize",&m_splitfontsize);
			Get(root,"splitsort",&m_splitsort);
			Get(root,"splitwidth",&m_splitwidth);
			Get(root,"splitdesc",&m_splitdesc);

			Get(root,"shownumticks",&m_shownumticks);
			Get(root,"labelfontsize",&m_labelfontsize);
			GetI(root,"labelalpha",&m_labelalpha);
			LabelFontSizeChanged();
			
			if(Get(root,"tablefontsize",&m_tablefontsize))
				TableFontSizeChanged();

			Get(root,"maxdownloads",&m_maxdownloads);

			Get(root,"usebrowser",&m_usebrowser);
			Get(root,"movelabels",&m_movelabels);
			Get(root,"movelabelsmax",&m_movelabelsmax);
			Get(root,"labelzoomsize",&m_labelzoomsize);

			Get(root,"distancetype",&m_disttype);
			Get(root,"zoom",&m_zoom);

			Get(root,"maptype",&m_maptypes);
			ChangeMapType();

			CalcDists();

			endtime.SetToday();
			m_debug.ASprintf("LoadPrefs 11(stuff) seconds=%d\n",m_starttime.GetDiffSeconds(&endtime));

			ReFilter();
			m_curmap->SetZoom(m_zoom);
			m_grid.SetBounds(0,0,m_curmap->GetWidth(),m_curmap->GetHeight());

			{
				GPXCoord c;
				int sx,sy;

				c.Set(root->Locate("scrolllat")->GetValueDouble(),root->Locate("scrolllon")->GetValueDouble());
				m_curmap->ToMap(&c,&sx,&sy);
				m_grid.SetScrollCorner(sx,sy);
			}

			endtime.SetToday();
			m_debug.ASprintf("LoadPrefs 12(maps) seconds=%d\n",m_starttime.GetDiffSeconds(&endtime));

			UpdateWPRender();

			endtime.SetToday();
			m_debug.ASprintf("LoadPrefs 13(udatewprender) seconds=%d\n",m_starttime.GetDiffSeconds(&endtime));

			m_browsersettings.Load(root);

			return;
		}
	}

	{
		/* no preferences file so generate some default */
		/* filters and label colors */
		DEFLABELLIST_DEF *dll;
		GPXWPRenderRow *wprender;

		ChangeMapType();

		m_disttype.SetSelection(2);	/* default to kms */
		m_filters.InitDefaultFilters();

		dll=defwptlabels;
		for(i=0;i<(sizeof(defwptlabels)/sizeof(DEFLABELLIST_DEF));++i)
		{
			wprender=new GPXWPRenderRow();
			wprender->m_filtercombo.SetSelection(dll->filtername);
			wprender->m_colourcombo.SetSelection(dll->colorname);
			m_labelcolourtable.AddRow(wprender);
			++dll;
		}
	}
}

void GPX::SavePrefs(bool showbusy)
{
	unsigned int i;
	int sx,sy;
	GPXCoord c;
	kGUIXMLItem *root;
	kGUIObj *obj;
	kGUIXML xml;
	kGUIMsgBoxReq *box;
	kGUIBusy *busy=0;

	if(showbusy)
	{
		busy=new kGUIBusy(kGUI::GetScreenWidth()>>1);
		busy->GetTitle()->SetString("Saving...");
		busy->SetMax(100);
	}

	xml.SetEncoding(ENCODING_UTF8);
	root=xml.GetRootItem()->AddChild("gtp");
	root->AddChild("language",m_language.GetSelection());
	root->AddChild("zoom",m_zoom);
	m_grid.GetScrollCorner(&sx,&sy);
	m_curmap->FromMap(sx,sy,&c);
#if 0
	GGPXMap::SaveVersions(root);
#endif
	root->AddChild("scrolllat",c.GetLat());
	root->AddChild("scrolllon",c.GetLon());
	root->AddChild("centerlon",m_clon.GetString());
	root->AddChild("centerlat",m_clat.GetString());
	root->AddChild("dividery",m_divider.GetZoneY());
	root->AddChild("maptype",m_maptypes.GetSelectionString());
	root->AddChild("printmap",m_printmap.GetString());
	root->AddChild("printgrid",m_printgrid.GetString());
	root->AddChild("printbrowser",m_printbrowser.GetString());
	root->AddChild("defpath",m_defpath.GetString());
#ifdef INCLUDEWIG
	root->AddChild("luapath",m_luapath.GetString());
	root->AddChild("wigpath",m_wigpath.GetString());
#endif
	root->AddChild("distancetype",m_disttype.GetSelectionString());
	root->AddChild("neardistance",m_neardist.GetString());
	root->AddChild("realtimegps",m_realtimegps.GetSelectionString());

	root->AddChild("clipfoundlogs",m_clipfoundlogs);
	root->AddChild("clipnotfoundlogs",m_clipnotfoundlogs);
	root->AddChild("basic",m_basicsource.GetString());

	/* save basic macro buttons */
	if(m_nummacrobuttons)
	{
		kGUIXMLItem *group;

		group=root->AddChild("macrobuttons");
		for(i=0;i<m_nummacrobuttons;++i)
			m_macrobuttons.GetEntryPtr(i)->Save(group->AddChild("macrobutton"));
	}

	root->AddChild("labelfontsize",m_labelfontsize.GetInt());
	root->AddChild("labelalpha",m_labelalpha.GetSelection());
	root->AddChild("tablefontsize",m_tablefontsize.GetInt());
	root->AddChild("maxdownloads",m_maxdownloads.GetInt());
	root->AddChild("shownumticks",m_shownumticks.GetInt());
	root->AddChild("usebrowser",m_usebrowser.GetSelected()==true?"1":"0");
	root->AddChild("movelabels",m_movelabels.GetSelected()==true?"1":"0");
	root->AddChild("movelabelsmax",m_movelabelsmax.GetInt());
	root->AddChild("labelzoomsize",m_labelzoomsize.GetInt());
	root->AddChild("wptname",m_wptname.GetString());

	root->AddChild("showchildwpts",m_showchildren.GetSelected()==true?"1":"0");

	root->AddChild("split",m_split);
	root->AddChild("splitfontsize",m_splitfontsize);
	root->AddChild("splitsort",m_splitsort);
	root->AddChild("splitwidth",m_splitwidth.GetString());
	root->AddChild("splitdesc",m_splitdesc.GetString());

	/* save filters */
	m_filters.SavePrefs(root);
	/* save GPSr definitions and current gpsr */
	m_gpsr.SavePrefs(root);
	if(busy)
		busy->SetCur(10);

	/* filter->color table */
	for(i=0;i<m_labelcolourtable.GetNumChildren(0);++i)
	{
		GPXWPRenderRow *rrow;
		obj=m_labelcolourtable.GetChild(i);
		rrow=static_cast<GPXWPRenderRow *>(obj);

		rrow->Save(root->AddChild("wptrender"));
	}

	/* mappaths table */
	for(i=0;i<m_mapdirstable.GetNumChildren(0);++i)
	{
		GPXMapPathRow *rrow;
		obj=m_mapdirstable.GetChild(i);
		rrow=static_cast<GPXMapPathRow *>(obj);
	
		/* don't save empty paths */
		if(rrow->GetPath()->GetLen())
			rrow->Save(root->AddChild("mappath"));
	}
	if(busy)
		busy->SetCur(20);

	m_stickers.SavePrefs(root);
	m_routes.SavePrefs(root);
	if(busy)
		busy->SetCur(30);
	m_lines.SavePrefs(root);
	m_tracks.SavePrefs(root);
	if(busy)
		busy->SetCur(40);
	m_notes.SavePrefs(root);
	m_download.SavePrefs(root);

	/* save default sort for filtered results table */
	{
		kGUIXMLItem *sort=root->AddChild("wptsort");
		kGUIXMLItem *sortcol;
		for(i=0;i<m_numsortcols;++i)
		{
			sortcol=sort->AddChild("entry");
			sortcol->AddParm("col",(int)i);
			sortcol->AddParm("name",wpcolnames[m_sortcols.GetEntry(i)]);
			sortcol->AddParm("rev",m_sortrevs.GetEntry(i)==true?"1":"0");
		}
	}

	/* save any column user hints */
	{
		kGUIXMLItem *hints=root->AddChild("userhints");
		kGUIXMLItem *hintscol;
		for(i=0;i<MAXUSERTICKS;++i)
		{
			if(m_userhints.GetEntry(i)->GetLen()>0)
			{
				hintscol=hints->AddChild("entry");
				hintscol->AddParm("col",(int)i);
				hintscol->AddParm("hint",m_userhints.GetEntry(i)->GetString());
			}
		}
	}

	/* save the full waypoint list */
	for(i=0;i<m_numwpts;++i)
	{
		if(busy)
			busy->SetCur(40+((i*40)/m_numwpts));
		m_wptlist.GetEntry(i)->Save(root->AddChild("wpt"));
	}

	m_browsersettings.Save(root);

	/* copy prefs to backup incase save crashes */

	if(busy)
		busy->SetCur(80);
	kGUI::FileCopy(PROFILEFILE,PROFILEFILEBACKUP);
	if(busy)
		busy->SetCur(90);

	if(xml.Save(PROFILEFILETEMP)==false)
	{
		if(busy)
			delete busy;
		if(showbusy)
			box=new kGUIMsgBoxReq(MSGBOX_OK,false,"Error saving file!");
	}
	else
	{
		if(busy)
			busy->SetCur(95);
		kGUI::FileCopy(PROFILEFILETEMP,PROFILEFILE);
		kGUI::FileDelete(PROFILEFILETEMP);
		if(busy)
			delete busy;
	}
}


void GPX::UpdateMacroButtons(void)
{
	unsigned int i;
	int oldy,newy,changey;
	MacroButton *bb;
	kGUIButtonObj *b;
	kGUITextObj *t;

	m_macrocontrols.DeleteChildren();
	m_macrocontrols.Reset();
	m_macrocontrols.SetRedo(true);

	if(m_nummacrobuttons)
	{
		m_macrocontrols.SetBorderGap(3);
		m_macrocontrols.SetObjectGap(8);

		t=new kGUITextObj();
		t->SetString("Macros:");
		t->SetSize(t->GetWidth()+8,t->GetLineHeight()+8);
		m_macrocontrols.AddObject(t);

		for(i=0;i<m_nummacrobuttons;++i)
		{
			bb=m_macrobuttons.GetEntryPtr(i);
			b=new kGUIButtonObj();
			if(bb->GetUseImage() && bb->GetImage()->IsValid())
			{
				kGUIImage *i;
	
				i=bb->GetImage();
				b->SetImage(i);
				b->SetSize(MIN(i->GetImageWidth(),64),MIN(i->GetImageHeight(),32));
			}
			else
			{
				b->SetString(bb->GetButtonText());
				b->SetSize(b->GetWidth()+8,b->GetLineHeight()+8);
			}
			b->SetHint(bb->GetButtonText());
			b->SetShowCurrent(false);	/* don't draw 'current' object frame on this button */
			bb->SetButton(b);
			m_macrocontrols.AddObject(b);
			b->SetEventHandler(this,CALLBACKNAME(MacroButtonEvent));
		}
	}
	else
	{
		m_macrocontrols.SetBorderGap(0);
		m_macrocontrols.SetObjectGap(0);
		m_macrocontrols.SetSize(0,0);
	}

	m_macrocontrols.SetRedo(false);

	/* move down tabs if need be */
	oldy=m_tabs.GetZoneY();
	newy=MAX(m_logo.GetZoneBY()-m_tabs.GetTabRowHeight(),m_macrocontrols.GetZoneBY());
	printf("mctop=%d,mcbot=%d,oldy=%d,newy=%d\n",m_macrocontrols.GetZoneY(),m_macrocontrols.GetZoneBY(),oldy,newy);
	if(oldy!=newy)
	{
		changey=oldy-newy;

		m_tabs.SetZoneY(newy);
		m_tabs.SetZoneH(m_tabs.GetZoneH()+changey);
		m_grid.SetZoneH(m_grid.GetZoneH()+changey);
		m_debug.SetZoneH(m_debug.GetZoneH()+changey);

		m_routes.Resize(changey);
		m_lines.Resize(changey);
		m_filters.Resize(changey);
		/* draw settings */
		m_drawsettingsarea.SetZoneH(m_drawsettingsarea.GetZoneH()+changey);
//		m_gpsr.Resize(changey);
		/* solver */
		m_stickers.Resize(changey);
		m_tracks.Resize(changey);
		m_notes.Resize(changey);
//		m_download.Resize(changey);
		m_basicoutput.SetZoneH(m_basicoutput.GetZoneH()+changey);
	}
}

/* run the macro */
void GPX::MacroButtonEvent(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		unsigned int i;
		MacroButton *bb;

		/* what button am i */
		for(i=0;i<m_nummacrobuttons;++i)
		{
			bb=m_macrobuttons.GetEntryPtr(i);
			if(bb->GetButton()==event->GetObj())
			{
				/* this is false if basic is already running */
				if(m_basicstart.GetEnabled()==true)
				{
					if(m_basic.Compile(&m_basicsource,&m_basicoutput)==true)
					{
						m_basicoutput.Clear();
						m_basicsource.SetLocked(true);
						m_basicstart.SetEnabled(false);
						m_basiccancel.SetEnabled(true);
						m_basic.Start(bb->GetFuncName()->GetString(),true);
					}
				}

				return;
			}
		}
	}
}

kGUIRenameDBReq::kGUIRenameDBReq()
{
	m_window.SetAllowButtons(WINDOWBUTTON_CLOSE);
	m_window.SetTitle("Rename Database");
	m_window.SetPos(kGUI::GetMouseX(),kGUI::GetMouseY());
	m_window.SetSize(800,200);
	kGUI::AddWindow(&m_window);
	m_window.SetEventHandler(this,CALLBACKNAME(WindowEvent));

	m_controls.SetPos(0,0);

	m_fromcaption.SetPos(0,0);
	m_fromcaption.SetFontSize(SMALLCAPTIONFONTSIZE);
	m_fromcaption.SetFontID(SMALLCAPTIONFONT);
	m_fromcaption.SetString("From");

	m_fromdb.SetPos(0,15);
	m_fromdb.SetSize(250,20);
	m_fromdb.SetHint("Select old database name.");

	{
		Hash names;			/* names are added to this list to check for duplicates */
		int def=0;
		HashEntry *he;
		unsigned int e;
		unsigned int num;
		GPXRow *row;

		names.Init(8,sizeof(def));

		for(e=0;e<gpx->m_numwpts;++e)
		{
			row=gpx->m_wptlist.GetEntry(e);
			if(!names.Find(row->GetDB()))
				names.Add(row->GetDB(),&def);
		}

		num=names.GetNum();
		m_fromdb.SetNumEntries(num);				/* list of filters */
		he=names.GetFirst();
		for(e=0;e<num;++e)
		{
			m_fromdb.SetEntry(e,he->m_string,e);
			he=he->GetNext();
		}
	}
	m_controls.AddObjects(2,&m_fromcaption,&m_fromdb);

	m_tocaption.SetPos(0,0);
	m_tocaption.SetFontSize(SMALLCAPTIONFONTSIZE);
	m_tocaption.SetFontID(SMALLCAPTIONFONT);
	m_tocaption.SetString("To");

	m_todb.SetPos(0,15);
	m_todb.SetSize(250,20);
	m_todb.SetHint("Select new database name.");

	m_controls.AddObjects(2,&m_tocaption,&m_todb);

	m_cancel.SetFontSize(11);
	m_cancel.SetPos(0,15);
	m_cancel.SetSize(70,20);
	m_cancel.SetString("Cancel");
	m_cancel.SetEventHandler(this,CALLBACKNAME(Cancel));
	m_controls.AddObjects(1,&m_cancel);

	/* if no from databases then don't allow rename */
	if(!m_fromdb.GetNumEntries())
		m_rename.SetEnabled(false);

	m_rename.SetFontSize(11);
	m_rename.SetPos(0,15);
	m_rename.SetSize(70,20);
	m_rename.SetString("Rename");
	m_rename.SetEventHandler(this,CALLBACKNAME(Rename));
	m_controls.AddObjects(1,&m_rename);

	m_window.AddObject(&m_controls);
	m_window.Shrink();
}

void kGUIRenameDBReq::WindowEvent(kGUIEvent *event)
{
	switch(event->GetEvent())
	{
	case EVENT_CLOSE:
		delete this;
	break;
	}
}

void kGUIRenameDBReq::DoRename(void)
{
	unsigned int e;
	const char *from;
	GPXRow *row;

	from=m_fromdb.GetSelectionString();

	for(e=0;e<gpx->m_numwpts;++e)
	{
		row=gpx->m_wptlist.GetEntry(e);
		if(!strcmp(from,row->GetDB()))
			row->SetDB(&m_todb);
	}

	gpx->UpdateDBList();
	gpx->ReFilter();
}


kGUIRenameDBReq::~kGUIRenameDBReq()
{
}

/*********************************************************************/

void AppInit(void)
{
	umask(0);

	kGUIXMLCODES::Init();

#if 0
	//testing smaller resolutions
	g_fullw=640;
	g_fullh=480;
#else
	g_fullw=kGUI::GetFullScreenWidth();
	g_fullh=kGUI::GetFullScreenHeight();
#endif

	if(g_fullw>1024)
	{
		kGUI::SetDefFontSize(11);
		GPX::SetAdjust(1.0f);
	}
	else if(g_fullw>=800)
	{
		kGUI::SetDefFontSize(9);
		GPX::SetAdjust(0.85f);
	}
	else
	{
		kGUI::SetDefFontSize(8);
		GPX::SetAdjust(0.75f);
	}
	kGUI::SetDefReportFontSize(11);
	assert((sizeof(shapenames)/sizeof(char *))==SHAPE_NUMSHAPES,"Shapes table and defines list not sync'd!");

	kGUI::SetCookieJar(new kGUICookieJar());

	gpx=new GPX();
	gpx->PreInit();
}

/* panic string contains callstack as well as error */
void GPX::Panic(kGUIString *error)
{
	SavePrefs(false);
}

void AppClose(void)
{
	kGUI::SetPanic(0,0);
	gpx->SavePrefs(false);
	delete gpx;
	MSGPXFName::Purge();
	MSGPXMap::Purge();
	kGUIXMLCODES::Purge();

	delete kGUI::GetCookieJar();
}

#define MAXDEBUGLEN 1024

void DebugPrint(const char *message,...)
{
	char fmessage[MAXDEBUGLEN];
	va_list args;

    va_start(args, message);
    _vsnprintf(fmessage, MAXDEBUGLEN, message, args);
    va_end(args);
	gpx->m_debug.Append(fmessage);
}

// garmin poi format csv files 
//Column1: Longitude
//Column2: Latitude
//Column3: POI Name (39 Charachters Max)
//Column4: POI Details (256 Charachters)

void GPX::PreLoadXML(int current,int size)
{
	if(!m_busy)
	{
		m_busy=new kGUIBusy(kGUI::GetScreenWidth()>>1);
		m_busy->GetTitle()->SetString("Loading GPX File");
		m_busy->SetMax(size);
	}
	m_busy->SetCur(current);
}

/* convert true/false from database to boolean true/false */
bool myatob(const char *p)
{
	if(!stricmp(p,"true"))
		return(true);
	if(!stricmp(p,"false"))
		return(false);

	if(p[0]=='1')
		return(true);
	return(false);
}

bool GPX::Get(kGUIXMLItem *i,const char *name,int *var)
{
	kGUIXMLItem *c;

	c=i->Locate(name);
	if(!c)
		return(false);
	*var=c->GetValueInt();
	return(true);
}

bool GPX::Get(kGUIXMLItem *i,const char *name,bool *var)
{
	kGUIXMLItem *c;

	c=i->Locate(name);
	if(!c)
		return(false);
	*var=myatob(c->GetValueString());
	return(true);
}

bool GPX::Get(kGUIXMLItem *i,const char *name,kGUIString *var)
{
	kGUIXMLItem *c;

	c=i->Locate(name);
	if(!c)
		return(false);
	var->SetString(c->GetValue());
	return(true);
}

bool GPX::GetI(kGUIXMLItem *i,const char *name,kGUIComboBoxObj *var)
{
	kGUIXMLItem *c;

	c=i->Locate(name);
	if(!c)
		return(false);

	var->SetSelectionz(c->GetValueInt());
	return(true);
}

bool GPX::Get(kGUIXMLItem *i,const char *name,kGUIComboBoxObj *var)
{
	kGUIXMLItem *c;

	c=i->Locate(name);
	if(!c)
		return(false);

	var->SetSelectionz(c->GetValueString());
	return(true);
}

bool GPX::Get(kGUIXMLItem *i,const char *name,kGUITickBoxObj *var)
{
	kGUIXMLItem *c;

	c=i->Locate(name);
	if(!c)
		return(false);
	var->SetSelected(myatob(c->GetValueString()));
	return(true);
}

void GPX::Browse(int mode,kGUIString *s)
{
	switch(mode)
	{
	case BROWSE_URL:
		if(!m_usebrowser.GetSelected())
			kGUI::FileShow(s->GetString());
		else
		{
			HtmlWindow *h;

			h=new HtmlWindow(BROWSE_URL,s);
		}
	break;
	case BROWSE_DATA:
		if(!m_usebrowser.GetSelected())
		{
			DataHandle dh;

			dh.SetFilename("temp.html");
			dh.OpenWrite("wb");
			dh.Write(s->GetString(),s->GetLen());
			dh.Close();
			kGUI::FileShow("temp.html");
		}
		else
		{
			HtmlWindow *h;

			h=new HtmlWindow(BROWSE_DATA,s);
		}
	break;
	}
}

#ifdef INCLUDEWIG
void GPX::ClickLoadLua(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		kGUIFileReq *req;

		req=new kGUIFileReq(FILEREQ_OPEN,m_luapath.GetString(),".lua",this,CALLBACKNAME(DoLoadLua));
	}
}

void GPX::DoLoadLua(kGUIFileReq *result,int pressed)
{
	if(pressed==MSGBOX_OK)
	{
		DataHandle dh;

		/* load the cart */
		dh.SetFilename(result->GetFilename());
		if(dh.Open())
		{
			dh.Read(&m_luasource,dh.GetSize());
			dh.Close();
			/* save default path for next time */
			m_luapath.SetString(result->GetPath());
		}
		else
		{
			/* error not a valid cart */
		}
	}
}

void GPX::ClickLoadWig(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		kGUIFileReq *req;

		req=new kGUIFileReq(FILEREQ_OPEN,m_wigpath.GetString(),".gwc",this,CALLBACKNAME(DoLoadWig));
	}
}

void GPX::DoLoadWig(kGUIFileReq *result,int pressed)
{
	if(pressed==MSGBOX_OK)
	{
		DataHandle dh;

		/* load the cart */
		dh.SetFilename(result->GetFilename());
		if(m_wigcart.Load(&dh,&m_wiginfo))
		{
			/* save default path for next time */
			m_wigpath.SetString(result->GetPath());
		}
		else
		{
			/* error not a valid cart */
		}
	}
}

void GPX::ClickPlayWig(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		m_wigcart.Play();
	}
}

void GPX::MoveLuaDivider(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_AFTERUPDATE)
	{
		int move=event->m_value[0].i;
		int currenty=m_luadivider.GetZoneY();
		int newy;

		newy=currenty+move;
		if(newy<100)
			newy=100;
		else if(newy>(m_tabs.GetChildZoneH()-100))
			newy=m_tabs.GetChildZoneH()-100;

		if(newy!=currenty)
		{
			int changey=currenty-newy;

			/* adjust size of top box */
			m_luasource.SetZoneH(m_luasource.GetZoneH()-changey);
			/* adjust position of divider */
			m_luadivider.SetZoneY(m_luadivider.GetZoneY()-changey);
			/* adjust position and size of bottom box */
			m_luaoutput.SetZoneY(m_luaoutput.GetZoneY()-changey);
			m_luaoutput.SetZoneH(m_luaoutput.GetZoneH()+changey);
		}
	}
}

void GPX::StartLua(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		m_luaoutput.Clear();
		m_lua.Start(m_luasource.GetString(),m_luasource.GetLen());
	}
}

void GPX::LuaCancel(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{

	}
}

#endif

void MacroButton::Load(kGUIXMLItem *xml)
{
	gpx->Get(xml,"funcname",&m_funcname);
	gpx->Get(xml,"buttontext",&m_buttontext);
	gpx->Get(xml,"useimage",&m_useimage);

	if(xml->Locate("base64image"))
	{
		unsigned int j;
		unsigned int binarylen;
		kGUIString *b;
		Array<unsigned char>base64image;
		Array<unsigned char>binaryimage;

		/* convert from base64 encoded to binary */
		b=xml->Locate("base64image")->GetValue();
		
		base64image.Init(b->GetLen(),1);
		for(j=0;j<b->GetLen();++j)
			base64image.SetEntry(j,b->GetChar(j));

		binarylen=kGUI::Base64Decode(b->GetLen(),&base64image,&binaryimage);

		m_image.SetMemory();
		m_image.OpenWrite("wb",binarylen);
		m_image.Write(binaryimage.GetArrayPtr(),(unsigned long)binarylen);
		m_image.Close();
	}
}

void MacroButton::Save(kGUIXMLItem *xml)
{
	xml->AddChild("funcname",&m_funcname);
	xml->AddChild("buttontext",&m_buttontext);
	xml->AddChild("useimage",m_useimage==true?1:0);

	/* is this a valid image */
	if(m_image.IsValid() && m_image.GetLoadableSize())
	{
		unsigned int j;
		unsigned int binarylen;
		unsigned int base64len;
		unsigned char c;
		kGUIString bs;
		Array<unsigned char>binaryimage;
		Array<unsigned char>base64image;
		
		binarylen=m_image.GetLoadableSize();

		/* fill the binary array */
		m_image.Open();
		binaryimage.Init(binarylen,1);
		for(j=0;j<binarylen;++j)
		{
			m_image.Read(&c,(unsigned long)1L);
			binaryimage.SetEntry(j,c);
		}
		m_image.Close();
		
		base64len=kGUI::Base64Encode(binarylen,&binaryimage,&base64image);
		bs.SetString((const char *)base64image.GetArrayPtr(),base64len);

		xml->AddParm("base64image",&bs);
	}
}

EditButtonRowObj::EditButtonRowObj(EditButtonWindowObj *w,MacroButton *bb)
{
	unsigned int i;

	m_funclist.SetNumEntries(w->m_numsubs);
	for(i=0;i<w->m_numsubs;++i)
		m_funclist.SetEntry(i,w->m_sublist.GetEntryPtr(i),i);

	m_browse.SetFontSize(25);	/* make bigger */
	m_browse.SetFontID(1);	/* BOLD */
	m_browse.SetString("...");
	m_objptrs[EBRCOL_FUNCNAME]=&m_funclist;
	m_objptrs[EBRCOL_BUTTONTEXT]=&m_buttontext;
	m_objptrs[EBROL_USEIMAGE]=&m_useimage;
	m_objptrs[EBRCOL_IMAGE]=&m_image;
	m_objptrs[EBRCOL_BROWSE]=&m_browse;

	if(bb)
	{
		m_funclist.SetSelectionz(bb->GetFuncName()->GetString());
		m_buttontext.SetString(bb->GetButtonText());
		m_useimage.SetSelected(bb->GetUseImage());
		m_image.Copy(bb->GetImage());
	}
	m_browse.SetEventHandler(this,CALLBACKNAME(Browse));
	SetRowHeight(24);
}

void EditButtonRowObj::Browse(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		kGUIFileReq *fr;

		fr=new kGUIFileReq(FILEREQ_OPEN,"",".jpg;.gif;.bmp;.png",this,CALLBACKNAME(GotFilename));
	}
}

void EditButtonRowObj::GotFilename(kGUIFileReq *fr,int pressed)
{
	if(pressed==MSGBOX_OK)
	{
		m_image.SetFilename(fr->GetFilename());
		m_useimage.SetSelected(m_image.IsValid());
	}
}

#if 0
void EditButtonRowObj::ChangeShow(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_AFTERUPDATE)
		m_t->SetColShow(m_xcol,m_tick.GetSelected());
}

void EditButtonRowObj::ChangeWidth(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_AFTERUPDATE)
		m_t->SetColWidth(m_xcol,m_width.GetInt());
}
#endif

EditButtonWindowObj::EditButtonWindowObj(unsigned int numsubs,ClassArray<kGUIString>*sublist)
{
	unsigned int i;

	m_numsubs=numsubs;
	m_sublist.Init(numsubs,0);
	for(i=0;i<numsubs;++i)
		m_sublist.GetEntryPtr(i)->SetString(sublist->GetEntryPtr(i));

	m_up.SetPos(10,10);
	m_up.SetString(gpx->GetString(STRING_UP));
	m_up.Contain();
	m_up.SetEventHandler(this,CALLBACKNAME(PressUp));
	m_window.AddObject(&m_up);

	m_down.SetString("Down");
	m_down.SetPos(80,10);
	m_down.SetSize(60,20);
	m_down.SetEventHandler(this,CALLBACKNAME(PressDown));
	m_window.AddObject(&m_down);

	m_cancel.SetString("Cancel");
	m_cancel.SetPos(150,10);
	m_cancel.SetSize(60,20);
	m_cancel.SetEventHandler(this,CALLBACKNAME(PressCancel));
	m_window.AddObject(&m_cancel);

	m_save.SetString("Save");
	m_save.SetPos(220,10);
	m_save.SetSize(60,20);
	m_save.SetEventHandler(this,CALLBACKNAME(PressSave));
	m_window.AddObject(&m_save);

	m_table.SetPos(10,40);
	m_table.SetNumCols(EBRCOL_NUMCOLS);
	m_table.SetColTitle(EBRCOL_FUNCNAME,"Function");
	m_table.SetColTitle(EBRCOL_BUTTONTEXT,"Button Text");
	m_table.SetColTitle(EBROL_USEIMAGE,"Use Image");
	m_table.SetColTitle(EBRCOL_IMAGE,"Image");
	m_table.SetColTitle(EBRCOL_BROWSE,"Browse");

	m_table.SetColWidth(EBRCOL_FUNCNAME,200);
	m_table.SetColWidth(EBRCOL_BUTTONTEXT,100);
	m_table.SetColWidth(EBROL_USEIMAGE,60);
	m_table.SetColWidth(EBRCOL_IMAGE,100);
	m_table.SetColWidth(EBRCOL_BROWSE,45);
	m_table.SetSize(m_table.CalcTableWidth(),400);
	m_window.AddObject(&m_table);
	m_table.SetEventHandler(this,CALLBACKNAME(TableEvent));
	m_table.SetAllowAddNewRow(true);

	for(i=0;i<gpx->m_nummacrobuttons;++i)
	{
		EditButtonRowObj *tcr;

		tcr=new EditButtonRowObj(this,gpx->m_macrobuttons.GetEntryPtr(i));
		m_table.AddRow(tcr);
	}

	m_window.SetSize(450,480);
	m_window.ExpandToFit();
	m_window.Center();
	m_window.SetTop(true);
	m_window.SetEventHandler(this,CALLBACKNAME(WindowEvent));
	kGUI::AddWindow(&m_window);
}

void EditButtonWindowObj::WindowEvent(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_CLOSE)
	{
		m_table.DeleteChildren();
		delete this;
	}
}

void EditButtonWindowObj::TableEvent(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_ADDROW)
	{
		EditButtonRowObj *er=new EditButtonRowObj(this,0);
		m_table.AddRow(er);
	}
}


void EditButtonWindowObj::PressUp(kGUIEvent *event)
{
	unsigned int line;

	if(event->GetEvent()==EVENT_PRESSED)
	{
		line=m_table.GetCursorRow();
		if(line>0 && line<m_table.GetNumberRows())
		{
			m_table.SwapRow(-1);
			m_table.MoveRow(-1);
		}
	}
}

void EditButtonWindowObj::PressDown(kGUIEvent *event)
{
	unsigned int line;

	if(event->GetEvent()==EVENT_PRESSED)
	{
		line=m_table.GetCursorRow();
		if(line<(m_table.GetNumberRows()-1))
		{
			m_table.SwapRow(1);
			m_table.MoveRow(1);
		}
	}
}

void EditButtonWindowObj::PressCancel(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
		m_window.Close();
}

void EditButtonWindowObj::PressSave(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		unsigned int i,nr;
		EditButtonRowObj *tr;
		MacroButton *bb;

		nr=m_table.GetNumChildren();
		gpx->m_nummacrobuttons=nr;
		for(i=0;i<nr;++i)
		{
			tr=static_cast<EditButtonRowObj *>(m_table.GetChild(i));
			bb=gpx->m_macrobuttons.GetEntryPtr(i);

			bb->SetFuncName(tr->GetFuncName());
			bb->SetButtonText(tr->GetButtonText());
			bb->SetUseImage(tr->GetUseImage());
			bb->SetImage(tr->GetImage());
		}
		gpx->UpdateMacroButtons();

		m_window.Close();
	}
}

