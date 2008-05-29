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

TSGPXMap::TSGPXMap(int type)
{	
	int i;
	double c;
	int tw,th;

	m_type=type;	/* 3 different types */
	SetZoomLevels(MINTSZOOM,MAXTSZOOM);
	SetTileSize(200,200);

	tw=50;
	th=52;
	for(i=MINTSZOOM;i<MAXTSZOOM;++i)
	{
		c=(double)(1<<i);
		m_pixelsPerLonDegree[i]=c/360.0f;
  		m_negpixelsPerLonRadian[i] = -(c / (2.0f*3.1415926535f));
		m_bitmapOrigo[i]=c/2.0f;
		m_twide[i]=tw;
		m_ttall[i]=th;
		SetSize(i,tw*200,th*200);
		tw<<=1;
		th<<=1;
	}
}

TSGPXMap::~TSGPXMap()
{
}

void TSGPXMap::ToMap(GPXCoord *c,int *sx,int *sy)
{
#if 1
	sx[0]=0;
	sy[0]=0;
#else
	double e;
	int z=GetZoom();

  	sx[0] = (int)(floor((m_bitmapOrigo[z] + c->m_lon.m_value * m_pixelsPerLonDegree[z])*256.0f));
  	e = sin(c->m_lat.m_value * (3.1415926535f/180.0f));

  	if(e > 0.9999)
    	e = 0.9999;

  	if(e < -0.9999)
    	e = -0.9999;

  	sy[0] = (int)(floor((m_bitmapOrigo[z] + 0.5f * log((1.0f + e) / (1.0f - e)) * m_negpixelsPerLonRadian[z])*256.0f));
#endif
}


/* convert from screen+scroll values to lon/lat */
void TSGPXMap::FromMap(int sx,int sy,GPXCoord *c)
{
	double e;
	int z=GetZoom();

	c->SetLon(((double)sx - (m_bitmapOrigo[z]*256.0f)) / (m_pixelsPerLonDegree[z]*256.0f));
	e = ((double)sy - (m_bitmapOrigo[z]*256.0f)) / (m_negpixelsPerLonRadian[z]*256.0f);
	c->SetLat((2.0f * atan(exp(e)) - 3.1415926535f / 2.0f) / (3.1415926535f/180.0f));
}

/* draw tile at this position */
int TSGPXMap::DrawTile(int tx,int ty)
{
	kGUIImage i;
	kGUIString fn;
	int zoom=GetZoom();
	int t,mz;
	int ttx,tty;

	/* convert grid number to a filename */

	ttx=tx;
	tty=m_ttall[zoom]-ty;

	mz=29-zoom;
	if(m_type==MAPTYPE_TERRASERVSAT)
		t=1;
	else
		t=2;	//MAPTYPE_TERRASERVTOPO

	fn.Sprintf(TERRASERVMAPDIR "ts%dz%dz%dz%d.jpg",t,mz,ttx,tty);
	i.SetFilename(fn.GetString());

	/* if image was not found on harddrive then try downloading it if user is online */
	if(i.IsValid()==false)
	{
		if(g_isonline==true)
		{
			kGUIString gurl;
			DataHandle dh;
			kGUIDownloadEntry dle;

	//http://terraserver-usa.com/tile.ashx?t=1&s=19&x=6&y=52&z=10
	//http://terraserver-usa.com/tile.ashx?t=2&s=19&x=6&y=52&z=10

			gurl.Sprintf("http://terraserver-usa.com/tile.ashx?t=%d&s=%d&x=%d&y=%d&z=10",t,mz,ttx,tty);
//			sprintf(gurl,"http://terraserver-usa.com/tile.ashx?t=1&s=19&x=6&y=52&z=10");
			dh.SetFilename(fn.GetString());
			dle.DownLoad(&dh,&gurl);

			/* since the file has now been loaded, re-trigger a load attempt */
			i.SetFilename(fn.GetString());
		}

		if(i.IsValid()==false)
		{
			i.Purge();
			return(TILE_ERROR);	/* tile not found! */
		}
	}
	i.Draw(0,0,0);
	i.Purge();
	return(TILE_OK);
}

