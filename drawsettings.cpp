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

const char *wprnames[WPRENDER_NUMCOLUMNS]={
	"Filter","Waypoint Box Colour","Matches"};
const int wprwidths[WPRENDER_NUMCOLUMNS]={200,300,80};

void GPX::InitSettings(void)
{
	unsigned int i;
	int y;
	int bw=m_tabs.GetChildZoneW();

	m_labelcontrols.SetPos(0,0);
	m_labelcontrols.SetSize(bw,20);

	m_labelup.SetFontSize(11);
	m_labelup.SetSize(30,25);
	m_labelup.SetString("Up");
	m_labelup.SetEventHandler(this,CALLBACKNAME(LabelUp));
	m_labelcontrols.AddObject(&m_labelup);

	m_labeldown.SetFontSize(11);
	m_labeldown.SetSize(30,25);
	m_labeldown.SetString("Dn");
	m_labeldown.SetEventHandler(this,CALLBACKNAME(LabelDown));
	m_labelcontrols.AddObject(&m_labeldown);

	/* labels table */
	m_tabs.AddObject(&m_labelcontrols);
	y=m_labelcontrols.GetZoneH();

	m_labelcolourtable.SetPos(0,y);
	m_labelcolourtable.SetSize(625,215);
	m_labelcolourtable.SetNumCols(WPRENDER_NUMCOLUMNS);
	for(i=0;i<WPRENDER_NUMCOLUMNS;++i)
	{
		m_labelcolourtable.SetColTitle(i,wprnames[i]);
		m_labelcolourtable.SetColWidth(i,wprwidths[i]);
	}
	m_labelcolourtable.SetAllowAddNewRow(true);
	m_labelcolourtable.SetEventHandler(this,CALLBACKNAME(WPTableEvent));
	m_tabs.AddObject(&m_labelcolourtable);

	y=m_labelcolourtable.GetZoneBY()+10;

	/* map path table */
	m_mapdirstable.SetPos(0,y);
	m_mapdirstable.SetSize(625,100);
	m_mapdirstable.SetNumCols(2);
	m_mapdirstable.SetColTitle(0,"Map Path");
	m_mapdirstable.SetColWidth(0,520);
	m_mapdirstable.SetColTitle(1,"Browse");
	m_mapdirstable.SetColWidth(1,50);

	m_mapdirstable.SetAllowAddNewRow(true);
	m_mapdirstable.SetEventHandler(this,CALLBACKNAME(NewMapPathEntry));
	m_tabs.AddObject(&m_mapdirstable);

	y=m_mapdirstable.GetZoneBY()+10;

	m_usebrowser.SetPos(10,y);
	m_usebrowserlabel.SetPos(30,y);
	m_usebrowserlabel.SetString("Use Built-In Browser");
	m_tabs.AddObject(&m_usebrowser);
	m_tabs.AddObject(&m_usebrowserlabel);
	y+=25;

	m_maxdownloads.SetString("8");
	m_maxdownloads.SetPos(10,y);
	m_maxdownloads.SetSize(40,20);
	m_maxdownloadslabel.SetPos(50,y);
	m_maxdownloadslabel.SetString("Maximum Concurrent Internet Downloads");
	m_tabs.AddObject(&m_maxdownloads);
	m_tabs.AddObject(&m_maxdownloadslabel);
	y+=25;

	m_tablefontsize.SetString("12");
	m_tablefontsize.SetPos(10,y);
	m_tablefontsize.SetSize(40,20);
	m_tablefontsize.SetEventHandler(this,CALLBACKNAME(TableFontSizeChangedEvent));
	m_tablefontsizelabel.SetPos(50,y);
	m_tablefontsizelabel.SetString("Font Size for Waypoint Tables");
	m_tabs.AddObject(&m_tablefontsize);
	m_tabs.AddObject(&m_tablefontsizelabel);
	y+=25;

	m_labelfontsize.SetString("12");
	m_labelfontsize.SetPos(10,y);
	m_labelfontsize.SetSize(40,20);
	m_labelfontsize.SetEventHandler(this,CALLBACKNAME(LabelFontSizeChangedEvent));
	m_labelfontsizelabel.SetPos(50,y);
	m_labelfontsizelabel.SetString("Font Size for Waypoint Labels on Map");
	m_tabs.AddObject(&m_labelfontsize);
	m_tabs.AddObject(&m_labelfontsizelabel);

	m_labelalpha.SetPos(m_labelfontsizelabel.GetZoneRX()+10,y);
	GPX::InitAlphaCombo(&m_labelalpha);
	m_labelalpha.SetSelection(70);
	m_tabs.AddObject(&m_labelalpha);
	m_labelalpha.SetEventHandler(this,CALLBACKNAME(MapDirtyEvent));
	y+=25;

	m_shownumticks.SetString("0");
	m_shownumticks.SetPos(10,y);
	m_shownumticks.SetSize(40,20);
	m_shownumticks.SetEventHandler(this,CALLBACKNAME(LabelFontSizeChangedEvent));
	m_shownumtickslabel.SetPos(50,y);
	m_shownumtickslabel.SetString("Number of User Ticks to show beside Label on Map");
	m_tabs.AddObject(&m_shownumticks);
	m_tabs.AddObject(&m_shownumtickslabel);
	y+=25;

	m_wptnamelabel.SetPos(10,y);
	m_wptnamelabel.SetString("Waypoint name to use on Map");
	m_wptname.SetPos(200,y);
	m_wptname.SetSize(400,20);
	m_wptname.SetEventHandler(this,CALLBACKNAME(LabelNamesChangedEvent));
	m_tabs.AddObject(&m_wptname);
	m_tabs.AddObject(&m_wptnamelabel);
	y+=25;

	m_showchildren.SetPos(10,y);
	m_showchildren.SetEventHandler(this,CALLBACKNAME(BSPDirtyEvent));
	m_showchildrenlabel.SetPos(30,y);
	m_showchildrenlabel.SetString("Show Labels for Additional Waypoints on Map");
	m_tabs.AddObject(&m_showchildren);
	m_tabs.AddObject(&m_showchildrenlabel);
	y+=25;

	m_movelabels.SetPos(10,y);
	m_movelabels.SetSelected(true);	/* default to selected */
	m_movelabels.SetEventHandler(this,CALLBACKNAME(BSPDirtyEvent));
	m_movelabelslabel.SetPos(30,y);
	m_movelabelslabel.SetString("Move Overlapping Labels");
	m_tabs.AddObject(&m_movelabels);
	m_tabs.AddObject(&m_movelabelslabel);
	y+=25;

	m_movelabelsmax.SetPos(10,y);
	m_movelabelsmax.SetSize(40,20);
	m_movelabelsmax.SetString("128");
	m_movelabelsmax.SetEventHandler(this,CALLBACKNAME(BSPDirtyEvent));
	m_movelabelsmaxlabel.SetPos(50,y);
	m_movelabelsmaxlabel.SetString("Move Overlapping Labels, Maximum number of pixels");

	m_tabs.AddObject(&m_movelabelsmax);
	m_tabs.AddObject(&m_movelabelsmaxlabel);
	y+=25;

	m_labelzoomsize.SetString("10");
	m_labelzoomsize.SetPos(10,y);
	m_labelzoomsize.SetSize(40,20);
	m_labelzoomsizelabel.SetPos(50,y);
	m_labelzoomsizelabel.SetString("Zoom level to start drawing labels at");
	m_tabs.AddObject(&m_labelzoomsize);
	m_tabs.AddObject(&m_labelzoomsizelabel);
	y+=25;

	/* distance units on map */
	m_disttype.SetPos(10,y);
	m_disttype.SetSize(100,20);
	InitDistCombo(&m_disttype);

	m_disttype.SetEventHandler(this,CALLBACKNAME(ReCalcDistsEvent));

	m_distcaption.SetPos(150,y);
	m_distcaption.SetString("Distance units");
	m_tabs.AddObject(&m_distcaption);
	m_tabs.AddObject(&m_disttype);
	y+=25;

}

void GPX::WPTableEvent(kGUIEvent *event)
{
	switch(event->GetEvent())
	{
	case EVENT_AFTERUPDATE:
		UpdateWPRender();
	break;
	case EVENT_ADDROW:
		GPXWPRenderRow *wpr=new GPXWPRenderRow();
		m_labelcolourtable.AddRow(wpr);
	break;
	}
}


GPXWPRenderRow::GPXWPRenderRow()
{
	int e,nf;

	m_objectlist[WPRENDER_FILTERNAME]=&m_filtercombo;
	m_objectlist[WPRENDER_BOXCOLOUR]=&m_colourcombo;
	m_objectlist[WPRENDER_NUMMATCHES]=&m_nummatches;
	m_filtercombo.SetSize(300,20);

	m_nummatches.SetLocked(true);

	nf=gpx->GetFilterObj()->GetNumFilters();
	m_filtercombo.SetNumEntries(nf);		/* list of select filters */
	for(e=0;e<nf;++e)
		m_filtercombo.SetEntry(e,gpx->GetFilterObj()->GetFilterName(e),e);

	gpx->InitColorCombo(&m_colourcombo);
}

/* load from prefs file */
void GPXWPRenderRow::Load(class kGUIXMLItem *wpr)
{
	m_filtercombo.SetSelection(wpr->Locate("filter")->GetValueString());
	m_colourcombo.SetSelection(wpr->Locate("boxcolour")->GetValueString());
}

/* save to prefs file */
void GPXWPRenderRow::Save(class kGUIXMLItem *wpr)
{
	wpr->AddParm("filter",m_filtercombo.GetSelectionString());
	wpr->AddParm("boxcolour",m_colourcombo.GetSelectionString());
}


GPXMapPathRow::GPXMapPathRow()
{
	m_objectlist[0]=&m_path;
	m_objectlist[1]=&m_browse;
	m_browse.SetFontID(1);		/* bold */
	m_browse.SetFontSize(20);	/* big! */
	m_browse.SetString("...");
	m_browse.SetEventHandler(this,CALLBACKNAME(Browse));
	m_path.SetEventHandler(this,CALLBACKNAME(PathChangedEvent));
	SetRowHeight(20);
}

void GPXMapPathRow::PathChanged(void)
{
	kGUIMsgBoxReq *box;

	if(m_path.GetLen())
	{
		/* ask to add maps right away! */
		box=new kGUIMsgBoxReq(MSGBOX_YES|MSGBOX_NO,this,CALLBACKNAME(DoAddMaps),true,"Scan Directory '%s' for Maps?",m_path.GetString());
	}
}

void GPXMapPathRow::DoAddMaps(int result)
{
	if(result==MSGBOX_YES)
	{
		kGUIString s;

		/* add maps right away! */
		gpx->AddMaps(m_path.GetString());
		gpx->UpdateMapMenu();
	}
}

/* load from prefs file */
void GPXMapPathRow::Load(class kGUIXMLItem *mpr)
{
	m_path.SetString(mpr->Locate("path")->GetValue());
}

/* save to prefs file */
void GPXMapPathRow::Save(class kGUIXMLItem *mpr)
{
	mpr->AddParm("path",m_path.GetString());
}

void GPXMapPathRow::Browse(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		kGUIFileReq *req;
	
		req=new kGUIFileReq(FILEREQ_OPEN,m_path.GetString(),".img;.map",this,CALLBACKNAME(BrowseDone));
	}
}

void GPXMapPathRow::BrowseDone(kGUIFileReq *result,int pressed)
{
	if(pressed==MSGBOX_OK)
	{
		m_path.SetString(result->GetPath());
		PathChanged();
	}
}

void GPX::LabelUp(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		unsigned int line;

		line=m_labelcolourtable.GetCursorRow();
		if(line>0 && line<m_labelcolourtable.GetNumChildren(0))
		{
			m_labelcolourtable.SwapRow(-1);
			m_labelcolourtable.MoveRow(-1);
			UpdateWPRender();
		}
	}
}

void GPX::LabelDown(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		int line;

		line=(int)m_labelcolourtable.GetCursorRow();
		if(line<((int)m_labelcolourtable.GetNumChildren(0)-1))
		{
			m_labelcolourtable.SwapRow(1);
			m_labelcolourtable.MoveRow(1);
			UpdateWPRender();
		}
	}
}

void GPX::UpdateWPRender(void)
{
	/* iterate through all waypoints and caculate which color to render them in */
	unsigned int e;
	GPXWPRenderRow *frow;

	for(e=0;e<m_labelcolourtable.GetNumChildren();++e)
	{
		frow=static_cast<GPXWPRenderRow *>(m_labelcolourtable.GetChild(e));	/* reset match counters */
		frow->ClearCount();
	}

	for(e=0;e<m_numwpts;++e)
		GetColour(m_wptlist.GetEntry(e));

	/* copy colors to the route table */
	m_routes.CopyColors();

	for(e=0;e<m_labelcolourtable.GetNumChildren();++e)
	{
		frow=static_cast<GPXWPRenderRow *>(m_labelcolourtable.GetChild(e));	/* reset match counters */
		frow->UpdateCount();
	}

	/* since the colours mave have changed, the map needs to be updated */
	MapDirty();
}

void GPX::NewMapPathEntry(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_ADDROW)
	{
		GPXMapPathRow *mpr=new GPXMapPathRow();
		m_mapdirstable.AddRow(mpr);
	}
}

void GPX::GetColour(GPXRow *row)
{
	int fe,fnr;
	kGUIObj *obj;
	GPXWPRenderRow *rfrow;
	kGUIColor colour;
	kGUIColor tcolour;

	fnr=m_labelcolourtable.GetNumChildren(0);
	colour=DrawColor(255,255,255);	/* default box to white */
	tcolour=DrawColor(0,0,0);	/* default text to black */
	for(fe=0;fe<fnr;++fe)
	{
		obj=m_labelcolourtable.GetChild(fe);
		rfrow=static_cast<GPXWPRenderRow *>(obj);	/* check against this filter */
		if(gpx->GetFilterObj()->FilterRow(rfrow->GetFilterNum(),row)==true)
		{
			rfrow->IncCount();
			colour=rfrow->GetColour();
			tcolour=rfrow->GetTColour();
			break;
		}
	}
	row->SetColour(colour);
	row->SetTColour(tcolour);
}

kGUIString *GPX::GetMapPath(unsigned int index)
{
	GPXMapPathRow *row;

	row=static_cast<GPXMapPathRow *>(m_mapdirstable.GetChild(index));
	return(row->GetPath());
}

