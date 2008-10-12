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

enum
{
OSMTAG_ND,
OSMTAG_WAY,
OSMTAG_NAME,
OSMTAG_NODE,
OSMTAG_TAG,
OSMTAG_CREATED_BY,
OSMTAG_SOURCE,
OSMTAG_IS_IN,
OSMTAG_REF,
OSMTAG_LCN_REF,

//these are tag types
OSMTAG_HIGHWAY,
OSMTAG_RAILWAY,
OSMTAG_LEISURE,
OSMTAG_NATURAL,
OSMTAG_UNDEFINED
};

typedef struct
{
	unsigned int id;
	const char *tag;
}OSMTAG_DEF;

static const OSMTAG_DEF osmtags[]={
	{OSMTAG_ND,"nd"},
	{OSMTAG_WAY,"way"},
	{OSMTAG_NAME,"name"},
	{OSMTAG_NODE,"node"},
	{OSMTAG_TAG,"tag"},
	{OSMTAG_IS_IN,"is_in"},
	{OSMTAG_SOURCE,"source"},
	{OSMTAG_REF,"ref"},
	{OSMTAG_LCN_REF,"lcn_ref"},
	{OSMTAG_HIGHWAY,"highway"},
	{OSMTAG_RAILWAY,"railway"},
	{OSMTAG_LEISURE,"leisure"},
	{OSMTAG_NATURAL,"natural"},
	{OSMTAG_CREATED_BY,"created_by"}};

enum
{
HIGHWAY_MOTORWAY,
HIGHWAY_MOTORWAY_LINK,
HIGHWAY_TRUNK,
HIGHWAY_TRUNK_LINK,
HIGHWAY_PRIMARY,
HIGHWAY_PRIMARY_LINK,
HIGHWAY_SECONDARY,
HIGHWAY_TERTIARY,
HIGHWAY_UNCLASSIFIED,
HIGHWAY_ROAD,
HIGHWAY_RESIDENTIAL,
HIGHWAY_LIVING_STREET,
HIGHWAY_SERVICE,
HIGHWAY_TRACK,
HIGHWAY_PEDESTRIAN,
HIGHWAY_SERVICES,
HIGHWAY_BUS_GUIDEWAY,
HIGHWAY_PATH,
HIGHWAY_CYCLEWAY,
HIGHWAY_FOOTWAY,
HIGHWAY_BRIDLEWAY,
HIGHWAY_BYWAY,
HIGHWAY_STEPS,
HIGHWAY_MINI_ROUNDABOUT,
HIGHWAY_STOP,
HIGHWAY_TRAFFIC_SIGNALS,
HIGHWAY_CROSSING,
HIGHWAY_GATE,
HIGHWAY_STILE,
HIGHWAY_CATTLE_GRID,
HIGHWAY_TOLL_BOOTH,
HIGHWAY_INCLINE,
HIGHWAY_INCLINE_STEEP,
HIGHWAY_FORD,
HIGHWAY_BUS_STOP,
HIGHWAY_TURNING_CIRCLE,
HIGHWAY_CONSTRUCTION,
HIGHWAY_EMERGENCY_ACCESS_POINT,
RAILWAY_RAIL,
RAILWAY_SUBWAY,
LEISURE_PARK,
LEISURE_PITCH,
NATURAL_WATER,
NATURAL_COASTLINE
};

enum
{
OSMRENDERTYPE_POLY,
OSMRENDERTYPE_POLYLINE,
OSMRENDERTYPE_TRACK,
OSMRENDERTYPE_ICON
};

enum
{
OSMLINETYPE_LINE,
OSMLINETYPE_CREEK,
OSMLINETYPE_NARROWSTREET,
OSMLINETYPE_STREET,
OSMLINETYPE_RAMPS,
OSMLINETYPE_COLLECTOR,
OSMLINETYPE_HIGHWAY,
OSMLINETYPE_NUM};

/* thicknes stable for lines */

static const double osmthickinfo[OSMLINETYPE_NUM][MAXMSZOOM]={
	//0    1    2    3    4    5    6    7    8    0   10    11    12    13    14    15    16    17    18    19
	{0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f},		/* line ( topo etc. )*/
	{0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,1.0f, 1.0f, 1.0f, 1.0f, 2.5f, 5.0f, 7.0f, 8.0f, 9.0f,10.0f},		/* creek */
	{0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.4f,0.5f, 0.6f, 0.7f, 1.0f, 2.0f, 5.0f, 8.0f, 7.0f,10.0f,11.0f},		/* narrow street */
	{0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,1.0f,1.0f, 1.0f, 1.0f, 2.0f, 4.5f, 9.0f,12.0f,13.0f,14.0f,15.0f},		/* side street */
	{0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,1.0f,2.0f,2.0f, 2.5f, 3.0f, 4.0f, 6.0f,11.0f,13.0f,14.0f,15.0f,16.0f},		/* ramps */
	{0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,1.0f,2.0f,2.5f,2.0f, 4.0f, 6.0f, 9.0f,10.0f,13.0f,14.0f,15.0f,16.0f,17.0f},		/* artery */
	{0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,2.0f,3.0f,4.0f,5.0f, 7.0f, 9.0f,12.0f,15.0f,16.0f,17.0f,18.0f,19.0f,20.0f}};	/* highway */

static const OSMRENDER_INFO osmrenderinfo[]={
	{HIGHWAY_MOTORWAY,		"highway:motorway",			OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_HIGHWAY,	DrawColor(242,191,36)},
	{HIGHWAY_MOTORWAY_LINK,	"highway:motorway_link",	OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_RAMPS,		DrawColor(242,181,36)},
	{HIGHWAY_TRUNK,			"highway:trunk",			OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_COLLECTOR,	DrawColor(255,250,112)},
	{HIGHWAY_TRUNK_LINK,	"highway:trunk_link",		OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_COLLECTOR,	DrawColor(255,250,112)},
	{HIGHWAY_PRIMARY,		"highway:primary",			OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255)},
	{HIGHWAY_PRIMARY_LINK,	"highway:primary_link",		OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255)},
	{HIGHWAY_SECONDARY,		"highway:secondary",		OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255)},
	{HIGHWAY_TERTIARY,		"highway:tertiary",			OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255)},
	{HIGHWAY_ROAD,			"highway:road",				OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255)},
	{HIGHWAY_RESIDENTIAL,	"highway:residential",		OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(255,255,255)},
	{HIGHWAY_CYCLEWAY,		"highway:cycleway",			OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_LINE,		DrawColor(32,32,255)},
	{HIGHWAY_SERVICE,		"highway:service",			OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(245,245,245)},
	{RAILWAY_RAIL,			"railway:rail",				OSMRENDERTYPE_TRACK,	0,						DrawColor(0,0,0)},
	{RAILWAY_SUBWAY,		"railway:subway",			OSMRENDERTYPE_POLYLINE,	OSMLINETYPE_STREET,		DrawColor(200,200,200)},
	{LEISURE_PARK,			"leisure:park",				OSMRENDERTYPE_POLY,		0,						DrawColor(167,204,149)},
	{LEISURE_PITCH,			"leisure:pitch",			OSMRENDERTYPE_POLY,		0,						DrawColor(167,200,149)},
	{NATURAL_WATER,			"natural:water",			OSMRENDERTYPE_POLY,		0,						DrawColor(153,179,224)},
	{NATURAL_COASTLINE,		"natural:coastline",		OSMRENDERTYPE_POLY,		0,						DrawColor(153,179,214)}};

#if 0
HIGHWAY_UNCLASSIFIED,
HIGHWAY_ROAD,
HIGHWAY_RESIDENTIAL,
HIGHWAY_LIVING_STREET,
HIGHWAY_SERVICE,
HIGHWAY_TRACK,
HIGHWAY_PEDESTRIAN,
HIGHWAY_SERVICES,
HIGHWAY_BUS_GUIDEWAY,
HIGHWAY_PATH,
HIGHWAY_CYCLEWAY,
HIGHWAY_FOOTWAY,
HIGHWAY_BRIDLEWAY,
HIGHWAY_BYWAY,
HIGHWAY_STEPS,
HIGHWAY_MINI_ROUNDABOUT,
HIGHWAY_STOP,
HIGHWAY_TRAFFIC_SIGNALS,
HIGHWAY_CROSSING,
HIGHWAY_GATE,
HIGHWAY_STILE,
HIGHWAY_CATTLE_GRID,
HIGHWAY_TOLL_BOOTH,
HIGHWAY_INCLINE,
HIGHWAY_INCLINE_STEEP,
HIGHWAY_FORD,
HIGHWAY_BUS_STOP,
HIGHWAY_TURNING_CIRCLE,
HIGHWAY_CONSTRUCTION,
HIGHWAY_EMERGENCY_ACCESS_POINT
#endif

kGUIDPoint2 OSMMap::m_ppoints[MAXPP];

/* openstreetmap.com format map renderer */

OSMMap::OSMMap(const char *fn)
{
	int unsigned i,j;
	int nt;
	double c;
	kGUIXML xml;
	kGUIXMLItem *root;
	kGUIXMLItem *item;
	kGUIXMLItem *itemchild;
	kGUIXMLItem *iid;
	kGUIXMLItem *ilat;
	kGUIXMLItem *ilon;
	OSMNODE_DEF n;
	unsigned int numnodes;
	OSMNODE_DEF **nlist;
	OSMNODE_DEF *np;
	OSMWAY *way;
	const OSMRENDER_INFO *ori;
	const OSMRENDER_INFO **pori;
	kGUIString *k;
	kGUIString *v;
	kGUIString kv;
	const OSMTAG_DEF *otag;

	/* used to log unhandled types */
	m_unknown.Init(12,0);

	/* used in the draw tile code, inited here */
	m_drawheap.Alloc(65536);
	m_drawways.Init(1024,256);

	kv.Alloc(1024);
	SetZoomLevels(MINOSMZOOM,MAXOSMZOOM);
	SetTileSize(256,256);
	
	/* build a hash table for tags */
	m_tags.Init(12,sizeof(OSMTAG_DEF *));
	otag=osmtags;
	for(i=0;i<sizeof(osmtags)/sizeof(OSMTAG_DEF);++i)
	{
		m_tags.Add(otag->tag,&otag);
		++otag;
	}

	/* build a hash table for rendering info */
	m_roadinfo.Init(16,sizeof(OSMRENDER_INFO *));
	ori=osmrenderinfo;
	for(i=0;i<sizeof(osmrenderinfo)/sizeof(OSMRENDER_INFO);++i)
	{
		m_roadinfo.Add(ori->name,&ori);
		++ori;
	}

	for(i=0;i<MAXOSMZOOM;++i)
	{
		c=(double)(1<<i);
		m_pixelsPerLonDegree[i]=c/360.0f;
  		m_negpixelsPerLonRadian[i] = -(c / (2.0f*3.1415926535f));
		m_bitmapOrigo[i]=c/2.0f;
		nt = 1<<i;
		SetSize(i,nt*256,nt*256);
	}

	/* save nodes in a hash table */
	m_nodes.Init(16,sizeof(OSMNODE_DEF));
	m_heap.Alloc(65536);
	m_numways=0;
	m_ways.Init(32768,16384);

	/* open street maps are in XML format so use our XML handler for reading them */
	if(xml.Load(fn))
	{

		root=xml.GetRootItem()->Locate("osm");
		if(!root)
			return;			/* NOT an openstreet format map */

		/* build a hash table for all the nodes */
		for(i=0;i<root->GetNumChildren();++i)
		{
			item=root->GetChild(i);
			if(!strcmp(item->GetName(),"node"))
			{
				/* add a node to the node hash table */
				iid=item->Locate("id");
				ilat=item->Locate("lat");
				ilon=item->Locate("lon");

				n.c.Set(ilat->GetValueDouble(),ilon->GetValueDouble());
				m_nodes.Add(iid->GetValueString(),&n);
			}
		}
		/* build a list of polygons */
		for(i=0;i<root->GetNumChildren();++i)
		{
			item=root->GetChild(i);
			switch(GetTag(item->GetName()))
			{
			case OSMTAG_WAY:
				/* count number of nd's */
				numnodes=0;
				for(j=0;j<item->GetNumChildren();++j)
				{
					itemchild=item->GetChild(j);
					if(!strcmp(itemchild->GetName(),"nd"))
						++numnodes;
				}
				/* if this way contained verts and save it in the list */
				if(numnodes)
				{
					assert(numnodes<MAXOPP,"Too many points!");

					/* allocate an attay of vert pointers from a heap */
					nlist=(OSMNODE_DEF **)m_heap.Alloc(numnodes*sizeof(OSMNODE_DEF *));	
					
					/* allocate another way */
					way=m_ways.GetEntryPtr(m_numways++);
					way->m_numnodes=numnodes;
					way->m_nodes=nlist;

					numnodes=0;
					for(j=0;j<item->GetNumChildren();++j)
					{
						itemchild=item->GetChild(j);
						switch(GetTag(itemchild->GetName()))
						{
						case OSMTAG_ND:
							np=(OSMNODE_DEF *)m_nodes.Find(itemchild->Locate("ref")->GetValueString());
							assert(np!=0,"Error locating node");
							if(!numnodes)
							{
								way->m_min.Set(np->c.GetLat(),np->c.GetLon());
								way->m_max.Set(np->c.GetLat(),np->c.GetLon());
							}
							else
							{
								way->m_min.SetLat(min(way->m_min.GetLat(),np->c.GetLat()));
								way->m_min.SetLon(min(way->m_min.GetLon(),np->c.GetLon()));

								way->m_max.SetLat(max(way->m_max.GetLat(),np->c.GetLat()));
								way->m_max.SetLon(max(way->m_max.GetLon(),np->c.GetLon()));
							}
							nlist[numnodes++]=np;
						break;
						case OSMTAG_TAG:
							k=itemchild->Locate("k")->GetValue();
							switch(GetTag(k->GetString()))
							{
							case OSMTAG_CREATED_BY:
								/* ignore these */
							break;
							case OSMTAG_NAME:
							case OSMTAG_IS_IN:
							case OSMTAG_LCN_REF:
							case OSMTAG_REF:
							case OSMTAG_SOURCE:
								/* ignore these for now */
							break;
							case OSMTAG_HIGHWAY:
							case OSMTAG_RAILWAY:
							case OSMTAG_LEISURE:
							case OSMTAG_NATURAL:
								v=itemchild->Locate("v")->GetValue();
								kv.Sprintf("%s:%s",k->GetString(),v->GetString());
								pori=(const OSMRENDER_INFO **)m_roadinfo.Find(kv.GetString());
								if(pori)
									way->m_ori=*(pori);
								else
								{
									if(!m_unknown.Find(kv.GetString()))
									{
										/* log unhandled types so I can add them */
										m_unknown.Add(kv.GetString(),0);
										DebugPrint("<tag> %s\n",kv.GetString());
									}
								}
							break;
							default:
								v=itemchild->Locate("v")->GetValue();
								kv.Sprintf("%s:%s",k->GetString(),v->GetString());
								if(!m_unknown.Find(kv.GetString()))
								{
									/* log unhandled types so I can add them */
									m_unknown.Add(kv.GetString(),0);
									DebugPrint("<tag> %s\n",kv.GetString());
								}
							break;
							}
						break;
						default:
							if(!m_unknown.Find(itemchild->GetName()))
							{
								/* log unhandled types so I can add them */
								m_unknown.Add(itemchild->GetName(),0);
								DebugPrint("tag='%s'\n",itemchild->GetName());
							}
						break;
						}
					}
					assert(numnodes==way->m_numnodes,"Error locating Node");
				}
			break;
			}
		}
	}
}

OSMMap::~OSMMap()
{

}

void OSMMap::ToMap(GPXCoord *c,int *sx,int *sy)
{
	double e;
	int z=GetZoom();

  	sx[0] = (int)(floor((m_bitmapOrigo[z] + c->GetLon() * m_pixelsPerLonDegree[z])*256.0f));
  	e = sin(c->GetLat() * (3.1415926535f/180.0f));

  	if(e > 0.9999)
    	e = 0.9999;

  	if(e < -0.9999)
    	e = -0.9999;

  	sy[0] = (int)(floor((m_bitmapOrigo[z] + 0.5f * log((1.0f + e) / (1.0f - e)) * m_negpixelsPerLonRadian[z])*256.0f));
}


/* convert from screen+scroll values to lon/lat */
void OSMMap::FromMap(int sx,int sy,GPXCoord *c)
{
	double e;
	int z=GetZoom();

	c->SetLon(((double)sx - (m_bitmapOrigo[z]*256.0f)) / (m_pixelsPerLonDegree[z]*256.0f));
	e = ((double)sy - (m_bitmapOrigo[z]*256.0f)) / (m_negpixelsPerLonRadian[z]*256.0f);
	c->SetLat((2.0f * atan(exp(e)) - 3.1415926535f / 2.0f) / (3.1415926535f/180.0f));
}

/* draw tile at this position */
int OSMMap::DrawTile(int tx,int ty)
{
	int i;
	unsigned int n;
	unsigned int j;
	HashEntry *he;
	OSMNODE_DEF *np;
	int txpix,typix,nx,ny;
	kGUICorners tc;
	kGUICorners wc;
	OSMWAY *way;
	const OSMRENDER_INFO *ori;
	double thickness,edge;

	/* temp heap used to save projected coords for multi-pass rendering */
	m_numdrawways=0;
	m_drawheap.Reset();

	txpix=tx<<8;
	typix=ty<<8;

	tc.lx=txpix;
	tc.rx=txpix+256;
	tc.ty=typix;
	tc.by=typix+256;

	kGUI::DrawRect(0,0,256,256,DrawColor(242,239,233));

	/* for now just draw a pixel for every node */
	he=m_nodes.GetFirst();
	for(i=0;i<m_nodes.GetNum();++i)
	{
		np=(OSMNODE_DEF *)he->m_data;

		/* convert lat/lon to pixel */
		ToMap(&(np->c),&nx,&ny);

		kGUI::DrawPixel(nx-txpix,ny-typix,DrawColor(0,0,0));

		he=he->GetNext();
	}

	/* draw all ways that overlap this tile */
	for(j=0;j<m_numways;++j)
	{
		way=m_ways.GetEntryPtr(j);

		/* project the corners so we can clip check */
		ToMap(&(way->m_min),&wc.lx,&wc.by);
		ToMap(&(way->m_max),&wc.rx,&wc.ty);
		
		if(kGUI::Overlap(&tc,&wc))
		{
			/* draw this way since it overlaps the tile */

			/* todo: remove duplicate points that are one after another */
			for(n=0;n<way->m_numnodes;++n)
			{
				ToMap(&(way->m_nodes[n]->c),&nx,&ny);
				m_ppoints[n].x=(nx-txpix);
				m_ppoints[n].y=(ny-typix);
			}
			if(way->m_ori)
			{
				ori=way->m_ori;
				switch(ori->rendertype)
				{
				case OSMRENDERTYPE_POLY:
					if(way->m_nodes[0]==way->m_nodes[way->m_numnodes-1])
						kGUI::DrawPoly(way->m_numnodes-1,m_ppoints,ori->colour);
					else
						kGUI::DrawPolyLine(way->m_numnodes,m_ppoints,ori->colour);
				break;
				case OSMRENDERTYPE_TRACK:
					if(GetZoom()>13)
						MSGPXMap::DrawTrainTracks(way->m_numnodes,m_ppoints);
					else
						kGUI::DrawPolyLine(way->m_numnodes,m_ppoints,DrawColor(0,0,0));
				break;
				case OSMRENDERTYPE_POLYLINE:
					thickness=osmthickinfo[ori->rendersubtype][GetZoom()];
					if(thickness==1.0f)
						kGUI::DrawPolyLine(way->m_numnodes,m_ppoints,ori->colour);
					else if(thickness>1.0f)
					{
						thickness=thickness*0.5f;

						/* save in table for pass 2 rendering */
						m_drawways.SetEntry(m_numdrawways++,way);

						/* copy projected points to a temporary heap so we don't need to */
						/* project them again, this is a speed optimization! */
						way->m_proj=(kGUIDPoint2 *)m_drawheap.Alloc(way->m_numnodes*sizeof(kGUIDPoint2));
						memcpy(way->m_proj,m_ppoints,way->m_numnodes*sizeof(kGUIDPoint2));
#if 0
						/* save labels associated with this polyline */
						if(GetZoom()>=13 && m_numlabels && pi->drawlabel)
						{
							skiplabel=true;
							ps.numlabels=m_numlabels;
							memcpy(ps.curlabels,m_curlabels,sizeof(m_curlabels));
						}
						else
							ps.numlabels=0;
						m_roadgroups[pi->thickindex].SetEntry(m_roadgroupspolys[pi->thickindex]++,ps);
#endif
						edge=min(max(0.75f,thickness*0.115f),1.5f);
						kGUI::DrawFatPolyLine(way->m_numnodes,m_ppoints,DrawColor(128,128,128),thickness+edge,0.66f);
					}
				break;
				case OSMRENDERTYPE_ICON:
				break;
				}
			}
			else
			{
				/* is this a closed poly? */
				if(way->m_nodes[0]==way->m_nodes[way->m_numnodes-1])
					kGUI::DrawPoly(way->m_numnodes-1,m_ppoints,DrawColor(0,0,0));
				else
					kGUI::DrawPolyLine(way->m_numnodes,m_ppoints,DrawColor(0,0,0));
			}
		}
	}

	/* pass 2 */
	for(j=0;j<m_numdrawways;++j)
	{
		way=m_drawways.GetEntry(j);

		ori=way->m_ori;
		switch(ori->rendertype)
		{
		case OSMRENDERTYPE_POLY:
		break;
		case OSMRENDERTYPE_POLYLINE:
			thickness=osmthickinfo[ori->rendersubtype][GetZoom()]*0.5f;
			kGUI::DrawFatPolyLine(way->m_numnodes,way->m_proj,ori->colour,thickness);
		break;
		}
	}

	return(TILE_OK);
}

unsigned int OSMMap::GetTag(const char *s)
{
	OSMTAG_DEF **potag;
	
	potag=(OSMTAG_DEF **)m_tags.Find(s);
	if(!potag)
		return(OSMTAG_UNDEFINED);
	return(potag[0]->id);
}
