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

#ifndef __KMAP__
#define __KMAP__

#include "map.h"
#include "zip.h"

class KMLArea
{
public:
	double m_north,m_south,m_east,m_west;
	kGUIImage m_image;
};

/* KML Map */
class KMLGPXMapOverlay : public GPXMapOverlay
{
public:
	KMLGPXMapOverlay();
	~KMLGPXMapOverlay();
	void SetFilename(const char *fn,ContainerFile *cf=0);
	void Draw(kGUICorners *c,double alpha);
private:
	double m_north,m_south,m_east,m_west;	//bounding box for all areas
	unsigned int m_numareas;
	ClassArray<KMLArea>m_areas;
};

/* KMZ Map */
class KMZGPXMapOverlay : public GPXMapOverlay
{
public:
	KMZGPXMapOverlay();
	~KMZGPXMapOverlay();
	void SetFilename(const char *fn);
	void Draw(kGUICorners *c,double alpha);
private:
	ZipFile m_zf;
	KMLGPXMapOverlay m_kml;
};


#endif
