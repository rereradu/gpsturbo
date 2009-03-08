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
#include "heap.h"

GPXMapStrings::GPXMapStrings()
{
	m_lcwindow=0;
}

void GPXMapStrings::Init(unsigned int w,unsigned int h)
{
	m_lcwindow=new kGUIDrawSurface();
	m_lcwindow->Init(w,h);
	m_lcbounds.lx=0;
	m_lcbounds.ty=0;
	m_lcbounds.rx=w;
	m_lcbounds.by=h;
}

void GPXMapStrings::Clear(void)
{
	kGUIDrawSurface *savesurface;

	/* clear the label collision window */
	kGUI::PushClip();
	savesurface=kGUI::GetCurrentSurface();
	kGUI::SetCurrentSurface(m_lcwindow);
	kGUI::ResetClip();	/* set clip to full surface on stack */
	kGUI::DrawRect(0,0,m_lcwindow->GetWidth(),m_lcwindow->GetHeight(),DrawColor(0,0,0));
	kGUI::SetCurrentSurface(savesurface);
	kGUI::PopClip();
}

bool GPXMapStrings::Check(kGUICorners *b,kGUIPoint2 *c,bool clipedge)
{
	kGUIDrawSurface *savesurface;
	bool dodraw;

	/* is this label totally inside the window? */
	if(clipedge)
	{
		if(kGUI::Inside(b,&m_lcbounds)==false)
			return(false);
	}
	else
	{
		if(kGUI::Overlap(b,&m_lcbounds)==false)
			return(false);
	}

	kGUI::PushClip();
	savesurface=kGUI::GetCurrentSurface();
	kGUI::SetCurrentSurface(m_lcwindow);
	kGUI::ResetClip();	/* set clip to full surface on stack */
	
	/* is this area of the window clear? */
	dodraw=kGUI::ReadPoly(4,c,DrawColor(0,0,0));
	if(dodraw==true)
	{
		/* ok, it was clear, so fill it in now! */
		kGUI::DrawPoly(4,c,DrawColor(255,255,255));
	}
	kGUI::SetCurrentSurface(savesurface);
	kGUI::PopClip();
	return(dodraw);
}

GPXMapStrings::~GPXMapStrings()
{
	if(m_lcwindow)
		delete m_lcwindow;
}
