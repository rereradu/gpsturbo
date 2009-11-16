#if 0
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
#include "kguicsv.h"

enum
{
DOWNLOAD_IBYCUS_TOPOCANADA,
DOWNLOAD_IBYCUS_TOPOUSA,
DOWNLOAD_NUM
};

static const char *dlnames[]={
	"Ibycus's TOPO Canada Maps",
	"Ibycus's TOPO USA Maps"};

enum
{
COL_ITEM,
COL_PATH,
COL_BROWSE,
COL_DOWNLOAD,
COL_NUM
};

class DownloadRow : public kGUITableRowObj
{
public:
	DownloadRow();
	void Load(class kGUIXMLItem *mpr);	/* load from prefs file */
	void Save(class kGUIXMLItem *mpr);	/* save to prefs file */
	int GetNumObjects(void) {return COL_NUM;}
	kGUIObj **GetObjectList(void) {return m_objectlist;} 
	kGUIObj *m_objectlist[COL_NUM];
private:
	CALLBACKGLUEPTR(DownloadRow,Browse,kGUIEvent);
	CALLBACKGLUEPTRVAL(DownloadRow,BrowseDone,kGUIFileReq,int);
	CALLBACKGLUEPTR(DownloadRow,Download,kGUIEvent);
	void Browse(kGUIEvent *event);
	void BrowseDone(kGUIFileReq *result,int pressed);
	void Download(kGUIEvent *event);

	kGUIButtonObj m_download;
	kGUIComboBoxObj m_item;
	kGUIInputBoxObj m_path;
	kGUIButtonObj m_browse;

	/* list of downloaded files */
	int m_numfiles;
	ClassArray<kGUIString>m_downloadfiles;
};

class DownloadWindow
{
public:
	DownloadWindow(int item,kGUIString *path);
	~DownloadWindow();
private:
	void WindowEvent(kGUIEvent *event);
	CALLBACKGLUEPTR(DownloadWindow,WindowEvent,kGUIEvent);
	void StopEvent(kGUIEvent *event);
	CALLBACKGLUEPTR(DownloadWindow,StopEvent,kGUIEvent);
	void Update(void);
	CALLBACKGLUE(DownloadWindow,Update);
	void DownloadThread(void);
	CALLBACKGLUE(DownloadWindow,DownloadThread);
	kGUIWindowObj m_window;
	kGUIBusyRectObj m_busyrect;
	kGUIInputBoxObj m_status;
	kGUIButtonObj m_stop;

	int m_item;
	kGUIString m_path;
	kGUIThread m_thread;
	kGUICommStack<kGUIString *> m_comm;
	bool m_abort;
};

DownloadPage::DownloadPage()
{
}

DownloadPage::~DownloadPage()
{
}

void DownloadPage::Init(kGUIContainerObj *obj)
{
	unsigned int i;
	int bh;
	int bw;
	kGUIText *t;

	bw=obj->GetChildZoneW();
	bh=obj->GetChildZoneH();
	m_table.SetPos(0,0);
	m_table.SetNumCols(COL_NUM);
	m_table.SetAllowAddNewRow(true);
	m_table.SetEventHandler(this,CALLBACKNAME(NewEntry));

	for(i=0;i<COL_NUM;++i)
	{
		t=m_table.GetColHeaderTextPtr(i);
		t->SetFontID(1);	/* bold */
	}
	m_table.SetColTitle(COL_DOWNLOAD,"Download");
	m_table.SetColWidth(COL_DOWNLOAD,100);
	m_table.SetColTitle(COL_ITEM,"Item");
	m_table.SetColWidth(COL_ITEM,250);
	m_table.SetColTitle(COL_PATH,"Path");
	m_table.SetColWidth(COL_PATH,350);
	m_table.SetColTitle(COL_BROWSE,"Browse");
	m_table.SetColWidth(COL_BROWSE,50);
	m_table.SetSize(MIN(bw,m_table.CalcTableWidth()),200);
	obj->AddObject(&m_table);
}

void DownloadPage::Purge(void)
{
	m_table.DeleteChildren();

}

void DownloadPage::LoadPrefs(kGUIXMLItem *root)
{
	unsigned int i;
	kGUIXMLItem *item;

	item=root->Locate("downloads");
	if(item)
		m_table.LoadConfig(item);

	for(i=0;i<root->GetNumChildren();++i)
	{
		item=root->GetChild(i);
		if(!strcmp(item->GetName(),"dl"))
		{
			DownloadRow *row;
			row=new DownloadRow();
			row->Load(item);
			m_table.AddRow(row);
		}
	}
}

void DownloadPage::SavePrefs(kGUIXMLItem *root)
{
	unsigned int i;

	m_table.SaveConfig(root,"downloads");

	for(i=0;i<m_table.GetNumChildren();++i)
	{
		DownloadRow *row;

		row=static_cast<DownloadRow *>(m_table.GetChild(i));
		row->Save(root->AddChild("dl"));
	}
}

void DownloadPage::NewEntry(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_ADDROW)
	{
		DownloadRow *nr=new DownloadRow();
		m_table.AddRow(nr);
	}
}

DownloadRow::DownloadRow()
{
	unsigned int i;

	m_item.SetNumEntries(DOWNLOAD_NUM);
	for(i=0;i<DOWNLOAD_NUM;++i)
		m_item.SetEntry(i,dlnames[i],i);

	m_download.SetString("Download");
	m_download.SetFontID(1);	/* bold */
	m_download.SetEventHandler(this,CALLBACKNAME(Download));

	m_browse.SetString(" ...");
	m_browse.SetFontSize(22);
	m_browse.SetFontID(1);	/* bold */
	m_browse.SetEventHandler(this,CALLBACKNAME(Browse));

	m_objectlist[COL_DOWNLOAD]=&m_download;
	m_objectlist[COL_ITEM]=&m_item;
	m_objectlist[COL_PATH]=&m_path;
	m_objectlist[COL_BROWSE]=&m_browse;

	SetRowHeight(20);
}

/* load from prefs file */
void DownloadRow::Load(class kGUIXMLItem *mpr)
{
	m_item.SetSelectionz(mpr->Locate("item")->GetValueString());
	m_path.SetString(mpr->Locate("path")->GetValue());
	/* load list of currently downloaded files */
}

/* save to prefs file */
void DownloadRow::Save(class kGUIXMLItem *mpr)
{
	mpr->AddParm("item",m_item.GetSelectionString());
	mpr->AddParm("path",m_path.GetString());
	/* todo: save list of currently downloaded files */
}

void DownloadRow::Browse(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		kGUIFileReq *req;
	
		req=new kGUIFileReq(FILEREQ_SAVE,m_path.GetString(),0,this,CALLBACKNAME(BrowseDone));
	}
}

void DownloadRow::BrowseDone(kGUIFileReq *result,int pressed)
{
	if(pressed==MSGBOX_OK)
		m_path.SetString(result->GetPath());
}

void DownloadRow::Download(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		DownloadWindow *dlw;
	
		dlw=new DownloadWindow(m_item.GetSelection(),&m_path);
	}
}

DownloadWindow::DownloadWindow(int item,kGUIString *path)
{
	m_item=item;
	m_path.SetString(path);

	m_busyrect.SetIsBar(false);
	m_busyrect.SetPos(2,2);
	m_busyrect.SetSize(500,16);
	m_window.AddObject(&m_busyrect);

	m_status.SetPos(2,26+2+8);
	m_status.SetSize(500,300);
	m_status.SetLocked(true);	/* not editable */
	m_window.AddObject(&m_status);

	m_stop.SetPos(500-75,m_status.GetZoneBY()+8);
	m_stop.SetString("Stop");
	m_stop.SetSize(75,20);
	m_window.AddObject(&m_stop);

	m_window.GetTitle()->SetString("Downloading...");
	m_window.SetSize(10,10);
	m_window.ExpandToFit();
	m_window.Center();
	m_window.SetTop(true);
	kGUI::AddWindow(&m_window);

	kGUI::AddUpdateTask(this,CALLBACKNAME(Update));
	m_window.SetEventHandler(this,CALLBACKNAME(WindowEvent));
	m_stop.SetEventHandler(this,CALLBACKNAME(StopEvent));

	m_comm.Init(32);
	/* start the download thread */
	m_thread.Start(this,CALLBACKNAME(DownloadThread));
}

void DownloadWindow::Update(void)
{
	kGUIString *s=0;

	if(m_thread.GetActive()==true)
		m_busyrect.Animate();

	/* get messages from Async thread and draw em in the status window */
	while(m_comm.GetIsEmpty()!=true)
	{
		m_comm.Read(&s);
		m_status.Append(s);
		delete s;
		m_status.MoveCursorRow(1);	/* move cursor down as lines are added */
	}

	/* if the thread is finished then change 'stop'  to 'done' */
	if(m_thread.GetActive()==false)
	{
		if(m_abort==false)
			m_stop.SetString("done");
		m_abort=true;
	}
}

static const char *canbasefiles[]={
	"IbycsTop.MDX",
	"IbycsTop.img",
	"IbycsTop.TDB"};

void DownloadWindow::DownloadThread(void)
{
	kGUIDownloadEntry dl;
	DataHandle dh;
	DataHandle wdh;
	kGUIString url;
	int status;
	kGUICSV csv;
	kGUIString *s;
	unsigned int i;
	int j,dlsize;
	int dlerror=0;
	unsigned int numlines,numcols;
	kGUIString filename;
	kGUIString mapdir;
	kGUIString fullfilename;
	kGUIString length;
	kGUIString datetime;
	kGUIString name;
	kGUIString temp;
	unsigned long fs;
	bool werror;
	char c;
	int numok=0,numdownloaded=0;
	bool add;
	bool img;

	m_abort=false;
	switch(m_item)
	{
	case DOWNLOAD_IBYCUS_TOPOCANADA:
		url.SetString("http://www.ibycus.com/ibycustopo/download/filelist.txt");

		s=new kGUIString();
		s->Sprintf("downloading '%s'\n",url.GetString());
		m_comm.Write(&s);

		csv.SetMemory();
		status=dl.DownLoad(&csv,&url);

		s=new kGUIString();
		s->Sprintf("download status=%s\n",status==DOWNLOAD_OK?"OK":"Error!");
		m_comm.Write(&s);
		if(status==DOWNLOAD_OK)
		{
			/* split on the space character */
			csv.SetIgnoreEmpty(true);
			csv.SetSplit(" ");
			csv.Load();

			numlines=csv.GetNumRows();
			numcols=csv.GetNumCols();

			//0 = r/w status, 1,2,3 = groups/owners etc
			//4 = size
			//5,6,7 = date
			//8 = filename

			if(numcols!=9)
			{
				s=new kGUIString();
				s->Sprintf("Error: File format not as expected!");
				m_comm.Write(&s);
				goto quit;
			}

			s=new kGUIString();
			s->Sprintf("number of img files=%d\n",numlines);
			m_comm.Write(&s);

#if 0
			/* these are the base files */
			for(i=0;i<sizeof(canbasefiles)/sizeof(char *);++i)
			{
				filename.SetString(canbasefiles[i]);
				kGUI::MakeFilename(&m_path,&filename,&fullfilename);
				if(kGUI::FileExists(fullfilename.GetString())==false)
				{
					s=new kGUIString();
					s->Sprintf("Loading filename='%s'\n",filename.GetString());
					/* if buffer is full then delay and try again */
					m_comm.Write(&s);

					dh.SetMemory();
					url.Sprintf("http://www.ibycus.com/ibycustopo/download/%s",filename.GetString());
					status=dl.DownLoad(&dh,&url);

					s=new kGUIString();
					s->Sprintf("download status=%s\n",status==DOWNLOAD_OK?"OK":"Error!");
					m_comm.Write(&s);

					dlsize=dh.GetLoadableSize();
					if(status==DOWNLOAD_OK && dlsize>0)
					{
						/* ok, save file to drive */
						werror=false;

						wdh.SetFilename(fullfilename.GetString());
						if(wdh.OpenWrite("wb",dlsize)==true)
						{
							dh.Open();
							for(j=0;j<dlsize;++j)
							{
								dh.Read(&c,(unsigned long)1L);
								wdh.Write(&c,1L);
							}
							dh.Close();
							if(wdh.Close()==false)
								werror=true;
						}
						else
							werror=true;

						if(!werror)
							++numdownloaded;
						else
						{
							s=new kGUIString();
							s->Sprintf("Error opening file '%s' for write!\n",fullfilename.GetString());
							m_comm.Write(&s);
							m_abort=true;
						}
					}
				}
			}
#endif

			fullfilename.SetString("imgs");
			kGUI::MakeFilename(&m_path,&fullfilename,&mapdir);

			/* is the 'imgs' directory there already? */
			if(kGUI::IsDir(mapdir.GetString())==false)
			{
				if(kGUI::MakeDirectory(mapdir.GetString())==false)
				{
					s=new kGUIString();
					s->Sprintf("Error cannor create imgs directory='%s'\n",mapdir.GetString());
					/* if buffer is full then delay and try again */
					while(m_comm.Write(&s)==false)
						kGUI::Sleep(1);
					m_abort=true;	
				}
			}

			//0 = r/w status, 1,2,3 = groups/owners etc
			//4 = size
			//5,6,7 = date
			//8 = filename

			if(m_abort==false)
			{
				for(i=0;i<numlines;++i)
				{
					if(csv.GetNumCols(i)==9)
					{
						csv.GetField(i,4,&length);
						csv.GetField(i,5,&datetime);
						csv.GetField(i,6,&temp);
						datetime.Append(" ");
						datetime.Append(&temp);
						csv.GetField(i,7,&temp);
						datetime.Append(" ");
						datetime.Append(&temp);
						csv.GetField(i,8,&filename);
						csv.GetField(i,8,&name);

						add=false;
						img=false;
						/* check for root list */
						for(unsigned int b=0;b<sizeof(canbasefiles)/sizeof(char *);++b)
						{
							if(!strcmp(filename.GetString(),canbasefiles[b]))
								add=true;
						}
						if(add==false)
						{
							if(filename.Str(".img")>=0)
							{
								add=true;
								img=true;
							}
						}

						if(add)
						{
							fs=length.GetInt();
							if(img)
								kGUI::MakeFilename(&mapdir,&filename,&fullfilename);
							else
								kGUI::MakeFilename(&m_path,&filename,&fullfilename);
							if(kGUI::FileSize(fullfilename.GetString())!=fs)
							{
								s=new kGUIString();
								s->Sprintf("Loading map '%s', filename='%s'\n",name.GetString(),filename.GetString());
								/* if buffer is full then delay and try again */
								while(m_comm.Write(&s)==false)
									kGUI::Sleep(1);

								dh.SetMemory();
								if(img)
									url.Sprintf("http://www.ibycus.com/ibycustopo/download/imgs/%s",filename.GetString());
								else
									url.Sprintf("http://www.ibycus.com/ibycustopo/download/%s",filename.GetString());
								status=dl.DownLoad(&dh,&url);

								dlsize=dh.GetLoadableSize();
								s=new kGUIString();
								s->Sprintf("download status=%s\n",status==DOWNLOAD_OK?"OK":"Error!");
								m_comm.Write(&s);

								if(status==DOWNLOAD_OK && dlsize>0)
								{
									/* ok, save file to drive */
									werror=false;

									wdh.SetFilename(fullfilename.GetString());
									if(wdh.OpenWrite("wb",dlsize)==true)
									{
										dh.Open();
										for(j=0;j<dlsize;++j)
										{
											dh.Read(&c,(unsigned long)1L);
											wdh.Write(&c,1L);
										}
										dh.Close();
										if(wdh.Close()==false)
											werror=true;
									}
									else
										werror=true;

									if(!werror)
										++numdownloaded;
									else
									{
										s=new kGUIString();
										s->Sprintf("Error opening file '%s' for write!\n",fullfilename.GetString());
										m_comm.Write(&s);
										m_abort=true;

									}

								}
								else
								{
									if(++dlerror==10)
									{
										s=new kGUIString();
										s->Sprintf("aborting download, too many errors!\n",status);
										m_comm.Write(&s);
										m_abort=true;
									}
								}
							}
							else
								++numok;
							if(m_abort)
								break;
						}
					}
				}
			}
			s=new kGUIString();
			s->Sprintf("Num OK=%d, Num Downloaded=%d\n",numok,numdownloaded);
			/* if buffer is full then delay and try again */
			while(m_comm.Write(&s)==false)
				kGUI::Sleep(1);
		}
	break;
	case DOWNLOAD_IBYCUS_TOPOUSA:
	break;
	}

quit:;
	m_thread.Close(true);
}


void DownloadWindow::WindowEvent(kGUIEvent *event)
{
	switch(event->GetEvent())
	{
	case EVENT_CLOSE:
		delete this;
	break;
	}
}

void DownloadWindow::StopEvent(kGUIEvent *event)
{
	switch(event->GetEvent())
	{
	case EVENT_PRESSED:
		m_window.Close();
	break;
	}
}

DownloadWindow::~DownloadWindow()
{
	kGUIString *s=0;

	/* wait for finished */
	m_abort=true;
	while(m_thread.GetActive());

	kGUI::DelUpdateTask(this,CALLBACKNAME(Update));

	/* delete any dangling strings */
	while(m_comm.GetIsEmpty()!=true)
	{
		m_comm.Read(&s);
		delete s;
	}

	kGUI::DelWindow(&m_window);

	//if this is the current map and new IMG files are downloaded then this will force them
	//to be re-loaded
	gpx->ChangeMapType();
}
#endif
