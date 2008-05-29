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

typedef struct
{
	bool found;
	double lon,lat;
}WPVERIFY_DEF;

UploadXML::UploadXML()
{
	m_addchildren=true;
	m_nameformat=UPNAME_TRUNCATE;
	m_maxpoints=-1;
	m_maxlen=8;

	m_numpoints=0;			/* sum of regular and child points added */
	m_numchildpoints=0;

	m_names.Init(8,sizeof(WPVERIFY_DEF));
	m_xml=new kGUIXML();

	m_root=m_xml->GetRootItem()->AddChild("gpx");
	m_root->AddParm("Version","1.0");
	m_root->AddParm("creator","gpsturbo");
	m_root->AddParm("xmlns:xsi","http://www.w3.org/2001/XMLSchema-instance");
	m_root->AddParm("xmlns","http://www.topografix.com/GPX/1/0");
	m_root->AddParm("xsi:schemaLocation","http://www.topografix.com/GPX/1/0 http://www.topografix.com/GPX/1/0/gpx.xsd");
}

UploadXML::~UploadXML()
{
	delete m_xml;
}

void UploadXML::Init(int maxpoints,int maxnamelen,bool addchildren,int nameformat)
{
//	SetMaxPoints(gpsrow->m_maxwpts.GetInt());
//	SetMaxLen(gpsrow->m_maxwptlen.GetInt());
//	SetAddChildren(gpsrow->m_children.GetSelected());
//	SetNameFormat(gpsrow->m_nameformat.GetSelection());
}

void UploadXML::GenName(kGUIString *name)
{
	unsigned int rindex;
	char cc[2];
	static char conschars[]={"BCDFGHJKLMNPQRSTVWXZbcdfghjklmnpqrstvwxz0123456789"};
	static char validchars[]={"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwnyz0123456789"};

	/* remove all invalid characters */
	name->Clean(validchars);

	switch(m_nameformat)
	{
	case UPNAME_WAYNAME:
	case UPNAME_TRUNCATE:
		/* if too long, remove spaces, then clip */
		rindex=0;
		while(rindex<m_maxlen && name->GetLen()>m_maxlen)
		{
			if(name->GetChar(rindex)==' ')
				name->Delete(rindex,1);
			else
				++rindex;
		}
		name->Clip(m_maxlen);
	break;
	case UPNAME_DROPVOWELS:	/* only use consonants or numbers, remove vowels and other characters */
		rindex=0;
		while(rindex<m_maxlen && name->GetLen()>m_maxlen)
		{
			cc[0]=name->GetChar(rindex);
			cc[1]=0;
			if(strstr(conschars,cc))
				++rindex;
			else
				name->Delete(rindex,1);
		}
		name->Clip(m_maxlen);
	break;
	}
	if(!name->GetLen())
		name->SetString("aaaa");

	rindex=name->GetLen()-1;

	do
	{
		if(!m_names.Find(name->GetString()))
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


void UploadXML::Add(GPXRow *row)
{
	kGUIXMLItem *point;
	double lat,lon;
	kGUIString name;
	kGUIString hint;
	kGUIString fullname;
	WPVERIFY_DEF def;
	static char validchars[]={" ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwnyz0123456789"};

	/* am I full already? */
	if(m_numpoints==m_maxpoints)
		return;

	point=m_root->AddChild("wpt");
	lat=row->m_llcoord.GetLat();
	lon=row->m_llcoord.GetLon();
	point->AddParm("lat",lat);
	point->AddParm("lon",lon);

	hint.SetString(row->m_hint.GetString());
	hint.Clean(validchars);

	switch(m_nameformat)
	{
	case UPNAME_WAYNAME:
		name.SetString(row->m_wptname.GetString());
	break;
	case UPNAME_TRUNCATE:
	case UPNAME_DROPVOWELS:
		name.SetString(row->m_name.GetString());
	break;
	}
	fullname.SetString(name.GetString());
	GenName(&name);
	def.found=false;
	def.lat=lat;
	def.lon=lon;
	m_names.Add(name.GetString(),&def);

	/* check hash table fo collisions */
	point->AddChild("name",name.GetString());
	fullname.Clean(validchars);
//	point->AddChild("desc",fullname.GetString());
//	point->AddChild("cmt",fullname.GetString());
	point->AddChild("desc",hint.GetString());
	point->AddChild("cmt",hint.GetString());
	point->AddChild("sym","Geocache");				/* hmmmm */
	AddToBounds(lat,lon);

	if(++m_numpoints==m_maxpoints)
		return;

	if(m_addchildren==true && row->m_numchildren)
	{
		/* download child waypoints too */
		unsigned int c;
		GPXChild *cc;

		for(c=0;c<row->m_numchildren;++c)
		{
			cc=row->m_children.GetEntry(c);
			point=m_root->AddChild("wpt");
			lat=cc->GetLat();
			lon=cc->GetLon();
			point->AddParm("lat",lat);
			point->AddParm("lon",lon);

			switch(m_nameformat)
			{
			case 0:
				name.SetString(cc->GetWptName());
			break;
			case 1:
			case 2:
				name.SetString(cc->GetName());
			break;
			}

			GenName(&name);
			def.found=false;
			def.lat=lat;
			def.lon=lon;
			m_names.Add(name.GetString(),&def);

			point->AddChild("name",name.GetString());
			point->AddChild("desc",name.GetString());
			point->AddChild("cmt",name.GetString());
			point->AddChild("sym","Geocache");			/* todo: other? */
			++m_numchildpoints;
			AddToBounds(lat,lon);
			if(++m_numpoints==m_maxpoints)
				break;
		}
	}
}

bool UploadXML::Save(const char *fn)
{
	kGUIXMLItem *bounds;

	bounds=m_root->AddChild("bounds");
	bounds->AddParm("minlat",m_bounds.GetMinLat());
	bounds->AddParm("minlon",m_bounds.GetMinLon());
	bounds->AddParm("maxlat",m_bounds.GetMaxLat());
	bounds->AddParm("maxlon",m_bounds.GetMaxLon());

	return(m_xml->Save(fn));
}

void UploadXML::Found(const char *name,double lat,double lon)
{
	WPVERIFY_DEF *f;
	HashEntry *he;
	int h,hh;

#if 0
	f=(WPVERIFY_DEF *)m_names.Find(name);
	if(f)
	{
		if(fabs(f->lat-lat)<0.0001f && fabs(f->lon-lon)<0.0001f)
		{
			f->found=true;
			return;
		}
	}
#endif

	/* no direct name match, so scan through and find */

	he=m_names.GetFirst();
	hh=m_names.GetNum();
	for(h=0;h<hh;++h)
	{
		f=(WPVERIFY_DEF *)he->m_data;
		if(fabs(f->lat-lat)<0.0001f && fabs(f->lon-lon)<0.0001f)
		{
			f->found=true;
		//	return;
		}
		he=he->GetNext();
	}

	/* this point is not one we downloaded */
}

int UploadXML::GetNotFound(kGUIString *s)
{
	WPVERIFY_DEF *f;
	HashEntry *he;
	int h,hh;
	int notfound;

	notfound=0;
	he=m_names.GetFirst();
	hh=m_names.GetNum();
	for(h=0;h<hh;++h)
	{
		f=(WPVERIFY_DEF *)he->m_data;
		if(f->found==false)
		{
			s->ASprintf("%s\n",he->m_string);
			++notfound;
		}
		he=he->GetNext();
	}
	return(notfound);
}
