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

#ifndef __MAP__
#define __MAP__

#include "gcoords.h"

enum
{
TILE_OK,
TILE_LOADING,
TILE_WAITING,
TILE_ERROR
};

#define GOOGLEMAPDIR "tiles" DIRCHAR
#define TOPOMAPDIR "tiles" DIRCHAR
#define TERRASERVMAPDIR "tiles" DIRCHAR
#define MAPDIR "maps" DIRCHAR

enum
{
	MAPTYPE_GOOGLESAT,
	MAPTYPE_GOOGLEHYBRID,
	MAPTYPE_GOOGLEMAP,
	MAPTYPE_GOOGLETERRAIN,
	MAPTYPE_OPENSTREETMAP,
	MAPTYPE_OZF2,
	MAPTYPE_MS,
	MAPTYPE_TERRASERVSAT,
	MAPTYPE_TERRASERVTOPO,
	MAPTYPE_NUM
};



/* all maps have this as their base class */
class GPXMap
{
public:
	GPXMap() {m_bad=false;m_zoom=0;m_mapwidths.Alloc(20);m_mapwidths.SetGrow(true);m_mapheights.Alloc(20);m_mapheights.SetGrow(true);}
	virtual ~GPXMap() {}
	void SetZoom(int z) {m_zoom=z;}			/* set the current zoom level */
	int GetZoom(void) {return m_zoom;}		/* get the current zoom level */
	int GetMaxZoom(void) {return m_maxzoom;}
	int GetMinZoom(void) {return m_minzoom;}
	void SetSize(int level,int width,int height) {m_mapwidths.SetEntry(level,width);m_mapheights.SetEntry(level,height);}
	int GetWidth(void) {return m_mapwidths.GetEntry(m_zoom);}		/* return width of map in pixels for current zoom level */
	int GetHeight(void) {return m_mapheights.GetEntry(m_zoom);}		/* return height of map in pixels for current zoom level */
	int GetTileWidth(void) {return m_tilewidth;}
	int GetTileHeight(void) {return m_tileheight;}
	void SetZoomLevels(int minz,int maxz) {m_minzoom=minz;m_maxzoom=maxz;}
	void SetTileSize(int tilew,int tileh) {m_tilewidth=tilew;m_tileheight=tileh;}

	virtual void ResetOnline(void) {}	/* only used in online based maps */
	virtual void ToMap(class GPXCoord *c,int *sx,int *sy)=0;
	virtual void FromMap(int sx,int sy,class GPXCoord *c)=0;
	virtual int DrawTile(int tx,int ty)=0;	/* draw the tile to the current display */
	bool m_bad;
	virtual int GetNumCopyrightLines(void)=0;
	virtual const char *GetCopyrightLine(int l)=0;
	virtual const char *GetStatusLine(void) {return 0;}

	/* search map for text? */
	virtual bool GetSearchable(void) {return false;}
	virtual void SearchMap(kGUIString *text) {}
	virtual void BrowseMap(void) {}
private:
	int m_zoom;						/* current zoom */
	int m_minzoom,m_maxzoom;
	int m_tilewidth,m_tileheight;
	Array<int>m_mapwidths;		/* map widths in pixels for each zoom level */
	Array<int>m_mapheights;		/* map heights in pixels for each zoom level */
};

//this class is for handling street name collisions

class GPXMapStrings
{
public:
	GPXMapStrings();
	~GPXMapStrings();
	void Init(unsigned int w,unsigned int h);
	void Clear(void);
	bool Check(kGUICorners *b,kGUIPoint2 *c,bool clipedge);
//private:
	kGUIDrawSurface *m_lcwindow;	/* label collision window */
	kGUICorners m_lcbounds;
};

/* all overlayed maps have this as their baseclass */
class GPXMapOverlay		//: DataHandle
{
public:
	virtual ~GPXMapOverlay() {}
	virtual void Draw(kGUICorners *c,double alpha)=0;
};

#endif
