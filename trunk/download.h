#ifndef __DOWNLOAD__
#define __DOWNLOAD__

/* this is the page for downloading maps etc from online sources */
/* it keeps track of downloaded places and will check for newer items */

class DownloadPage
{
public:
	DownloadPage();
	~DownloadPage();
	void Init(kGUIContainerObj *obj);
	void Purge(void);
	void LoadPrefs(kGUIXMLItem *root);
	void SavePrefs(kGUIXMLItem *root);
private:
	CALLBACKGLUEPTR(DownloadPage,NewEntry,kGUIEvent)
	void NewEntry(kGUIEvent *event);
	kGUITableObj m_table;
};

#endif
