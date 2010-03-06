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

#ifndef __GPSRS__
#define __GPSRS__

class GPSrPage
{
public:
	GPSrPage();
	~GPSrPage();
	void Init(kGUIContainerObj *obj);
	void Purge(void);
	void LoadPrefs(kGUIXMLItem *root);
	void SavePrefs(kGUIXMLItem *root);
	void SetInput(int index,class BabelGlue *babel);
	void UpdateGPSList(void);

	/* these are called by Basic */
	int UploadToGPSr(const char *gpsname,unsigned int num,Array<GPXRow *>*list);
	int UploadToGPSr(const char *gpsname,const char *filename);
private:
	void TableEvent(kGUIEvent *event);
	void InitGPSRs(void);
	void ClickUploadWptsToGPSr(kGUIEvent *event);
	void ClickDownloadWptsFromGPSr(kGUIEvent *event);
	void ClickUploadTracksToGPSr(kGUIEvent *event);
	void ClickUploadTracksToGPSr2(int pressed);
	void ClickDownloadTracksFromGPSr(kGUIEvent *event);
	void ClickDownloadFindsFromGPSr(kGUIEvent *event);

	bool DownloadWptsFromGPSr(void);
	bool DownloadTracksFromGPSr(void);
	void DownloadWP(int mode);
	bool UploadToGPS(int what,class GPXGPSRow *gpsrow,const char *filename,const char *title);
	void GenerateWptName(int format,unsigned int maxlen,kGUIString *name,Hash *names);
	void LoadWaypoints(void);

	/* gui items */
	kGUIControlBoxObj m_gpscontrols;
	kGUIComboBoxObj m_currentgps;
	kGUIButtonObj m_uploadwptstogps;
	kGUIButtonObj m_downloadwptsfromgps;
	kGUIButtonObj m_uploadtrackstogps;
	kGUIButtonObj m_downloadtracksfromgps;
	kGUIButtonObj m_downloadfindsfromgps;
	kGUITableObj m_gpstable;

	/* static callbacks */
	CALLBACKGLUEPTR(GPSrPage,TableEvent,kGUIEvent)
	CALLBACKGLUEPTR(GPSrPage,ClickUploadWptsToGPSr,kGUIEvent)
	CALLBACKGLUEPTR(GPSrPage,ClickDownloadWptsFromGPSr,kGUIEvent)
	CALLBACKGLUEPTR(GPSrPage,ClickUploadTracksToGPSr,kGUIEvent)
	CALLBACKGLUEVAL(GPSrPage,ClickUploadTracksToGPSr2,int)
	CALLBACKGLUEPTR(GPSrPage,ClickDownloadTracksFromGPSr,kGUIEvent)
	CALLBACKGLUE(GPSrPage,LoadWaypoints)
	CALLBACKGLUEPTR(GPSrPage,ClickDownloadFindsFromGPSr,kGUIEvent)
};

#endif
