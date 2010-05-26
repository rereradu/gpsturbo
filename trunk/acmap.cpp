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
#include "acmap.h"

#if 0
#define TILEW 3200
#define TILEH 1600

/* the tile format is a grid that is 12 tiles wide by 10 tiles tall */
/* each tile is subdivided into 16 smaller tiles and then each child */
/* tile is again subdivided into 16 pieces */
/* there are only 2 levels of tiles, the rest are drawn using scaling */

enum
{
MAPLEVEL_1,
MAPLEVEL_2
};

typedef struct
{
	int maplevel;
	int rshift;
}ACLEVEL_DEF;

ACLEVEL_DEF aclevels[MAXACZOOM-MINACZOOM]={
	{MAPLEVEL_1,0},
	{MAPLEVEL_1,1},
	{MAPLEVEL_1,2},
	{MAPLEVEL_1,3},
	{MAPLEVEL_2,2},
	{MAPLEVEL_2,3},
	{MAPLEVEL_2,4},
	{MAPLEVEL_2,5}};

ACGPXMap::ACGPXMap()
{	
	int i;
	int tw,th;

	m_copyright.Sprintf("%c Government of Canada",169);
	SetZoomLevels(MINACZOOM,MAXACZOOM);
	SetTileSize(TILEW>>3,TILEH>>3);

	tw=(12*16)*TILEW>>5;
	th=(10*16)*TILEH>>5;
	for(i=MINACZOOM;i<MAXACZOOM;++i)
	{
		SetSize(i,tw,th);
		tw<<=1;
		th<<=1;
	}
}

ACGPXMap::~ACGPXMap()
{
	m_image.Purge();
}

int ACGPXMap::RShift(int value,int numbits)
{
	if(numbits==0)
		return(value);
	if(numbits>0)
		return(value>>numbits);
	return(value<<(-numbits));
}

void ACGPXMap::ToMap(GPXCoord *c,int *sx,int *sy)
{
	int z=GetZoom();

    sx[0]=(int)((c->GetLon()+144.0f)*RShift(TILEW,12-z));	/* .5 degrees per tile */
	sy[0]=(int)((80-c->GetLat())*RShift(TILEH,11-z));		/* .25 degrees per tile */
}


/* convert from screen+scroll values to lon/lat */
void ACGPXMap::FromMap(int sx,int sy,GPXCoord *c)
{
	int z=GetZoom();
	
	c->Set(80.0f-((double)sy/RShift(TILEW,12-z)),((double)sx/RShift(TILEW,12-z))-144.0f);
}

/* draw tile at this position */
int ACGPXMap::DrawTile(int tx,int ty)
{
	kGUIString hstr;
	kGUIString fn;
	char xch[]={"mnoplkjiefghdcba"};
	int xin[]={13,14,15,16,12,11,10,9,5,6,7,8,4,3,2,1};
	int ftx,fty;
	int xgroup,ygroup,lgroup,ngroup;

	/* convert grid number to a filename */
	int z=GetZoom();
	ACLEVEL_DEF *acl;

	acl=&(aclevels[z-MINACZOOM]);
	ftx=tx>>acl->rshift;
	fty=ty>>acl->rshift;
	if(acl->maplevel==MAPLEVEL_1)
	{
		xgroup=11-(ftx>>2);
		ygroup=9-(fty>>2);
		lgroup=((ftx)&3)+(((fty)&3)<<2);
		hstr.Sprintf("%02dz%02d%d%c",xgroup,xgroup,ygroup,xch[lgroup]);
	}
	else
	{
		xgroup=11-(ftx>>4);
		ygroup=9-(fty>>4);
		lgroup=((ftx>>2)&3)+(((fty>>2)&3)<<2);
		ngroup=(ftx&3)+((fty&3)<<2);

		hstr.Sprintf("%02dz%02d%d%c%02d",xgroup,xgroup,ygroup,xch[lgroup],xin[ngroup]);
	}

	fn.Sprintf(TOPOMAPDIR "t%s.gif",hstr.GetString());
	m_image.SetFilename(fn.GetString());

	/* if image was not found on harddrive then try downloading it if user is online */
	if(m_image.IsValid()==false)
	{
		if(g_isonline==true)
		{
			kGUIString gurl;
			DataHandle dh;
			kGUIDownloadEntry dle;
			kGUIString hx;
			kGUIString busytitle;

			hx.SetString(hstr.GetString());
			hx.Replace("z","/");

			if(acl->maplevel==MAPLEVEL_1)
				gurl.Sprintf("http://toporama.cits.rncan.gc.ca/images/b250k/%s.gif",hx.GetString());
			else
				gurl.Sprintf("http://toporama.cits.rncan.gc.ca/images/b50k/%s.gif",hx.GetString());
			
			dh.SetFilename(&fn);
			dle.DownLoad(&dh,&gurl);

			/* since it is now downloaded try again */
			m_image.SetFilename(fn.GetString());
		}
	}

	if(m_image.IsValid()==false)
		return(TILE_ERROR);	/* tile not found! */

	switch(acl->rshift)
	{
	case 0:
		m_image.SetScale(1.0f/8.0f,1.0f/8.0f);
		m_image.Draw(0,0,0);
	break;
	case 1:
		m_image.SetScale(1.0f/4.0f,1.0f/4.0f);
		m_image.Draw(0,-((tx&1)*(TILEW>>3)),-((ty&1)*(TILEH>>3)));
	break;
	case 2:
		m_image.SetScale(1.0f/2.0f,1.0f/2.0f);
		m_image.Draw(0,-((tx&3)*(TILEW>>3)),-((ty&3)*(TILEH>>3)));
	break;
	case 3:
		m_image.SetScale(1.0f,1.0f);
		m_image.Draw(0,-((tx&7)*(TILEW>>3)),-((ty&7)*(TILEH>>3)));
	break;
	case 4:
		m_image.SetScale(1.0f/0.5f,1.0f/0.5f);
		m_image.Draw(0,-((tx&15)*(TILEW>>3)),-((ty&15)*(TILEH>>3)));
	break;
	case 5:
		m_image.SetScale(1.0f/0.25f,1.0f/0.25f);
		m_image.Draw(0,-((tx&31)*(TILEW>>3)),-((ty&31)*(TILEH>>3)));
	break;
	}
	return(TILE_OK);
}
#endif
