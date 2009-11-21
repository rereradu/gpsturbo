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
#include "zip.h"

/* KML Map */
KMLGPXMapOverlay::KMLGPXMapOverlay()
{
	m_numareas=0;
}

KMLGPXMapOverlay::~KMLGPXMapOverlay()
{
}

void KMLGPXMapOverlay::SetFilename(const char *fn,ContainerFile *cf)
{
	unsigned int i;
	kGUIXML xml;
	kGUIXMLItem *xmlitem;
	kGUIXMLItem *overlayitem;
	kGUIXMLItem *iconitem;
	kGUIXMLItem *llitem;
	kGUIXMLItem *hrefitem;
	kGUIXMLItem *nitem;
	kGUIXMLItem *sitem;
	kGUIXMLItem *eitem;
	kGUIXMLItem *witem;
	KMLArea *area;
	double n,s,e,w;
	kGUIString longfn,path,imgfn;

	/* if this is inside a KMZ file then cf is not zero */
	/* if it is zero, then we need to extract the path for locating the images */
	if(!cf)
	{
		longfn.SetString(fn);
		kGUI::ExtractPath(&longfn,&path);
	}

	m_numareas=0;
	m_areas.Init(8,-1);
	xml.SetFilename(fn,cf);
	xml.Load();

	xmlitem=xml.GetRootItem()->Locate("kml");
	if(xmlitem)
	{
		xmlitem=xmlitem->Locate("Folder");
		if(xmlitem)
		{
			/* iterate through and grab 'GroundOverlays' */
			for(i=0;i<xmlitem->GetNumChildren();++i)
			{
				overlayitem=xmlitem->GetChild(i);
				if(!strcmp(overlayitem->GetName(),"GroundOverlay"))
				{
					/* got an overlay */
					iconitem=overlayitem->Locate("Icon");
					llitem=overlayitem->Locate("LatLonBox");
					if(iconitem && llitem)
					{
						hrefitem=iconitem->Locate("href");
						nitem=llitem->Locate("north");
						sitem=llitem->Locate("south");
						eitem=llitem->Locate("east");
						witem=llitem->Locate("west");
						if(hrefitem && nitem && sitem && eitem && witem)
						{
							n=nitem->GetValueDouble();
							s=sitem->GetValueDouble();
							e=eitem->GetValueDouble();
							w=witem->GetValueDouble();

							if(!m_numareas)
							{
								/* set overall */
								m_north=n;
								m_south=s;
								m_east=e;
								m_west=w;
							}
							else
							{
								m_north=MAX(n,m_north);
								m_south=MIN(s,m_south);
								m_east=MAX(e,m_east);
								m_west=MIN(w,m_west);
							}
							/* ok, we have enough!, we can add it */
							area=m_areas.GetEntryPtr(m_numareas++);
							area->m_north=n;
							area->m_south=s;
							area->m_east=e;
							area->m_west=w;

							if(cf)
								area->m_image.SetFilename(hrefitem->GetValue(),cf);
							else
							{
								kGUI::MakeFilename(&path,hrefitem->GetValue(),&imgfn);
								area->m_image.SetFilename(&imgfn);
							}
						}
					}
				}
			}
		}
	}
}

void KMLGPXMapOverlay::Draw(kGUICorners *c,double alpha)
{
	unsigned int i;
	GPXCoord nw;
	GPXCoord se;
	kGUICorners subcorners;
	KMLArea *area;
	double scalex,scaley;
	kGUIImage *img;

	if(!m_numareas)
		return;

	nw.SetLat(m_north);
	nw.SetLon(m_west);
	se.SetLat(m_south);
	se.SetLon(m_east);
	gpx->m_curmap->ToMap(&nw,&subcorners.lx,&subcorners.ty);
	gpx->m_curmap->ToMap(&se,&subcorners.rx,&subcorners.by);

	/* does this tile overlap this overlay? */
	if(kGUI::Overlap(c,&subcorners))
	{
		/* yes, so render all overlapping areas */
		for(i=0;i<m_numareas;++i)
		{
			area=m_areas.GetEntryPtr(i);
			img=&area->m_image;
			if(img->IsValid())
			{
				nw.SetLat(area->m_north);
				nw.SetLon(area->m_west);
				se.SetLat(area->m_south);
				se.SetLon(area->m_east);
				gpx->m_curmap->ToMap(&nw,&subcorners.lx,&subcorners.ty);
				gpx->m_curmap->ToMap(&se,&subcorners.rx,&subcorners.by);
				/* does the tile overlap this area? */
				if(kGUI::Overlap(c,&subcorners))
				{
					/* calc scale */
					scalex=(double)(subcorners.rx-subcorners.lx)/(double)img->GetImageWidth();
					scaley=(double)(subcorners.by-subcorners.ty)/(double)img->GetImageHeight();
					img->SetScale(scalex,scaley);
					img->DrawAlpha(0,subcorners.lx-c->lx,subcorners.ty-c->ty,alpha);
				}
			}
		}
//		kGUI::DrawRect(subcorners.lx-c->lx,subcorners.ty-c->ty,subcorners.rx-c->lx,subcorners.by-c->ty,col,alpha);
	}
}

/* KMZ Map */
KMZGPXMapOverlay::KMZGPXMapOverlay()
{
}

KMZGPXMapOverlay::~KMZGPXMapOverlay()
{
}

void KMZGPXMapOverlay::SetFilename(const char *fn)
{
	kGUIDir dir;

	/* filename should point to a zipfile */
	m_zf.SetFilename(fn);
//	m_zf.LoadDirectory();
	/* find first .kml file inside of the zip */
	dir.LoadDir(&m_zf,".kml");
	if(dir.GetNumFiles())
		m_kml.SetFilename(dir.GetFilename(0),&m_zf);
}

void KMZGPXMapOverlay::Draw(kGUICorners *c,double alpha)
{
	m_kml.Draw(c,alpha);
}
