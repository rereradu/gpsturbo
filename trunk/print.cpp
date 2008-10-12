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

#include "kguireport.h"

extern const char *wpcolnames[GPXCOL_NUMCOLUMNS];

enum
{
GRIDTYPE_NONE,
GRIDTYPE_FULL,
GRIDTYPE_EDGES
};

class PrintMap : public kGUIReport
{
public:
	PrintMap();
	~PrintMap();
private:
	CALLBACKGLUEPTR(PrintMap,PanUp,kGUIEvent);
	CALLBACKGLUEPTR(PrintMap,PanDown,kGUIEvent);
	CALLBACKGLUEPTR(PrintMap,PanLeft,kGUIEvent);
	CALLBACKGLUEPTR(PrintMap,PanRight,kGUIEvent);
	int GetPPI(void) {return 125;}					/* pixels per inch */
	double GetPageWidth(void) {return -1.0f;}		/* inches */
	double GetPageHeight(void) {return -1.0f;}		/* inches */
	double GetLeftMargin(void) {return 0.0f;}		/* inches */
	double GetRightMargin(void) {return 0.0f;}		/* inches */
	double GetTopMargin(void)	{return 0.0f;}		/* inches */
	double GetBottomMargin(void) {return 0.0f;}	/* inches */
	void Setup(void);
	void Setup(int page);
	const char *GetName(void) {return "Map";}
	static int SplitSortEntry(const void *o1,const void *o2);

	/********************************/
	kGUIComboBoxObj m_numwide;
	kGUITextObj m_numwidecaption;
	kGUIComboBoxObj m_numtall;
	kGUITextObj m_numtallcaption;

	kGUIComboBoxObj m_gridtype;			/* none, full, edges */
	kGUITextObj m_gridstepcaption;
	kGUIInputBoxObj m_gridstep;			/* every n minutes */
	kGUITextObj m_gridfontsizecaption;
	kGUIInputBoxObj m_gridfontsize;		/* draw text at n size */

	kGUITextObj m_pancaption;
	kGUIButtonObj m_panleft;
	kGUIButtonObj m_panup;
	kGUIButtonObj m_pandown;
	kGUIButtonObj m_panright;

	kGUITextObj m_splitcaption;
	kGUITickBoxObj m_split;				/* draw hints etc along right edge of map */
	kGUITextObj m_splitwidthcaption;
	kGUIInputBoxObj m_splitwidth;
	kGUITextObj m_splitfontsizecaption;
	kGUIInputBoxObj m_splitfontsize;
	kGUITextObj m_splitsortcaption;
	kGUIComboBoxObj m_splitsort;
	kGUITextObj m_splitdesccaption;
	kGUIInputBoxObj m_splitdesc;

	kGUIImage m_iup;
	kGUIImage m_idown;
	kGUIImage m_ileft;
	kGUIImage m_iright;

	int m_numimageobjs;
	kGUIDrawSurface m_image;

	int m_offx;
	int m_offy;
	/* 1/2 inch per click */
	void PanUp(kGUIEvent *event) {if(event->GetEvent()==EVENT_PRESSED){m_offy-=GetPPI()/2;ChangeFormat();}}
	void PanDown(kGUIEvent *event) {if(event->GetEvent()==EVENT_PRESSED){m_offy+=GetPPI()/2;ChangeFormat();}}
	void PanLeft(kGUIEvent *event) {if(event->GetEvent()==EVENT_PRESSED){m_offx-=GetPPI()/2;ChangeFormat();}}
	void PanRight(kGUIEvent *event) {if(event->GetEvent()==EVENT_PRESSED){m_offx+=GetPPI()/2;ChangeFormat();}}

	/* position and size in pixels for split area */
	int m_splitx;
	int m_splitw;
	int m_splith;
};

void GPX::DrawTextOutline(kGUIText *t,int x,int y,int s)
{
	t->Draw(x-s,y,0,0,DrawColor(0,0,0));
	t->Draw(x+s,y,0,0,DrawColor(0,0,0));
	t->Draw(x,y-s,0,0,DrawColor(0,0,0));
	t->Draw(x,y+s,0,0,DrawColor(0,0,0));
	t->Draw(x,y,0,0,DrawColor(255,255,255));
}

void GPX::DrawTextRotOutline(kGUIText *t,int x,int y,int s,double heading)
{
	t->DrawRot(x-s,y,heading,DrawColor(0,0,0));
	t->DrawRot(x+s,y,heading,DrawColor(0,0,0));
	t->DrawRot(x,y-s,heading,DrawColor(0,0,0));
	t->DrawRot(x,y+s,heading,DrawColor(0,0,0));
	t->DrawRot(x,y,heading,DrawColor(255,255,255));
}

void GPX::DrawGrid(int w,int h,int cx,int cy,int gridtype,double gridstep,int gridfontsize)
{
	kGUIContainerObj *parent;
	kGUIZone z;
	int oldsx,oldsy;

	parent=m_grid.GetParent();
	m_grid.SetParent(0);
	m_grid.CopyZone(&z);
	m_grid.GetScrollCenter(&oldsx,&oldsy);

	m_grid.SetPos(0,0);
	m_grid.SetPrintMode(true);
	m_grid.SetSize(w,h);
//	m_grid.SetScrollCenter(oldsx+offx-((w-z.GetZoneW())>>1),oldsy+offy-((h-z.GetZoneH())>>1));
	m_grid.SetScrollCenter(cx,cy);

	/* turn off async tile loading for printing */
	gpx->SetMapAsync(false);
	m_grid.Draw();
	gpx->SetMapAsync(true);

	/* draw gridlines on map */
	if(gridtype!=GRIDTYPE_NONE)
	{
		double l,l1,l2;
		int x,y,s;
		int lx,rx,ty,by;
		int tp,th;
		GPXCoord tl;
		GPXCoord br;
		GPXCoord c;
		kGUIText t;

		if(gridfontsize>0)
			t.SetFontSize(gridfontsize);
		s=gridfontsize/12;
		if(s<1)
			s=1;
		m_grid.GetScrollCorner(&lx,&ty);
		rx=lx+w;
		by=ty+h;

		m_curmap->FromMap(lx,ty,&tl);
		m_curmap->FromMap(rx,by,&br);

		/* draw lines on map for all degrees */
		l1=(double)((int)tl.GetLon()-1);
		l2=(double)(((int)br.GetLon())+1);
		for(l=l1;l<l2;l+=gridstep)
		{
			c.Set(tl.GetLat(),l);
			m_curmap->ToMap(&c,&x,&y);
			x-=lx;
			c.OutputLon(&t,true);
			tp=t.GetWidth();

			if(gridtype==GRIDTYPE_FULL)
			{
				kGUI::DrawLine(x,0,x,h-1,DrawColor(255,255,255));
				kGUI::DrawLine(x+1,0,x+1,h-1,DrawColor(0,0,0));
			}
			else	/* GRIDTYPE_EDGES */
			{
				kGUI::DrawLine(x,0,x,tp<<1,DrawColor(255,255,255));
				kGUI::DrawLine(x+1,0,x+1,tp<<1,DrawColor(0,0,0));

				kGUI::DrawLine(x,h-(tp<<1),x,h-1,DrawColor(255,255,255));
				kGUI::DrawLine(x+1,h-(tp<<1),x+1,h-1,DrawColor(0,0,0));

			}
			if(gridfontsize>0)
			{
				DrawTextRotOutline(&t,x-10,10,s,PI+(PI/2.0f));
				DrawTextRotOutline(&t,x+10,h-10,s,PI/2.0f);
			}
		}

		/* draw lines on map for all degrees */
		l1=(double)((int)tl.GetLat()-1);
		l2=(double)(((int)br.GetLat())+1);
		for(l=l1;l<l2;l+=gridstep)
		{
			c.Set(l,tl.GetLon());
			m_curmap->ToMap(&c,&x,&y);
			c.OutputLat(&t,true);
			tp=t.GetWidth();
			th=t.GetLineHeight();
			y-=ty;
			if(gridtype==GRIDTYPE_FULL)
			{
				kGUI::DrawLine(0,y,w-1,y,DrawColor(255,255,255));
				kGUI::DrawLine(0,y+1,w-1,y+1,DrawColor(0,0,0));
			}
			else	/* GRIDTYPE_EDGES */
			{
				kGUI::DrawLine(0,y,tp<<1,y,DrawColor(255,255,255));
				kGUI::DrawLine(0,y+1,tp<<1,y+1,DrawColor(0,0,0));

				kGUI::DrawLine(w-(tp<<1),y,w-1,y,DrawColor(255,255,255));
				kGUI::DrawLine(w-(tp<<1),y+1,w-1,y+1,DrawColor(0,0,0));
			}
			if(gridfontsize>0)
			{
				DrawTextOutline(&t,10,y+2,s);					/* left side of page below line */
				DrawTextOutline(&t,(w-tp)-10,y-(th+2),s);		/* right side of page above line */
			}
		}
	}

	m_grid.SetPrintMode(false);
	m_grid.SetSize(z.GetZoneW(),z.GetZoneH());
	m_grid.SetZone(&z);
	m_grid.SetScrollCenter(oldsx,oldsy);
	m_grid.SetParent(parent);
}

enum
{
SPLITSORT_NAME,
SPLITSORT_LAT,
SPLITSORT_LON,
SPLITSORT_ROUTEINDEX,
SPLITSORT_NUM};

const char *splitsortname[]={"Name","Latitude","Longitude","Route"};

int g_splitsort;	/* sort index */

int PrintMap::SplitSortEntry(const void *o1,const void *o2)
{
	GPXRow *r1=*(static_cast<GPXRow **>((void *)o1));
	GPXRow *r2=*(static_cast<GPXRow **>((void *)o2));

	switch(g_splitsort)
	{
	case SPLITSORT_NAME:
		return(strcmp(r1->m_name.GetString(),r2->m_name.GetString()));
	break;
	case SPLITSORT_LAT:
		return(sgnd(r2->m_llcoord.GetLat()-r1->m_llcoord.GetLat()));
	break;
	case SPLITSORT_LON:
		return(sgnd(r1->m_llcoord.GetLon()-r2->m_llcoord.GetLon()));
	break;
	case SPLITSORT_ROUTEINDEX:
		return(r1->GetIndex()-r2->GetIndex());
	break;
	}
	return(0);
}

PrintMap::PrintMap()
{
	int i;

	SetBitmapMode(true);			/* send to printer as a bitmap */
	SetAllowMultiPages(false);		/* only allow 1 page per page */

	gpx->GetGridPtr()->GetScrollCenter(&m_offx,&m_offy);
	SetPID(kGUI::LocatePrinter(gpx->m_printmap.GetString(),true));

	m_numwidecaption.SetString("Pages Wide:");
	m_numwide.SetNumEntries(10);
	m_numwide.SetSize(35,20);
	m_numtallcaption.SetString("Pages Tall:");
	m_numtall.SetNumEntries(10);
	m_numtall.SetSize(35,20);
	for(i=0;i<10;++i)
	{
		kGUIString ns;

		ns.Sprintf("%d",i+1);
		m_numwide.SetEntry(i,ns.GetString(),i+1);
		m_numtall.SetEntry(i,ns.GetString(),i+1);
	}

	m_gridtype.SetNumEntries(3);
	m_gridtype.SetSize(100,20);
	m_gridtype.SetEntry(0,"No Grid Lines",GRIDTYPE_NONE);
	m_gridtype.SetEntry(1,"Full Grid Lines",GRIDTYPE_FULL);
	m_gridtype.SetEntry(2,"Edge Grid Lines",GRIDTYPE_EDGES);

	m_gridstepcaption.SetString("Minute Step:");
	m_gridstep.SetSize(50,20);
	m_gridstep.Sprintf("%d",5);

	m_gridfontsizecaption.SetString("Font Size:");
	m_gridfontsize.SetSize(50,20);
	m_gridfontsize.Sprintf("%d",15);

	m_iup.SetFilename("pup.gif");
	m_idown.SetFilename("pdown.gif");
	m_ileft.SetFilename("pleft.gif");
	m_iright.SetFilename("pright.gif");

	m_pancaption.SetString("Pan:");
	m_panleft.SetSize(20,20);
	m_panleft.SetImage(&m_ileft);
	m_panleft.SetEventHandler(this,CALLBACKNAME(PanLeft));

	m_panup.SetSize(20,20);
	m_panup.SetImage(&m_iup);
	m_panup.SetEventHandler(this,CALLBACKNAME(PanUp));

	m_pandown.SetSize(20,20);
	m_pandown.SetImage(&m_idown);
	m_pandown.SetEventHandler(this,CALLBACKNAME(PanDown));

	m_panright.SetSize(20,20);
	m_panright.SetImage(&m_iright);
	m_panright.SetEventHandler(this,CALLBACKNAME(PanRight));

	m_split.SetSelected(gpx->m_split==1);
	m_splitcaption.SetString("List Mode:");
	m_splitwidthcaption.SetString("List Width(Inches):");
	m_splitwidth.SetSize(50,20);
	m_splitwidth.SetString(gpx->m_splitwidth.GetString());
	m_splitfontsizecaption.SetString("List Font Size:");
	m_splitfontsize.Sprintf("%d",gpx->m_splitfontsize);
	m_splitfontsize.SetSize(50,20);

	m_splitsortcaption.SetString("List Sorting:");
	m_splitsort.SetSize(125,20);
	m_splitsort.SetNumEntries(SPLITSORT_NUM);
	for(i=0;i<SPLITSORT_NUM;++i)
		m_splitsort.SetEntry(i,splitsortname[i],i);
	m_splitsort.SetSelection(gpx->m_splitsort);

	m_splitdesccaption.SetString("List Desc:");
	m_splitdesc.SetSize(200,20);
	m_splitdesc.SetString(gpx->m_splitdesc.GetString());

	AddUserControl(&m_numwidecaption);
	AddUserControl(&m_numwide);
	AddUserControl(&m_numtallcaption);
	AddUserControl(&m_numtall);

	AddUserControl(&m_gridtype);
	AddUserControl(&m_gridstepcaption);
	AddUserControl(&m_gridstep);
	AddUserControl(&m_gridfontsizecaption);
	AddUserControl(&m_gridfontsize);

	AddUserControl(&m_pancaption);
	AddUserControl(&m_panleft);
	AddUserControl(&m_panup);
	AddUserControl(&m_pandown);
	AddUserControl(&m_panright);

	AddUserControl(&m_splitcaption);
	AddUserControl(&m_split);
	AddUserControl(&m_splitwidthcaption);
	AddUserControl(&m_splitwidth);
	AddUserControl(&m_splitfontsizecaption);
	AddUserControl(&m_splitfontsize);
	AddUserControl(&m_splitsortcaption);
	AddUserControl(&m_splitsort);
	AddUserControl(&m_splitdesccaption);
	AddUserControl(&m_splitdesc);
}

PrintMap::~PrintMap()
{
	gpx->m_printmap.SetString(kGUI::GetPrinterObj(GetPID())->GetName());

	/* save these in preferences for next time */
	gpx->m_split=m_split.GetSelected()?1:0;
	gpx->m_splitsort=m_splitsort.GetSelection();
	gpx->m_splitfontsize=m_splitfontsize.GetInt();
	gpx->m_splitdesc.SetString(m_splitdesc.GetString());
	gpx->m_splitwidth.SetString(m_splitwidth.GetString());
}

void PrintMap::Setup(void)
{
	int i,pw,ph,numpages;
	kGUIReportImageObj *obj;
	int numwide,numtall;
	int wptnamenumentries;
	Array<WNENT_DEF>wptnameentries;

	kGUI::SetMouseCursor(MOUSECURSOR_BUSY);

	numwide=m_numwide.GetSelection();
	numtall=m_numtall.GetSelection();

	GetPageSizePixels(&pw,&ph);
	if(m_split.GetSelected())
	{
		/* remove width of split area from map */
		m_splitw=(int)(m_splitwidth.GetDouble()*GetPPI());
		/* max 1/2 width */
		if(m_splitw>(pw>>1))
			m_splitw=pw>>1;

		pw-=m_splitw;
		m_splitx=pw;
		m_splith=ph;
	}
	else
	{
		m_splitw=0;
		m_splith=0;
		m_splitx=0;
	}
	m_image.Init(pw,ph);

	if(m_split.GetSelected())
	{
		/* compile waypoint definitions into fast format for quick printing */
		wptnameentries.Init(24,12);
		gpx->CompileLabelDefinition(&m_splitdesc,&wptnamenumentries,&wptnameentries);
	}

	numpages=numwide*numtall;
	for(i=0;i<numpages;++i)
	{
		obj=new kGUIReportImageObj();
		obj->SetPos(0,ph*i);
		obj->SetMemImage(0,GUISHAPE_SURFACE,pw,ph,m_image.GetBPP(),(const unsigned char *)m_image.GetSurfacePtr(0,0));
		obj->SetSize(pw,ph);
		AddObjToSection(REPORTSECTION_BODY,obj,true);

		if(m_split.GetSelected())
		{
			/* add descriptions for all wayponts on this map */
			int ty,th;
			kGUICorners c;
			kGUIReportTextObj *t;
			kGUIReportRectObj *line;
			kGUIBSPRectEntry *bspe;
			GPXRow *row;
			int r,numrows;
			Array<GPXRow *>rows;

			/* area of the map that is shown on the page */
			c.lx=m_offx+((pw*(i%numwide))-(pw*numwide/2));
			c.rx=c.lx+pw;
			c.ty=m_offy+((ph*(i/numwide))-(ph*numtall/2));
			c.by=c.ty+ph;

			/* build a list of all labels in this area */
			gpx->m_bsp.Select(&c);

			g_splitsort=m_splitsort.GetSelection();

			numrows=0;
			rows.Init(256,128);
			/* each row has 4 points in the tree for the 4 corners so we need to */
			/* check and not draw labels 4 times if all corners are on the same square */
			gpx->m_wasdrawn++;
			ty=0;
			do
			{
				bspe=gpx->m_bsp.GetEntry();
				if(!bspe)
					break;	/* end of list */
			
				GPXLabel *label=static_cast<GPXLabel *>(bspe);

				/* since BSP returns rough zones we need to make sure point is in area */
				if(label->m_draw==true)
				{
					if(kGUI::Overlap(&c,&label->m_c)==true)
					{
						if(label->m_wasdrawn!=gpx->m_wasdrawn)
						{
							row=label->m_row;
		
							/* if we are sorting by Route then only show entries for item in the current route */
							if(g_splitsort==SPLITSORT_ROUTEINDEX)
							{
								if(gpx->m_routes.InAnyRoute(row))
								{
									rows.SetEntry(numrows++,row);
									label->m_wasdrawn=gpx->m_wasdrawn;	/* set drawn flag so it isn't added again */
								}
							}
							else
							{
								rows.SetEntry(numrows++,row);
								label->m_wasdrawn=gpx->m_wasdrawn;	/* set drawn flag so it isn't added again */
							}
						}
					}
				}
			}while(1);

			/* sort the list */

			rows.Sort(numrows,SplitSortEntry);

			/* draw the list */
			ty=0;
			for(r=0;r<numrows;++r)
			{
				row=rows.GetEntry(r);
				t=new kGUIReportTextObj();
				t->SetFontSize(m_splitfontsize.GetInt());
				t->SetColor(DrawColor(0,0,0));
				gpx->ExpandLabel(row,t,&m_splitdesc,wptnamenumentries,&wptnameentries);
				t->SetPos(m_splitx+5,ty+(ph*i));
				th=t->CalcHeight(m_splitw-5);	/* a little extra inter line spacing */
				t->SetSize(m_splitw,th);
				if((ty+th)>m_splith)
				{
					delete t;
					break;
				}
				ty+=th+1;
				AddObjToSection(REPORTSECTION_BODY,t,true);
				if((ty+1)>=m_splith)
					break;
				line=new kGUIReportRectObj();
				line->SetPos(m_splitx,ty+(ph*i));
				line->SetSize(m_splitw,1);
				AddObjToSection(REPORTSECTION_BODY,line,true);
				ty+=1;
			}
		}
	}

	kGUI::SetMouseCursor(MOUSECURSOR_DEFAULT);

}

void PrintMap::Setup(int page)
{
	int numwide,numtall;
	int centerx,centery;
	int pw,ph;
	
	/* the report object on each page all point to a single image instance so before each page is */
	/* printed or previewed we need to update the contents of this image to point to the correct place on the map */

	/* why? if the user printed 40 pages then it would take a ton of memory, so by reusing the image buffer over */
	/* and over we don't use up too much memory */

	kGUIDrawSurface *savesurface;

	kGUI::SetMouseCursor(MOUSECURSOR_BUSY);

	GetPageSizePixels(&pw,&ph);
	pw-=m_splitw;

	numwide=m_numwide.GetSelection();
	numtall=m_numtall.GetSelection();
	centerx=m_offx+((pw*((page-1)%numwide))-(pw*numwide/2)+(pw/2));
	centery=m_offy+((ph*((page-1)/numwide))-(ph*numtall/2)+(ph/2));

	kGUI::PushClip();
	savesurface=kGUI::GetCurrentSurface();
	kGUI::SetCurrentSurface(&m_image);
	kGUI::PushClip();
	kGUI::ResetClip();	/* set clip to full surface on stack */

	gpx->DrawGrid(pw,ph,centerx,centery,m_gridtype.GetSelection(),(double)m_gridstep.GetInt()/60.0f,m_gridfontsize.GetInt());

	kGUI::SetCurrentSurface(savesurface);
	kGUI::PopClip();

	kGUI::SetMouseCursor(MOUSECURSOR_DEFAULT);

}

void GPX::DoPrintMap(void)
{
	PrintMap *map=new PrintMap();
	map->SetLandscape(true);
	map->Preview();
}

/******************************************************************************/

class GridLine : public kGUIReportRowObj
{
public:
	GridLine(kGUIReportRowHeaderObj *p,kGUITableObj *table,GPXRow *rec,bool shaded);
	kGUIReportObj **GetObjectList(void) {return m_xobjectlist;}
private:
	kGUIReportObj *m_objectlist[GPXCOL_NUMCOLUMNS];
	kGUIReportObj *m_xobjectlist[GPXCOL_NUMCOLUMNS];
	kGUIReportTickboxObj m_na;
	kGUIReportTickboxObj m_found;
	kGUIReportImageRefObj m_typeshape;
	kGUIReportTickboxObj m_user[MAXUSERTICKS];
	kGUIReportTextObj m_name;
	kGUIReportTextObj m_url;
	kGUIReportTextObj m_wptname;
	kGUIReportTickboxObj m_corrected;

	kGUIReportTextObj m_shortname;
	kGUIReportTextObj m_container;
	kGUIReportTextObj m_difficulty;
	kGUIReportTextObj m_terrain;
	kGUIReportTextObj m_dist;
	kGUIReportTextObj m_lat;
	kGUIReportTextObj m_lon;
	kGUIReportTextObj m_state;
	kGUIReportTextObj m_country;
	kGUIReportTextObj m_hint;
	kGUIReportTextObj m_owner;
	kGUIReportTextObj m_log;
	kGUIReportTextObj m_usernotes;
	kGUIReportTextObj m_gendate;
	kGUIReportTextObj m_near;
};

GridLine::GridLine(kGUIReportRowHeaderObj *p,kGUITableObj *table,GPXRow *rec, bool shaded)
{
	int i,j,xcol;

	SetBGShade(shaded);
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
	m_objectlist[GPXCOL_NUMNEAR]=&m_near;
	
	/* convert pointers to user hidden/shown/reordered */
	j=0;
	for(i=0;i<GPXCOL_NUMCOLUMNS;++i)
	{
		xcol=table->GetColOrder(i);
		if(table->GetColShow(xcol)==true)
			m_xobjectlist[j++]=m_objectlist[xcol];
	}

	m_na.SetSelected(rec->m_na.GetSelected());
	m_found.SetSelected(rec->m_found.GetSelected());
	m_typeshape.SetImage(gpx->GetShape(rec->GetType()));
	for(i=0;i<MAXUSERTICKS;++i)
		m_user[i].SetSelected(rec->m_user[i].GetSelected());
	m_name.SetString(&rec->m_name);
	m_wptname.SetString(&rec->m_wptname);
	m_wptname.SetTextColor(rec->m_label.m_tcolour);
	m_wptname.SetBGColor(rec->m_label.m_colour);
	m_wptname.SetFrame(true);

	m_corrected.SetSelected(rec->m_corrected.GetSelected());

	m_shortname.SetString(&rec->m_shortname);
	m_container.SetString(rec->m_container.GetSelectionString());
	m_difficulty.SetString(&rec->m_difficulty);
	m_terrain.SetString(&rec->m_terrain);
	m_dist.SetString(&rec->m_dist);
	m_log.SetString(&rec->m_log);

	m_log.InitRichInfo();
	j=m_log.GetLen();
	for(i=0;i<j;++i)
	{
		RICHINFO_DEF *ri;

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

	m_lat.SetString(&rec->m_lat);
	m_lon.SetString(&rec->m_lon);
	m_state.SetString(&rec->m_state);
	m_country.SetString(&rec->m_country);
	m_hint.SetString(&rec->m_hint);
	m_owner.SetString(&rec->m_owner);
	m_usernotes.SetString(&rec->m_usernotes);
	m_gendate.SetString(&rec->m_gendate);
	m_near.SetString(&rec->m_numnear);

	SetHeader(p);
	SetZoneH(m_hint.Height(m_hint.GetZoneW()));
}

class PrintGrid : public kGUIReport
{
public:
	PrintGrid(kGUITableObj *table);
	~PrintGrid();
private:
	int GetPPI(void) {return 125;}		/* pixels per inch */
	double GetPageWidth(void) {return -1.0f;}		/* inches */
	double GetPageHeight(void) {return -1.0f;}		/* inches */
	double GetLeftMargin(void) {return 0.25f;}		/* inches */
	double GetRightMargin(void) {return 0.25f;}		/* inches */
	double GetTopMargin(void)	{return 0.25f;}		/* inches */
	double GetBottomMargin(void) {return 0.25f;}	/* inches */
	void Setup(void);
	void Setup(int page);
	const char *GetName(void) {return "Grid";}
	/********************************/
	kGUITableObj *m_table;
	kGUIReportRowHeaderObj m_rowheader;
};

PrintGrid::PrintGrid(kGUITableObj *table)
{
	SetPID(kGUI::LocatePrinter(gpx->m_printgrid.GetString(),true));
	m_table=table;

}

void PrintGrid::Setup(void)
{
	int i,ne,x,y,w,xcol;
	GPXRow *row;
	GridLine *line;
	int oldsize;

	oldsize=kGUI::GetDefReportFontSize();
	kGUI::SetDefReportFontSize(gpx->GetTableFontSize());
//	SetBitmapMode(true);	/* send to printer as a bitmap */

	/* count number of visible columns */
	ne=0;
	for(i=0;i<m_table->GetNumCols();++i)
	{
		if(m_table->GetColShow(i)==true)
			++ne;
	}	

	m_rowheader.SetNumColumns(m_table->GetNumCols());
	x=0;
	y=0;
	for(i=0;i<m_table->GetNumCols();++i)
	{
		xcol=m_table->GetColOrder(i);
		if(m_table->GetColShow(xcol)==true)
		{
			w=m_table->GetColWidth(xcol)+12;

			m_rowheader.SetColX(y,x);
			m_rowheader.SetColWidth(y,w);
			m_rowheader.SetColName(y,wpcolnames[xcol]);
			x+=w;
			++y;
		}
	}
	m_rowheader.SetZoneH(20);
	AddObjToSection(REPORTSECTION_PAGEHEADER,&m_rowheader,false);

	ne=m_table->GetNumChildren();
	y=0;
	for(i=0;i<ne;++i)
	{
		row=static_cast<GPXRow *>(m_table->GetChild(i));
		line=new GridLine(&m_rowheader,m_table,row,(i&1)==1);
		line->SetZoneY(y);
		AddObjToSection(REPORTSECTION_BODY,line,true);
		y+=line->GetZoneH();
	}
	kGUI::SetDefReportFontSize(oldsize);
}

void PrintGrid::Setup(int page)
{
	/* todo change page number on bottom of table */
	/* for multi-wide/tall pages, show page numbers in a */
	/* page 1,3 format */
}

PrintGrid::~PrintGrid()
{
	gpx->m_printgrid.SetString(kGUI::GetPrinterObj(GetPID())->GetName());
}


void GPX::DoPrintTable(kGUITableObj *table)
{
	PrintGrid *grid=new PrintGrid(table);
	grid->SetLandscape(true);
	grid->Preview();
}
