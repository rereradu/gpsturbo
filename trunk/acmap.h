#if 1
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

#ifndef __ACMAP__
#define __ACMAP__

#include "map.h"

#define MINACZOOM 8
#define MAXACZOOM 16

/* atlas of canada map */
class ACGPXMap : public GPXMap
{
public:
	ACGPXMap();
	~ACGPXMap();
	int DrawTile(int tx,int ty);	/* draw the tile to the current display */
	void ToMap(class GPXCoord *c,int *sx,int *sy);
	void FromMap(int sx,int sy,class GPXCoord *c);
	int GetNumCopyrightLines(void) {return 1;}
	const char *GetCopyrightLine(int l) {return m_copyright.GetString();}
private:
	int RShift(int value,int numbits);
	int m_type;
	kGUIImage m_image;	
	kGUIString m_copyright;
};
#endif
#endif
