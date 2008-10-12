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
#include "kguireport.h"

enum
{
STICKER_TEXT,
STICKER_LINE,
STICKER_PICTURE
};

class GPXStickerEntry
{
public:
	kGUIString m_type;
	kGUIString m_color;
	kGUIString m_text;	/* text or picture filename */
	kGUIImage m_image;
	int m_x;
	int m_y;
	int m_w;
	int m_h;
	double m_size;
	void Load(kGUIXMLItem *xml);
	void Save(kGUIXMLItem *xml);
};

class GPXSticker
{
public:
	GPXSticker(kGUIString *name) {m_name.SetString(name);m_numentries=0;m_entries.SetGrow(true);m_entries.Alloc(16);}
	~GPXSticker();
	void SetName(kGUIString *name) {m_name.SetString(name);}
	const char *GetName(void) {return m_name.GetString();}
	void Load(kGUITableObj *table);
	bool Compare(kGUITableObj *table);
	void Save(kGUITableObj *table);
	void Load(kGUIXMLItem *xml);
	void Save(kGUIXMLItem *xml);
	bool StickerRow(GPXRow *trow);
	GPXStickerEntry *AddEntry(void) {GPXStickerEntry *fe=new GPXStickerEntry();m_entries.SetEntry(m_numentries++,fe);return fe;}
private:
	kGUIString m_name;
	unsigned int m_numentries;
	Array<GPXStickerEntry *>m_entries;
};

enum
{
STICKERCOL_TYPE,
STICKERCOL_COLOR,
STICKERCOL_X,
STICKERCOL_Y,
STICKERCOL_W,
STICKERCOL_H,
STICKERCOL_SIZE,
STICKERCOL_TEXT,
STICKERCOL_NUMCOLUMNS};

enum
{
STICKERPRIM_TEXT,
STICKERPRIM_IMAGE,
STICKERPRIM_RECT,
STICKERPRIM_NUM};

const char *stprimnames[STICKERPRIM_NUM]={
	"Text","Image","Rectangle"};

const char *stcolnames[STICKERCOL_NUMCOLUMNS]={
	"Type","Color","X","Y","W","H","Size","Text/Filename"};

const char *stcoldesc[STICKERCOL_NUMCOLUMNS]={
	"Type","Color","X Position","Y Position","Width","Height","Font Size","Text / Filename"};
	
const int stcolwidths[STICKERCOL_NUMCOLUMNS]={90,225,50,50,50,50,50,450};

class GPXStickerRow : public kGUITableRowObj
{
public:
	GPXStickerRow();
	int GetNumObjects(void) {return STICKERCOL_NUMCOLUMNS;}
	kGUIObj **GetObjectList(void) {return m_objectlist;} 
	kGUIObj *m_objectlist[STICKERCOL_NUMCOLUMNS];
	CALLBACKGLUEPTRVAL(GPXStickerRow,GotFilename,kGUIFileReq,int);
	void GotFilename(kGUIFileReq *fr,int pressed);
	//private:
	kGUIComboBoxObj m_type;
	kGUIComboBoxObj m_color;
	kGUIInputBoxObj m_x;
	kGUIInputBoxObj m_y;
	kGUIInputBoxObj m_w;
	kGUIInputBoxObj m_h;
	kGUIInputBoxObj m_size;
	kGUIInputBoxObj m_text;

	/* only used if this is an image */
	kGUIImage m_image;
private:
	CALLBACKGLUEPTR(GPXStickerRow,DoubleClick,kGUIEvent);
	void DoubleClick(kGUIEvent *event);
};

StickersPage::StickersPage()
{
	m_numstickers=0;
	m_stickers.Init(4,2);
}

StickersPage::~StickersPage()
{
}

void StickersPage::LoadPrefs(kGUIXMLItem *root)
{
	unsigned int i;
	kGUIXMLItem *item;

	item=root->Locate("stickertable");
	if(item)
		m_stickertable.LoadConfig(item);
	item=root->Locate("printsticker");
	if(item)
		m_printsticker.SetString(item->GetValue());

	for(i=0;i<root->GetNumChildren();++i)
	{
		item=root->GetChild(i);
		if(!strcmp(item->GetName(),"sticker"))
		{
			GPXSticker *s=new GPXSticker(item->Locate("name")->GetValue());
			m_stickers.SetEntry(m_numstickers++,s);
			s->Load(item);
		}
	}
	UpdateStickersList();
}

void StickersPage::SavePrefs(kGUIXMLItem *root)
{
	unsigned int i;

	m_stickertable.SaveConfig(root,"stickertable");
	root->AddChild("printsticker",m_printsticker.GetString());

	for(i=0;i<m_numstickers;++i)
		m_stickers.GetEntry(i)->Save(root);
}

/* init sticker gui items */
void StickersPage::Init(kGUIContainerObj *obj)
{
	int i,y,bw,bh;
	kGUIText *t;

	bw=obj->GetChildZoneW();

	m_editstickercontrols.SetSize(bw,20);

	m_editstickerlist.SetFontSize(BUTTONFONTSIZE);
	m_editstickerlist.SetSize(300,20);
	m_editstickerlist.SetNumEntries(1);				/* list of Stickers */
	m_editstickerlist.SetEntry(0,gpx->GetString(STRING_NEWSTICKER),0);
	m_editstickerlist.SetEventHandler(this,CALLBACKNAME(EditStickersChangedEvent));
	m_editstickercontrols.AddObject(&m_editstickerlist);

	m_savesticker.SetFontSize(BUTTONFONTSIZE);
	m_savesticker.SetString(gpx->GetString(STRING_SAVESTICKER));
	m_savesticker.Contain();
	m_savesticker.SetEventHandler(this,CALLBACKNAME(ClickSaveSticker));
	m_editstickercontrols.AddObject(&m_savesticker);

	m_undosticker.SetFontSize(BUTTONFONTSIZE);
	m_undosticker.SetString(gpx->GetString(STRING_UNDOCHANGES));
	m_undosticker.Contain();
	m_undosticker.SetEventHandler(this,CALLBACKNAME(ClickUndoStickers));
	m_editstickercontrols.AddObject(&m_undosticker);

	m_deletesticker.SetFontSize(BUTTONFONTSIZE);
	m_deletesticker.SetString(gpx->GetString(STRING_DELETESTICKER));
	m_deletesticker.Contain();
	m_deletesticker.SetEventHandler(this,CALLBACKNAME(ClickDeleteStickers));
	m_editstickercontrols.AddObject(&m_deletesticker);

	m_renamesticker.SetFontSize(BUTTONFONTSIZE);
	m_renamesticker.SetString(gpx->GetString(STRING_RENAMESTICKER));
	m_renamesticker.Contain();
	m_renamesticker.SetEventHandler(this,CALLBACKNAME(ClickRenameSticker));
	m_editstickercontrols.AddObject(&m_renamesticker);

	m_copysticker.SetFontSize(BUTTONFONTSIZE);
	m_copysticker.SetString(gpx->GetString(STRING_COPYSTICKER));
	m_copysticker.Contain();
	m_copysticker.SetEventHandler(this,CALLBACKNAME(ClickCopySticker));
	m_editstickercontrols.AddObject(&m_copysticker);

	m_doprintsticker.SetFontSize(BUTTONFONTSIZE);
	m_doprintsticker.SetString(gpx->GetString(STRING_PRINTSTICKER));
	m_doprintsticker.Contain();
	m_doprintsticker.SetEventHandler(this,CALLBACKNAME(ClickPrintStickers));
	m_editstickercontrols.AddObject(&m_doprintsticker);

	m_deletesticker.SetEnabled(false);
	m_renamesticker.SetEnabled(false);
	m_copysticker.SetEnabled(false);

	obj->AddObject(&m_editstickercontrols);
	
	y=m_editstickercontrols.GetZoneH();

	/* preview image for sticker */
	m_previewstickersurface.Init(350,100);

	m_previewsticker.SetPos(10,y);
	m_previewsticker.SetSize(350,100);
	m_previewsticker.SetMemImage(0,m_previewstickersurface.GetFormat(),m_previewstickersurface.GetWidth(),m_previewstickersurface.GetHeight(),m_previewstickersurface.GetBPP(),(unsigned char *)m_previewstickersurface.GetSurfacePtrABS(0,0));
	y+=110;

	bh=(obj->GetChildZoneH()-y);
	m_stickertable.SetPos(0,y);
	m_stickertable.SetSize(bw,bh);

	m_stickertable.SetNumCols(STICKERCOL_NUMCOLUMNS);
	for(i=0;i<STICKERCOL_NUMCOLUMNS;++i)
	{
		t=m_stickertable.GetColHeaderTextPtr(i);
		t->SetFontID(1);	/* bold */

		m_stickertable.SetColTitle(i,stcolnames[i]);
		m_stickertable.SetColHint(i,stcoldesc[i]);
		m_stickertable.SetColWidth(i,stcolwidths[i]);
	}
	m_stickertable.SetAllowAddNewRow(true);
	m_stickertable.SetEventHandler(this,CALLBACKNAME(TableEvent));

	obj->AddObject(&m_editstickercontrols);
	obj->AddObject(&m_previewsticker);
	obj->AddObject(&m_stickertable);
	PreviewSticker();
}

void StickersPage::Resize(int changey)
{
	m_stickertable.SetZoneH(m_stickertable.GetZoneH()+changey);
}

void StickersPage::TableEvent(kGUIEvent *event)
{
	switch(event->GetEvent())
	{
	case EVENT_AFTERUPDATE:
		Changed();
	break;
	case EVENT_ADDROW:
	{
		GPXStickerRow *fr=new GPXStickerRow();
		m_stickertable.AddRow(fr);
	}
	break;
	}
}

void StickersPage::Purge(void)
{
	unsigned int i;

	m_stickertable.DeleteChildren();

	for(i=0;i<m_numstickers;++i)
        delete m_stickers.GetEntry(i);
}

GPXStickerRow::GPXStickerRow()
{
	int e;

	m_objectlist[STICKERCOL_TYPE]=&m_type;
	m_objectlist[STICKERCOL_COLOR]=&m_color;
	m_objectlist[STICKERCOL_X]=&m_x;
	m_objectlist[STICKERCOL_Y]=&m_y;
	m_objectlist[STICKERCOL_W]=&m_w;
	m_objectlist[STICKERCOL_H]=&m_h;
	m_objectlist[STICKERCOL_SIZE]=&m_size;
	m_objectlist[STICKERCOL_TEXT]=&m_text;

	m_type.SetSize(300,20);
	m_type.SetNumEntries(STICKERPRIM_NUM);
	for(e=0;e<STICKERPRIM_NUM;++e)
		m_type.SetEntry(e,stprimnames[e],e);

	gpx->InitColorCombo(&m_color);

	m_x.SetInt(0);
	m_y.SetInt(0);
	m_w.SetInt(0);
	m_h.SetInt(0);
	m_size.SetInt(1);
	
	m_text.SetEventHandler(this,CALLBACKNAME(DoubleClick));
}

void GPXStickerRow::DoubleClick(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_LEFTDOUBLECLICK)
	{
		if(m_type.GetSelection()==STICKERPRIM_IMAGE)
		{
			kGUIFileReq *fr;

			fr=new kGUIFileReq(FILEREQ_OPEN,"",".jpg;.gif;.bmp;.png",this,CALLBACKNAME(GotFilename));
		}
	}
}

void GPXStickerRow::GotFilename(kGUIFileReq *fr,int pressed)
{
	if(pressed==MSGBOX_OK)
	{
		m_text.SetString(fr->GetFilename());

		m_image.SetFilename(fr->GetFilename());
		//trigger table callback to redraw
		m_text.CallAfterUpdate();
	}
}

/* load from prefs file */
void GPXStickerEntry::Load(class kGUIXMLItem *wpr)
{
	m_type.SetString(wpr->Locate("type")->GetValue());
	m_color.SetString(wpr->Locate("colour")->GetValue());
	m_text.SetString(wpr->Locate("text")->GetValue());
	m_x=wpr->Locate("x")->GetValueInt();
	m_y=wpr->Locate("y")->GetValueInt();
	m_w=wpr->Locate("w")->GetValueInt();
	m_h=wpr->Locate("h")->GetValueInt();
	m_size=wpr->Locate("size")->GetValueDouble();

	/* load the base64 encoded image */
	if(!strcmp(m_type.GetString(),stprimnames[STICKERPRIM_IMAGE]))
	{
		if(wpr->Locate("base64image"))
		{
			unsigned int j;
			unsigned int binarylen;
			kGUIString *b;
			Array<unsigned char>base64image;
			Array<unsigned char>binaryimage;

			/* convert from base64 encoded to binary */
			b=wpr->Locate("base64image")->GetValue();
			
			base64image.Init(b->GetLen(),1);
			for(j=0;j<b->GetLen();++j)
				base64image.SetEntry(j,b->GetChar(j));

			binarylen=kGUI::Base64Decode(b->GetLen(),&base64image,&binaryimage);

			m_image.SetMemory();
			m_image.OpenWrite("wb",binarylen);
			m_image.Write(binaryimage.GetArrayPtr(),(unsigned long)binarylen);
			m_image.Close();
		}
		else
			m_image.SetFilename(m_text.GetString());
	}
}

/* save to prefs file */
void GPXStickerEntry::Save(class kGUIXMLItem *wpr)
{
	wpr->AddParm("type",m_type.GetString());
	wpr->AddParm("colour",m_color.GetString());
	wpr->AddParm("text",m_text.GetString());
	wpr->AddParm("x",m_x);
	wpr->AddParm("y",m_y);
	wpr->AddParm("w",m_w);
	wpr->AddParm("h",m_h);
	wpr->AddParm("size",m_size);

	/* if this is an image then save the base64 encoded version */
	if(!strcmp(m_type.GetString(),stprimnames[STICKERPRIM_IMAGE]))
	{
		/* is this a valid image */
		if(m_image.IsValid() && m_image.GetLoadableSize())
		{
			unsigned int j;
			unsigned int binarylen;
			unsigned int base64len;
			unsigned char c;
			kGUIString bs;
			Array<unsigned char>binaryimage;
			Array<unsigned char>base64image;
			
			binarylen=m_image.GetLoadableSize();

			/* fill the binary array */
			m_image.Open();
			binaryimage.Init(binarylen,1);
			for(j=0;j<binarylen;++j)
			{
				m_image.Read(&c,(unsigned long)1L);
				binaryimage.SetEntry(j,c);
			}
			m_image.Close();
			
			base64len=kGUI::Base64Encode(binarylen,&binaryimage,&base64image);
			bs.SetString((const char *)base64image.GetArrayPtr(),base64len);

			wpr->AddParm("base64image",&bs);
		}
	}
}

void StickersPage::EditStickersChanged(void)
{
	int fnum;

	fnum=m_editstickerlist.GetSelection();
	if(!fnum)
		m_stickertable.DeleteChildren();
	else
		m_stickers.GetEntry(fnum-1)->Load(&m_stickertable);
	Changed();
}

/* update the save/undo button status based on if the filter has changed */

void StickersPage::Changed(void)
{
	bool changed=false;

	PreviewSticker();
	if(!m_editstickerlist.GetSelection())
	{
		/* should be empty, so any rows means changed */
		if(m_stickertable.GetNumChildren(0))
			changed=true;
		m_deletesticker.SetEnabled(false);
		m_renamesticker.SetEnabled(false);
		m_copysticker.SetEnabled(false);
	}
	else
	{
		int i=GetSticker(m_editstickerlist.GetSelectionString());
		changed=m_stickers.GetEntry(i)->Compare(&m_stickertable);
		m_deletesticker.SetEnabled(!changed);
		m_renamesticker.SetEnabled(!changed);
		m_copysticker.SetEnabled(!changed);
	}

	m_savesticker.SetEnabled(changed);	/* enabled after editing */
	m_undosticker.SetEnabled(changed); /* enabled after editing */
	m_editstickerlist.SetLocked(changed);
}

void StickersPage::PreviewSticker(void)
{
	unsigned int i;
	GPXStickerRow *row;
	kGUIText t;

	kGUI::PushClip();
	kGUI::SetCurrentSurface(&m_previewstickersurface);
	kGUI::ResetClip();	/* set clip to full surface on stack */

	/* draw sticker in preview window */
	m_previewstickersurface.Clear(DrawColor(255,255,255));

	for(i=0;i<m_stickertable.GetNumChildren();++i)
	{
		row=static_cast<GPXStickerRow *>(m_stickertable.GetChild(i));
		switch(row->m_type.GetSelection())
		{
		case STICKERPRIM_TEXT:
			t.SetString(row->m_text.GetString());
			t.SetFontSize(row->m_size.GetInt());
			t.Draw(row->m_x.GetInt(),row->m_y.GetInt(),row->m_w.GetInt(),row->m_h.GetInt(),GPX::GetTableColor(row->m_color.GetSelection()));
		break;
		case STICKERPRIM_IMAGE:
//			row->m_image.SetFilename(row->m_text.GetString());
			row->m_image.SetScale(row->m_size.GetDouble(),row->m_size.GetDouble());
			row->m_image.Draw(0,row->m_x.GetInt(),row->m_y.GetInt());
		break;
		case STICKERPRIM_RECT:
			kGUI::DrawRect(row->m_x.GetInt(),row->m_y.GetInt(),row->m_w.GetInt(),row->m_h.GetInt(),GPX::GetTableColor(row->m_color.GetSelection()));
		break;
		}
	}
	kGUI::RestoreScreenSurface();
	kGUI::PopClip();
	m_previewsticker.Dirty();
}

int StickersPage::GetSticker(const char *name)
{
	unsigned int i;
	GPXSticker *s;

	for(i=0;i<m_numstickers;++i)
	{
		s=m_stickers.GetEntry(i);
		if(!strcmp(s->GetName(),name))
			return(i);
	}
	return(-1);
}


void StickersPage::ClickSaveSticker(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		int fnum;
		kGUIInputBoxReq *box;

		fnum=m_editstickerlist.GetSelection();
		if(!fnum)
			box=new kGUIInputBoxReq(this,CALLBACKNAME(ClickSaveSticker2),"Stickers Name?");
		else
			SaveSticker(m_editstickerlist.GetSelectionStringObj());
	}
}

void StickersPage::ClickSaveSticker2(kGUIString *result,int closebutton)
{
	kGUIMsgBoxReq *box;

	if(result->GetLen() && closebutton==MSGBOX_OK)
	{
		if(GetSticker(result->GetString())>=0)
			box=new kGUIMsgBoxReq(MSGBOX_OK,false,"Error: name already used, try again with a different name!");
		else
			SaveSticker(result);
	}
}

void StickersPage::SaveSticker(kGUIString *name)
{
	int i;
	GPXSticker *s;

	/* is this an existing sticker? */
	i=GetSticker(name->GetString());
	if(i>=0)
	{
		s=m_stickers.GetEntry(i);
		s->Save(&m_stickertable);
	}
	else
	{
		/* new sticker */
		s=new GPXSticker(name);
		s->Save(&m_stickertable);
		m_stickers.SetEntry(m_numstickers,s);
		++m_numstickers;
		UpdateStickersList();
		m_editstickerlist.SetSelection(m_numstickers);
	}
	EditStickersChanged();
	//SavePrefs();
}

void StickersPage::ClickDeleteStickers(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		kGUIMsgBoxReq *box;
	
		box=new kGUIMsgBoxReq(MSGBOX_YES|MSGBOX_NO,this,CALLBACKNAME(ClickDeleteStickers2),true,"Delete Stickers '%s'?",m_editstickerlist.GetSelectionString());
	}
}

void StickersPage::ClickDeleteStickers2(int result)
{
	int i;
	GPXSticker *f;

	if(result==MSGBOX_YES)
	{
		i=GetSticker(m_editstickerlist.GetSelectionString());
		if(i>=0)
		{
			f=m_stickers.GetEntry(i);
			delete f;
			m_stickers.DeleteEntry(i);
			--m_numstickers;
			m_editstickerlist.SetSelection(0);

			UpdateStickersList();
			EditStickersChanged();
			//SavePrefs();
		}
	}
}

void StickersPage::ClickRenameSticker(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		kGUIInputBoxReq *box;
	
		box=new kGUIInputBoxReq(this,CALLBACKNAME(ClickRenameSticker2),"New Sticker Name?");
	}
}

void StickersPage::ClickRenameSticker2(kGUIString *result,int closebutton)
{
	kGUIString newname;
	kGUIMsgBoxReq *box;

	if(result->GetLen() && closebutton==MSGBOX_OK )
	{
		newname.SetString(result);
		if(GetSticker(newname.GetString())>=0)
			box=new kGUIMsgBoxReq(MSGBOX_OK,false,"Error: name already used!");
		else
		{
			int i;
			GPXSticker *s;

			i=GetSticker(m_editstickerlist.GetSelectionString());
			if(i>=0)
			{
				s=m_stickers.GetEntry(i);
				s->SetName(&newname);
				/* rename stickers name in select combo too! */
				m_editstickerlist.SetEntry(i+1,newname.GetString(),i+1);
			}
		}
	}
}

void StickersPage::ClickCopySticker(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		kGUIInputBoxReq *box;
	
		box=new kGUIInputBoxReq(this,CALLBACKNAME(ClickCopySticker2),"Copied Sticker Name?");
	}
}

void StickersPage::ClickCopySticker2(kGUIString *result,int closebutton)
{
	kGUIString copyname;
	kGUIMsgBoxReq *box;

	if(result->GetLen() && closebutton==MSGBOX_OK)
	{
		copyname.SetString(result->GetString());
		if(GetSticker(copyname.GetString())>=0)
			box=new kGUIMsgBoxReq(MSGBOX_OK,false,"Error: name already used!");
		else
		SaveSticker(&copyname);
	}
}

void StickersPage::ClickUndoStickers(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		kGUIMsgBoxReq *box;
	
		box=new kGUIMsgBoxReq(MSGBOX_YES|MSGBOX_NO,this,CALLBACKNAME(ClickUndoStickers2),false,"Undo Changes?");
	}
}

void StickersPage::ClickUndoStickers2(int result)
{
	if(result==MSGBOX_YES)
		EditStickersChanged();
}

void StickersPage::UpdateStickersList(void)
{
	unsigned int e;
	kGUIString editStickers;

	editStickers.SetString(m_editstickerlist.GetSelectionString());

	m_editstickerlist.SetNumEntries(m_numstickers+1);			/* list of select Stickerss */
	m_editstickerlist.SetEntry(0,gpx->GetString(STRING_NEWSTICKER),0);
	for(e=0;e<m_numstickers;++e)
		m_editstickerlist.SetEntry(e+1,m_stickers.GetEntry(e)->GetName(),e+1);
	m_editstickerlist.SetSelection(editStickers.GetString());
}

GPXSticker::~GPXSticker()
{
	unsigned int i;

	for(i=0;i<m_numentries;++i)
		delete m_entries.GetEntry(i);
}

void GPXSticker::Load(kGUITableObj *table)
{
	unsigned int e;
	GPXStickerRow *row;
	GPXStickerEntry *re;

	table->DeleteChildren();
	for(e=0;e<m_numentries;++e)
	{
		re=m_entries.GetEntry(e);	/* sticker record */
		row=new GPXStickerRow();
	
		row->m_type.SetSelection(re->m_type.GetString());
		row->m_color.SetSelection(re->m_color.GetString());
		row->m_text.SetString(re->m_text.GetString());
		row->m_x.SetInt(re->m_x);
		row->m_y.SetInt(re->m_y);
		row->m_w.SetInt(re->m_w);
		row->m_h.SetInt(re->m_h);
		row->m_size.SetDouble("%.2f",re->m_size);
		if(re->m_image.IsValid())
			row->m_image.Copy(&re->m_image);
		table->AddRow(row);
	}
}

/* true=changed, false=same */

bool GPXSticker::Compare(kGUITableObj *table)
{
	unsigned int e;
	kGUIObj *obj;
	GPXStickerRow *row;
	GPXStickerEntry *re;

	if(table->GetNumChildren(0)!=m_numentries)
		return(true);

	for(e=0;e<m_numentries;++e)
	{
		re=m_entries.GetEntry(e);	/* sticker record */
		obj=table->GetChild(e);
		row=static_cast<GPXStickerRow *>(obj);

		if(strcmp(re->m_type.GetString(),row->m_type.GetSelectionString()))
			return(true);	/* different */
		if(strcmp(re->m_color.GetString(),row->m_color.GetSelectionString()))
			return(true);	/* different */
		if(strcmp(re->m_text.GetString(),row->m_text.GetString()))
			return(true);	/* different */
		if(re->m_x!=row->m_x.GetInt())
			return(true);	/* different */
		if(re->m_y!=row->m_y.GetInt())
			return(true);	/* different */
		if(re->m_w!=row->m_w.GetInt())
			return(true);	/* different */
		if(re->m_h!=row->m_h.GetInt())
			return(true);	/* different */
		if(re->m_size!=row->m_size.GetDouble())
			return(true);	/* different */
	}
	return(false);	/* same! */
}


void GPXSticker::Save(kGUITableObj *table)
{
	unsigned int e;
	unsigned int nr;
	kGUIObj *obj;
	GPXStickerRow *row;
	GPXStickerEntry *re;

	/* delete old entries if there are any? */
	for(e=0;e<m_numentries;++e)
		delete m_entries.GetEntry(e);

	nr=table->GetNumChildren(0);
	/* number of valid entries */
	m_numentries=nr;
	m_entries.Alloc(nr);
	for(e=0;e<nr;++e)
	{
		obj=table->GetChild(e);
		row=static_cast<GPXStickerRow *>(obj);
		re=new GPXStickerEntry();
				
		re->m_type.SetString(row->m_type.GetSelectionString());
		re->m_color.SetString(row->m_color.GetSelectionString());
		re->m_text.SetString(row->m_text.GetString());
		re->m_x=row->m_x.GetInt();
		re->m_y=row->m_y.GetInt();
		re->m_w=row->m_w.GetInt();
		re->m_h=row->m_h.GetInt();
		re->m_size=row->m_size.GetDouble();
		m_entries.SetEntry(e,re);
	}
}

/* Load from preferences file */

void GPXSticker::Load(kGUIXMLItem *xml)
{
	int i,nc;
	GPXStickerEntry *fe;
	kGUIXMLItem *flxml;

	nc=xml->GetNumChildren();
	for(i=0;i<nc;++i)
	{
		flxml=xml->GetChild(i);
		if(!strcmp(flxml->GetName(),"entry"))
		{
			fe=new GPXStickerEntry();
			fe->Load(flxml);
			m_entries.SetEntry(m_numentries++,fe);
		}
	}
}

/* save to preferences file */

void GPXSticker::Save(kGUIXMLItem *xml)
{
	unsigned int e;
	GPXStickerEntry *re;
	kGUIXMLItem *fxml;
	kGUIXMLItem *flxml;

	/* if no entries in sticker then don't save it */
	if(m_numentries)
	{
		fxml=xml->AddChild("sticker");
		fxml->AddParm("name",m_name.GetString());

		for(e=0;e<m_numentries;++e)
		{
			re=m_entries.GetEntry(e);	/* sticker record */
			flxml=fxml->AddChild("entry");
			re->Save(flxml);
		}
	}
}

class PrintStickers : public kGUIReport
{
public:
	PrintStickers(StickersPage *sp);
	~PrintStickers();
private:
	int GetPPI(void) {return 200;}					/* pixels per inch */
	double GetPageWidth(void) {return 8.5f;}		/* inches */
	double GetPageHeight(void) {return 11.0f;}		/* inches */
	double GetLeftMargin(void) {return 0.33f;}		/* inches */
	double GetRightMargin(void) {return 0.33f;}		/* inches */
	double GetTopMargin(void)	{return 0.50f;}		/* inches */
	double GetBottomMargin(void) {return 0.50f;}	/* inches */
	void Setup(void);
	void Setup(int page) {}
	const char *GetName(void) {return "Map";}
	/********************************/
	StickersPage *m_sp;
};

PrintStickers::PrintStickers(StickersPage *sp)
{
	m_sp=sp;
	SetPID(kGUI::LocatePrinter(m_sp->m_printsticker.GetString(),true));
}

PrintStickers::~PrintStickers()
{
	m_sp->m_printsticker.SetString(kGUI::GetPrinterObj(GetPID())->GetName());
}

void PrintStickers::Setup(void)
{
	unsigned int i;
	int x,y;
	int offx,offy;
	int offx2,offy2;
	GPXStickerRow *row;
	int ppi;

	ppi=GetPPI();

	for(y=0;y<20;++y)
	{
		offy=(int)((0.5f*y)*ppi);
		offy2=(int)((0.5f*(y+1))*ppi);
		for(x=0;x<4;++x)
		{
			offx=(int)(((1.75f+0.28f)*x)*ppi);
			offx2=(int)(((1.75f+0.28f)*(x+1))*ppi);
			for(i=0;i<m_sp->m_stickertable.GetNumChildren();++i)
			{
				row=static_cast<GPXStickerRow *>(m_sp->m_stickertable.GetChild(i));

				switch(row->m_type.GetSelection())
				{
				case STICKERPRIM_TEXT:
				{
					kGUIReportTextObj *t;

					t=new kGUIReportTextObj();
					t->SetString(row->m_text.GetString());
					t->SetFontSize(row->m_size.GetInt());
					t->SetColor(row->m_color.GetSelection());
					t->SetPos(row->m_x.GetInt()+offx,row->m_y.GetInt()+offy);
					t->SetPage(1);

					/* constrain to label since unused (white) descender space can overflow onto the label below and cause 2 pages to print */
					if((t->GetZoneY()+t->GetZoneH())>offy2)
						t->SetZoneH(offy2-t->GetZoneY());

					AddObjToSection(REPORTSECTION_BODY,t,true);
				}
				break;
				case STICKERPRIM_IMAGE:
				{
					kGUIReportImageRefObj *i;		
					kGUIImage *orig;

					orig=&(row->m_image);

					i=new kGUIReportImageRefObj();
					i->SetImage(orig);
					i->SetScale(row->m_size.GetDouble(),row->m_size.GetDouble());
					i->SetPos(row->m_x.GetInt()+offx,row->m_y.GetInt()+offy);
					i->SetSize((int)(orig->GetImageWidth()*row->m_size.GetDouble()),(int)(orig->GetImageHeight()*row->m_size.GetDouble()));
					i->SetPage(1);
					AddObjToSection(REPORTSECTION_BODY,i,true);
				}
				break;
				case STICKERPRIM_RECT:
				{
					kGUIReportRectObj *r;

					r=new kGUIReportRectObj();
					r->SetZone(row->m_x.GetInt()+offx,row->m_y.GetInt()+offy,row->m_w.GetInt(),row->m_h.GetInt());
					r->SetColor(row->m_color.GetSelection());
					r->SetPage(1);
					AddObjToSection(REPORTSECTION_BODY,r,true);
				}
				break;
				}
			}
		}
	}
}

void StickersPage::ClickPrintStickers(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		PrintStickers *ps=new PrintStickers(this);

		ps->Preview();
	}
}
