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

#ifndef __TSMAP__
#define __TSMAP__

#include "map.h"

#define MINTSZOOM 10
#define MAXTSZOOM 20

//http://terraserver-usa.com/tile.ashx?t=1&s=19&x=6&y=52&z=10
//http://terraserver-usa.com/tile.ashx?t=2&s=19&x=6&y=52&z=10

/* terra server USA map */
class TSGPXMap : public GPXMap
{
public:
	TSGPXMap(int type);
	~TSGPXMap();
	int DrawTile(int tx,int ty);	/* draw the tile to the current display */
	void ToMap(class GPXCoord *c,int *sx,int *sy);
	void FromMap(int sx,int sy,class GPXCoord *c);
	int GetNumCopyrightLines(void) {return 1;}
	const char *GetCopyrightLine(int l) {return "xxx";}
private:
	int m_type;
	double m_pixelsPerLonDegree[MAXTSZOOM];
	double m_negpixelsPerLonRadian[MAXTSZOOM];
	double m_bitmapOrigo[MAXTSZOOM];
	int m_twide[MAXTSZOOM];
	int m_ttall[MAXTSZOOM];
};
#endif
