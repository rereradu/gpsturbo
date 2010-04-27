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
#include "filters.h"
#include "uploadxml.h"

/* class wrappers between 'basic' and the GPSTurbo internal objects */

typedef struct
{
	int id;
	const char *name;
	int np;
	const char *parmdef;
	const char *desc1;
	const char *desc2;
}fntoid_def;

/***************************************************************************/

enum
{
GPXRECORDSET_LOADALL=GPXCOL_NUMCOLUMNS,
GPXRECORDSET_LOADFILTER,
GPXRECORDSET_NUMRECORDS,
GPXRECORDSET_SEEKRECORD,
GPXRECORDSET_LOADFILE,
GPXRECORDSET_SAVEFILE,
GPXRECORDSET_MERGERECORD,
GPXRECORDSET_MERGERECORDS,
GPXRECORDSET_UPLOADTOGPSR,
GPXRECORDSET_DOWNLOADFROMGPSR,

GPXRECORDSET_USERARRAY,
GPXRECORDSET_NUMLOGS,
GPXRECORDSET_LOGID,
GPXRECORDSET_LOGDATE,
GPXRECORDSET_LOGTYPE,
GPXRECORDSET_LOGFINDER,
GPXRECORDSET_LOGTEXT,

GPXRECORDSET_NUMENTRIES
};

fntoid_def GPXRecordSetFuncs[]={
	{GPXRECORDSET_LOADALL,		"LoadAll",			0,"",				"LoadAll()",							"Load All Records from Global RecordSet"},
	{GPXRECORDSET_LOADFILTER,	"LoadFilter",		1,"byval string",	"LoadFilter(filtername as string)",		"Load Records from Global RecordSet using Filter"},
	{GPXRECORDSET_SEEKRECORD,	"SeekRecord",		1,"byval integer",	"SeekRecord(recordnum as string)",		"Seek Record #0-NumRecords-1"},
	{GPXRECORDSET_NUMRECORDS,	"NumRecords",		0,"",				"NumRecord() as integer",				"Return number of Records in Set"},
	{GPXRECORDSET_MERGERECORD,	"MergeRecord",		0,"",				"MergeRecord()",						"Merge Current Record into Global RecordSet"},
	{GPXRECORDSET_MERGERECORDS,	"MergeRecords",		0,"",				"MergeRecords()",						"Merge all Records into Global RecordSet"},
	{GPXRECORDSET_LOADFILE,		"LoadFile",			3,"byval string,byval string,byval boolean","LoadFile(filename as string,databasename as string,Append as boolean)","Load RecordSet from GPX File. If Append=true then records are appended to current records in set."},
	{GPXRECORDSET_SAVEFILE,		"SaveFile",			1,"byval string",	"SaveFile(filename as string)",			"Save RecordSet to GPX File"},
	{GPXRECORDSET_UPLOADTOGPSR,	"UploadToGPSR",		1,"byval string",	"UploadToGPSR(gpsrname as string)",		"Upload waypoints in RecordSet to the named GPSR."},
	{GPXRECORDSET_DOWNLOADFROMGPSR,	"DownloadFromGPSR",	1,"byval string",	"DownloadFromGPSR(gpsrname as string)",	"Download points from GPSR."}};

class GPXRecordSet;

class GPXRecordVar : public kGUIBasicVarObj
{
	friend class GPXRecordSet;
public:
	GPXRecordVar() {m_incallback=false;m_arrayentries=0;}
	virtual ~GPXRecordVar() {if(m_arrayentries) delete []m_arrayentries;}
	void PreRead(void);
	void PostWrite(void);
	virtual void AllocArray(int n) {if(m_arrayentries){delete []m_arrayentries;}m_arrayentries=new GPXRecordVar[n];}
	virtual kGUIBasicVarObj *GetEntry(int n) {return m_arrayentries+n;}
	void SetIndex(GPXRecordSet *rs,int i) {m_rs=rs;m_index=i;}
//	int GetIndex(void) {return m_index;}
private:
	bool m_incallback;
	int m_index;
	GPXRecordSet *m_rs;
	GPXRecordVar *m_arrayentries;
};

/* todo: add code for children points */

class GPXRecordSet : public kGUIBasicClassObj
{
	friend class GPXRecordVar;
public:
	GPXRecordSet();
	~GPXRecordSet();
	static kGUIBasicClassObj *Alloc(void) {return new GPXRecordSet();}
	static void RegisterErrors(kGUIBasic *b);
	kGUIBasicObj *GetObj(const char *name);
	int Function(int fieldid,kGUIBasicVarObj *result,CallParms *p);
private:
	void LoadRecord(void);
	void LoadField(GPXRecordVar *var,int ud);
	void SaveField(GPXRecordVar *var,int ud);

	void PurgeRecords(void);
	GPXRow *Locate(const char *wptname);
	void MergeRecord(GPXRow *row);
	kGUIBasicObj *m_fieldobjects[GPXCOL_NUMCOLUMNS+GPXRECORDSET_NUMENTRIES];

	GPXRecordVar m_rowvalues[GPXCOL_NUMCOLUMNS];
	GPXRecordVar m_userarray;
	GPXRecordVar m_numlogs;

	GPXRecordVar m_logid;
	GPXRecordVar m_logdate;
	GPXRecordVar m_logtype;
	GPXRecordVar m_logfinder;
	GPXRecordVar m_logtext;
	kGUIBasicSysClassFuncObj m_commands[sizeof(GPXRecordSetFuncs)/sizeof(fntoid_def)];

	unsigned int m_curindex;
	unsigned int m_numwpts;
	Array<GPXRow *>m_wptlist;
	Hash m_fieldhash;	/* small hash list for field name to index conversion */
	GPXRow *m_currow;
	unsigned int m_maxlogs;

	/* error message values */
	static unsigned int m_errorfile;
	static unsigned int m_errorbadname;
	static unsigned int m_errorcomm;
};

unsigned int GPXRecordSet::m_errorfile;
unsigned int GPXRecordSet::m_errorbadname;
unsigned int GPXRecordSet::m_errorcomm;

/******************************************************************/

void GPXRecordVar::PreRead(void)
{
	if(m_incallback==false)
	{
		m_incallback=true;
		m_rs->LoadField(this,m_index);
		m_incallback=false;
	}
}

void GPXRecordVar::PostWrite(void)
{
	if(m_incallback==false)
	{
		m_incallback=true;
		m_rs->SaveField(this,m_index);
		m_incallback=false;
	}
}

/******************************************************************/

/******************************************************************/

extern const char *wpcolnames[GPXCOL_NUMCOLUMNS];
extern const char *wpcoldesc[GPXCOL_NUMCOLUMNS];

GPXRecordSet::GPXRecordSet()
{
	unsigned int i;

	m_maxlogs=0;	/* size of allocated log arrays */
	m_numwpts=0;
	SetType(VARTYPE_CLASS);
	m_fieldhash.Init(8,sizeof(int));

	/* add row value field names and set var pointers to them */
	for(i=0;i<GPXCOL_NUMCOLUMNS;++i)
	{
		m_fieldobjects[i]=&m_rowvalues[i];
		m_fieldhash.Add(wpcolnames[i],&i);
		m_rowvalues[i].SetType(VARTYPE_VARIANT);
		m_rowvalues[i].SetIndex(this,i);
	}
	/* add functions next */
	for(i=0;i<sizeof(GPXRecordSetFuncs)/sizeof(fntoid_def);++i)
	{
		m_fieldhash.Add(GPXRecordSetFuncs[i].name,&GPXRecordSetFuncs[i].id);
		m_commands[i].Init(this,GPXRecordSetFuncs[i].id,GPXRecordSetFuncs[i].np,GPXRecordSetFuncs[i].parmdef);
		m_fieldobjects[GPXRecordSetFuncs[i].id]=&m_commands[i];
	}

	/* add an array reference to the user fields as well */
	i=GPXRECORDSET_USERARRAY;
	m_fieldobjects[i]=&m_userarray;
	m_fieldhash.Add("U",&i);

	m_userarray.SetType(VARTYPE_BOOLEAN);
	m_userarray.SetIsUndefined(false);
	m_userarray.AddIndice(MAXUSERTICKS);
	m_userarray.SetIsArray(true);
	m_userarray.m_arrayentries=&m_rowvalues[GPXCOL_USER1];

	i=GPXRECORDSET_NUMLOGS;
	m_fieldobjects[i]=&m_numlogs;
	m_fieldhash.Add("NumLogs",&i);
	m_numlogs.SetIsConstant(true);
	m_numlogs.SetType(VARTYPE_INTEGER);
	m_numlogs.SetIndex(this,i);

	i=GPXRECORDSET_LOGID;
	m_fieldobjects[i]=&m_logid;
	m_fieldhash.Add("LogID",&i);
	m_logid.SetType(VARTYPE_VARIANT);
	m_logid.SetIndex(this,i);

	i=GPXRECORDSET_LOGDATE;
	m_fieldobjects[i]=&m_logdate;
	m_fieldhash.Add("LogDate",&i);
	m_logdate.SetType(VARTYPE_VARIANT);

	i=GPXRECORDSET_LOGTYPE;
	m_fieldobjects[i]=&m_logtype;
	m_fieldhash.Add("LogType",&i);
	m_logtype.SetType(VARTYPE_VARIANT);

	i=GPXRECORDSET_LOGFINDER;
	m_fieldobjects[i]=&m_logfinder;
	m_fieldhash.Add("LogFinder",&i);
	m_logfinder.SetType(VARTYPE_VARIANT);

	i=GPXRECORDSET_LOGTEXT;
	m_fieldobjects[i]=&m_logtext;
	m_fieldhash.Add("LogText",&i);
	m_logtext.SetType(VARTYPE_VARIANT);
}

void GPXRecordSet::RegisterErrors(kGUIBasic *b)
{
	m_errorfile=b->RegisterError("Cannot open file!");
	m_errorbadname=b->RegisterError("Named GPSr not found in GPSr list!");
	m_errorcomm=b->RegisterError("Error communicating with GPSr!");
}

GPXRecordSet::~GPXRecordSet()
{
	/* since this is not "allocated" we need to remove it before the */
	/* destructor is called so it doesn't get freed twice */
	m_userarray.SetIsArray(false);
	m_userarray.m_arrayentries=0;

	/* purge any allocated records */
	PurgeRecords();
}

/* class object can be any of the objects types, for example, it can */
/* be any variable type as well as a function or subroutine call object too. */

kGUIBasicObj *GPXRecordSet::GetObj(const char *name)
{
	int *pid;

	pid=(int *)m_fieldhash.Find(name);
	if(!pid)
		return(0);
	return(m_fieldobjects[pid[0]]);	/* return pointer to object */
}

GPXRow *GPXRecordSet::Locate(const char *wptname)
{
	unsigned int e;
	GPXRow *row;

	for(e=0;e<m_numwpts;++e)
	{
		row=m_wptlist.GetEntry(e);
		if(!strcmp(wptname,row->m_wptname.GetString()))
			return(row);
	}
	return(0);
}


int GPXRecordSet::Function(int fieldid,kGUIBasicVarObj *result,CallParms *p)
{	
	unsigned int i;
	GPXRow *row;

	switch(fieldid)
	{
	case GPXRECORDSET_LOADALL:
		if(p->GetNumParms()!=0)
			return(ERROR_WRONGNUMBEROFPARAMETERS);
		
		PurgeRecords();
		m_numwpts=gpx->m_numwpts;
		m_wptlist.Alloc(m_numwpts);
		for(i=0;i<m_numwpts;++i)
		{
			row=gpx->m_wptlist.GetEntry(i);
			row->m_purge=false;
			m_wptlist.SetEntry(i,row);
		}
		m_curindex=0;
		LoadRecord();

		result->Set((int)0);
		return(ERROR_OK);
	break;
	case GPXRECORDSET_LOADFILTER:
	{
		kGUIBasicVarObj *p1;

		PurgeRecords();

		if(p->GetNumParms()!=1)
			return(ERROR_WRONGNUMBEROFPARAMETERS);
		p1=p->GetParm(0);
		p1->ChangeType(VARTYPE_STRING);

		/* allocate space for whole list */
		m_wptlist.Alloc(gpx->m_numwpts);
		
		m_numwpts=gpx->m_filters.Filter(p1->GetStringObj()->GetString(),&m_wptlist);
		for(i=0;i<m_numwpts;++i)
		{
			row=m_wptlist.GetEntry(i);
			row->m_purge=false;
		}
		m_wptlist.Sort(m_numwpts,GPX::SortEntry);
		result->Set((int)0);
		return(ERROR_OK);
	}
	break;
	case GPXRECORDSET_NUMRECORDS:
		if(p->GetNumParms()!=0)
			return(ERROR_WRONGNUMBEROFPARAMETERS);
		result->Set((int)m_numwpts);
		return(ERROR_OK);
	break;
	case GPXRECORDSET_SEEKRECORD:
		if(p->GetNumParms()!=1)
			return(ERROR_WRONGNUMBEROFPARAMETERS);

		m_curindex=p->GetParm(0)->GetInt();
		if(m_curindex>m_numwpts)
			return(ERROR_SYNTAX);
		LoadRecord();
		result->Set((int)0);
		return(ERROR_OK);
	break;
	case GPXRECORDSET_LOADFILE:
	{
		int result=m_errorfile;
		unsigned int i;
		kGUIXML *xml;
		kGUIXMLItem *xmlitem;
		kGUIXMLItem *wp;
		kGUIString gendate;
		GPXRow *row;

		// parms (filename,dbname,append);

		if(p->GetParm(2)->GetBoolean()==false)
			PurgeRecords();

		gpx->SetDefDB(p->GetParm(1)->GetStringObj()->GetString());

		xml=new kGUIXML();
		xml->SetNameCache(&gpx->m_xmlnamecache);
		xml->SetFilename(p->GetParm(0)->GetStringObj()->GetString());
		if(xml->Load())
		{
			xmlitem=xml->GetRootItem()->Locate("gpx");
			if(xmlitem)
			{
				result=ERROR_OK;
				/* date file was generated */
				gendate.SetString(xmlitem->Locate("time")->GetValue());
				if(gendate.GetLen()>10)
					gendate.Clip(10);

				m_wptlist.Alloc(m_numwpts+xmlitem->GetNumChildren());
				for(i=0;i<xmlitem->GetNumChildren();++i)
				{
					wp=xmlitem->GetChild(i);
					if(!strcmp(wp->GetName(),"wpt"))
					{
						row=Locate(wp->Locate("name")->GetValueString());
						if(row)
						{
							/* is this one older than the one we already have? */
							if(strcmp(gendate.GetString(),row->m_gendate.GetString())>0)
								row->Load(&gendate,wp);
						}
						else
						{
							row=new GPXRow(&gendate,wp);
							row->m_purge=true;
							m_wptlist.SetEntry(m_numwpts++,row);
						}
					}
				}
			}
		}
		delete xml;
		return(result);
	}
	break;
	case GPXRECORDSET_SAVEFILE:
	{
		unsigned int i;
		kGUIXML xml;
		kGUIXMLItem *root;
		GPXRow *row;
		kGUIBasicVarObj *p1;

		if(p->GetNumParms()!=1)
			return(ERROR_WRONGNUMBEROFPARAMETERS);
		p1=p->GetParm(0);
		p1->ChangeType(VARTYPE_STRING);

		root=xml.GetRootItem()->AddChild("gpx");
		for(i=0;i<m_numwpts;++i)
		{
			row=gpx->m_wptlist.GetEntry(i);
			row->Save(&xml,root->AddChild("wpt"),true);
		}

		xml.SetFilename(p1->GetStringObj()->GetString());
		if(xml.Save()==false)
		{
//			kGUIMsgBoxReq *box=new kGUIMsgBoxReq(MSGBOX_OK,"Error saving file!");
			return(ERROR_SYNTAX);
		}
		result->Set((int)0);
		return(ERROR_OK);
	}
	break;
	case GPXRECORDSET_MERGERECORD:
		/* merge the current record into the database */
		if(!m_numwpts)
			return(ERROR_SYNTAX);	/* no current record! */

		MergeRecord(m_wptlist.GetEntry(m_curindex));
		return(ERROR_OK);
	break;
	case GPXRECORDSET_MERGERECORDS:
	{
		unsigned int i;

		/* merge all the records in the set into the database */
		for(i=0;i<m_numwpts;++i)
			MergeRecord(m_wptlist.GetEntry(i));
		return(ERROR_OK);
	}
	break;
	case GPXRECORDSET_UPLOADTOGPSR:	/*(gpsrname,children)*/
	{
		int rc;
		kGUIBasicVarObj *p1;

		/* send wapoints in current record set to the named GPSR */
		if(p->GetNumParms()!=1)
			return(ERROR_WRONGNUMBEROFPARAMETERS);
		p1=p->GetParm(0);
		p1->ChangeType(VARTYPE_STRING);

		/* send waypoints to the named GPSr */
		rc=gpx->GetGPSrObj()->UploadToGPSr(p1->GetStringObj()->GetString(),m_numwpts,&m_wptlist);

		/* 0=ok, 1=no gpsrnamed found, 2=comm error */
		switch(rc)
		{
		case 1:
			return(m_errorbadname);
		break;
		case 2:
			return(m_errorcomm);
		break;
		}
		return(ERROR_OK);
	}
	break;
	case GPXRECORDSET_DOWNLOADFROMGPSR:
		/* todo! */
	break;
	}
	return(ERROR_SYNTAX);
}

/* this is the pre-read callback triggered when basic accesses a variable */

void GPXRecordSet::LoadField(GPXRecordVar *var,int ud)
{
	int a;

	if(!m_currow)
	{
		var->SetIsUndefined(true);
		return;
	}

	a=ud>>8;	/* array index for vars in arrays */
	var->SetIsUndefined(false);
	switch(ud&255)
	{
	case GPXCOL_NA:
		var->Set(m_currow->m_na.GetSelected());
	break;
	case GPXCOL_FOUND:
		var->Set(m_currow->m_found.GetSelected());
	break;
	case GPXCOL_TYPE:
		var->Set(m_currow->GetType());
	break;
	case GPXCOL_USER1:
		var->Set(m_currow->m_user[0].GetSelected());
	break;
	case GPXCOL_USER2:
		var->Set(m_currow->m_user[1].GetSelected());
	break;
	case GPXCOL_USER3:
		var->Set(m_currow->m_user[2].GetSelected());
	break;
	case GPXCOL_USER4:
		var->Set(m_currow->m_user[3].GetSelected());
	break;
	case GPXCOL_USER5:
		var->Set(m_currow->m_user[4].GetSelected());
	break;
	case GPXCOL_USER6:
		var->Set(m_currow->m_user[5].GetSelected());
	break;
	case GPXCOL_USER7:
		var->Set(m_currow->m_user[6].GetSelected());
	break;
	case GPXCOL_USER8:
		var->Set(m_currow->m_user[7].GetSelected());
	break;
	case GPXCOL_NAME:
		var->Set(m_currow->m_name.GetString());
	break;
	case GPXCOL_WPTNAME:
		var->Set(m_currow->m_wptname.GetString());
	break;
	case GPXCOL_LOG:
		var->Set(m_currow->m_log.GetString());
	break;
	case GPXCOL_CONTAINER:
		var->Set(m_currow->m_container.GetSelectionString());
	break;
	case GPXCOL_DIFFICULTY:
		var->Set(m_currow->m_difficulty.GetDouble());
	break;
	case GPXCOL_TERRAIN:
		var->Set(m_currow->m_terrain.GetDouble());
	break;
	case GPXCOL_DIST:
		var->Set(m_currow->m_distval);
	break;
	case GPXCOL_CORRECTED:
		var->Set(m_currow->m_corrected.GetSelected());
	break;
	case GPXCOL_LAT:
		var->Set(m_currow->m_llcoord.GetLat());
	break;
	case GPXCOL_LON:
		var->Set(m_currow->m_llcoord.GetLon());
	break;
	case GPXCOL_HINT:
		var->Set(m_currow->m_hint.GetString());
	break;
	case GPXCOL_USERNOTES:
		var->Set(m_currow->m_usernotes.GetString());
	break;
	case GPXCOL_OWNER:
		var->Set(m_currow->m_owner.GetString());
	break;
	case GPXCOL_STATE:
		var->Set(m_currow->m_state.GetString());
	break;
	case GPXCOL_COUNTRY:
		var->Set(m_currow->m_country.GetString());
	break;
	case GPXCOL_GENDATE:
		var->Set(m_currow->m_gendate.GetString());
	break;
	case GPXRECORDSET_NUMLOGS:
		var->Set((int)m_currow->m_logs.GetNumChildren());
	break;
	case GPXRECORDSET_LOGID:
		var->Set(m_currow->m_logs.GetChild(a)->Locate("id")->GetValue());
	break;
	case GPXRECORDSET_LOGDATE:
		var->Set(m_currow->m_logs.GetChild(a)->Locate("groundspeak:date")->GetValue());
	break;
	case GPXRECORDSET_LOGTYPE:
		var->Set(m_currow->m_logs.GetChild(a)->Locate("groundspeak:type")->GetValue());
	break;
	case GPXRECORDSET_LOGFINDER:
		var->Set(m_currow->m_logs.GetChild(a)->Locate("groundspeak:finder")->GetValue());
	break;
	case GPXRECORDSET_LOGTEXT:
		var->Set(m_currow->m_logs.GetChild(a)->Locate("groundspeak:text")->GetValue());
	break;
	}
}

/* this is the post-write callback triggered after basic writes to a variable */

void GPXRecordSet::SaveField(GPXRecordVar *var,int ud)
{
	int a;
	kGUIString s;

	assert(m_currow!=0,"Internal error, no row to update!");

	a=ud>>8;	/* array index for vars in arrays */
	switch(ud&255)
	{
	case GPXCOL_NA:
		m_currow->m_na.SetSelected(var->GetBoolean());
	break;
	case GPXCOL_FOUND:
		m_currow->m_found.SetSelected(var->GetBoolean());
	break;
	case GPXCOL_TYPE:
		m_currow->SetType(var->GetInt());
	break;
	case GPXCOL_USER1:
		m_currow->m_user[0].SetSelected(var->GetBoolean());
	break;
	case GPXCOL_USER2:
		m_currow->m_user[1].SetSelected(var->GetBoolean());
	break;
	case GPXCOL_USER3:
		m_currow->m_user[2].SetSelected(var->GetBoolean());
	break;
	case GPXCOL_USER4:
		m_currow->m_user[3].SetSelected(var->GetBoolean());
	break;
	case GPXCOL_USER5:
		m_currow->m_user[4].SetSelected(var->GetBoolean());
	break;
	case GPXCOL_USER6:
		m_currow->m_user[5].SetSelected(var->GetBoolean());
	break;
	case GPXCOL_USER7:
		m_currow->m_user[6].SetSelected(var->GetBoolean());
	break;
	case GPXCOL_USER8:
		m_currow->m_user[7].SetSelected(var->GetBoolean());
	break;
	case GPXCOL_NAME:
		var->GetString(&m_currow->m_name);
	break;
	case GPXCOL_WPTNAME:
		var->GetString(&m_currow->m_wptname);
	break;
	case GPXCOL_LOG:
		var->GetString(&m_currow->m_log);
	break;
	case GPXCOL_CONTAINER:
		if(var->GetType()==VARTYPE_INTEGER)
			m_currow->m_container.SetSelectionz(var->GetInt());
		else
		{
			var->GetString(&s);
			m_currow->m_container.SetSelectionz(s.GetString());
		}
	break;
	case GPXCOL_DIFFICULTY:
		m_currow->m_difficulty.SetDouble("%.1f",var->GetDouble());
	break;
	case GPXCOL_TERRAIN:
		m_currow->m_terrain.SetDouble("%.1f",var->GetDouble());
	break;
	case GPXCOL_DIST:
		m_currow->m_distval=var->GetDouble();
	break;
	case GPXCOL_CORRECTED:
		m_currow->m_corrected.SetSelected(var->GetBoolean());
	break;
	case GPXCOL_LAT:
		m_currow->m_llcoord.SetLat(var->GetDouble());
		//todo: rebuild string ?
	break;
	case GPXCOL_LON:
		m_currow->m_llcoord.SetLon(var->GetDouble());
		//todo: rebuild string ?
	break;
	case GPXCOL_HINT:
		var->GetString(&m_currow->m_hint);
	break;
	case GPXCOL_USERNOTES:
		var->GetString(&m_currow->m_usernotes);
	break;
	case GPXCOL_OWNER:
		var->GetString(&m_currow->m_owner);
	break;
	case GPXCOL_STATE:
		var->GetString(&m_currow->m_state);
	break;
	case GPXCOL_COUNTRY:
		var->GetString(&m_currow->m_country);
	break;
	case GPXCOL_GENDATE:
		var->GetString(&m_currow->m_gendate);
	break;
	case GPXRECORDSET_NUMLOGS:
		/* this is not changeable! */
	break;
	case GPXRECORDSET_LOGID:
		var->GetString(&s);
		m_currow->m_logs.GetChild(a)->Locate("id")->SetValue(s.GetString());
	break;
	case GPXRECORDSET_LOGDATE:
		var->GetString(&s);
		m_currow->m_logs.GetChild(a)->Locate("groundspeak:date")->SetValue(s.GetString());
	break;
	case GPXRECORDSET_LOGTYPE:
		var->GetString(&s);
		m_currow->m_logs.GetChild(a)->Locate("groundspeak:type")->SetValue(s.GetString());
	break;
	case GPXRECORDSET_LOGFINDER:
		var->GetString(&s);
		m_currow->m_logs.GetChild(a)->Locate("groundspeak:finder")->SetValue(s.GetString());
	break;
	case GPXRECORDSET_LOGTEXT:
		var->GetString(&s);
		m_currow->m_logs.GetChild(a)->Locate("groundspeak:text")->SetValue(s.GetString());
	break;
	}
}

void GPXRecordSet::LoadRecord(void)
{
	unsigned int i;
	unsigned int nl;
//	kGUIXMLItem *log;
	kGUIBasicIndices indices;

	if(!m_numwpts)
	{
		for(i=0;i<GPXCOL_NUMCOLUMNS;++i)
			m_rowvalues[i].SetIsUndefined(true);
		m_numlogs.SetIsUndefined(true);
		/* todo, invalidate log value entries */
		m_currow=0;
	}
	else
	{
		m_currow=m_wptlist.GetEntry(m_curindex);

		nl=m_currow->m_logs.GetNumChildren();
		m_numlogs.SetIsConstant(true);

		m_numlogs.SetIsUndefined(false);
		m_numlogs.Set((int)nl);

		/* these are 1 dimensional arrays */
		
		if(nl>m_maxlogs)
		{
			m_maxlogs=nl;

			indices.AddIndice(nl);

			m_logid.ReDim(&indices,&m_logid,false);
			m_logdate.ReDim(&indices,&m_logdate,false);
			m_logtype.ReDim(&indices,&m_logtype,false);
			m_logfinder.ReDim(&indices,&m_logfinder,false);
			m_logtext.ReDim(&indices,&m_logtext,false);

			/* set info for array entries */
			for(i=0;i<nl;++i)
			{
				m_logid.m_arrayentries[i].SetIndex(this,GPXRECORDSET_LOGID+(i<<8));
				m_logdate.m_arrayentries[i].SetIndex(this,GPXRECORDSET_LOGDATE+(i<<8));
				m_logtype.m_arrayentries[i].SetIndex(this,GPXRECORDSET_LOGTYPE+(i<<8));
				m_logfinder.m_arrayentries[i].SetIndex(this,GPXRECORDSET_LOGFINDER+(i<<8));
				m_logtext.m_arrayentries[i].SetIndex(this,GPXRECORDSET_LOGTEXT+(i<<8));
			}
		}
	}
}

void GPXRecordSet::MergeRecord(GPXRow *row)
{
	GPXRow *oldrow;
	GPXRow *newrow;
	GPXRow *rrow;

	/* since we are running async we need to grab access to app globals */
	kGUI::GetAccess();

	gpx->GetColour(row);	/* assign label colors */

	oldrow=gpx->Locate(&row->m_wptname);
	if(oldrow)
	{
		oldrow->Copy(row,true);
		rrow=gpx->InTable(gpx->m_routes.GetTable(),row->m_wptname.GetString());
		if(rrow)
			rrow->Copy(oldrow);
	}
	else
	{
		newrow=new GPXRow();
		newrow->Copy(row,true);
		gpx->m_wptlist.SetEntry(gpx->m_numwpts++,newrow);
	}
	kGUI::ReleaseAccess();
}

void GPXRecordSet::PurgeRecords(void)
{
	unsigned int i;
	GPXRow *row;

	for(i=0;i<m_numwpts;++i)
	{
		row=m_wptlist.GetEntry(i);
		if(row->m_purge==true)
			delete row;
	}
	m_numwpts=0;
}

/******************************************/
/* interface to upload/download from GPSr */
/******************************************/

enum
{
GPSR_SENDTOGPS,
GPSR_RECEIVEFROMGPS,
GPSR_NUMENTRIES
};

fntoid_def GPSRFuncs[]={
	{GPSR_SENDTOGPS,		"Send",			0,"byval string",	"Send(gpsrname as string,filename as string)",			"Send GPX (xml) file to GPSR"},
	{GPSR_RECEIVEFROMGPS,	"Receive",		1,"byval string",	"Receive(gpsrname as string,filename as string)",		"Receive GPX (xml) file from GPSR"}};

class GPSR : public kGUIBasicClassObj
{
public:
	GPSR();
	~GPSR() {}
	static kGUIBasicClassObj *Alloc(void) {return new GPSR();}
	static void RegisterErrors(kGUIBasic *b);
	kGUIBasicObj *GetObj(const char *name);
	int Function(int fieldid,kGUIBasicVarObj *result,CallParms *p);
private:
	kGUIBasicSysClassFuncObj m_commands[sizeof(GPSRFuncs)/sizeof(fntoid_def)];
	kGUIBasicObj *m_fieldobjects[GPSR_NUMENTRIES];
	Hash m_fieldhash;	/* small hash list for field name to index conversion */
	/* error message values */
	static unsigned int m_errorfile;
	static unsigned int m_errorbadname;
	static unsigned int m_errorcomm;
};

unsigned int GPSR::m_errorfile;
unsigned int GPSR::m_errorbadname;
unsigned int GPSR::m_errorcomm;

GPSR::GPSR()
{
	unsigned int i;

	SetType(VARTYPE_CLASS);
	m_fieldhash.Init(8,sizeof(int));

	/* add functions next */
	for(i=0;i<sizeof(GPSRFuncs)/sizeof(fntoid_def);++i)
	{
		m_fieldhash.Add(GPSRFuncs[i].name,&GPSRFuncs[i].id);
		m_commands[i].Init(this,GPSRFuncs[i].id,GPSRFuncs[i].np,GPSRFuncs[i].parmdef);
		m_fieldobjects[GPSRFuncs[i].id]=&m_commands[i];
	}
}

void GPSR::RegisterErrors(kGUIBasic *b)
{
	m_errorfile=b->RegisterError("Cannot open file!");
	m_errorbadname=b->RegisterError("Named GPSr not found in GPSr list!");
	m_errorcomm=b->RegisterError("Error communicating with GPSr!");
}

/* class object can be any of the objects types, for example, it can */
/* be any variable type as well as a function or subroutine call object too. */

kGUIBasicObj *GPSR::GetObj(const char *name)
{
	int *pid;

	pid=(int *)m_fieldhash.Find(name);
	if(!pid)
		return(0);
	return(m_fieldobjects[pid[0]]);	/* return pointer to object */
}

int GPSR::Function(int fieldid,kGUIBasicVarObj *result,CallParms *p)
{	
	switch(fieldid)
	{
	case GPSR_SENDTOGPS:
	{
		int rc;
		kGUIBasicVarObj *p1;
		kGUIBasicVarObj *p2;

		if(p->GetNumParms()!=2)
			return(ERROR_WRONGNUMBEROFPARAMETERS);

		p1=p->GetParm(0);
		p1->ChangeType(VARTYPE_STRING);
		p2=p->GetParm(0);
		p2->ChangeType(VARTYPE_STRING);

		/* send gpx file to GPSR */
		rc=gpx->GetGPSrObj()->UploadToGPSr(p1->GetStringObj()->GetString(),p2->GetStringObj()->GetString());
		/* 0=ok, 1=no gpsrnamed found, 2=comm error, 3=file not found */
		switch(rc)
		{
		case 1:
			return(m_errorbadname);
		break;
		case 2:
			return(m_errorcomm);
		break;
		case 3:
			return(m_errorfile);
		break;
		}
		result->Set((int)0);
		return(ERROR_OK);
	}
	break;
	case GPSR_RECEIVEFROMGPS:
	{
		kGUIBasicVarObj *p1;
		kGUIBasicVarObj *p2;

		if(p->GetNumParms()!=2)
			return(ERROR_WRONGNUMBEROFPARAMETERS);

		p1=p->GetParm(0);
		p1->ChangeType(VARTYPE_STRING);
		p2=p->GetParm(0);
		p2->ChangeType(VARTYPE_STRING);

		/* todo, download file from GPSR */

		result->Set((int)0);
		return(ERROR_OK);
	}
	break;
	}
	return(ERROR_SYNTAX);
}

/****************************************************************************************/

typedef struct
{
	int id;
	const char *name;
}cid_def;

cid_def APPConstants[]={
	{CACHETYPE_UNDEFINED,	"CACHETYPE_UNDEFINED"},
	{CACHETYPE_TRADITIONAL,"CACHETYPE_TRADITIONAL"},
	{CACHETYPE_MULTI,		"CACHETYPE_MULTI"},
	{CACHETYPE_VIRTUAL,		"CACHETYPE_VIRTUAL"},
	{CACHETYPE_EVENT,		"CACHETYPE_EVENT"},
	{CACHETYPE_UNKNOWN,		"CACHETYPE_UNKNOWN"},
	{CACHETYPE_REVERSE,		"CACHETYPE_REVERSE"},
	{CACHETYPE_EARTH,		"CACHETYPE_EARTH"},
	{CACHETYPE_NGS,			"CACHETYPE_NGS"},
	{CACHETYPE_LETTERBOX,	"CACHETYPE_LETTERBOX"},
	{CACHETYPE_WEBCAM,		"CACHETYPE_WEBCAM"},
	{CACHETYPE_CITO,		"CACHETYPE_CITO"},
	{CACHETYPE_PROJECTAPE,	"CACHETYPE_PROJECTAPE"},
	{CACHETYPE_OTHER,		"CACHETYPE_OTHER"},
	{CACHETYPE_NUM,			"CACHETYPE_NUM"},
	{CONTAINERTYPE_MICRO,	"CONTAINERTYPE_MICRO"},
	{CONTAINERTYPE_SMALL,	"CONTAINERTYPE_SMALL"},
	{CONTAINERTYPE_REGULAR,	"CONTAINERTYPE_REGULAR"},
	{CONTAINERTYPE_LARGE,	"CONTAINERTYPE_LARGE"},
	{CONTAINERTYPE_OTHER,	"CONTAINERTYPE_OTHER"},
	{CONTAINERTYPE_VIRTUAL,	"CONTAINERTYPE_VIRTUAL"},
	{CONTAINERTYPE_NOTCHOSEN,"CONTAINERTYPE_NOTCHOSEN"},
	{CONTAINERTYPE_NUM,		 "CONTAINERTYPE_NUM"}};

typedef struct
{
	int numentries;
	const char *name;
	const char **entries;
}cida_def;

extern const char *cachetypenames[CACHETYPE_NUM];
extern const char *containernames[CONTAINERTYPE_NUM];

cida_def AppConstantArrays[]={
	{CACHETYPE_NUM,			"CACHETYPENAMES",		cachetypenames},
	{CONTAINERTYPE_NUM,		"CONTAINERTYPENAMES",	containernames}};

void GPX::AddCClasses(void)
{
	unsigned int i;
	kGUIBasicVarObj *vobj;
	kGUIBasicVarObj *ve;

	/* register the integer APP constants */
	for(i=0;i<sizeof(APPConstants)/sizeof(cid_def);++i)
	{
		vobj=new kGUIBasicVarObj();
		vobj->SetIsGlobal(true);
		vobj->SetIsUndefined(false);
		vobj->SetIsConstant(true);
		vobj->Set(APPConstants[i].id);
		m_basic.AddAppObject(APPConstants[i].name,vobj);
	}

	/* register the string APP array constants */
	for(i=0;i<sizeof(AppConstantArrays)/sizeof(cida_def);++i)
	{
		vobj=new kGUIBasicVarObj();
		vobj->SetType(VARTYPE_STRING);
		vobj->SetIsGlobal(true);
		vobj->SetIsUndefined(false);
		vobj->SetIsConstant(true);
		vobj->SetIsArray(true);
		vobj->AddIndice(AppConstantArrays[i].numentries);
		vobj->AllocArray(AppConstantArrays[i].numentries);
		for(int j=0;j<AppConstantArrays[i].numentries;++j)
		{
			ve=vobj->GetEntry(j);	//&(vobj->m_arrayentries[j]);
			ve->SetIsGlobal(true);
			ve->SetIsUndefined(false);
			ve->SetIsConstant(true);
			ve->Set(AppConstantArrays[i].entries[j]);
		}
		m_basic.AddAppObject(AppConstantArrays[i].name,vobj);
	}

	/* register the app classes */
	vobj=new kGUIBasicVarObj();
	vobj->SetType(VARTYPE_SYSCLASSDEF);
	vobj->SetSystemClassAllocator(&(GPXRecordSet::Alloc));
	GPXRecordSet::RegisterErrors(&m_basic);
	m_basic.AddAppObject("GPXRecordSet",vobj);

	/* register the app classes */
	vobj=new kGUIBasicVarObj();
	vobj->SetType(VARTYPE_SYSCLASSDEF);
	vobj->SetSystemClassAllocator(&(GPSR::Alloc));
	GPSR::RegisterErrors(&m_basic);
	m_basic.AddAppObject("GPSR",vobj);
}

/**********************************************************************/

void GPX::GetAppBasicInstructions(kGUIString *h)
{
	unsigned int i;

	h->Append(
		"<xTABSTART>Constants<xTABEND>"
		"<xGROUPSTART>"
		"<table border=1 cellpadding=4 cellspacing=0>"
		"<tr><xCOLSTART>Name<xCOLEND></tr>");

	for(i=0;i<sizeof(APPConstants)/sizeof(cid_def);++i)
		h->ASprintf("<tr><td>%s</td></tr>",APPConstants[i].name);

	h->Append(
		"</table><xGROUPEND><BR><BR>"
		"<xTABSTART>GPXRecordSet<xTABEND>"
		"<xGROUPSTART>"
		"<table border=1 cellpadding=4 cellspacing=0>"
		"<tr><xCOLSTART>Name<xCOLEND><xCOLSTART>Description<xCOLEND></tr>");

	for(i=0;i<GPXCOL_NUMCOLUMNS;++i)
		h->ASprintf("<tr><td>%s</td><td>%s</td></tr>",wpcolnames[i],wpcoldesc[i]);

	h->ASprintf("<tr><td>U[]</td><td>UserTicks - Can be accessed using this array reference as well.</td></tr>");
	h->ASprintf("<tr><td>NumLogs</td><td>Number of Logs.</td></tr>");

	h->ASprintf("<tr><td>LogID[]</td><td>Log ID Array</td></tr>");
	h->ASprintf("<tr><td>LogDate[]</td><td>Log Date Array</td></tr>");
	h->ASprintf("<tr><td>LogType[]</td><td>Log Type Array</td></tr>");
	h->ASprintf("<tr><td>LogFinder[]</td><td>Log Finder Array</td></tr>");
	h->ASprintf("<tr><td>LogText[]</td><td>Log Text Array</td></tr>");

	for(i=0;i<sizeof(GPXRecordSetFuncs)/sizeof(fntoid_def);++i)
		h->ASprintf("<tr><td>%s</td><td><B>%s</B><BR>%s</td></tr>",GPXRecordSetFuncs[i].name,GPXRecordSetFuncs[i].desc2,GPXRecordSetFuncs[i].desc1);

	h->Append("</table><xGROUPEND><BR><BR>");
}
