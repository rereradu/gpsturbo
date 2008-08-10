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
	int bw=m_tabs.GetChildZoneW();
	int bh=m_tabs.GetChildZoneH();

	m_drawsettingcontrols.SetPos(0,0);
	m_drawsettingcontrols.SetSize(bw,0);
	m_drawsettingcontrols.SetMaxHeight(9999);

	m_labelup.SetFontSize(BUTTONFONTSIZE);
	m_labelup.SetString(gpx->GetString(STRING_UP));
	m_labelup.Contain();
	m_labelup.SetEventHandler(this,CALLBACKNAME(LabelUp));
	m_drawsettingcontrols.AddObject(&m_labelup);

	m_labeldown.SetFontSize(BUTTONFONTSIZE);
	m_labeldown.SetString(gpx->GetString(STRING_DOWN));
	m_labeldown.Contain();
	m_labeldown.SetEventHandler(this,CALLBACKNAME(LabelDown));
	m_drawsettingcontrols.AddObject(&m_labeldown);
	m_drawsettingcontrols.NextLine();

	/* labels table */
	m_labelcolourtable.SetSize(625,215);
	m_labelcolourtable.SetNumCols(WPRENDER_NUMCOLUMNS);
	for(i=0;i<WPRENDER_NUMCOLUMNS;++i)
	{
		m_labelcolourtable.SetColTitle(i,wprnames[i]);
		m_labelcolourtable.SetColWidth(i,wprwidths[i]);
	}
	m_labelcolourtable.SetAllowAddNewRow(true);
	m_labelcolourtable.SetEventHandler(this,CALLBACKNAME(WPTableEvent));
	m_drawsettingcontrols.AddObject(&m_labelcolourtable);
	m_drawsettingcontrols.NextLine();

	/* map path table */
	m_mapdirstable.SetSize(625,100);
	m_mapdirstable.SetNumCols(2);
	m_mapdirstable.SetColTitle(0,"Map Path");
	m_mapdirstable.SetColWidth(0,520);
	m_mapdirstable.SetColTitle(1,"Browse");
	m_mapdirstable.SetColWidth(1,50);

	m_mapdirstable.SetAllowAddNewRow(true);
	m_mapdirstable.SetEventHandler(this,CALLBACKNAME(NewMapPathEntry));
	m_drawsettingcontrols.AddObject(&m_mapdirstable);
	m_drawsettingcontrols.NextLine();

	m_language.SetNumEntries(m_locstrings.GetNumLanguages());
	for(i=0;i<m_locstrings.GetNumLanguages();++i)
		m_language.SetEntry(i,kGUI::GetString(KGUISTRING_ENGLISH+i),i);
	m_language.SetSize(200,25);

	m_languagelabel.SetString(kGUI::GetString(KGUISTRING_LANGUAGE));
//	m_language.SetSize(200,25);
//	m_languagelabel.Contain();
	m_drawsettingcontrols.AddObject(&m_language);
	m_drawsettingcontrols.AddObject(&m_languagelabel);
	m_drawsettingcontrols.NextLine();

	m_usebrowser.SetPos(10,0);
	m_usebrowserlabel.SetPos(30,0);
	m_usebrowserlabel.SetString("Use Built-In Browser");
	m_drawsettingcontrols.AddObjects(2,&m_usebrowser,&m_usebrowserlabel);
	m_drawsettingcontrols.NextLine();

	m_maxdownloads.SetString("8");
	m_maxdownloads.SetPos(10,0);
	m_maxdownloads.SetSize(40,20);
	m_maxdownloadslabel.SetPos(50,0);
	m_maxdownloadslabel.SetString("Maximum Concurrent Internet Downloads");
	m_drawsettingcontrols.AddObjects(2,&m_maxdownloads,&m_maxdownloadslabel);
	m_drawsettingcontrols.NextLine();

	m_tablefontsize.SetString("12");
	m_tablefontsize.SetPos(10,0);
	m_tablefontsize.SetSize(40,20);
	m_tablefontsize.SetEventHandler(this,CALLBACKNAME(TableFontSizeChangedEvent));
	m_tablefontsizelabel.SetPos(50,0);
	m_tablefontsizelabel.SetString("Font Size for Waypoint Tables");
	m_drawsettingcontrols.AddObjects(2,&m_tablefontsize,&m_tablefontsizelabel);
	m_drawsettingcontrols.NextLine();

	m_labelfontsize.SetString("12");
	m_labelfontsize.SetPos(10,0);
	m_labelfontsize.SetSize(40,20);
	m_labelfontsize.SetEventHandler(this,CALLBACKNAME(LabelFontSizeChangedEvent));
	m_labelfontsizelabel.SetPos(50,0);
	m_labelfontsizelabel.SetString("Font Size for Waypoint Labels on Map");
	m_drawsettingcontrols.AddObjects(2,&m_labelfontsize,&m_labelfontsizelabel);

	GPX::InitAlphaCombo(&m_labelalpha);
	m_labelalpha.SetSelection(70);
	m_labelalpha.SetEventHandler(this,CALLBACKNAME(MapDirtyEvent));
	m_drawsettingcontrols.AddObject(&m_labelalpha);
	m_drawsettingcontrols.NextLine();

	m_shownumticks.SetString("0");
	m_shownumticks.SetPos(10,0);
	m_shownumticks.SetSize(40,20);
	m_shownumticks.SetEventHandler(this,CALLBACKNAME(LabelFontSizeChangedEvent));
	m_shownumtickslabel.SetPos(50,0);
	m_shownumtickslabel.SetString("Number of User Ticks to show beside Label on Map");
	m_drawsettingcontrols.AddObjects(2,&m_shownumticks,&m_shownumtickslabel);
	m_drawsettingcontrols.NextLine();

	m_wptnamelabel.SetPos(10,0);
	m_wptnamelabel.SetString("Waypoint name to use on Map");
	m_wptname.SetPos(200,0);
	m_wptname.SetSize(400,20);
	m_wptname.SetEventHandler(this,CALLBACKNAME(LabelNamesChangedEvent));
	m_drawsettingcontrols.AddObjects(2,&m_wptname,&m_wptnamelabel);
	m_drawsettingcontrols.NextLine();

	m_showchildren.SetPos(10,0);
	m_showchildren.SetEventHandler(this,CALLBACKNAME(BSPDirtyEvent));
	m_showchildrenlabel.SetPos(30,0);
	m_showchildrenlabel.SetString("Show Labels for Additional Waypoints on Map");
	m_drawsettingcontrols.AddObjects(2,&m_showchildren,&m_showchildrenlabel);
	m_drawsettingcontrols.NextLine();

	m_movelabels.SetPos(10,0);
	m_movelabels.SetSelected(true);	/* default to selected */
	m_movelabels.SetEventHandler(this,CALLBACKNAME(BSPDirtyEvent));
	m_movelabelslabel.SetPos(30,0);
	m_movelabelslabel.SetString("Move Overlapping Labels");
	m_drawsettingcontrols.AddObjects(2,&m_movelabels,&m_movelabelslabel);
	m_drawsettingcontrols.NextLine();

	m_movelabelsmax.SetPos(10,0);
	m_movelabelsmax.SetSize(40,20);
	m_movelabelsmax.SetString("128");
	m_movelabelsmax.SetEventHandler(this,CALLBACKNAME(BSPDirtyEvent));
	m_movelabelsmaxlabel.SetPos(50,0);
	m_movelabelsmaxlabel.SetString("Move Overlapping Labels, Maximum number of pixels");
	m_drawsettingcontrols.AddObjects(2,&m_movelabelsmax,&m_movelabelsmaxlabel);
	m_drawsettingcontrols.NextLine();

	m_labelzoomsize.SetString("10");
	m_labelzoomsize.SetPos(10,0);
	m_labelzoomsize.SetSize(40,20);
	m_labelzoomsizelabel.SetPos(50,0);
	m_labelzoomsizelabel.SetString("Zoom level to start drawing labels at");
	m_drawsettingcontrols.AddObjects(2,&m_labelzoomsize,&m_labelzoomsizelabel);
	m_drawsettingcontrols.NextLine();

	/* distance units on map */
	m_disttype.SetPos(10,0);
	m_disttype.SetSize(100,20);
	InitDistCombo(&m_disttype);
	m_disttype.SetEventHandler(this,CALLBACKNAME(ReCalcDistsEvent));

	m_distcaption.SetPos(150,0);
	m_distcaption.SetString("Distance units");
	m_drawsettingcontrols.AddObjects(2,&m_distcaption,&m_disttype);
	m_drawsettingcontrols.NextLine();

	m_drawsettingsarea.SetPos(0,0);
	m_drawsettingsarea.SetMaxWidth(m_drawsettingcontrols.GetZoneW());
	m_drawsettingsarea.SetMaxHeight(m_drawsettingcontrols.GetZoneH());
	m_drawsettingsarea.SetSize(bw,bh);
	printf("bw=%d,bh=%d,maxw=%d,maxh=%d\n",bw,bh,m_drawsettingcontrols.GetZoneW(),m_drawsettingcontrols.GetZoneH());
	fflush(stdout);
	m_drawsettingsarea.AddObject(&m_drawsettingcontrols);
	m_tabs.AddObject(&m_drawsettingsarea);
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

