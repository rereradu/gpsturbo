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

#ifndef __ROUTES__
#define __ROUTES__

class RoutesPage
{
public:
	RoutesPage();
	~RoutesPage();
	void Init(kGUIContainerObj *obj);
	void Purge(void);
	void LoadPrefs(kGUIXMLItem *root);
	void SavePrefs(kGUIXMLItem *root);
	const char *GetCurrentName(void) {return m_editroutelist.GetSelectionString();}
	GPXRow *InTable(const char *name);
	void Remove(GPXRow *obj);
	kGUITableObj *GetTable(void) {return &m_routewptable;}
	void AddRow(GPXRow *row);
	void Remove(const char *name);
	void Changed(void);
	void CopyColors(void);
	void DrawMap(kGUICorners *c);
	bool InAnyRoute(GPXRow *row);
private:
	/* route controls */
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
	void ClickWpntUp(kGUIEvent *event);
	void ClickWpntDown(kGUIEvent *event);
	void ClickAutoOrder(kGUIEvent *event);

	void TableEvent(kGUIEvent *event);
	void ChangedEvent(kGUIEvent *event) {if(event->GetEvent()==EVENT_AFTERUPDATE)Changed();}
	void LoadEvent(kGUIEvent *event) {if(event->GetEvent()==EVENT_AFTERUPDATE)Load();}

	void Load(void);
	int GetIndex(const char *name);
	void UpdateRouteList(void);
	double UpdateInfo(void);

	unsigned int m_numroutes;
	Array<class GPXRoute *>m_routes;

	/* gui objects */
	kGUIControlBoxObj m_editcontrols;
	kGUIButtonObj m_pntup;
	kGUIButtonObj m_pntdown;
	kGUIButtonObj m_autoorder;
	kGUIComboBoxObj m_editroutelist;
	kGUIButtonObj m_save;
	kGUIButtonObj m_undo;
	kGUIButtonObj m_delete;
	kGUIButtonObj m_rename;
	kGUIButtonObj m_copy;
	kGUITableObj m_routewptable;

	kGUITextObj m_colorcaption;
	kGUIComboBoxObj m_color;
	kGUITextObj m_drawcaption;
	kGUITickBoxObj m_draw;

	kGUITextObj m_numcaption;
	kGUIInputBoxObj m_num;
	kGUITextObj m_distcaption;
	kGUIInputBoxObj m_dist;

	/* static callbacks */
	CALLBACKGLUEPTR(RoutesPage,ClickSave,kGUIEvent)
	CALLBACKGLUEPTR(RoutesPage,ClickUndo,kGUIEvent)
	CALLBACKGLUEVAL(RoutesPage,ClickUndo2,int)
	CALLBACKGLUEPTR(RoutesPage,ClickRename,kGUIEvent)
	CALLBACKGLUEPTRVAL(RoutesPage,ClickRename2,kGUIString,int)
	CALLBACKGLUEPTR(RoutesPage,ClickCopy,kGUIEvent)
	CALLBACKGLUEPTRVAL(RoutesPage,ClickCopy2,kGUIString,int)
	CALLBACKGLUEPTR(RoutesPage,ClickDelete,kGUIEvent)
	CALLBACKGLUEVAL(RoutesPage,ClickDelete2,int)
	CALLBACKGLUEPTRVAL(RoutesPage,ClickSave2,kGUIString,int)
	CALLBACKGLUEPTR(RoutesPage,ClickWpntUp,kGUIEvent)
	CALLBACKGLUEPTR(RoutesPage,ClickWpntDown,kGUIEvent)
	CALLBACKGLUEPTR(RoutesPage,ClickAutoOrder,kGUIEvent)
	CALLBACKGLUEPTR(RoutesPage,LoadEvent,kGUIEvent)
	CALLBACKGLUEPTR(RoutesPage,ChangedEvent,kGUIEvent)
	CALLBACKGLUEPTR(RoutesPage,TableEvent,kGUIEvent)
};

#endif
