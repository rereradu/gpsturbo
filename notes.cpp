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

#include "gpsturbo.h"

class GPXNoteRow : public kGUITableRowObj
{
public:
	GPXNoteRow();
	void Load(class kGUIXMLItem *mpr);	/* load from prefs file */
	void Save(class kGUIXMLItem *mpr);	/* save to prefs file */
	int GetNumObjects(void) {return 2;}
	kGUIObj **GetObjectList(void) {return m_objectlist;} 
	kGUIObj *m_objectlist[2];
private:
	kGUIInputBoxObj m_note;
	kGUIInputBoxObj m_value;
};

NotesPage::NotesPage()
{
}

NotesPage::~NotesPage()
{
}

void NotesPage::Init(kGUIContainerObj *obj)
{
	int bh;
	int bw;
	kGUIText *t;

	bw=obj->GetChildZoneW();
	bh=obj->GetChildZoneH();
	m_table.SetPos(0,0);
	m_table.SetSize(bw,bh);
	m_table.SetNumCols(2);
	m_table.SetAllowAddNewRow(true);
	m_table.SetEventHandler(this,CALLBACKNAME(NewEntry));

	t=m_table.GetColHeaderTextPtr(0);
	t->SetFontID(1);	/* bold */
	t=m_table.GetColHeaderTextPtr(1);
	t->SetFontID(1);	/* bold */
	m_table.SetColTitle(0,"Note");
	m_table.SetColWidth(0,bw>>1);
	m_table.SetColTitle(1,"Value");
	m_table.SetColWidth(1,bw>>1);
	obj->AddObject(&m_table);
}

void NotesPage::Purge(void)
{
	m_table.DeleteChildren();

}

void NotesPage::LoadPrefs(kGUIXMLItem *root)
{
	unsigned int i;
	kGUIXMLItem *item;

	item=root->Locate("ntable");
	if(item)
		m_table.LoadConfig(item);

	for(i=0;i<root->GetNumChildren();++i)
	{
		item=root->GetChild(i);
		if(!strcmp(item->GetName(),"notes"))
		{
			GPXNoteRow *row;
			row=new GPXNoteRow();
			row->Load(item);
			m_table.AddRow(row);
		}
	}

}

void NotesPage::SavePrefs(kGUIXMLItem *root)
{
	unsigned int i;

	m_table.SaveConfig(root,"ntable");

	for(i=0;i<m_table.GetNumChildren();++i)
	{
		GPXNoteRow *row;

		row=static_cast<GPXNoteRow *>(m_table.GetChild(i));
		row->Save(root->AddChild("notes"));
	}
}

void NotesPage::NewEntry(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_ADDROW)
	{
		GPXNoteRow *nr=new GPXNoteRow();
		m_table.AddRow(nr);
	}
}

GPXNoteRow::GPXNoteRow()
{
	m_objectlist[0]=&m_note;
	m_objectlist[1]=&m_value;
	SetRowHeight(20);
}

/* load from prefs file */
void GPXNoteRow::Load(class kGUIXMLItem *mpr)
{
	SetRowHeight(mpr->Locate("height")->GetValueInt());
	m_note.SetString(mpr->Locate("note")->GetValue());
	m_value.SetString(mpr->Locate("value")->GetValue());
}

/* save to prefs file */
void GPXNoteRow::Save(class kGUIXMLItem *mpr)
{
	mpr->AddParm("note",m_note.GetString());
	mpr->AddParm("value",m_value.GetString());
	mpr->AddParm("height",(int)(GetRowHeight()));
}
