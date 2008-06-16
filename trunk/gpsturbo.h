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

#ifndef __GPSTURBO__
#define __GPSTURBO__

/* kgui library includes */
#include "kgui.h"
#include "kguigrid.h"
#include "kguireq.h"
#include "kguixml.h"
#include "kguibsp.h"
#include "kguidl.h"
#include "kguibasic.h"
#include "kguibrowse.h"
#include <math.h>

/* my other classes */
#include "gcoords.h"

/* map classes */
#include "map.h"
#include "gmap.h"
#include "acmap.h"
#include "ozmap.h"
#include "msmap.h"
#include "tsmap.h"

enum
{
FONT_ARIAL,
FONT_ARIALBOLD,
FONT_COURIER
};

#define MAXUSERTICKS 8

/* if adding types please also update: */
/* the name table 'containernames' in gpsturbo.cpp */
/* and the constants for basic in basicclasses.cpp */

enum
{
CONTAINERTYPE_MICRO,
CONTAINERTYPE_SMALL,
CONTAINERTYPE_REGULAR,
CONTAINERTYPE_LARGE,
CONTAINERTYPE_OTHER,
CONTAINERTYPE_VIRTUAL,
CONTAINERTYPE_NOTCHOSEN,
CONTAINERTYPE_NUM};

/* if adding types please also update: */
/* the name table 'cachetypenames' in gpsturbo.cpp */
/* the shape table 'shapenames' in gpsturbo.cpp */
/* and SHAPE_xxxxx in thi file too */
/* and the constants for basic in basicclasses.cpp */

enum
{
CACHETYPE_UNDEFINED,
CACHETYPE_TRADITIONAL,
CACHETYPE_MULTI,
CACHETYPE_VIRTUAL,
CACHETYPE_EVENT,
CACHETYPE_MEGAEVENT,
CACHETYPE_UNKNOWN,
CACHETYPE_REVERSE,
CACHETYPE_EARTH,
CACHETYPE_NGS,
CACHETYPE_LETTERBOX,
CACHETYPE_WEBCAM,
CACHETYPE_CITO,
CACHETYPE_PROJECTAPE,
CACHETYPE_WHERIGO,
CACHETYPE_MAZE,
CACHETYPE_OTHER,
CACHETYPE_NUM};	

enum
{
GPXCOL_NA,
GPXCOL_FOUND,
GPXCOL_TYPE,
GPXCOL_USER1,
GPXCOL_USER2,
GPXCOL_USER3,
GPXCOL_USER4,
GPXCOL_USER5,
GPXCOL_USER6,
GPXCOL_USER7,
GPXCOL_USER8,
GPXCOL_NAME,
GPXCOL_WPTNAME,
GPXCOL_LOG,
GPXCOL_CONTAINER,
GPXCOL_DIFFICULTY,
GPXCOL_TERRAIN,
GPXCOL_DIST,
GPXCOL_CORRECTED,
GPXCOL_LAT,
GPXCOL_LON,
GPXCOL_HINT,
GPXCOL_USERNOTES,
GPXCOL_OWNER,
GPXCOL_STATE,
GPXCOL_COUNTRY,
GPXCOL_GENDATE,
GPXCOL_NUMNEAR,
GPXCOL_NUMCOLUMNS
};

#define WPTNAMEFONTSIZE 9

/* used for each row and for each child */

class GPXLabel : public kGUIBSPRectEntry, public kGUIText
{
public:
	GPXLabel() {m_row=0;m_draw=false;m_wasdrawn=-1;SetFontSize(WPTNAMEFONTSIZE);}
	void SetRow(class GPXRow *row) {m_row=row;}
	void Draw(int cxpix,int cypix);
	bool m_draw;
	GPXCoord m_loc;
	int m_icon;
	int m_origx,m_origy;	/* original position before moved ( if moved ) */
	int m_lw,m_lh;
	int m_wasdrawn;
	kGUIColor m_colour;	/* rectangle color */
	kGUIColor m_tcolour;	/* text color */
	class GPXRow *m_row;

	void StringChanged(void) {Changed();}
	void FontChanged(void) {Changed();}
private:
	void Changed(void);
};

class GPXChild
{
public:
	kGUIString *GetWptName(void) {return &m_wptname;}
	kGUIString *GetType(void) {return &m_type;}
	kGUIString *GetName(void) {return &m_name;}
	double GetLat(void) {return m_loc.GetLat();}
	double GetLon(void) {return m_loc.GetLon();}
	GPXCoord *GetPosPtr(void) {return &m_loc;} 
	void SetWptName(kGUIString *wptname) {m_wptname.SetString(wptname);}
	void SetType(kGUIString *type) {m_type.SetString(type);}
	void SetName(kGUIString *name) {m_name.SetString(name);}
	void SetPos(double lat,double lon) {m_loc.Set(lat,lon);}
	void SetPos(GPXCoord *c) {m_loc.Set(c);}
	void SetLat(double lat) {return m_loc.SetLat(lat);}
	void SetLon(double lon) {return m_loc.SetLon(lon);}
	GPXLabel m_label;
private:
	kGUIString m_wptname;	/* xxyyyy */
	kGUIString m_type;		/* parking coord, trailhead etc */		
	kGUIString m_name;
	GPXCoord m_loc;
};

#define USESHAREDCOMBOS 1

class GPXRow : public kGUITableRowObj
{
	friend class GPX;
	friend class GPXFilter;
	friend class GPSrPage;
	friend class GridLine;
	friend class GPXRecordSet;
	friend class UploadXML;
	friend class PrintMap;
public:
	GPXRow();
	GPXRow(kGUIString *ld,kGUIXMLItem *wp);
	void DelChildren(void);
	~GPXRow();
	void Copy(GPXRow *copy,bool copyall=false);
	void Init(void);
	void Load(kGUIString *ld,kGUIXMLItem *wp);
	void Save(kGUIXMLItem *wp, bool gpx=false);
	void AddChild(kGUIString *wptname,kGUIString *type,kGUIString *name,double lon,double lat);
	GPXRow *GetCopiedFrom(void) {return m_copiedfrom;}
	int GetNumObjects(void) {return GPXCOL_NUMCOLUMNS;}
	kGUIObj **GetObjectList(void) {return m_objectlist;} 
	kGUIObj *m_objectlist[GPXCOL_NUMCOLUMNS];
	void SetNA(bool f) {m_na.SetSelected(f);}
	bool GetFound(void) {return m_found.GetSelected();}
	void SetFound(bool f) {m_found.SetSelected(f);}
	bool GetUser(int index) {return m_user[index].GetSelected();}
	void SetUser(int index,bool f) {m_user[index].SetSelected(f);}
	void SetCorrected(bool f) {m_corrected.SetSelected(f);}
	const char *GetGenDate(void) {return m_gendate.GetString();}
	void SetGenDate(kGUIString *gdate) {m_gendate.SetString(gdate);}
	void SetType(int type) {m_xtype=type;m_label.m_icon=type;}
	kGUIString *GetURL(void) {return &m_url;}
	void SetURL(kGUIString *url) {m_url.SetString(url);}
	const char *GetWptName(void) {return m_wptname.GetString();}
	void SetWptName(kGUIString *name) {m_wptname.SetString(name);}
	const char *GetName(void) {return m_name.GetString();}
	void SetName(kGUIString *name) {m_name.SetString(name);}
	void UpdateFontSize(void);
	void UpdateLabelName(void);
	void SetDist(double dist) {m_dist.Sprintf("%.02f",dist);m_distval=dist;}
	double GetDist(void) {return m_distval;}
	void SetNumNear(int n)  {m_numnear.Sprintf("%d",n);m_nearval=n;}
	GPXCoord *GetCoord(void) {return &m_llcoord;}
	void SetCoord(double lat,double lon) {m_llcoord.Set(lat,lon);m_llcoord.Output(&m_lat,&m_lon);}

	void GetPos(GPXCoord *c) {c->Set(&m_llcoord);}
	double GetLat(void) {return m_llcoord.GetLat();}
	double GetLon(void) {return m_llcoord.GetLon();}
	void SetLon(kGUIString *lon) {m_lon.SetString(lon);}
	void SetLat(kGUIString *lat) {m_lat.SetString(lat);}

	void SetShortDesc(kGUIString *desc) {m_shortdesc.SetString(desc);}
	void SetDesc(kGUIString *desc) {m_desc.SetString(desc);}
	void SetHint(kGUIString *hint) {m_hint.SetString(hint);}
	void SetDB(kGUIString *db) {m_db.SetString(db);}
	const char *GetDB(void ) {return m_db.GetString();}
	void CalcHeight(void);
	void SetColour(kGUIColor c) {m_label.m_colour=c;m_wptname.SetBGColor(c);}
	void SetTColour(kGUIColor c) {m_label.m_tcolour=c;m_wptname.SetColor(c);}
	kGUIColor GetColour(void) {return m_label.m_colour;}
	kGUIColor GetTColour(void) {return m_label.m_tcolour;}
	void CopyColours(GPXRow *copy) {SetColour(copy->GetColour());SetTColour(copy->GetTColour());}
	int GetType(void) {return m_xtype;}
	void SetLabelDraw(bool d);
	bool GetLabelDraw(void) {return m_label.m_draw;}

	/* these are used in the route code for sorting the map labels into route order */
	void SetIndex(int index) {m_index=index;}
	int GetIndex(void) {return m_index;}

	/* called by lines code for set_waypoint so needs to be public */
	CALLBACKGLUEPTR(GPXRow,LocChanged,kGUIEvent)
	void LocChanged(kGUIEvent *event);

private:
	CALLBACKGLUEPTR(GPXRow,NameChanged,kGUIEvent)
	CALLBACKGLUEPTR(GPXRow,CorrChangedEvent,kGUIEvent)
	CALLBACKGLUEPTR(GPXRow,EditGenDate,kGUIEvent)
	CALLBACKGLUEPTR(GPXRow,EditGenDateDone,kGUIDate)
	void SubMenu(void);
	void GotoOnMap(void);
	void NameChanged(kGUIEvent *event);
	void CorrChangedEvent(kGUIEvent *event);
	void UpdateLogColors(void);
	void EditGenDate(kGUIEvent *event);
	void EditGenDateDone(kGUIDate *date);
	void CorrChanged(void);

	GPXChild *LocateChild(const char *type,bool create=true);
	GPXRow *m_copiedfrom;	/* points to entry in full table from route or filtered table */
	int m_xtype;
	int m_index;
	kGUITickBoxObj m_na;
	kGUITickBoxObj m_found;
	kGUIImageRefObj m_typeshape;
	kGUITickBoxObj m_user[MAXUSERTICKS];
	kGUIInputBoxObj m_name;
	kGUIInputBoxObj m_url;
	kGUIInputBoxObj m_wptname;
	kGUITickBoxObj m_corrected;

	kGUIInputBoxObj m_shortname;
#if USESHAREDCOMBOS
	kGUISharedComboboxObj m_container;
#else
	kGUIComboBoxObj m_container;
#endif
	kGUIInputBoxObj m_difficulty;
	kGUIInputBoxObj m_terrain;
	kGUIInputBoxObj m_dist;
	kGUIInputBoxObj m_numnear;
	kGUIInputBoxObj m_lat;
	kGUIInputBoxObj m_lon;
	kGUIInputBoxObj m_state;
	kGUIInputBoxObj m_country;
	kGUIInputBoxObj m_hint;
	kGUIInputBoxObj m_owner;
	kGUIInputBoxObj m_log;
	kGUIInputBoxObj m_usernotes;
	kGUIInputBoxObj m_gendate;
	bool m_haslogs;
	kGUIXMLItem m_logs;

	kGUIInputBoxObj m_placedby;

	kGUIString m_placeddate;	/* not displayed */
	kGUIString m_db;			/* last database this was attached to */
	kGUIInputBoxObj m_shortdesc;
	kGUIInputBoxObj m_desc;
	GPXCoord m_llcoord;
	double m_distval;
	int m_nearval;
	GPXLabel m_label;

	/* child waypoints */
	unsigned int m_numchildren;
	Array<GPXChild *>m_children;

	bool m_purge;	/* used by the basic interface only */
};


/***********************************************************/


#define MINZOOM 3
#define MAXZOOM 19

/* cache shapes */
enum
{
SHAPE_USER,	/* undefined */
SHAPE_TRCA,	/* tranditional */
SHAPE_MUCA, /* multi */
SHAPE_VICA, /* virtual */
SHAPE_EVCA, /* event */
SHAPE_MECA, /* mega-event */
SHAPE_UNCA, /* unknown */
SHAPE_RVCA, /* locationless / reverse */
SHAPE_EACA, /* earth caches */
SHAPE_NGCA, /* NGS benchmarks */
SHAPE_LBCA, /* letterbox hybrid */
SHAPE_WCCA, /* webcam */
SHAPE_CICA,	/* cache in trash out event */
SHAPE_PAPE,	/* project APE */
SHAPE_WIGO,	/* WheriGo */
SHAPE_MAZE,	/* GPS Adventures Maze Exhibit */
SHAPE_OTHR,	/* other */
SHAPE_MARKER,	/* marker shape */
SHAPE_LOADINGIMAGE,
SHAPE_BROKENIMAGE,
SHAPE_NUMSHAPES};

class GPXMAPInfo
{
public:
	GPXMAPInfo(int type,const char *name,const char *fn) {m_type=type;m_name.SetString(name);m_filename.SetString(fn);}
	int m_type;
	kGUIString m_name;
	kGUIString m_filename;
};


#define MAXLL 64

typedef struct
{
	GPXRow *row;
	int y;
}ROWY_DEF;

typedef struct
{
	bool isfield;
	int index;
	int len;
}WNENT_DEF;

enum
{
GPSR_WAYPOINTS,
GPSR_TRACKS
};

enum
{
BROWSE_URL,
BROWSE_DATA
};

#include "routes.h"
#include "tracks.h"
#include "lines.h"
#include "filters.h"
#include "stickers.h"
#include "gpsrs.h"
#include "notes.h"

class GPX
{
	/* todo, remove all these and force public access only! */
	friend class GPXRow;
	friend class GPXFilter;
	friend class FiltersPage;
	friend class kGUIRenameDBReq;
	friend class GPXRecordSet;
	friend class RemoveStale;
	friend class LoadAs;
	friend class PrintMap;
public:
	GPX();
	~GPX();
	void PreInit(void);
	void AddMaps(const char *path);
	int FindMapz(const char *name);
	void UpdateMapMenu(void);
	void Init(void);
	int GetColNum(const char *colname);
	int GetColNumz(const char *colname);
	void LoadMapPaths(kGUIXML *xml,bool xmlstatus);
	void LoadPrefs(kGUIXML *xml,bool xmlstatus);
	void SavePrefs(void);
	bool IsOnline(void) {return m_online.GetSelected();}
	void SetScrollCenter(GPXCoord *c);
	void InitPopMenu(kGUIMenuColObj *menu,int numentries,int *entrylist);
	TracksPage *GetTrackObj(void) {return &m_tracks;}

//	CALLBACKGLUEVAL(GPX,GoogleUpdateLoaded,int);
	GPXRow *Locate(kGUIString *wptname);
	GPXRow *InTable(kGUITableObj *table,const char *wptname);
	void SearchMap(void);
	void CalcDists(void);
	void ReCalcDists(void);
	void ReCalcNear(void);
	void UpdateWPRender(void);
	void GetColour(GPXRow *row);
	void DrawGrid(int w,int h,int cx,int cy,int gridtype,double gridstep,int gridfontsize);

	void Browse(int mode,kGUIString *s);

	void StartBasic(kGUIEvent *event);
	kGUIBasic m_basic;
	kGUIInputBoxObj m_basicsource;
	kGUIButtonObj m_basicstart;
	kGUIButtonObj m_basiccancel;
	kGUIButtonObj m_basicaddbutton;
	kGUIMenuColObj m_basicstartmenu;
	kGUIDividerObj m_basicdivider;
	kGUIInputBoxObj m_basicoutput;
	
	kGUIInputBoxObj m_debug;
	kGUIString m_printmap;	/* name of printer to use for printing maps */
	kGUIString m_printgrid;	/* name of printer to use for printing grids */
	kGUIString m_printbrowser;	/* name of printer to use for printing with the browser */
	/* these are here because they are saved in the users preferences */
	int m_split;
	int m_splitfontsize;
	int m_splitsort;
	kGUIString m_splitwidth;
	kGUIString m_splitdesc;

	void DrawIcon(int icon,double scale,int x,int y) {m_shapes[icon].SetScale(scale,scale);m_shapes[icon].Draw(0,x,y);}
	int GetRow(GPXLabel *l);
	kGUIImage *GetShape(int s) {return m_shapes+s;}
	void DoLoad(kGUIXML *xml,const char *dbname,const char *filename);
	void DoLoadUserFound(kGUIXML *xml,int userslot);

	int GetNumShowUserTicks(void) {return m_numshowticks;}
	/* this tile has been loaded sucessfully so redraw it! */
	void GridDirty(int x,int y) {m_grid.FlushCell(x,y);}
	bool GetMapAsync(void) {return m_mapasync;}
	void SetMapAsync(bool a) {m_mapasync=a;}
	kGUIGridObj *GetGridPtr(void) {return &m_grid;}
	void AddCClasses(void);
	void SetUserHint(int usercol,kGUIString *hinttext);
	void SetClipFoundLogs(int n) {m_clipfoundlogs=n;}
	void SetClipNotFoundLogs(int n) {m_clipnotfoundlogs=n;}
	/* -1 = don't clip */
	int GetClipFoundLogs(void) {return m_clipfoundlogs;}
	int GetClipNotFoundLogs(void) {return m_clipnotfoundlogs;}
	static int SortEntry(const void *o1,const void *o2);

	int GetTableFontSize(void) {return m_tablefontsize.GetInt();}

#if USESHAREDCOMBOS
	kGUISharedComboEntries m_sharedcontainer;
#endif

//	bool CallBabel(kGUIBusy *busy,int argc, char *argv[]);
//	static int GetBabelInputFormats(int index,const char **name,const char **desc,const char **ext);

	static int GetIndex(const char *t,int num,const char **strings);
	static int GetIndexz(const char *t,int num,const char **strings,int nomatchval);

	static void InitHeadingCombo(kGUIComboBoxObj *box);
	static int GetHeadingIndex(const char *v);
	static void InitDistCombo(kGUIComboBoxObj *box);
	static int GetDistIndex(const char *v);
	static void InitColorCombo(kGUIComboBoxObj *box);
	static kGUIColor GetTableColor(int index);
	static kGUIColor GetTableTColor(int index);
	static unsigned int GetTableColorIndex(const char *c);
	static const char *GetTableColorName(int index);
	static void InitAlphaCombo(kGUIComboBoxObj *box);
	static void Project(double distance,int disttype,int model,double heading,int headtype,GPXCoord *from,GPXCoord *to);
	static void GetDistInfo(int distmode,int modelmode,double *r,double *v1,double *v2);
	static double GetMeters(double distance,int disttype);

	int GetMaxActiveDownloads(void) {return m_maxdownloads.GetInt();}

	int GetCurrentDist(void) {return m_disttype.GetSelection();}
	const char *GetCurrentDistString(void) {return m_disttype.GetSelectionString();}

	void MapRedraw(void) {m_grid.Dirty();}
	void MapDirty(void);
	GPXMap *m_curmap;
	void SetCurrentTab(int t) {m_tabs.SetCurrentTab(t);}

	void UpdateDBList() {m_filters.UpdateDBList();}

	void ReFilter() {m_filters.ReFilter();}
	void ReFilter(GPXRow *row) {m_filters.ReFilter(row);}
	FiltersPage *GetFilterObj(void) {return &m_filters;}
	GPSrPage *GetGPSrObj(void) {return &m_gpsr;}
	/* used by both main table and route table so needs to be public */
	CALLBACKGLUE(GPX,ColClick)
	void ColClick(void);

	unsigned int m_numwpts;
	Array<GPXRow *>m_wptlist;
	/* pointer to filtered results table that lives in m_filters */
	kGUITableObj *m_fwt;
	/* these are temporary variables used for save/load etc */
	Hash m_temphash;
	kGUIString m_tempfilename;
	kGUIString m_tempfiletype;
	kGUIXML *m_tempxml;
	CALLBACKGLUEVAL(GPX,LoadTracks,int)
	void LoadTracks(int pressed);
	void BSPDirty(void) {m_rebuildbsp=true;MapDirty();}

	/* if an unknown type was loaded then this gets set to true */
	bool m_badtype;
	kGUIString m_badtypename;

	/* this is the realtime tracking connection to GPSbabel */
	kGUIComboBoxObj m_realtimegps;
	class BabelGlue *m_realtime;
	int m_gpsdelay;		/* delay between getting position events */

	kGUIDate m_starttime;

	/* used to gracefully get vars from the prefs file since if they don't */
	/* exist then it leaves them with their previous values */
	static bool Get(kGUIXMLItem *i,const char *name,int *var);
	static bool Get(kGUIXMLItem *i,const char *name,bool *var);
	static bool Get(kGUIXMLItem *i,const char *name,kGUIString *var);
	static bool Get(kGUIXMLItem *i,const char *name,kGUIComboBoxObj *var);
	static bool Get(kGUIXMLItem *i,const char *name,kGUITickBoxObj *var);

	/* events */
	void MapDirtyEvent(kGUIEvent *event) {if(event->GetEvent()==EVENT_AFTERUPDATE)MapDirty();}
	void BSPDirtyEvent(kGUIEvent *event) {if(event->GetEvent()==EVENT_AFTERUPDATE)BSPDirty();}
	void LabelFontSizeChangedEvent(kGUIEvent *event) {if(event->GetEvent()==EVENT_AFTERUPDATE)LabelFontSizeChanged();}
	void TableFontSizeChangedEvent(kGUIEvent *event) {if(event->GetEvent()==EVENT_AFTERUPDATE)TableFontSizeChanged();}
	void ReCalcDistsEvent(kGUIEvent *event) {if(event->GetEvent()==EVENT_AFTERUPDATE)ReCalcDists();}
	void UpdateWPRenderEvent(kGUIEvent *event) {if(event->GetEvent()==EVENT_AFTERUPDATE)UpdateWPRender();}
	void LabelNamesChangedEvent(kGUIEvent *event) {if(event->GetEvent()==EVENT_AFTERUPDATE)LabelNamesChanged();}
	void ReCalcNearEvent(kGUIEvent *event) {if(event->GetEvent()==EVENT_AFTERUPDATE) ReCalcNear();}
	//void ReCalcDistEvent(kGUIEvent *event) {if(event->GetEvent()==EVENT_AFTERUPDATE) ReCalcDist();}
	double GetLabelAlpha(void) {return (m_labelalpha.GetSelection()/100.0f);}

	kGUIBrowseSettings m_browsersettings;
	kGUIHTMLItemCache m_browseritemcache;
	kGUIHTMLVisitedCache m_browservisitedcache;
	Hash m_xmlnamecache;
private:
	kGUIString m_newname;	/* used for saving filters, routes etc */
	void MoveDividerEvent(kGUIEvent *event);
	void MoveDivider(int delta);
	void WPTableEvent(kGUIEvent *event);

	void StartBasicMenuDone(kGUIEvent *event);
	void BasicDone(void);
	void BasicCancel(kGUIEvent *event);
	void BasicAddButton(kGUIEvent *event);
	void SetBasicCClasses(void);
	void BasicError(int s,int e) {m_basicsource.Activate();m_basicsource.Select(s,e);}
	void MoveBasicDivider(kGUIEvent *event);

	void UpdateCenter(kGUIEvent *event);
	void ZoomGoto(kGUIEvent *event);
	void ZoomIn(void);
	void ZoomOut(void);
	void ClickZoomIn(kGUIEvent *event);
	void ClickZoomOut(kGUIEvent *event);
	void ZoomChanged(void);
	void UpdateZoomButtons(void);

	void InitArt(void);
	/* settings */

	void InitSettings(void);
	unsigned int GetNumMapPaths(void) {return m_mapdirstable.GetNumChildren();}
	kGUIString *GetMapPath(unsigned int index);

	void SelectLoadTracks(void);
	

	int MapEntryUnderMouse(int mx,int my);
	void OverMap(int mx,int my);
	void DoMapMenu(kGUIEvent *event);
	void DoWPMenu(kGUIEvent *event);
	void BuildPage(DataHandle *dh,GPXRow *page,bool head,bool tail);
	void DoRCLabel(GPXLabel *label);
	void Online(kGUIEvent *event);
	void ChangeMapType(void);
	void ChangeMapTypeEvent(kGUIEvent *event) {if(event->GetEvent()==EVENT_AFTERUPDATE)ChangeMapType();}
	void LabelFontSizeChanged(void);
	void TableFontSizeChanged(void);
	void CompileLabelDefinition(kGUIString *in,int *numptr,Array<WNENT_DEF>*arrayptr);
	void ExpandLabel(GPXRow *row,kGUIString *in,kGUIString *def,int numptr,Array<WNENT_DEF>*arrayptr);
	void LabelNamesChanged(void);
	void PreDrawMap(void);
	void PostDrawMap(void);
	void DrawMapCell(kGUICellObj *cell);
	void DrawBSPMapCellPoints(int cxpix,int cypix);
	void TabChanged(kGUIEvent *event);
	void CheckSpecialKeys(void);
	void ShowFileMenu(kGUIEvent *event);
	void ShowHelpMenu(kGUIEvent *event);
	void ShowGeocaching(kGUIEvent *event);
	void DoFileMenu(kGUIEvent *event);
	void DoHelpMenu(kGUIEvent *event);
	void DoMenu(int selection);
	void DoColMenu(kGUIEvent *event);
	void PreLoadXML(int current,int size);
	void GetLoadSettings(kGUIFileReq *result,int pressed);
	void OpenLoadSettings(const char *fn,const char *defdb);
	void DoSave(kGUIFileReq *result,int pressed);
	void DoSave2(int pressed);
	void DoSaveAs(kGUIFileReq *result,int pressed);
	void DoSaveAs2(const char *fn,const char *ft);
	void DoSaveAs3(int pressed);
	void SaveXML(const char *fn);
	void DoLoadOther(kGUIFileReq *result,int pressed);
	void DoSaveMapShape(kGUIFileReq *result,int pressed);
	void NewMapPathEntry(kGUIEvent *event);
	void GPSConnectChanged(kGUIEvent *event);
	void DoPrintMap(void);
	void DoPrintTable(kGUITableObj *table);
	void DrawTextOutline(kGUIText *t,int x,int y,int s);
	void DrawTextRotOutline(kGUIText *t,int x,int y,int s,double heading);
	void SetHintDone(kGUIString *result,int closebutton);
	void GetAppBasicInstructions(kGUIString *h);

	/* generate a file for uploading to a GPSR */
	kGUIXMLItem *InitUploadXML(kGUIXML *xml);

	/* solver functions */ 
	void InitSolvers(void);
	void ChangeEncode(kGUIEvent *event);
	void SolveEncode(kGUIEvent *event);
	void SolveDecode(kGUIEvent *event);
	void SolveProject(kGUIEvent *event);
	void SolveDistance(kGUIEvent *event);
	void SolveSun(kGUIEvent *event);

	void LabelUp(kGUIEvent *event);
	void LabelDown(kGUIEvent *event);

	void SetDefDB(const char *defdb) {m_defdb.SetString(defdb);}
	kGUIString *GetDefDB(void) {return &m_defdb;}

	Array<ROWY_DEF>m_swptlist;	/* sorted by Label Y */

	RoutesPage m_routes;
	TracksPage m_tracks;
	LinesPage m_lines;
	FiltersPage m_filters;
	StickersPage m_stickers;
	GPSrPage m_gpsr;
	NotesPage m_notes;


	kGUIControlBoxObj m_labelcontrols;
	kGUIButtonObj m_labelup;
	kGUIButtonObj m_labeldown;
	kGUITableObj m_labelcolourtable;

	kGUITableObj m_mapdirstable;

	kGUIInputBoxObj m_wptname;
	kGUITextObj m_wptnamelabel;

	int m_wptnamenumentries;
	Array<WNENT_DEF> m_wptnameentries;

	kGUITickBoxObj m_showchildren;
	kGUITextObj m_showchildrenlabel;

	kGUIInputBoxObj m_maxdownloads;
	kGUITextObj m_maxdownloadslabel;

	kGUIInputBoxObj m_tablefontsize;
	kGUITextObj m_tablefontsizelabel;
	kGUIInputBoxObj m_labelfontsize;
	kGUITextObj m_labelfontsizelabel;
	kGUIComboBoxObj m_labelalpha;
	
	kGUIInputBoxObj m_shownumticks;
	int m_numshowticks;					/* this is the same as above just cached for speed */
	kGUITextObj m_shownumtickslabel;

	kGUITickBoxObj m_usebrowser;		/* move overlapping labels */
	kGUITextObj m_usebrowserlabel;	

	kGUITickBoxObj m_movelabels;		/* move overlapping labels */
	kGUITextObj m_movelabelslabel;	
	kGUIInputBoxObj m_movelabelsmax;
	kGUITextObj m_movelabelsmaxlabel;	
	kGUIInputBoxObj m_labelzoomsize;	/* stop drawing labels if level<this */
	kGUITextObj m_labelzoomsizelabel;
	kGUIComboBoxObj m_disttype;
	kGUITextObj m_distcaption;

	kGUIDividerObj m_divider;
	kGUIGridObj m_grid;
	kGUIImageObj m_logo;
	kGUITextObj m_defdb;
	kGUITextObj m_filemenulabel;
	kGUITextObj m_helpmenulabel;

	kGUITabObj m_tabs;
	kGUITabObj m_solvertabs;

	kGUIControlBoxObj m_solvdecodecontrols;
	kGUITextObj m_solvdecodeincaption;
	kGUIInputBoxObj m_solvdecodein;

	kGUIComboBoxObj m_solvdecodetype;
	kGUITextObj m_solvdecodekeycaption;
	kGUIInputBoxObj m_solvdecodekey;
	kGUIButtonObj m_solvdecodebuttonenc;
	kGUIButtonObj m_solvdecodebuttondec;

	kGUITextObj m_solvdecodeoutcaption;
	kGUIInputBoxObj m_solvdecodeout;

	kGUIControlBoxObj m_solvprojcontrols;
	kGUITextObj m_solvprojfromcaption;
	kGUIInputBoxObj m_solvprojfromlon;
	kGUIInputBoxObj m_solvprojfromlat;

	kGUITextObj m_solvprojdistcaption;
	kGUIInputBoxObj m_solvprojdistunits;
	kGUIComboBoxObj m_solvprojdisttype;

	kGUITextObj m_solvprojheadcaption;
	kGUIInputBoxObj m_solvprojheadunits;
	kGUIComboBoxObj m_solvprojheadtype;

	kGUITextObj m_solvprojearthmodelcaption;
	kGUIComboBoxObj m_solvprojearthmodeltype;

	kGUIButtonObj m_solvprojbutton;

	kGUITextObj m_solvprojtocaption;
	kGUIInputBoxObj m_solvprojtolon;
	kGUIInputBoxObj m_solvprojtolat;

	/* solver: distance / heading between two points */

	kGUIControlBoxObj m_solvdistcontrols;

	kGUITextObj m_solvdistfromcaption;
	kGUIInputBoxObj m_solvdistfromlon;
	kGUIInputBoxObj m_solvdistfromlat;

	kGUITextObj m_solvdisttocaption;
	kGUIInputBoxObj m_solvdisttolon;
	kGUIInputBoxObj m_solvdisttolat;

	kGUITextObj m_solvdistdistcaption;
	kGUIComboBoxObj m_solvdistdisttype;

	kGUITextObj m_solvdistheadcaption;
	kGUIComboBoxObj m_solvdistheadtype;

	kGUITextObj m_solvdistearthmodelcaption;
	kGUIComboBoxObj m_solvdistearthmodeltype;

	kGUIButtonObj m_solvdistbutton;

	kGUITextObj m_solvdistresultdistcaption;
	kGUIInputBoxObj m_solvdistdist;
	kGUITextObj m_solvdistresultheadcaption1;
	kGUIInputBoxObj m_solvdisthead1;
	kGUITextObj m_solvdistresultheadcaption2;
	kGUIInputBoxObj m_solvdisthead2;

	/***************************************/

	kGUIControlBoxObj m_solvsscontrols;

	kGUITextObj m_solvssfromcaption;
	kGUIInputBoxObj m_solvssfromlon;
	kGUIInputBoxObj m_solvssfromlat;

	kGUITextObj m_solvssdaycaption;
	kGUIInputBoxObj m_solvssday;
	kGUITextObj m_solvssmonthcaption;
	kGUIComboBoxObj m_solvssmonth;
	kGUITextObj m_solvssyearcaption;
	kGUIInputBoxObj m_solvssyear;

	kGUITextObj m_solvsstimecaption;
	kGUIComboBoxObj m_solvsstimetype;

	kGUITextObj m_solvsszenithcaption;
	kGUIComboBoxObj m_solvsszenithtype;


	kGUIButtonObj m_solvssbutton;

	kGUITextObj m_solvssrisecaption;
	kGUIInputBoxObj m_solvssrise;
	kGUITextObj m_solvsssetcaption;
	kGUIInputBoxObj m_solvssset;

	/*************************************/

	kGUIControlBoxObj m_maincontrols;
	kGUIControlBoxObj m_controls;
	kGUIControlBoxObj m_mapcontrols;

	kGUIImage m_zin;
	kGUIImage m_zout;
	kGUITextObj m_zoomcaption;
	kGUIComboBoxObj m_zoomgoto;
	kGUIButtonObj m_zoomin;
	kGUIButtonObj m_zoomout;

	/* online tickbox */
	kGUITextObj m_onlinecaption;
	kGUITickBoxObj m_online;

	kGUITextObj m_maptypecaption;
	kGUIComboBoxObj m_maptypes;
	
	int m_currowindex;
	GPXRow *m_currow;
	kGUIMenuColObj m_filemenu;
	kGUIMenuColObj m_helpmenu;
	kGUIMenuColObj m_wpmenu;
	kGUIMenuColObj m_rcmenu;
	kGUIMenuColObj m_colmenu;
	GPXLabel *m_llp[MAXLL];		/* pointers to labels for multiples under the mouse */

	kGUITextObj m_cposcaption;
	kGUIInputBoxObj m_clon;
	kGUIInputBoxObj m_clat;

	kGUITextObj m_nearcaption;
	kGUIInputBoxObj m_neardist;

	/* search the current map */
	kGUITextObj m_quicksearchmapcaption;
	kGUIInputBoxObj m_quicksearchmap;

	/* controls for the GPS18 connection */
	kGUITextObj m_gpscaption;

	kGUITickBoxObj m_gpsconnect;
	kGUITickBoxObj m_gpsfollow;
	GPXCoord m_gpspos;
	GPXCoord m_lastgpspos;
	kGUIDPoint2 m_gpsdeltas[10];
	double m_gpsangle;
	kGUIInputBoxObj m_gpslon;
	kGUIInputBoxObj m_gpslat;

	kGUITextObj m_ucaption;
	kGUIInputBoxObj m_ulon;
	kGUIInputBoxObj m_ulat;
	GPXCoord m_upos;	

	int m_mapsel;		// 0=none selected,1=ul,2=lr,3=both selected
	GPXCoord m_mapselul;	
	GPXCoord m_mapsellr;	

	GPXCoord m_center;	/* this is the numerical values for the clon and clat above */

	kGUIImage m_shapes[SHAPE_NUMSHAPES];

	int m_maptype;
	int m_wpttype;

	unsigned int m_numsortcols;
	Array<int>m_sortcols;
	Array<bool>m_sortrevs;	/* reverse flag */

	int m_zoom;

	bool m_onmap;	/* caches are drawn on the map */
//	Hash m_tilehash;
	kGUIBSPRect m_bsp;
	bool m_rebuildbsp;
	bool m_mapasync;
	bool m_mapdirty;
	int m_wasdrawn;				/* incremented each render */

	unsigned int m_nummaps;
	Array<GPXMAPInfo *>m_mapinfo;

	kGUIString m_defpath;

	int m_ucol;
	Array<kGUIString *>m_userhints;

	int m_clipfoundlogs;
	int m_clipnotfoundlogs;
	kGUIBusy *m_busy;

	CALLBACKGLUEPTR(GPX,UpdateCenter,kGUIEvent)
	CALLBACKGLUEPTR(GPX,DrawMapCell,kGUICellObj)
	CALLBACKGLUE(GPX,PreDrawMap)
	CALLBACKGLUE(GPX,PostDrawMap)
	CALLBACKGLUEPTR(GPX,ZoomGoto,kGUIEvent)
	CALLBACKGLUEPTR(GPX,ClickZoomIn,kGUIEvent)
	CALLBACKGLUEPTR(GPX,ClickZoomOut,kGUIEvent)

	CALLBACKGLUEVALVAL(GPX,OverMap,int,int)
	CALLBACKGLUEPTR(GPX,DoMapMenu,kGUIEvent)
	CALLBACKGLUEPTR(GPX,DoWPMenu,kGUIEvent)
	CALLBACKGLUEPTR(GPX,Online,kGUIEvent)
	CALLBACKGLUEPTR(GPX,ChangeMapTypeEvent,kGUIEvent)
	CALLBACKGLUEPTR(GPX,MoveDividerEvent,kGUIEvent)

	CALLBACKGLUE(GPX,SearchMap)
	CALLBACKGLUEPTR(GPX,TabChanged,kGUIEvent)
	CALLBACKGLUE(GPX,CheckSpecialKeys)
	CALLBACKGLUEPTR(GPX,ShowFileMenu,kGUIEvent)
	CALLBACKGLUEPTR(GPX,ShowHelpMenu,kGUIEvent)
	CALLBACKGLUEPTR(GPX,ShowGeocaching,kGUIEvent)
	CALLBACKGLUEPTR(GPX,DoFileMenu,kGUIEvent)
	CALLBACKGLUEPTR(GPX,DoHelpMenu,kGUIEvent)
	CALLBACKGLUEPTR(GPX,DoColMenu,kGUIEvent)

	CALLBACKGLUEVALVAL(GPX,PreLoadXML,int,int)
	CALLBACKGLUEPTRVAL(GPX,GetLoadSettings,kGUIFileReq,int)
	CALLBACKGLUEPTRVAL(GPX,DoSave,kGUIFileReq,int)
	CALLBACKGLUEVAL(GPX,DoSave2,int)
	CALLBACKGLUEPTRVAL(GPX,DoSaveAs,kGUIFileReq,int)
	CALLBACKGLUEVAL(GPX,DoSaveAs3,int)
	CALLBACKGLUEPTRVAL(GPX,DoSaveMapShape,kGUIFileReq,int)
	CALLBACKGLUEPTRVAL(GPX,DoLoadOther,kGUIFileReq,int)
	CALLBACKGLUEPTR(GPX,UpdateWPRenderEvent,kGUIEvent)
	CALLBACKGLUEPTR(GPX,WPTableEvent,kGUIEvent)
	CALLBACKGLUEPTR(GPX,NewMapPathEntry,kGUIEvent)
	CALLBACKGLUEPTR(GPX,GPSConnectChanged,kGUIEvent)
	CALLBACKGLUE(GPX,SelectLoadTracks)
	CALLBACKGLUEPTR(GPX,LabelFontSizeChangedEvent,kGUIEvent)
	CALLBACKGLUEPTR(GPX,TableFontSizeChangedEvent,kGUIEvent)
	CALLBACKGLUEPTR(GPX,LabelNamesChangedEvent,kGUIEvent)
	CALLBACKGLUEPTR(GPX,MapDirtyEvent,kGUIEvent)
	CALLBACKGLUEPTR(GPX,BSPDirtyEvent,kGUIEvent)
	CALLBACKGLUEPTR(GPX,ChangeEncode,kGUIEvent)
	CALLBACKGLUEPTR(GPX,SolveEncode,kGUIEvent)
	CALLBACKGLUEPTR(GPX,SolveDecode,kGUIEvent)
	CALLBACKGLUEPTR(GPX,SolveProject,kGUIEvent)
	CALLBACKGLUEPTR(GPX,SolveDistance,kGUIEvent)
	CALLBACKGLUEPTR(GPX,SolveSun,kGUIEvent)
	CALLBACKGLUEPTR(GPX,ReCalcDistsEvent,kGUIEvent)
	CALLBACKGLUEPTR(GPX,ReCalcNearEvent,kGUIEvent)
	CALLBACKGLUEPTR(GPX,StartBasic,kGUIEvent)
	CALLBACKGLUE(GPX,BasicDone)
	CALLBACKGLUEPTR(GPX,BasicCancel,kGUIEvent)
	CALLBACKGLUEPTR(GPX,BasicAddButton,kGUIEvent)
	CALLBACKGLUEPTR(GPX,StartBasicMenuDone,kGUIEvent);
	CALLBACKGLUEVALVAL(GPX,BasicError,int,int);
	CALLBACKGLUEPTR(GPX,MoveBasicDivider,kGUIEvent);
	CALLBACKGLUE(GPX,AddCClasses);
	CALLBACKGLUEPTRVAL(GPX,SetHintDone,kGUIString,int)
	CALLBACKGLUE(GPX,MapDirty);
	CALLBACKGLUEPTR(GPX,LabelUp,kGUIEvent)
	CALLBACKGLUEPTR(GPX,LabelDown,kGUIEvent)
};

enum
{
WPRENDER_FILTERNAME,
WPRENDER_BOXCOLOUR,
WPRENDER_NUMMATCHES,
WPRENDER_NUMCOLUMNS};

class GPXWPRenderRow : public kGUITableRowObj
{
public:
	GPXWPRenderRow();
	void Load(class kGUIXMLItem *wpr);	/* load from prefs file */
	void Save(class kGUIXMLItem *wpr);	/* save to prefs file */
	int GetNumObjects(void) {return WPRENDER_NUMCOLUMNS;}
	kGUIObj **GetObjectList(void) {return m_objectlist;} 
	kGUIObj *m_objectlist[WPRENDER_NUMCOLUMNS];
	int GetFilterNum(void) {return m_filtercombo.GetSelection();}
	kGUIColor GetColour(void) {return GPX::GetTableColor(m_colourcombo.GetSelection());}
	kGUIColor GetTColour(void) {return GPX::GetTableTColor(m_colourcombo.GetSelection());}
	void ClearCount(void) {m_count=0;}
	void IncCount(void) {++m_count;}
	void UpdateCount(void) {m_nummatches.Sprintf("%d",m_count);}
	//private:
	kGUIComboBoxObj m_filtercombo;
	kGUIComboBoxObj m_colourcombo;
	int m_count;
	kGUIInputBoxObj m_nummatches;
};

class GPXMapPathRow : public kGUITableRowObj
{
public:
	GPXMapPathRow();
	void Load(class kGUIXMLItem *mpr);	/* load from prefs file */
	void Save(class kGUIXMLItem *mpr);	/* save to prefs file */
	int GetNumObjects(void) {return 2;}
	kGUIString *GetPath(void){return &m_path;}
	kGUIObj **GetObjectList(void) {return m_objectlist;} 
	kGUIObj *m_objectlist[2];
private:
	CALLBACKGLUEPTR(GPXMapPathRow,Browse,kGUIEvent);
	CALLBACKGLUEPTRVAL(GPXMapPathRow,BrowseDone,kGUIFileReq,int);
	CALLBACKGLUEPTR(GPXMapPathRow,PathChangedEvent,kGUIEvent);
	CALLBACKGLUEVAL(GPXMapPathRow,DoAddMaps,int);
	void Browse(kGUIEvent *event);
	void BrowseDone(kGUIFileReq *result,int pressed);
	void PathChanged(void);
	void PathChangedEvent(kGUIEvent *event) {if(event->GetEvent()==EVENT_AFTERUPDATE)PathChanged();}
	void DoAddMaps(int result);

	kGUIInputBoxObj m_path;
	kGUIButtonObj m_browse;

};

class kGUIRenameDBReq
{
public:
	kGUIRenameDBReq();
	~kGUIRenameDBReq();
private:
	CALLBACKGLUEPTR(kGUIRenameDBReq,WindowEvent,kGUIEvent)
	CALLBACKGLUEPTR(kGUIRenameDBReq,Cancel,kGUIEvent)
	CALLBACKGLUEPTR(kGUIRenameDBReq,Rename,kGUIEvent)
	void WindowEvent(kGUIEvent *event);
	void Cancel(kGUIEvent *event) {if(event->GetEvent()==EVENT_PRESSED)m_window.Close();}
	void Rename(kGUIEvent *event) {if(event->GetEvent()==EVENT_PRESSED){DoRename();m_window.Close();}}
	void DoRename(void);
	kGUIWindowObj m_window;
	kGUIControlBoxObj m_controls;

	kGUITextObj m_fromcaption;
	kGUIComboBoxObj m_fromdb;
	kGUITextObj m_tocaption;
	kGUIInputBoxObj m_todb;
	kGUIButtonObj m_rename;
	kGUIButtonObj m_cancel;
};

class BasicButtonEdit
{
public:
	BasicButtonEdit();
	~BasicButtonEdit() {}
	void Init(void);
private:
	CALLBACKGLUEPTR(BasicButtonEdit,WindowEvent,kGUIEvent)
	CALLBACKGLUEPTR(BasicButtonEdit,Delete,kGUIEvent)
	CALLBACKGLUEPTR(BasicButtonEdit,Cancel,kGUIEvent)
	CALLBACKGLUEPTR(BasicButtonEdit,Save,kGUIEvent)
	void WindowEvent(kGUIEvent *event);
	void Cancel(kGUIEvent *event) {if(event->GetEvent()==EVENT_PRESSED)m_window.Close();}
	void Save(kGUIEvent *event) {if(event->GetEvent()==EVENT_PRESSED)m_window.Close();}
	void Delete(kGUIEvent *event) {if(event->GetEvent()==EVENT_PRESSED)m_window.Close();}
	kGUIWindowObj m_window;
	kGUIControlBoxObj m_controls;

	kGUITextObj m_subcaption;
	kGUIComboBoxObj m_sublist;
	kGUITextObj m_buttoncaption;
	kGUIInputBoxObj m_button;
	kGUIButtonObj m_save;
	kGUIButtonObj m_delete;
	kGUIButtonObj m_cancel;
};

class SelectTracks
{
public:
	SelectTracks(Hash *hash,void *codeobj,void (*code)(void *,int),kGUIXML *xml);
	~SelectTracks() {m_list.DeleteChildren();}
private:
	CALLBACKGLUEPTR(SelectTracks,WindowEvent,kGUIEvent)
	CALLBACKGLUEPTR(SelectTracks,PressDone,kGUIEvent)
	CALLBACKGLUEPTR(SelectTracks,PressCancel,kGUIEvent)
	void WindowEvent(kGUIEvent *event);
	void PressCancel(kGUIEvent *event);
	void PressDone(kGUIEvent *event);
	Hash *m_hash;
	
	kGUIWindowObj m_window;
	kGUIControlBoxObj m_controls;

	kGUITableObj m_list;

	kGUIButtonObj m_ok;
	kGUIButtonObj m_cancel;
	
	int m_pressed;
	kGUICallBackInt m_donecallback;
};

/* used by download from GPSr code */

class GPSrWaypoints
{
public:
	GPSrWaypoints() {}
	void SetLat(double lat) {m_lat=lat;}
	void SetLon(double lon) {m_lon=lon;}
	void SetName(kGUIString *n) {m_name.SetString(n);}
	void SetDesc(kGUIString *n) {m_desc.SetString(n);}
	void SetType(kGUIString *t) {m_type.SetString(t);}
	double GetLat(void) {return m_lat;}
	double GetLon(void) {return m_lon;}
	kGUIString *GetName(void) {return &m_name;}
	kGUIString *GetDesc(void) {return &m_desc;}
	kGUIString *GetType(void) {return &m_type;}
private:
	double m_lat;
	double m_lon;
	kGUIString m_name;
	kGUIString m_type;
	kGUIString m_desc;
};

class SelectWaypoints
{
public:
	SelectWaypoints(unsigned int num,ClassArray<GPSrWaypoints> *waypoints,int mode);
	~SelectWaypoints() {m_list.DeleteChildren();}
private:
	CALLBACKGLUEPTR(SelectWaypoints,PressDone,kGUIEvent)
	CALLBACKGLUEPTR(SelectWaypoints,PressCancel,kGUIEvent)
	CALLBACKGLUEPTR(SelectWaypoints,PressToggle,kGUIEvent)
	CALLBACKGLUEPTR(SelectWaypoints,PressUpdateFinds,kGUIEvent)
	CALLBACKGLUEPTR(SelectWaypoints,WindowEvent,kGUIEvent)
	void WindowEvent(kGUIEvent *event);
	void PressToggle(kGUIEvent *event);
	void PressUpdateFinds(kGUIEvent *event);
	void PressCancel(kGUIEvent *event);
	void PressDone(kGUIEvent *event);
	ClassArray<GPSrWaypoints> *m_wp;

	int m_mode;
	kGUIWindowObj m_window;
	kGUIControlBoxObj m_controls;

	kGUITableObj m_list;

	kGUIButtonObj m_ok;
	kGUIButtonObj m_cancel;
	kGUIButtonObj m_toggle;
	kGUIButtonObj m_update;
};


/* update text array 'tabnames' in gpsturbo.cpp if adding to this list */

enum
{
TAB_MAIN,
TAB_ROUTE,
TAB_TRACKS,
TAB_LINES,
TAB_FILTERS,
TAB_SETTINGS,
TAB_GPSES,
TAB_SOLVER,
TAB_STICKERS,
TAB_NOTES,
TAB_BASIC,
TAB_DEBUG,
TAB_NUMTABS
};

enum
{
SOLVERHEAD_DEGREES,
SOLVERHEAD_DEGREESMAGNETIC,
SOLVERHEAD_NUMTYPES};

#define SMALLCAPTIONFONT 1
#define SMALLCAPTIONSIZE 9

extern void DebugPrint(const char *message,...);

#define sgn(x) (x)==0?0:(x>0?1:-1)
#define sgnd(x) (x)==0.0f?0:(x>0.0f?1:-1)

extern GPX *gpx;
extern bool g_isonline;


#endif
