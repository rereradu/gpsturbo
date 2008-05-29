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

#ifndef __OZMAP__
#define __OZMAP__

#include "map.h"

typedef struct
{
	unsigned char r,g,b,a;
}OZ2FCOLOUR_DEF;

#pragma pack( push, 1)
typedef struct
{
	int dwImageWidth;
	int dwImageHeight;

	short  wXTilesNum;
	short  wYTilesNum;

	OZ2FCOLOUR_DEF dwColor [256];

	int dwTileDataOffset [1];
}OZ2FBITMAP_DEF;
#pragma pack( pop)

typedef struct
{
	int x,y;
	double lat,lon;
}OZF2Corners;

/* ozi explorer2 map */
class OZF2GPXMap : public GPXMap
{
public:
	OZF2GPXMap(const char *fn);
	~OZF2GPXMap();
	void ToMap(class GPXCoord *c,int *sx,int *sy);
	void FromMap(int sx,int sy,class GPXCoord *c);
	int DrawTile(int tx,int ty);	/* draw the tile to the current display */
	int GetNumCopyrightLines(void) {return 0;}
	const char *GetCopyrightLine(int l) {return 0;}

private:
	const unsigned char *m_filedata;
	OZF2Corners *m_coords;
	int m_minx,m_miny;
	double m_minlat,m_minlon;
	double m_lsx,m_lsy;
	double m_slx,m_sly;
	Array<OZ2FBITMAP_DEF *>m_bitmaps;
};
#endif
