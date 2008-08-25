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
#include "babelglue.h"

#if defined(WIN32) || defined(MINGW)
#define BABELCODE "gpsbabel.exe"
#elif defined(LINUX) || defined(MACINTOSH)
#define BABELCODE "./gpsbabel"
#else
#error
#endif

BabelGlue::BabelGlue()
{
	m_asyncactive=false;
	m_types=0;
	m_filter=0;
	m_argc=0;
	m_strings.Init(16,4);
}

BabelGlue::~BabelGlue()
{
	if(m_thread.GetActive()==true)
		m_thread.Close(false);
	
	/* might take a while to shutdown */
	while(m_asyncactive==true)
		kGUI::Sleep(1);

	Purge();
}

void BabelGlue::GetVersion(kGUIString *s)
{
	m_strings.GetEntryPtr(m_argc++)->SetString(BABELCODE);
	m_strings.GetEntryPtr(m_argc++)->SetString("-V");

	Call(false,0);
	s->SetString(&m_output);
	s->Trim();
}


bool BabelGlue::GetInputFormats(unsigned int index,const char **name,const char **desc,const char **ext)
{
	if(!index)
	{
		/* get input formats */
		m_strings.GetEntryPtr(m_argc++)->SetString(BABELCODE);
		m_strings.GetEntryPtr(m_argc++)->SetString("-^^1");

		Call(false,0);
		/* load the results into a CSV file */
		m_filetypes.SetMemory((const unsigned char *)m_output.GetString(),m_output.GetLen());
		/* change split character from comma to tab */
		m_filetypes.SetSplit("\t");
		m_filetypes.Load();
	}

	if(index<m_filetypes.GetNumRows())
	{
		*(name)=m_filetypes.GetFieldPtr(index,0)->GetString();
		*(ext)=m_filetypes.GetFieldPtr(index,1)->GetString();
		*(desc)=m_filetypes.GetFieldPtr(index,2)->GetString();
		return(true);
	}
	return(false);
}



void BabelGlue::Purge(void)
{
	m_argc=0;
}

void BabelGlue::BuildParms(void)
{
	kGUIString *s;

	Purge();

	m_strings.GetEntryPtr(m_argc++)->SetString(BABELCODE);

	//verbose mode used for tracaking progress
	m_strings.GetEntryPtr(m_argc++)->SetString("-vs");

	if(m_types&BABELTYPE_WAYPOINTS)
		m_strings.GetEntryPtr(m_argc++)->SetString("-w");

	if(m_types&BABELTYPE_TRACKS)
		m_strings.GetEntryPtr(m_argc++)->SetString("-t");

	if(m_types&BABELTYPE_REALTIME)
		m_strings.GetEntryPtr(m_argc++)->SetString("-T");

	m_strings.GetEntryPtr(m_argc++)->SetString("-i");

	s=m_strings.GetEntryPtr(m_argc++);
	s->SetString(m_informat.GetString());
	if(m_inbaud.GetLen())
	{
		if(!stricmp(m_informat.GetString(),"magellan"))
			s->ASprintf(",baud=%s",m_inbaud.GetString());
	}

	m_strings.GetEntryPtr(m_argc++)->SetString("-f");

	s=m_strings.GetEntryPtr(m_argc++);
	s->SetString(m_inname.GetString());
#if defined(WIN32) || defined(MINGW)
	s->Replace("USB","usb:");
#endif

	switch(m_filter)
	{
	case BABELFILTER_SIMPLIFYTRACK:
		m_strings.GetEntryPtr(m_argc++)->SetString("-x");

		m_strings.GetEntryPtr(m_argc++)->Sprintf("simplify,count=%d",m_stnum);
	break;
	}

	if(m_types&BABELTYPE_REALTIME)
	{
		m_strings.GetEntryPtr(m_argc++)->SetString("-o");
		m_strings.GetEntryPtr(m_argc++)->SetString("kml,max_position_points=1");
		m_strings.GetEntryPtr(m_argc++)->SetString("-F");
		m_strings.GetEntryPtr(m_argc++)->SetString("gpsbabel.kml");

		m_created.Clear();
		m_crc=-1;
		kGUI::FileDelete("gpsbabel.kml");
	}
	else if(m_outformat.GetLen())
	{
		m_strings.GetEntryPtr(m_argc++)->SetString("-o");

		s=m_strings.GetEntryPtr(m_argc++);
		s->SetString(m_outformat.GetString());
		if(m_outbaud.GetLen())
		{
			if(!stricmp(m_outformat.GetString(),"magellan"))
				s->ASprintf(",baud=%s",m_outbaud.GetString());
		}

		m_strings.GetEntryPtr(m_argc++)->SetString("-F");

		s=m_strings.GetEntryPtr(m_argc++);
		s->SetString(m_outname.GetString());
#if defined(WIN32) || defined(MINGW)
		s->Replace("USB","usb:");
#endif
	}
}

bool BabelGlue::Call(bool build,kGUIBusy *busy)
{
	int i;
	bool rc;
	kGUIMsgBoxReq *box;
	kGUIString line;
	kGUIString *s;

//todo, if realtime tracking is on then turn it off!

	if(build)
		BuildParms();

	//trap errors!
#if defined(LINUX) || defined(MACINTOSH)
	kGUI::FileDelete("gpsbabel.err");
#if 0
	m_strings.GetEntryPtr(m_argc++)->SetString("2>gpsbabel.err");
#endif
#endif

	m_output.Clear();

	/* pass parms to GPSBabel */

	for(i=0;i<m_argc;++i)
	{
		if(i)
			line.Append(" ");

		/* if parm has space in it then enclose in quotes */
		s=m_strings.GetEntryPtr(i);
		if(strstr(s->GetString()," "))
		{
			line.Append("\"");
			line.Append(s);
			line.Append("\"");
		}
		else
			line.Append(s);
	}
	m_busy=busy;
	if(build)
		m_ct.SetUpdateCallback(this,CALLBACKNAME(OutputChanged));

	//DebugPrint(line.GetString());

	rc=m_ct.Start(line.GetString(),CALLTHREAD_READ);
	m_output.SetString(m_ct.GetString());
	m_output.Trim();
#if defined(LINUX) || defined(MACINTOSH)
	//errors are also redirected to gpsbabel.err so append these to output if found
	{
		unsigned long fs;
		const char *file;

		file=(const char *)kGUI::LoadFile("gpsbabel.err",&fs);
		if(file)
		{
			m_output.Append(file,fs);
			m_output.Trim();
			delete []file;
			kGUI::FileDelete("gpsbabel.err");
		}
	}
#endif

	if(build)
		m_ct.SetUpdateCallback(0,0);
	if(busy)
		delete busy;

	if(m_asyncactive==false)
	{
		if(m_output.GetLen() && build)
		{
			box=new kGUIMsgBoxReq(MSGBOX_OK,false,m_output.GetString());
			return(false);
		}
	}
	return(true);	/* ok */
}

/* external thread output string has been updated */
void BabelGlue::OutputChanged(void)
{
	kGUIString *s=m_ct.GetString();
	kGUIStringSplit ss;
	const char *cp;
	int nw;

	//m_types&BABELTYPE_REALTIME

	//delete any leading c/r or l/f
	if(s->GetLen()==1)
	{
		cp=s->GetString();
		if((cp[0]==0x0a) || (cp[0]==0x0d))
			s->Clear();
		return;
	}

	cp=strstr(s->GetString(),"\r");
	if(!cp)
		return;
	if(cp[1])	/* if this the last character in the string? */
		return;
	
	cp=s->GetString();
	if(cp[0]>='0' && cp[0]<='9')
	{
		nw=ss.Split(s,"/",false,false);
		if(nw==3)
		{
			int a=ss.GetWord(1)->GetInt();
			int b=ss.GetWord(2)->GetInt();
			
			if(m_busy)
			{
				m_busy->SetMax(b);
				m_busy->SetCur(a);
			}
			s->Clear();
		}
	}
}

/* used for realtime 'tracking' mode */
void BabelGlue::StartTracking(void)
{
	if(m_thread.GetActive()==true)
		return;

	m_types=BABELTYPE_REALTIME;
	m_thread.Start(this,CALLBACKNAME(CallThread));
}

void BabelGlue::CallThread(void)
{
	m_asyncactive=true;
	Call(true,0);
	if(m_thread.GetActive())
		m_thread.Close(false);

	m_asyncactive=false;
}

/* used for realtime 'tracking' mode */
void BabelGlue::StopTracking(void)
{
	m_ct.Stop();
	if(m_thread.GetActive()==true)
		m_thread.Close(false);

	/* might take a while to shutdown */
	while(m_asyncactive==true)
		kGUI::Sleep(1);
}

/* NOTE: loadfile appends a null to the end of the loaded file! */

bool BabelGlue::GetChunk(const char *tag,const char *file,kGUIString *s)
{
	const char *sp;
	const char *ep;

	sp=strstr(file,tag);
	if(!sp)
		return(false);	/* start tag not found */
	sp+=strlen(tag);
	ep=strstr(sp,"<");
	if(!ep)
		return(false);	/* start of end tag not found! */
	s->SetString(sp,(int)(ep-sp));
	return(true);
}

bool BabelGlue::GetPos(GPXCoord *pos)
{
	const char *file;
	unsigned long fs;
	int num;
	long crc;
	kGUIString created;
	kGUIString coords;
	kGUIStringSplit ss;
	bool gotnew=false;

	crc=kGUI::FileCRC("gpsbabel.kml");
	file=(const char *)kGUI::LoadFile("gpsbabel.kml",&fs);
	if(file)
	{
		/* get created string */
		if(GetChunk("<Snippet>",file,&created))
		{
			/* get coords */
			if(GetChunk("<coordinates>",file,&coords))
			{
				/* created only has 1 second resolution so we use crc too */
				if(strcmp(created.GetString(),m_created.GetString()) || m_crc!=crc)
				{
					num=ss.Split(&coords,",");
					if((num==2) || (num==3))
					{
						/* lon/lat/alt */
						m_created.SetString(&created);
						m_crc=crc;
						m_lat=ss.GetWord(1)->GetDouble();
						m_lon=ss.GetWord(0)->GetDouble();
						pos->Set(m_lat,m_lon);
						gotnew=true;
					}
				}
			}
		}
		delete []file;
	}
	return(gotnew);
};
