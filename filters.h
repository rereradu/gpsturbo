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

#ifndef __FILTERS__
#define __FILTERS__

class FiltersPage
{
public:
	FiltersPage();
	~FiltersPage();
	void InitControls(kGUIControlBoxObj *obj);
	kGUITableObj *InitTable(kGUIContainerObj *obj,int y,int h);
	void Init(kGUIContainerObj *obj);
	void InitDefaultFilters(void);					/* called if no prefs file exists */
	void Purge(void);
	void LoadPrefs(kGUIXMLItem *root);
	void SavePrefs(kGUIXMLItem *root);
	void UpdateDBList(void);
	void ReFilter(void);
	void ReFilterEvent(kGUIEvent *event) {if(event->GetEvent()==EVENT_AFTERUPDATE)ReFilter();}
	void ReFilter(GPXRow *row);

	unsigned int GetNumFilters(void) {return m_numfilters;}
	int GetFilter(const char *name);
	bool FilterRow(int index,GPXRow *row);
	const char *GetFilterName(int index);

	/* this is called by the track code when tracknames have changed so that the */
	/* combo boxes can be updated for isinside command */
	/* oldname and newname are only passed if a track has been renamed, else they are null */
	void UpdateTrackNames(const char *oldname,const char *newname);
	/* this is called by the track code when a track is saved, it is to trigger a refilter */
	/* if the track is used by the current filter, if it isn't then it just returns */
	void TrackSaved(const char *trackname);

	/* used by the basic code */
	int Filter(const char *filtername,Array<GPXRow *>*list);
private:
	class GPXFilter *LocateFilterz(const char *name);
	void ClickSave(kGUIEvent *event);
	void ClickSave2(kGUIString *result,int closebutton);
	void ClickSave3(int  result);
	void ClickUndo(kGUIEvent *event);
	void ClickUndo2(int result);
	void ClickDelete(kGUIEvent *event);
	void ClickDelete2(int result);
	void ClickRename(kGUIEvent *event);
	void Rename(kGUIString *result,int closebutton);
	void ClickCopy(kGUIEvent *event);
	void Copy(kGUIString *result,int closebutton);
	void Save(kGUIString *name);
	void UpdateFilterList(void);

	void Changed(void);
	void ChangedEvent(kGUIEvent *event) {if(event->GetEvent()==EVENT_AFTERUPDATE)Changed();}
	void EditFilterChanged(void);
	void EditFilterChangedEvent(kGUIEvent *event) {if(event->GetEvent()==EVENT_AFTERUPDATE)EditFilterChanged();}
	void TableEvent(kGUIEvent *event);
	void MainTableEvent(kGUIEvent *event);

	void NewEntry(void);

	/* gui items */
	kGUIControlBoxObj m_editcontrols;
	kGUIComboBoxObj m_editfilterlist;
	kGUIButtonObj m_save;
	kGUIButtonObj m_undo;
	kGUIButtonObj m_delete;
	kGUIButtonObj m_rename;
	kGUIButtonObj m_copy;
	kGUITableObj m_table;

	unsigned int m_numfilters;
	Array<class GPXFilter *>m_filters;

	kGUITableObj m_filteredwptable;

	/* current filter */
	kGUITextObj m_filterlistcaption;
	kGUIComboBoxObj m_filterlist;		/* list of filters */

	/* current database */
	kGUITextObj m_currentdbcaption;
	kGUIComboBoxObj m_currentdb;

	kGUITextObj m_revfiltercaption;
	kGUITickBoxObj m_revfilter;

	kGUITextObj m_quickfiltercaption;
	kGUIInputBoxObj m_quickfilter;

	kGUITextObj m_resultsfiltercaption;
	kGUIInputBoxObj m_resultsfilter;

	kGUIString m_newname;

	/* static callbacks */
	CALLBACKGLUEPTR(FiltersPage,ClickSave,kGUIEvent)
	CALLBACKGLUEPTRVAL(FiltersPage,ClickSave2,kGUIString,int)
	CALLBACKGLUEVAL(FiltersPage,ClickSave3,int)
	CALLBACKGLUEPTR(FiltersPage,ClickUndo,kGUIEvent)
	CALLBACKGLUEVAL(FiltersPage,ClickUndo2,int)
	CALLBACKGLUEPTR(FiltersPage,ClickRename,kGUIEvent)
	CALLBACKGLUEPTRVAL(FiltersPage,Rename,kGUIString,int)
	CALLBACKGLUEPTR(FiltersPage,ClickCopy,kGUIEvent)
	CALLBACKGLUEPTRVAL(FiltersPage,Copy,kGUIString,int)
	CALLBACKGLUEPTR(FiltersPage,ClickDelete,kGUIEvent)
	CALLBACKGLUEVAL(FiltersPage,ClickDelete2,int)
	CALLBACKGLUEPTR(FiltersPage,EditFilterChangedEvent,kGUIEvent)
	CALLBACKGLUEPTR(FiltersPage,ReFilterEvent,kGUIEvent)
	CALLBACKGLUEPTR(FiltersPage,ChangedEvent,kGUIEvent)
	CALLBACKGLUEPTR(FiltersPage,TableEvent,kGUIEvent)
	CALLBACKGLUEPTR(FiltersPage,MainTableEvent,kGUIEvent)
};

#endif
