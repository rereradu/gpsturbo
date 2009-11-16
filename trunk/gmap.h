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

#ifndef __GMAP__
#define __GMAP__

#include "map.h"

#define MINGZOOM 3
#define MAXGZOOM 19

/* google map tile download */
class GGPXMapTile
{
public:
	GGPXMapTile() {m_waiting=false;m_serverid=-1;}
	void SetOwner(class GGPXMap *owner) {m_owner=owner;}
	void LoadTile(kGUIString *url,kGUIString *fn,int x,int y,int serverid);
	kGUIString *GetURL(void) {return m_dle.GetURL();}
	kGUIString *GetFN(void) {return m_dh.GetFilename();}
	bool GetAsyncActive(void) {return m_dle.GetAsyncActive();}
	void WaitFinished(void) {m_dle.WaitFinished();}
	int GetServerID(void) {return m_serverid;}
	void Abort(void) {m_dle.Abort();}

	CALLBACKGLUEVAL(GGPXMapTile,TileLoaded,int)
private:
	void TileLoaded(int result);
	class GGPXMap *m_owner;
	kGUIDownloadEntry m_dle;
	DataHandle m_dh;
	bool m_waiting;
	int m_x;
	int m_y;
	int m_serverid;
};

#define MAXDLS 8

/* google map */
class GGPXMap : public GPXMap
{
public:
	GGPXMap(int type);
	~GGPXMap();

	/* these are the static classes used to handle the versioning of the tiles */
	static void InitVersions(void);
	static void CheckVersions(void);
	static void CheckVersionsThread(void *unused);
	static void LoadVersions(kGUIXMLItem *root);
	static void SaveVersions(kGUIXMLItem *root);
	static void GenerateURL(unsigned int type,kGUIString *url,kGUIString *hstr,int *serverid,int tx,int ty,int zoom);

	int DrawTile(int tx,int ty);	/* draw the tile to the current display */
	void ResetOnline(void);
	void ToMap(class GPXCoord *c,int *sx,int *sy);
	void FromMap(int sx,int sy,class GPXCoord *c);
	int GetNumCopyrightLines(void) {return 1;}
	const char *GetCopyrightLine(int l) {return m_copyright.GetString();}
	const char *GetStatusLine(void) {UpdateStatus();return m_status.GetString();}
	void UpdateStatus(void);

	bool LoadTile(kGUIString *url,kGUIString *fn,int x,int y,int serverid);
	void AddBadURL(kGUIString *url) {m_badurls.Add(url->GetString(),0);}

	bool GetSearchable(void) {return true;}
	void SearchMap(kGUIString *text) {}
	void BrowseMap(void) {}

	/* leave these public since they are displayed in the credit screen */
	static kGUIString m_mapver;
	static kGUIString m_satmapver;
	static kGUIString m_overlayver;
	static kGUIString m_terver;
private:
	int m_type;
	double m_pixelsPerLonDegree[MAXGZOOM];
	double m_negpixelsPerLonRadian[MAXGZOOM];
	double m_bitmapOrigo[MAXGZOOM];
	int m_numTiles[MAXGZOOM];
	kGUIString m_copyright;
	kGUIString m_status;
	GGPXMapTile m_dles[MAXDLS];
	Hash m_badurls;

	//current Version numbers for google maps
	static kGUIThread m_checkthread;
};
#endif
