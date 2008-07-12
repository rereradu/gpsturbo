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

#ifndef __LINES__
#define __LINES__

class LinesPage
{
public:
	LinesPage();
	~LinesPage();
	void Init(kGUIContainerObj *obj);
	void Resize(int changey);
	void Purge(void);
	void LoadPrefs(kGUIXMLItem *root);
	void SavePrefs(kGUIXMLItem *root);
	void DrawMap(kGUICorners *c);
	void AddPointToLine(const char *name);
	void AddPointToLine(double lat,double lon);
	const char *GetCurrentName(void) {return m_editlinelist.GetSelectionString();}

private:
	int GetIndex(const char *name);

	void Draw(kGUICorners *c,class GPXLine *line);
	void TableEvent(kGUIEvent *event);
	void NewEntryMenuDone(kGUIEvent *event);
	void UpdateList(void);
	void ClickUp(kGUIEvent *event);
	void ClickDown(kGUIEvent *event);
	void ClickSave(kGUIEvent *event);
	void ClickSave2(kGUIString *result,int closebutton);
	void ClickUndo(kGUIEvent *event);
	void ClickUndo2(int result);
	void ClickDelete(kGUIEvent *event);
	void ClickDelete2(int result);
	void ClickRename(kGUIEvent *event);
	void ClickRename2(kGUIString *result,int closebutton);
	void ClickCopy(kGUIEvent *event);
	void ClickCopy2(kGUIString *result,int closebutton);
	void Save(kGUIString *name);
	void ProcessInit(void);
	void ProcessLine(kGUICorners *c,GPXBounds *bounds,int type,kGUIString *v1,kGUIString *v2);
	void UpdateBounds(GPXLine *line,GPXBounds *bounds);
	void Changed(void);
	void Load(void);
	void ChangedEvent(kGUIEvent *event) {if(event->GetEvent()==EVENT_AFTERUPDATE)Changed();}
	void LoadEvent(kGUIEvent *event) {if(event->GetEvent()==EVENT_AFTERUPDATE)Load();}

	/* lines structures */
	unsigned int m_numlines;
	Array<class GPXLine *>m_lines;

	/* gui controls */
	kGUIControlBoxObj m_editlinecontrols;
	kGUIButtonObj m_lineup;
	kGUIButtonObj m_linedown;
	kGUIComboBoxObj m_editlinelist;
	kGUIButtonObj m_saveline;
	kGUIButtonObj m_undoline;
	kGUIButtonObj m_deleteline;
	kGUIButtonObj m_renameline;
	kGUIButtonObj m_copyline;

	kGUIMenuColObj m_addmenu;

	kGUITableObj m_linetable;
	GPXBounds m_curbounds;

	kGUITextObj m_drawlinecaption;
	kGUITickBoxObj m_drawline;

	kGUITextObj m_settingscaption;
	kGUIInputBoxObj m_settings;

	/* settings used for traversing a lines list */
	GPXCoord m_pos1;
	GPXCoord m_pos2;
	kGUIColor m_col;
	double m_heading;
	int m_disttype;
	double m_distance;
	int m_radiustype;
	double m_radius;
	int m_thickness;
	double m_degrees;	/* only used for arcs */

	/* static callbacks */
	CALLBACKGLUEPTR(LinesPage,ChangedEvent,kGUIEvent)
	CALLBACKGLUEPTR(LinesPage,LoadEvent,kGUIEvent)
	CALLBACKGLUEPTR(LinesPage,ClickUp,kGUIEvent)
	CALLBACKGLUEPTR(LinesPage,ClickDown,kGUIEvent)
	CALLBACKGLUEPTR(LinesPage,ClickSave,kGUIEvent)
	CALLBACKGLUEPTR(LinesPage,ClickUndo,kGUIEvent)
	CALLBACKGLUEVAL(LinesPage,ClickUndo2,int)
	CALLBACKGLUEPTR(LinesPage,ClickRename,kGUIEvent)
	CALLBACKGLUEPTRVAL(LinesPage,ClickRename2,kGUIString,int)
	CALLBACKGLUEPTR(LinesPage,ClickCopy,kGUIEvent)
	CALLBACKGLUEPTRVAL(LinesPage,ClickCopy2,kGUIString,int)
	CALLBACKGLUEPTR(LinesPage,ClickDelete,kGUIEvent)
	CALLBACKGLUEVAL(LinesPage,ClickDelete2,int)
	CALLBACKGLUEPTRVAL(LinesPage,ClickSave2,kGUIString,int)
	CALLBACKGLUEPTR(LinesPage,NewEntryMenuDone,kGUIEvent)
	CALLBACKGLUEPTR(LinesPage,TableEvent,kGUIEvent)
};

#endif
