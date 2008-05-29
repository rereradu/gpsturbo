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

#ifndef __STICKERS__
#define __STICKERS__

class StickersPage
{
	friend class PrintStickers;
public:
	StickersPage();
	~StickersPage();
	void Init(kGUIContainerObj *obj);
	void Purge(void);
	void LoadPrefs(kGUIXMLItem *root);
	void SavePrefs(kGUIXMLItem *root);
private:
	/* stickers controls */
	void Changed(void);
	void UpdateStickersList(void);
	void ClickSaveSticker(kGUIEvent *event);
	void ClickUndoStickers(kGUIEvent *event);
	void ClickUndoStickers2(int result);
	void ClickDeleteStickers(kGUIEvent *event);
	void ClickDeleteStickers2(int result);
	void ClickRenameSticker(kGUIEvent *event);
	void ClickRenameSticker2(kGUIString *result,int closebutton);
	void ClickCopySticker(kGUIEvent *event);
	void ClickCopySticker2(kGUIString *result,int closebutton);
	void ClickPrintStickers(kGUIEvent *event);
	void ClickSaveSticker2(kGUIString *result,int closebutton);
	void SaveSticker(kGUIString *name);
	void PreviewSticker(void);
	int GetSticker(const char *name);
	void EditStickersChanged(void);
	void EditStickersChangedEvent(kGUIEvent *event) {if(event->GetEvent()==EVENT_AFTERUPDATE)EditStickersChanged();}
	void ChangedEvent(kGUIEvent *event) {if(event->GetEvent()==EVENT_AFTERUPDATE)Changed();}
	void TableEvent(kGUIEvent *event);

	/* gui items */
	kGUIControlBoxObj m_editstickercontrols;
	kGUIComboBoxObj m_editstickerlist;
	kGUIButtonObj m_savesticker;
	kGUIButtonObj m_undosticker;
	kGUIButtonObj m_deletesticker;
	kGUIButtonObj m_renamesticker;
	kGUIButtonObj m_copysticker;
	kGUIButtonObj m_doprintsticker;
	kGUIImageObj m_previewsticker;
	kGUIDrawSurface m_previewstickersurface;

	kGUITableObj m_stickertable;

	unsigned int m_numstickers;
	Array<class GPXSticker *>m_stickers;


	kGUIString m_printsticker;	/* name of printer to use for printing stickers */

	/* static callbacks */
	CALLBACKGLUEPTR(StickersPage,TableEvent,kGUIEvent)
	CALLBACKGLUEPTR(StickersPage,ChangedEvent,kGUIEvent)
	CALLBACKGLUEPTR(StickersPage,EditStickersChangedEvent,kGUIEvent)
	CALLBACKGLUEPTR(StickersPage,ClickSaveSticker,kGUIEvent)
	CALLBACKGLUEPTR(StickersPage,ClickUndoStickers,kGUIEvent)
	CALLBACKGLUEVAL(StickersPage,ClickUndoStickers2,int)
	CALLBACKGLUEPTR(StickersPage,ClickRenameSticker,kGUIEvent)
	CALLBACKGLUEPTRVAL(StickersPage,ClickRenameSticker2,kGUIString,int)
	CALLBACKGLUEPTR(StickersPage,ClickCopySticker,kGUIEvent)
	CALLBACKGLUEPTRVAL(StickersPage,ClickCopySticker2,kGUIString,int)
	CALLBACKGLUEPTR(StickersPage,ClickPrintStickers,kGUIEvent)
	CALLBACKGLUEPTR(StickersPage,ClickDeleteStickers,kGUIEvent)
	CALLBACKGLUEVAL(StickersPage,ClickDeleteStickers2,int)
	CALLBACKGLUEPTRVAL(StickersPage,ClickSaveSticker2,kGUIString,int)
};

#endif
