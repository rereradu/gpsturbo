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

#ifndef __TRACKS__
#define __TRACKS__

class TracksPage
{
public:
	TracksPage();
	~TracksPage();
	void Init(kGUIContainerObj *obj);
	void Purge(void);
	void LoadPrefs(kGUIXMLItem *root,Hash *hash=0);
	int SavePrefs(kGUIXMLItem *root,Hash *hash=0,GPXBounds *bounds=0);
	unsigned int GetNumTracks(void) {return m_numtracks;}
	const char *GetTrackName(int index);
	unsigned int GetNumTableEntries(void) {return m_table.GetNumChildren();}
	class GPXTrack *GetIndex(unsigned int index) {return m_tracks.GetEntry(index);}
	void DrawMap(kGUICorners *c);
	void AddPointToTrack(double lat,double lon);
	const char *GetCurrentName(void) {return m_edittracklist.GetSelectionString();}
	void PostDraw(int cx,int cy);
	void GotoTrackPosition(double lat,double lon);
	void LoadTracks(int pressed);
	void Verify(const char *filename,Hash *hash,kGUIString *results);
	bool IsPointInside(const char *trackname,double lat,double lon);

	//used by the poly draw code for transformed points
	static Array<kGUIPoint2> m_polypoints; 
private:
	void UpdateTracksList(void);
	void Load(void);
	void Changed(void);
	void TableEvent(kGUIEvent *event);
	void ChangedEvent(kGUIEvent *event) {if(event->GetEvent()==EVENT_AFTERUPDATE)Changed();}
	void LoadEvent(kGUIEvent *event) {if(event->GetEvent()==EVENT_AFTERUPDATE)Load();}
	void ClickUndo(kGUIEvent *event);
	void ClickUndo2(int result);
	void ClickDelete(kGUIEvent *event);
	void ClickDelete2(int result);
	void ClickRename(kGUIEvent *event);
	void ClickRename2(kGUIString *result,int closebutton);
	void ClickCopy(kGUIEvent *event);
	void ClickCopy2(kGUIString *result,int closebutton);
	void ClickSave(kGUIEvent *event);
	void ClickSave2(kGUIString *result,int closebutton);
	void ClickSimplify(kGUIEvent *event);
	void ClickSimplify2(kGUIString *result,int closebutton);
	void Save(const char *name);
	int GetIndex(const char *name);
	void UpdateTrackInfo(void);

	kGUIControlBoxObj m_editcontrols;
	kGUIComboBoxObj m_edittracklist;
	kGUIButtonObj m_save;
	kGUIButtonObj m_undo;
	kGUIButtonObj m_delete;
	kGUIButtonObj m_rename;
	kGUIButtonObj m_copy;
	kGUIButtonObj m_simplify;

	kGUITextObj m_drawcaption;
	kGUIComboBoxObj m_draw;
	kGUITextObj m_colorcaption;
	kGUIComboBoxObj m_color;
	kGUITextObj m_alphacaption;
	kGUIComboBoxObj m_alpha;

	kGUITextObj m_numcaption;
	kGUIInputBoxObj m_num;
	kGUITextObj m_distcaption;
	kGUIInputBoxObj m_dist;
	kGUITextObj m_timecaption;
	kGUIInputBoxObj m_time;

	kGUITableObj m_table;
	GPXBounds m_curbounds;		/* bounds for currently edited track */

	unsigned int m_numtracks;
	Array<class GPXTrack *>m_tracks;

	/* static callbacks */
	CALLBACKGLUEPTR(TracksPage,TableEvent,kGUIEvent)
	CALLBACKGLUEPTR(TracksPage,ChangedEvent,kGUIEvent)
	CALLBACKGLUEPTR(TracksPage,LoadEvent,kGUIEvent)
	CALLBACKGLUEPTR(TracksPage,ClickUndo,kGUIEvent)
	CALLBACKGLUEVAL(TracksPage,ClickUndo2,int)
	CALLBACKGLUEPTR(TracksPage,ClickRename,kGUIEvent)
	CALLBACKGLUEPTRVAL(TracksPage,ClickRename2,kGUIString,int)
	CALLBACKGLUEPTR(TracksPage,ClickCopy,kGUIEvent)
	CALLBACKGLUEPTRVAL(TracksPage,ClickCopy2,kGUIString,int)
	CALLBACKGLUEPTR(TracksPage,ClickDelete,kGUIEvent)
	CALLBACKGLUEVAL(TracksPage,ClickDelete2,int)
	CALLBACKGLUEPTR(TracksPage,ClickSave,kGUIEvent)
	CALLBACKGLUEPTRVAL(TracksPage,ClickSave2,kGUIString,int)
	CALLBACKGLUEPTR(TracksPage,ClickSimplify,kGUIEvent)
	CALLBACKGLUEPTRVAL(TracksPage,ClickSimplify2,kGUIString,int)
	CALLBACKGLUEVAL(TracksPage,LoadTracks,int)
};

#endif
