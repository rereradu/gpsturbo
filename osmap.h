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

#ifndef __OSMMAP__
#define __OSMMAP__

#include "map.h"

#define MINOSMZOOM 3
#define MAXOSMZOOM 19

typedef struct
{
	GPXCoord c;
}OSMNODE_DEF;

typedef struct
{
	int id;
	const char *name;
	int rendertype;
	int rendersubtype;
	kGUIColor colour;
}OSMRENDER_INFO;

class OSMWAY
{
public:
	OSMWAY() {m_ori=0;}
	unsigned int m_numnodes;
	kGUICorners m_corners;
	GPXCoord m_min;
	GPXCoord m_max;
	OSMNODE_DEF **m_nodes;
	const OSMRENDER_INFO *m_ori;
	kGUIDPoint2 *m_proj;		/* used for pass 2 */
};

#define MAXOPP 8192

/* openstreetmap.com map ( xml format ) */
class OSMMap : public GPXMap
{
public:
	OSMMap(const char *fn);
	~OSMMap();
	void ToMap(class GPXCoord *c,int *sx,int *sy);
	void FromMap(int sx,int sy,class GPXCoord *c);
	int DrawTile(int tx,int ty);	/* draw the tile to the current display */
	int GetNumCopyrightLines(void) {return 1;}
	const char *GetCopyrightLine(int l) {return "\xa9 www.openstreetmap.org Creative Commons Attribution-ShareAlike 2.0 license";}

private:
	unsigned int GetTag(const char *s);
	double m_pixelsPerLonDegree[MAXOSMZOOM];
	double m_negpixelsPerLonRadian[MAXOSMZOOM];
	double m_bitmapOrigo[MAXOSMZOOM];
	Hash m_tags;
	Hash m_nodes;
	Hash m_roadinfo;
	Heap m_heap;
	Heap m_drawheap;
	unsigned int m_numways;
	ClassArray<OSMWAY>m_ways;

	unsigned int m_numdrawways;
	Array<OSMWAY *>m_drawways;
	static kGUIDPoint2 m_ppoints[MAXOPP];

	//print out unknown tags, this is there to stop duplicates from being printed
	Hash m_unknown;
};
#endif
