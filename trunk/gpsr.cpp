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

enum
{
GPS_NAME,			/* username */
GPS_BRAND,			/* garmin, magellan */
GPS_PORT,			/* port */
GPS_RATE,			/* speed */
GPS_NUMWPTS,		/* max number of points the GPS can hold */
GPS_MAXNAMELEN,		/* max # characters in name length*/
GPS_CHILDREN,		/* upload children too? */
GPS_NAMEFORMAT,		/* name format to use for uploading */
GPS_NUMCOLUMNS};

class GPXGPSRow : public kGUITableRowObj
{
public:
	GPXGPSRow();
	void Load(class kGUIXMLItem *gpr);	/* load from prefs file */
	void Save(class kGUIXMLItem *gpr);	/* save to prefs file */
	int GetNumObjects(void) {return GPS_NUMCOLUMNS;}
	kGUIObj **GetObjectList(void) {return m_objectlist;} 
	kGUIObj *m_objectlist[GPS_NUMCOLUMNS];
	//private:
	kGUIInputBoxObj m_name;
	kGUIComboBoxObj m_brand;
	kGUIComboBoxObj m_port;
	kGUIComboBoxObj m_rate;
	kGUIInputBoxObj m_maxwpts;
	kGUIInputBoxObj m_maxwptlen;
	kGUITickBoxObj m_children;
	kGUIComboBoxObj m_nameformat;
private:
	CALLBACKGLUEPTR(GPXGPSRow,BPChanged,kGUIEvent)
	void BPChanged(kGUIEvent *event);
};

/*****************/
/* list of GPSRs */
/*****************/

const char *gpsnames[GPS_NUMCOLUMNS]={
	"Name","Brand","Port","Rate","Max Waypoints","Max Name Length","Children","Name Format"};

const int gpswidths[GPS_NUMCOLUMNS]={300,120,60,75,85,105,65,150};

GPSrPage::GPSrPage()
{

}

GPSrPage::	~GPSrPage()
{
}

void GPSrPage::Init(kGUIContainerObj *obj)
{
	unsigned int i;
	int bw;

	bw=obj->GetChildZoneW();

	m_currentgps.SetPos(5,5);
	m_currentgps.SetSize(250,20);
	m_currentgps.SetNumEntries(1);				/* list of gpsrs */
	m_currentgps.SetEntry(0,"No GPSrs defined",-1);
	m_currentgps.SetSelection(-1);
	obj->AddObject(&m_currentgps);

	m_uploadwptstogps.SetFontSize(11);
	m_uploadwptstogps.SetPos(265,5);
	m_uploadwptstogps.SetSize(150,30);
	m_uploadwptstogps.SetString("Upload Waypoints to GPSr");
	m_uploadwptstogps.SetEventHandler(this,CALLBACKNAME(ClickUploadWptsToGPSr));
	obj->AddObject(&m_uploadwptstogps);

	m_downloadwptsfromgps.SetFontSize(11);
	m_downloadwptsfromgps.SetPos(425,5);
	m_downloadwptsfromgps.SetSize(150,30);
	m_downloadwptsfromgps.SetString("Download Waypoints from GPSr");
	m_downloadwptsfromgps.SetEventHandler(this,CALLBACKNAME(ClickDownloadWptsFromGPSr));
	obj->AddObject(&m_downloadwptsfromgps);

	m_uploadtrackstogps.SetFontSize(11);
	m_uploadtrackstogps.SetPos(585,5);
	m_uploadtrackstogps.SetSize(150,30);
	m_uploadtrackstogps.SetString("Upload Tracks to GPSr");
	m_uploadtrackstogps.SetEventHandler(this,CALLBACKNAME(ClickUploadTracksToGPSr));
	obj->AddObject(&m_uploadtrackstogps);

	m_downloadtracksfromgps.SetFontSize(11);
	m_downloadtracksfromgps.SetPos(745,5);
	m_downloadtracksfromgps.SetSize(150,30);
	m_downloadtracksfromgps.SetString("Download Tracks from GPSr");
	m_downloadtracksfromgps.SetEventHandler(this,CALLBACKNAME(ClickDownloadTracksFromGPSr));
	obj->AddObject(&m_downloadtracksfromgps);

	m_downloadfindsfromgps.SetFontSize(11);
	m_downloadfindsfromgps.SetPos(905,5);
	m_downloadfindsfromgps.SetSize(150,30);
	m_downloadfindsfromgps.SetString("Download Finds from GPSr");
	m_downloadfindsfromgps.SetEventHandler(this,CALLBACKNAME(ClickDownloadFindsFromGPSr));
	obj->AddObject(&m_downloadfindsfromgps);

	m_gpstable.SetPos(0,35);
	m_gpstable.SetSize(bw,150);
	m_gpstable.SetNumCols(GPS_NUMCOLUMNS);
	for(i=0;i<GPS_NUMCOLUMNS;++i)
	{
		m_gpstable.SetColTitle(i,gpsnames[i]);
		m_gpstable.SetColWidth(i,gpswidths[i]);
	}
	m_gpstable.SetAllowAddNewRow(true);
	m_gpstable.SetEventHandler(this,CALLBACKNAME(TableEvent));
	obj->AddObject(&m_gpstable);
}

void GPSrPage::TableEvent(kGUIEvent *event)
{
	switch(event->GetEvent())
	{
	case EVENT_AFTERUPDATE:
		UpdateGPSList();
	break;
	case EVENT_ADDROW:
	{
		GPXGPSRow *newgps;

		newgps=new GPXGPSRow();
		m_gpstable.AddRow(newgps);
	}
	break;
	}
}


void GPSrPage::Purge(void)
{
	m_gpstable.DeleteChildren();
}

void GPSrPage::LoadPrefs(kGUIXMLItem *root)
{
	unsigned int i;
	kGUIXMLItem *item;

	for(i=0;i<root->GetNumChildren();++i)
	{
		item=root->GetChild(i);
		if(!strcmp(item->GetName(),"gps"))
		{
			/* add gps to table */
			GPXGPSRow *lgps=new GPXGPSRow();
			lgps->Load(item);
			m_gpstable.AddRow(lgps);
		}
	}
	UpdateGPSList();
	gpx->Get(root,"currentgps",&m_currentgps);
//	if(root->Locate("currentgps"))
//		m_currentgps.SetSelectionz(root->Locate("currentgps")->GetValueString());
}

void GPSrPage::SavePrefs(kGUIXMLItem *root)
{
	unsigned int i;

	if(m_currentgps.GetSelection()>=0)
		root->AddChild("currentgps",m_currentgps.GetSelectionString());

	for(i=0;i<m_gpstable.GetNumChildren();++i)
	{
		GPXGPSRow *sgps=static_cast<GPXGPSRow *>(m_gpstable.GetChild(i));

		sgps->Save(root->AddChild("gps"));
	}
}

void GPSrPage::GenerateWptName(int format,unsigned int maxlen,kGUIString *name,Hash *names)
{
	unsigned int rindex;
	char cc[2];
	static char conschars[]={"BCDFGHJKLMNPQRSTVWXZbcdfghjklmnpqrstvwxz0123456789"};
	static char validchars[]={"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwnyz0123456789"};

	/* remove all invalid characters */
	name->Clean(validchars);

	switch(format)
	{
	case 0:
	case 1:
		/* if too long, remove spaces, then clip */
		rindex=0;
		while(rindex<maxlen && name->GetLen()>maxlen)
		{
			if(name->GetChar(rindex)==' ')
				name->Delete(rindex,1);
			else
				++rindex;
		}
		name->Clip(maxlen);
	break;
	case 2:	/* only use consonants or numbers, remove vowels and other characters */
		rindex=0;
		while(rindex<maxlen && name->GetLen()>maxlen)
		{
			cc[0]=name->GetChar(rindex);
			cc[1]=0;
			if(strstr(conschars,cc))
				++rindex;
			else
				name->Delete(rindex,1);
		}
		name->Clip(maxlen);
	break;
	}
	if(!name->GetLen())
		name->SetString("aaaa");

	rindex=name->GetLen()-1;

	do
	{
		if(!names->Find(name->GetString()))
			return;
		/* a collision occured, modify name */
back:	cc[0]=name->GetChar(rindex);
		if(cc[0]!='z' && cc[0]!='Z' && cc[0]!='9')
			name->SetChar(rindex,cc[0]+1);
		else
		{
			assert(rindex!=0,"nothing left to change!");
			--rindex;
			goto back;
		}
	}while(1);
}

void GPSrPage::ClickUploadWptsToGPSr(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		unsigned int i;
		unsigned int nc;
		int notfound;
		GPXGPSRow *gpsrow;
		UploadXML upload;
		kGUIString title;
		kGUIXML vxml;
		kGUIXMLItem *xmlitem;
		kGUIXMLItem *wp;
		kGUIString notfoundstring;
		kGUIMsgBoxReq *box;

		/* get pointer to selected gps */
		gpsrow=static_cast<GPXGPSRow *>(m_gpstable.GetChild(m_currentgps.GetSelection()));
		if(!gpsrow->m_maxwpts.GetInt())
		{
			box=new kGUIMsgBoxReq(MSGBOX_OK,false,"Maximum Waypoints allowed for this GPSr is currently set to zero, Please update the GPSr table entry for this GPSr to have a larger number.");
			return;
		}
		if(!gpsrow->m_maxwptlen.GetInt())
		{
			box=new kGUIMsgBoxReq(MSGBOX_OK,false,"Maximum Waypoint name length for this GPS is set to 0!");
			return;
		}

		nc=gpx->m_fwt->GetNumChildren(0);
		/* generate the gpx file to upload */
		upload.Init(gpsrow->m_maxwpts.GetInt(),gpsrow->m_maxwptlen.GetInt(),gpsrow->m_children.GetSelected(),gpsrow->m_nameformat.GetSelection());

		/* add all points to the upload file */
		for(i=0;i<nc;++i)
			upload.Add(static_cast<GPXRow *>(gpx->m_fwt->GetChild(i)));

		upload.Save("babel.gpx");

		if(gpsrow->m_children.GetSelected())
			title.Sprintf("Uploading %d Waypoints ( %d are Children ) to %s",upload.GetNumPoints(),upload.GetNumChildPoints(),gpsrow->m_name.GetString());
		else
			title.Sprintf("Uploading %d Waypoints to %s",upload.GetNumPoints(),gpsrow->m_name.GetString());

		if(UploadToGPS(GPSR_WAYPOINTS,gpsrow,"babel.gpx",title.GetString())==true)
		{
			/* verify waypoints */
			kGUI::FileDelete("babel.gpx");
			if(DownloadWptsFromGPSr()==true)
			{
				vxml.SetNameCache(&gpx->m_xmlnamecache);
				if(vxml.Load("babel.gpx")==true)
				{
					xmlitem=vxml.GetRootItem()->Locate("gpx");
					if(xmlitem)
					{
						for(i=0;i<xmlitem->GetNumChildren();++i)
						{
							wp=xmlitem->GetChild(i);
							if(!strcmp(wp->GetName(),"wpt"))
							{
								upload.Found(wp->Locate("name")->GetValueString(),wp->Locate("lat")->GetValueDouble(),wp->Locate("lon")->GetValueDouble());
							}
						}
					}
					kGUI::FileDelete("babel.gpx");
				}
			}
			notfound=upload.GetNotFound(&notfoundstring);
			box=new kGUIMsgBoxReq(MSGBOX_OK,true,"Number of verified points=%d, num not found=%d\n%s",upload.GetNumPoints()-notfound,notfound,notfoundstring.GetString());
		}
	}
}

bool GPSrPage::UploadToGPS(int what,GPXGPSRow *gpsrow,const char *filename,const char *title)
{
	kGUIBusy *busy;
	BabelGlue babel;

	babel.SetInput("gpx","babel.gpx");
	switch(what)
	{
	case GPSR_WAYPOINTS:
		babel.AddType(BABELTYPE_WAYPOINTS);
	break;
	case GPSR_TRACKS:
		babel.AddType(BABELTYPE_TRACKS);
	break;
	}
	babel.SetOutput(gpsrow->m_brand.GetSelectionString(),gpsrow->m_port.GetSelectionString(),gpsrow->m_rate.GetSelectionString());

	if(title)
	{
		busy=new kGUIBusy(600);
		busy->GetTitle()->SetString(title);
		busy->SetMax(500);
	}
	else
		busy=0;
	
	return(babel.Call(true,busy));
}

enum
{
	DOWNLOAD_WP,
	DOWNLOAD_FINDS
};

void GPSrPage::ClickDownloadWptsFromGPSr(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
		DownloadWP(DOWNLOAD_WP);
}

void GPSrPage::ClickDownloadFindsFromGPSr(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
		DownloadWP(DOWNLOAD_FINDS);
}

void GPSrPage::DownloadWP(int mode)
{
	unsigned int i;
	unsigned int c;
	unsigned int e;
	int numadded=0;
	bool found;
	double lat,lon;
	kGUIXML xml;
	kGUIXMLItem *xmlitem;
	kGUIXMLItem *wp;
	GPXRow *row;
	GPXChild *child;
	kGUIMsgBoxReq *box;
	kGUIString ll;
	kGUIString name;
	int id=0;
	int numdownloaded=0;
	GPSrWaypoints *gwp;
	ClassArray<GPSrWaypoints>*waypoints=0;
	kGUIString type;
	kGUIString desc;
	bool add;

	kGUI::FileDelete("babel.gpx");
	if(DownloadWptsFromGPSr()==true)
	{
		xml.SetNameCache(&gpx->m_xmlnamecache);
		if(xml.Load("babel.gpx")==true)
		{
			xmlitem=xml.GetRootItem()->Locate("gpx");
			if(xmlitem)
			{
				for(i=0;i<xmlitem->GetNumChildren();++i)
				{
					wp=xmlitem->GetChild(i);
					if(!strcmp(wp->GetName(),"wpt"))
					{
						++numdownloaded;
						lat=wp->Locate("lat")->GetValueDouble();
						lon=wp->Locate("lon")->GetValueDouble();
						bool gfound=false;

						if(wp->Locate("sym"))
						{
							type.SetString(wp->Locate("sym")->GetValue());
							if(!stricmp(type.GetString(),"geocache found"))
								gfound=true;
						}
						else
							type.Clear();

						/* does this position already exist in the fulllist? */
						found=false;
						add=false;
						row=0;
						for(e=0;e<gpx->m_numwpts;++e)
						{
							row=gpx->m_wptlist.GetEntry(e);
			
							if(fabs(row->GetLat()-lat)<0.0001f && fabs(row->GetLon()-lon)<0.0001f)
								found=true;
							else
							{
								/* look for match in all child waypoints */
								for(c=0;(c<row->m_numchildren) && (found==false);++c)
								{
									child=row->m_children.GetEntry(c);
									if(fabs(child->GetLat()-lat)<0.0001f && fabs(child->GetLon()-lon)<0.0001f)
										found=true;
								}
							}
							if(found)
							{
								if(mode==DOWNLOAD_FINDS)
								{
									if(row->GetFound()==false && gfound==true)
										add=true;
								}
								break;
							}
						}
						if(!found && mode==DOWNLOAD_WP)
							add=true;

						/* if not found, then add to list as user point */
						if(add)
						{
							if(mode==DOWNLOAD_FINDS)
								name.SetString(row->GetWptName());
							else
							{
								if(wp->Locate("name"))
									name.SetString(wp->Locate("name")->GetValue());
								else
								{
									/* no name so pick USER_xxx */
									do
									{
										++id;
										name.Sprintf("USER_%d",id);
										if(!gpx->Locate(&name))
											break;
									}while(1);
								}
							}

							if(!waypoints)
							{
								waypoints=new ClassArray<GPSrWaypoints>;
								waypoints->Init(500,100);
							}
							if(wp->Locate("desc"))
								desc.SetString(wp->Locate("desc")->GetValue());

							gwp=waypoints->GetEntryPtr(numadded++);
							gwp->SetLat(lat);
							gwp->SetLon(lon);
							gwp->SetName(&name);
							gwp->SetType(&type);
							gwp->SetDesc(&desc);
						}
					}
				}
			}
		}
		gpx->BSPDirty();
		gpx->ReFilter();
		if(!numadded)
		{
			if(mode==DOWNLOAD_WP)
				box=new kGUIMsgBoxReq(MSGBOX_OK,true,"%d Waypoints were recieved, all of them already exist.",numdownloaded);
			else
				box=new kGUIMsgBoxReq(MSGBOX_OK,true,"No geocaches found that need updating");
		}
		else
		{
			SelectWaypoints *sw;

			sw=new SelectWaypoints(numadded,waypoints,mode);
		}
	}
}

enum
{
LOADWP_SELECTED,
LOADWP_LOG,
LOADWP_LAT,
LOADWP_LON,
LOADWP_NAME,
LOADWP_TYPE,
LOADWP_DESC,
LOADWP_NUMCOLUMNS
};

class SelectWaypointsRow : public kGUITableRowObj
{
public:
	SelectWaypointsRow(double lat,double lon,kGUIString *name,kGUIString *type,kGUIString *desc);
	~SelectWaypointsRow() {}
	int GetNumObjects(void) {return LOADWP_NUMCOLUMNS;}
	kGUIObj **GetObjectList(void) {return m_objectlist;} 
	kGUIObj *m_objectlist[LOADWP_NUMCOLUMNS];
	bool GetSelected(void) {return m_selected.GetSelected();}
	void ToggleSelected(void) {m_selected.SetSelected(!m_selected.GetSelected());}
private:
	void ClickLog(kGUIEvent *event);
	CALLBACKGLUEPTR(SelectWaypointsRow,ClickLog,kGUIEvent)
	kGUITickBoxObj m_selected;
	kGUIButtonObj m_log;
	kGUIInputBoxObj m_lat;
	kGUIInputBoxObj m_lon;
	kGUIInputBoxObj m_name;
	kGUIInputBoxObj m_type;
	kGUIInputBoxObj m_desc;
};

SelectWaypointsRow::SelectWaypointsRow(double lat,double lon,kGUIString *name,kGUIString *type,kGUIString *desc)
{
	GPXCoord c;

	m_objectlist[LOADWP_SELECTED]=&m_selected;
	m_objectlist[LOADWP_LOG]=&m_log;
	m_objectlist[LOADWP_LAT]=&m_lat;
	m_objectlist[LOADWP_LON]=&m_lon;
	m_objectlist[LOADWP_NAME]=&m_name;
	m_objectlist[LOADWP_TYPE]=&m_type;
	m_objectlist[LOADWP_DESC]=&m_desc;

	m_log.SetFontSize(12);
	m_lat.SetFontSize(12);
	m_lon.SetFontSize(12);
	m_name.SetFontSize(12);
	m_type.SetFontSize(12);
	m_desc.SetFontSize(12);

	m_log.SetString("Log Find");
	m_log.SetEventHandler(this,CALLBACKNAME(ClickLog));

	c.Set(lat,lon);
	c.Output(&m_lat,&m_lon);
	m_name.SetString(name);
	m_type.SetString(type);
	m_desc.SetString(desc);

	m_lat.SetLocked(true);
	m_lon.SetLocked(true);
	m_name.SetLocked(true);
	m_type.SetLocked(true);
	m_desc.SetLocked(true);
	SetRowHeight(20);
}

void SelectWaypointsRow::ClickLog(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		GPXRow *gpxrow;

		gpxrow=gpx->Locate(&m_name);
		if(!gpxrow)
			return;

		gpx->Browse(BROWSE_URL,gpxrow->GetURL());
		m_selected.SetSelected(!m_selected.GetSelected());
	}
}

const char *lwcolnames[LOADWP_NUMCOLUMNS]={
	"Select","Log","Lat","Lon","Name","Type","Description"};

const char *lwcoldesc[LOADWP_NUMCOLUMNS]={
	"Select Waypoint","Log Find","Latitude","Longitude","Name","Type","Description"};
	
const int lwcolwidths[LOADWP_NUMCOLUMNS]={40,65,80,80,130,140,300};

SelectWaypoints::SelectWaypoints(unsigned int num,ClassArray<GPSrWaypoints> *waypoints,int mode)
{
	unsigned int i;
	SelectWaypointsRow *tsr;
	GPSrWaypoints *gwp;

	m_mode=mode;
	m_wp=waypoints;
	if(m_mode==DOWNLOAD_WP)
		m_window.SetTitle("Select Waypoints for download");
	else
		m_window.SetTitle("Found Geocaches");

	m_window.SetEventHandler(this,CALLBACKNAME(WindowEvent));

	m_list.SetSize(760,300);
	m_list.SetNumCols(LOADWP_NUMCOLUMNS);
	if(m_mode==DOWNLOAD_WP)
		m_list.SetColShow(LOADWP_LOG,false);		/* hide this column if waypoint download mode, show if log finds mode */

	for(i=0;i<LOADWP_NUMCOLUMNS;++i)
	{
		m_list.SetColTitle(i,lwcolnames[i]);
		m_list.SetColHint(i,lwcoldesc[i]);
		m_list.SetColWidth(i,lwcolwidths[i]);
	}

	m_controls.AddObject(&m_list);
	m_controls.NextLine();

	m_toggle.SetSize(100,20);
	m_toggle.SetString("Toggle");
	m_toggle.SetEventHandler(this,CALLBACKNAME(PressToggle));

	m_update.SetSize(100,20);
	m_update.SetString("Update Finds");
	m_update.SetEventHandler(this,CALLBACKNAME(PressUpdateFinds));

	m_cancel.SetSize(100,20);
	m_cancel.SetString("Cancel");
	m_cancel.SetEventHandler(this,CALLBACKNAME(PressCancel));

	m_ok.SetSize(100,20);
	m_ok.SetString("Done");
	m_ok.SetEventHandler(this,CALLBACKNAME(PressDone));

	m_controls.AddObject(&m_toggle);
	m_controls.AddObject(&m_update);
	m_controls.AddObject(&m_cancel);
	m_controls.AddObject(&m_ok);
	m_controls.NextLine();

	m_window.AddObject(&m_controls);
	m_window.SetSize(100,100);
	m_window.ExpandToFit();
	m_window.Center();
	m_window.SetTop(true);
	kGUI::AddWindow(&m_window);

	for(i=0;i<num;++i)
	{
		gwp=waypoints->GetEntryPtr(i);
		tsr=new SelectWaypointsRow(gwp->GetLat(),gwp->GetLon(),gwp->GetName(),gwp->GetType(),gwp->GetDesc());
		m_list.AddRow(tsr);
	}
}

void SelectWaypoints::WindowEvent(kGUIEvent *event)
{
	switch(event->GetEvent())
	{
	case EVENT_CLOSE:
		delete m_wp;
		delete this;
	break;
	}
}

void SelectWaypoints::PressCancel(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
		m_window.Close();
}

void SelectWaypoints::PressToggle(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		unsigned int i;
		SelectWaypointsRow *rsr;

		/* toggle checkboxes */
		for(i=0;i<m_list.GetNumChildren();++i)
		{
			rsr=static_cast<SelectWaypointsRow *>(m_list.GetChild(i));
			rsr->ToggleSelected();
		}
	}
}


void SelectWaypoints::PressUpdateFinds(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		unsigned int i;
		SelectWaypointsRow *rsr;
		GPXRow *gpxrow;
		GPSrWaypoints *gwp;

		/* set the finds to the tickboxes */
		for(i=0;i<m_list.GetNumChildren();++i)
		{
			rsr=static_cast<SelectWaypointsRow *>(m_list.GetChild(i));
			gwp=m_wp->GetEntryPtr(i);
			gpxrow=gpx->Locate(gwp->GetName());
			if(gpxrow)
				gpxrow->SetFound(rsr->GetSelected());
		}
	}
}

void SelectWaypoints::PressDone(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		unsigned int i;
		SelectWaypointsRow *rsr;
		GPXRow *userrow;
		GPSrWaypoints *gwp;

		/* add names of selected tracks to hash table */
		for(i=0;i<m_list.GetNumChildren();++i)
		{
			rsr=static_cast<SelectWaypointsRow *>(m_list.GetChild(i));
			if(rsr->GetSelected()==true)
			{
				gwp=m_wp->GetEntryPtr(i);

				userrow=new GPXRow();
				userrow->SetName(gwp->GetName());
				userrow->SetWptName(gwp->GetName());
				
				userrow->SetCoord(gwp->GetLat(),gwp->GetLon());
				userrow->UpdateLabelName();

				gpx->m_wptlist.SetEntry(gpx->m_numwpts++,userrow);
				gpx->GetColour(userrow);		/* calc label colour */
			}
		}

		m_window.Close();
	}
}


bool GPSrPage::DownloadWptsFromGPSr(void)
{
	kGUIBusy *busy;
	kGUIString title;
	GPXGPSRow *gpsrow;
	BabelGlue babel;

	/* get pointer to selected gps */
	gpsrow=static_cast<GPXGPSRow *>(m_gpstable.GetChild(m_currentgps.GetSelection()));
	title.Sprintf("Downloading Waypoints from %s",gpsrow->m_name.GetString());
	
	busy=new kGUIBusy(600);
	busy->GetTitle()->SetString(title.GetString());
	busy->SetMax(500);

	babel.SetInput(gpsrow->m_brand.GetSelectionString(),gpsrow->m_port.GetSelectionString(),gpsrow->m_rate.GetSelectionString());
	babel.AddType(BABELTYPE_WAYPOINTS);
	babel.SetOutput("gpx","babel.gpx");

	return(babel.Call(true,busy));
}

/* step 1, select tracks to upload */
/* step 2, upload them */
/* step 3, verify */

void GPSrPage::ClickUploadTracksToGPSr(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		SelectTracks *lt;

		gpx->m_temphash.Init(12,0);
		lt=new SelectTracks(&gpx->m_temphash,this,CALLBACKNAME(ClickUploadTracksToGPSr2),0);
	}
}

/* hash list contains names of tracks to upload */

void GPSrPage::ClickUploadTracksToGPSr2(int pressed)
{
	int numupload;
	GPXGPSRow *gpsrow;
	UploadXML upload;
	kGUIString title;
	kGUIXML vxml;
	kGUIString notfoundstring;
	kGUIMsgBoxReq *box;

	/* build the xml file to upload, then upload it */

	if(pressed!=MSGBOX_OK)
		return;

	numupload=gpx->GetTrackObj()->SavePrefs(upload.GetXMLRoot(),&gpx->m_temphash,upload.GetBoundsObj());

	if(!numupload)
	{
		box=new kGUIMsgBoxReq(MSGBOX_OK,false,"No Tracks selected to upload");
		return;
	}

	/* save the upload file */

	upload.Save("babel.gpx");

	/* get pointer to selected gps */
	gpsrow=static_cast<GPXGPSRow *>(m_gpstable.GetChild(m_currentgps.GetSelection()));
	title.Sprintf("Uploading %d Tracks to %s",numupload,gpsrow->m_name.GetString());

	if(UploadToGPS(GPSR_TRACKS,gpsrow,"babel.gpx",title.GetString())==true)
	{
		kGUIString results;

		/* verify tracks */
		kGUI::FileDelete("babel.gpx");
		
		if(DownloadTracksFromGPSr()==false)
			return;

		gpx->GetTrackObj()->Verify("babel.gpx",&gpx->m_temphash,&results);

		kGUI::FileDelete("babel.gpx");

		box=new kGUIMsgBoxReq(MSGBOX_OK,false,results.GetString());
	}
}

/* download all tracks to babel.gpx */
bool GPSrPage::DownloadTracksFromGPSr(void)
{
	kGUIBusy *busy;
	kGUIString title;
	GPXGPSRow *gpsrow;
	BabelGlue babel;

	/* get pointer to selected gps */
	gpsrow=static_cast<GPXGPSRow *>(m_gpstable.GetChild(m_currentgps.GetSelection()));
	title.Sprintf("Downloading Tracks from %s",gpsrow->m_name.GetString());
	
	busy=new kGUIBusy(600);
	busy->GetTitle()->SetString(title.GetString());
	busy->SetMax(500);

	babel.SetInput(gpsrow->m_brand.GetSelectionString(),gpsrow->m_port.GetSelectionString(),gpsrow->m_rate.GetSelectionString());
	babel.AddType(BABELTYPE_TRACKS);
	babel.SetOutput("gpx","babel.gpx");

	return(babel.Call(true,busy));
}

void GPSrPage::PreLoadXML(int current,int size)
{
	if(!m_busy)
	{
		m_busy=new kGUIBusy(600);
		m_busy->GetTitle()->SetString("Loading GPX File");
		m_busy->SetMax(size);
	}
	m_busy->SetCur(current);
}

void GPSrPage::ClickDownloadTracksFromGPSr(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		/* 1. download from gps to babel.gpx */
		/* 2. show list of tracks and let user decide which ones to import */
		kGUIXML *xml;
		kGUIXMLItem *xroot;
		kGUIXMLItem *xi;
		kGUIMsgBoxReq *box;
		int numtrk;

		if(DownloadTracksFromGPSr()==false)
			return;

		/* load and verify that file contains tracks */

		m_busy=0;
		xml=new kGUIXML();
		xml->SetNameCache(&gpx->m_xmlnamecache);
		xml->SetLoadingCallback(this,CALLBACKNAME(PreLoadXML));

		if(xml->Load("babel.gpx")==false)
		{
			box=new kGUIMsgBoxReq(MSGBOX_OK,true,"Error: cannot opening file '%s'!","babel.gpx");
			delete xml;
			delete m_busy;
			return;
		}
		delete m_busy;

		/* check to make sure file contains tracks */

		numtrk=0;
		xroot=xml->GetRootItem()->Locate("gpx");
		if(xroot)
		{
			unsigned int i;

			for(i=0;i<xroot->GetNumChildren();++i)
			{
				xi=xroot->GetChild(i);
				if(!strcmp(xi->GetName(),"trk"))
					++numtrk;
			}
		}

		/* 2. Step 2, bring up load track select requestor */

		if(numtrk)
		{
			/* load track settings */
			SelectTracks *lt;

			gpx->m_tempxml=xml;
			lt=new SelectTracks(&gpx->m_temphash,gpx,CALLBACKCLASSNAME(GPX,LoadTracks),xml);
		}
		else
		{
			box=new kGUIMsgBoxReq(MSGBOX_OK,false,"Error: No tracks were downloaded!");
			delete xml;
		}
	}
}


/********************************************************************************************/

GPXGPSRow::GPXGPSRow()
{
	m_objectlist[GPS_NAME]=&m_name;
	m_objectlist[GPS_BRAND]=&m_brand;
	m_objectlist[GPS_PORT]=&m_port;
	m_objectlist[GPS_RATE]=&m_rate;
	m_objectlist[GPS_NUMWPTS]=&m_maxwpts;
	m_objectlist[GPS_MAXNAMELEN]=&m_maxwptlen;
	m_objectlist[GPS_CHILDREN]=&m_children;
	m_objectlist[GPS_NAMEFORMAT]=&m_nameformat;

	m_brand.SetEventHandler(this,CALLBACKNAME(BPChanged));
	m_port.SetEventHandler(this,CALLBACKNAME(BPChanged));
	
	m_brand.SetNumEntries(3);		/* list of brands */
	m_brand.SetEntry(0,"Garmin",0);
	m_brand.SetEntry(1,"Magellan",0);
	m_brand.SetEntry(2,"nmea",0);

	m_port.SetNumEntries(9);		/* list of ports */
	m_port.SetEntry(0,"COM1",0);
	m_port.SetEntry(1,"COM2",1);
	m_port.SetEntry(2,"COM3",2);
	m_port.SetEntry(3,"COM4",3);
	m_port.SetEntry(4,"COM5",4);
	m_port.SetEntry(5,"COM6",5);
	m_port.SetEntry(6,"COM7",6);
	m_port.SetEntry(7,"COM8",7);
	m_port.SetEntry(8,"USB",8);

	m_rate.SetNumEntries(7);		/* list of rates */
	m_rate.SetEntry(0,"1200",0);
	m_rate.SetEntry(1,"4800",1);
	m_rate.SetEntry(2,"9600",2);
	m_rate.SetEntry(3,"19200",3);
	m_rate.SetEntry(4,"57600",4);
	m_rate.SetEntry(5,"115200",5);
	m_rate.SetEntry(6,"N/A",6);

	m_nameformat.SetNumEntries(3);
	m_nameformat.SetEntry(0,"Waypoint Name",UPNAME_WAYNAME);
	m_nameformat.SetEntry(1,"Name, Truncated",UPNAME_TRUNCATE);
	m_nameformat.SetEntry(2,"Name, Drop Vowels",UPNAME_DROPVOWELS);

	SetRowHeight(20);
}

/* brand or port has been changed */
void GPXGPSRow::BPChanged(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_AFTERUPDATE)
	{
		/* if USB, then set rate to N/A */
		if(m_port.GetSelection()==8)	/* USB? */
		{
			m_rate.SetSelection(6);
			m_rate.SetLocked(true);
		}
		else
		{
			/* rate for garmin is locked at 9600 */

			if(!m_brand.GetSelection())
			{
				m_rate.SetSelection(2);
				m_rate.SetLocked(true);
			}
			else
			{
				m_rate.SetLocked(false);
			}
		}
	}
}

/* load from prefs file */
void GPXGPSRow::Load(class kGUIXMLItem *gpr)
{
	m_name.SetString(gpr->Locate("name")->GetValue());
	m_brand.SetSelection(gpr->Locate("brand")->GetValueString());
	m_port.SetSelection(gpr->Locate("port")->GetValueString());
	m_rate.SetSelection(gpr->Locate("rate")->GetValueString());
	m_maxwpts.SetString(gpr->Locate("maxwpnts")->GetValue());
	m_maxwptlen.SetString(gpr->Locate("maxwpntlen")->GetValue());
	if(gpr->Locate("children"))
		m_children.SetSelected(!strcmp(gpr->Locate("children")->GetValueString(),"False")?false:true);
	if(gpr->Locate("nameformat"))
		m_nameformat.SetSelection(gpr->Locate("nameformat")->GetValueString());
}

/* save to prefs file */
void GPXGPSRow::Save(class kGUIXMLItem *gpr)
{
	gpr->AddParm("name",m_name.GetString());
	gpr->AddParm("brand",m_brand.GetSelectionString());
	gpr->AddParm("port",m_port.GetSelectionString());
	gpr->AddParm("rate",m_rate.GetSelectionString());
	gpr->AddParm("maxwpnts",m_maxwpts.GetString());
	gpr->AddParm("maxwpntlen",m_maxwptlen.GetString());
	gpr->AddParm("children",m_children.GetSelected()==false?"False":"True");
	gpr->AddParm("nameformat",m_nameformat.GetSelectionString());
}

#if 0
GPXGPSRow *GPSrPage::LocateGPSR(const char *name)
{
	int e,numgpses;
	GPXGPSRow *gpsrow;

	numgpses=m_gpstable.GetNumChildren();
	for(e=0;e<numgpses;++e)
	{
		gpsrow=static_cast<GPXGPSRow *>(m_gpstable.GetChild(e));
		if(!stricmp(gpsrow->m_name.GetString(),name))
			return(gpsrow);
	}
	return(0);	/* gpsr not found in table */
}
#endif

void GPSrPage::UpdateGPSList(void)
{
	/* update pulldowns that have a gps names in them */
	int e,numgpses;

	numgpses=m_gpstable.GetNumChildren();
	if(!numgpses)
	{
		m_uploadwptstogps.SetEnabled(false);
		m_downloadwptsfromgps.SetEnabled(false);
		m_uploadtrackstogps.SetEnabled(false);
		m_downloadtracksfromgps.SetEnabled(false);

		m_currentgps.SetNumEntries(1);			/* no valid entry flag */
		m_currentgps.SetEntry(0,"No GPSrs defined",-1);
		m_currentgps.SetSelection(-1);

		gpx->m_realtimegps.SetNumEntries(1);			/* no valid entry flag */
		gpx->m_realtimegps.SetEntry(0,"No GPSrs defined",-1);
		gpx->m_realtimegps.SetSelection(-1);
	}
	else
	{
		kGUIString curgps;
		kGUIString currtgps;

		/* save current upload/down GPSr */
		if(m_currentgps.GetSelection()>=0)
			curgps.SetString(m_currentgps.GetSelectionString());

		/* save current realtime tracking GPSr */
		if(gpx->m_realtimegps.GetSelection()>=0)
			currtgps.SetString(gpx->m_realtimegps.GetSelectionString());

		m_uploadwptstogps.SetEnabled(gpx->m_fwt->GetNumChildren(0)>0);
		m_uploadtrackstogps.SetEnabled(gpx->GetTrackObj()->GetNumTracks()>0);
		m_downloadwptsfromgps.SetEnabled(true);
		m_downloadtracksfromgps.SetEnabled(true);

		m_currentgps.SetNumEntries(numgpses);
		gpx->m_realtimegps.SetNumEntries(numgpses);
		for(e=0;e<numgpses;++e)
		{
			GPXGPSRow *gpsrow;

			gpsrow=static_cast<GPXGPSRow *>(m_gpstable.GetChild(e));

			m_currentgps.SetEntry(e,gpsrow->m_name.GetString(),e);
			gpx->m_realtimegps.SetEntry(e,gpsrow->m_name.GetString(),e);
		}

		/* put selected GPSrs back ( if they still exist ) */
		if(curgps.GetLen())
			m_currentgps.SetSelectionz(curgps.GetString());
		if(currtgps.GetLen())
			gpx->m_realtimegps.SetSelectionz(currtgps.GetString());
	}
}

//0=ok, 1=gpsrname not found,2=error communicating with GPSr,3=file not found!
int GPSrPage::UploadToGPSr(const char *gpsname,const char *filename)
{
	int rc;
	unsigned int e;
	GPXGPSRow *gpsrow;

	for(e=0;e<m_gpstable.GetNumChildren();++e)
	{
		gpsrow=static_cast<GPXGPSRow *>(m_gpstable.GetChild(e));
		if(!stricmp(gpsrow->m_name.GetString(),gpsname))
		{
			kGUI::GetAccess();
			if(UploadToGPS(GPSR_WAYPOINTS,gpsrow,filename,0)==true)
				rc=0;
			else
				rc=2;
			kGUI::ReleaseAccess();
			return(rc);
		}
	}
	return(1);	/* gpsr not found in table */

}

//0=ok, 1=gpsrname not found,2=error communicating with GPSr

int GPSrPage::UploadToGPSr(const char *gpsname,unsigned int num,Array<GPXRow *>*list)
{
	int rc;
	unsigned int e;
	unsigned int i;
	GPXGPSRow *gpsrow;
	UploadXML upload;

	for(e=0;e<m_gpstable.GetNumChildren();++e)
	{
		gpsrow=static_cast<GPXGPSRow *>(m_gpstable.GetChild(e));
		if(!stricmp(gpsrow->m_name.GetString(),gpsname))
		{
			/* generate the gpx file to upload */
			upload.Init(gpsrow->m_maxwpts.GetInt(),gpsrow->m_maxwptlen.GetInt(),gpsrow->m_children.GetSelected(),gpsrow->m_nameformat.GetSelection());

			/* add all points to the upload file */
			for(i=0;i<num;++i)
				upload.Add(list->GetEntry(i));
			upload.Save("babel.gpx");

			kGUI::GetAccess();
			if(UploadToGPS(GPSR_WAYPOINTS,gpsrow,"babel.gpx",0)==true)
				rc=0;
			else
				rc=2;
			kGUI::ReleaseAccess();
			return(rc);
		}
	}
	return(1);	/* gpsr not found in table */
}

/* given a GPSr number, set input for a given babel object */

void GPSrPage::SetInput(int index,class BabelGlue *babel)
{
	GPXGPSRow *gpsrow;

	/* get pointer to selected gps */
	gpsrow=static_cast<GPXGPSRow *>(m_gpstable.GetChild(index));
	babel->SetInput(gpsrow->m_brand.GetSelectionString(),gpsrow->m_port.GetSelectionString(),gpsrow->m_rate.GetSelectionString());
}
