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

/* oziformat uses zlib compression */
#include "zlib/zlib.h"

#pragma pack( push, 1)
typedef struct
{
	short  wSignature;    // 0x7778

	int dw0;           // 0x00000000
	int dw1;           // 0x00010040
	int dw2;           // 0x00000436
	int dw3;           // 0x00000028

	int dwWidth;
	int dwHeight;
	short  wPlanes;       // ?? - 0x0001
	short  wBitsPerPixel; // ?? - 0x0008

	int dw4;           // 0x00000000

	int dwMemSize;     // Assumed that 1 unsigned char per pixel, = dwWidth*dwHeight.

	int dw5;           // 0x00000000
	int dw6;           // 0x00000000
	int dw7;           // 0x00000100
	int dw8;           // 0x00000000 for 1.x, 0x00000100 for 2.x
}OZ2FHEADER_DEF;
#pragma pack( pop)

OZF2GPXMap::OZF2GPXMap(const char *fn)
{
	kGUIString ozfn;
	DataHandle dh;
	int linenum;
	kGUIString line;
	kGUIStringSplit ss;
	int index;
	int z;
	int nf;
	unsigned char *fp;
	long filesize;
	OZ2FHEADER_DEF *oh;
	double maxlat,maxlon;
	double maxx,maxy;

	m_coords=0;

	dh.SetFilename(fn);
	if(dh.Open()==false)
		return;	/* couldn't load file */

	linenum=0;
	do
	{
		dh.ReadLine(&line);
		nf=ss.Split(&line,",");

		if(linenum==2)
		{
			int sl;
			static char dirchar[]={ DIRCHAR };

			ozfn.SetString(fn);	/* extract path from filename */
			sl=ozfn.GetLen()-1;
			/* cut off last subdir */
			while(sl>0)
			{
				if(ozfn.GetChar(sl)==dirchar[0])
				{
					++sl;
					break;
				}
				--sl;
			};
			ozfn.Clip(sl);
			ozfn.Append(line.GetString());
		}
		else if(linenum==4)
		{
			/* datum */
			if(!strcmp(ss.GetWord(0)->GetString(),"WGS 84"))
			{
				/* ok */
			}
			else
			{
				dh.Close();
				return;
			}
			//if (! FindOziDatumToWGS84 (strDatum.c_str (), m_pDatumTransformation)) {
			//	m_pDatumTransformation = NULL;
			//	ReportWarning ("Some unsupported datum is used: '%s'. No datum conversion performed.", strDatum.c_str ());
			//fuck
		}
		else
		{
			if(!strcmp(ss.GetWord(0)->GetString(),"Map Projection"))
			{
				/* todo */
			}
			else if(!strcmp(ss.GetWord(0)->GetString(),"MMPNUM"))
			{
				m_coords=new OZF2Corners[ss.GetWord(1)->GetInt()];
			}
			else if(!strcmp(ss.GetWord(0)->GetString(),"MMPXY"))
			{
				/* pixel position */
				index=atoi(ss.GetWord(1)->GetString())-1;
				m_coords[index].x=atoi(ss.GetWord(2)->GetString());
				m_coords[index].y=atoi(ss.GetWord(3)->GetString());
			}
			else if(!strcmp(ss.GetWord(0)->GetString(),"MMPLL"))
			{
				/* latitude/longitude position */
				index=atoi(ss.GetWord(1)->GetString())-1;
				m_coords[index].lon=atof(ss.GetWord(2)->GetString());
				m_coords[index].lat=atof(ss.GetWord(3)->GetString());
			}
		}
		++linenum;
	}while(dh.Eof()==false);
	dh.Close();

	/* calculate bounding rectanle for lat/lon */
	m_minlat=m_coords[0].lat;
	maxlat=m_coords[0].lat;
	m_minlon=m_coords[0].lon;
	maxlon=m_coords[0].lon;
	m_minx=m_coords[0].x;
	maxx=m_coords[0].x;
	m_miny=m_coords[0].y;
	maxy=m_coords[0].y;
	for(index=1;index<4;++index)
	{
		if(m_coords[index].lat<m_minlat)
		{
			m_minlat=m_coords[index].lat;
			m_miny=m_coords[index].y;
		}
		if(m_coords[index].lat>maxlat)
		{
			maxlat=m_coords[index].lat;
			maxy=m_coords[index].y;
		}

		if(m_coords[index].lon<m_minlon)
		{
			m_minlon=m_coords[index].lon;
			m_minx=m_coords[index].x;
		}
		if(m_coords[index].lon>maxlon)
		{
			maxlon=m_coords[index].lon;
			maxx=m_coords[index].x;
		}
	}

	/* assume linear */
	m_slx=(maxlon-m_minlon)/(maxx-m_minx);
	m_sly=(maxlat-m_minlat)/(maxy-m_miny);
	m_lsx=(maxx-m_minx)/(maxlon-m_minlon);
	m_lsy=(maxy-m_miny)/(maxlat-m_minlat);

	for(index=0;index<4;++index)
	{
		GPXCoord c;
		int sx,sy;

		c.Set(m_coords[index].lat,m_coords[index].lon);
		ToMap(&c,&sx,&sy);
		DebugPrint("Coord testing %f,%f %d,%d - %d,%d",m_coords[index].lat,m_coords[index].lon,m_coords[index].x,m_coords[index].y,sx,sy);
	}

	/* load the map file with the tiles */
	fp=kGUI::LoadFile(ozfn.GetString(),&filesize);
	if(!fp)
		return;	/* couldn't load file */

	m_filedata=fp;	/* save pointer to data so destructor will free it up upon exiting */
	oh=(OZ2FHEADER_DEF *)fp;
	
	if(oh->wSignature!=0x7778)
		return;	/* not valid format */

	assert ((oh->wPlanes==1) && (oh->wBitsPerPixel==8),"Unsupported bitmap format" );

	SetTileSize(64,64);

	/* parse the tile data */

	const int dwMasterTblOffset = * reinterpret_cast<const int *> (fp + filesize - 4);
	const int * const pdwMasterTbl = reinterpret_cast<const int *> (fp + dwMasterTblOffset);

	const int zoomlevels = (filesize - 4 - dwMasterTblOffset)/4;
	SetZoomLevels(1,zoomlevels);
	m_bitmaps.Alloc(zoomlevels);
	z=zoomlevels-1;
	for (int zoomlevel = 0; zoomlevel < zoomlevels; ++ zoomlevel)
	{
		OZ2FBITMAP_DEF *ih = (OZ2FBITMAP_DEF *) (fp + pdwMasterTbl [zoomlevel]);

		m_bitmaps.SetEntry(z,ih);
		SetSize(z,ih->dwImageWidth,ih->dwImageHeight);
		--z;
	}
}

OZF2GPXMap::~OZF2GPXMap()
{
	if(m_coords)
		delete []m_coords;
	if(m_filedata)
		 delete []m_filedata;
}

void OZF2GPXMap::ToMap(class GPXCoord *c,int *sx,int *sy)
{
	double lat=c->GetLat();
	double lon=c->GetLon();

	*(sx)=(int)((lon-m_minlon)*m_lsx)+m_minx;
	*(sy)=(int)((lat-m_minlat)*m_lsy)+m_miny;
}

void OZF2GPXMap::FromMap(int sx,int sy,class GPXCoord *c)
{
	c->Set(((sy-m_miny)*m_sly)+m_minlat,((sx-m_minx)*m_slx)+m_minlon);
}

/* draw tile at this position */
int OZF2GPXMap::DrawTile(int tx,int ty)
{
	int x,y;
	OZ2FBITMAP_DEF *b=m_bitmaps.GetEntry(GetZoom());
	OZ2FCOLOUR_DEF *c;
	unsigned char *up;
	unsigned char unpacked[64*64];
    z_stream d_stream; /* decompression stream */

	int dwTileOffset;
	unsigned char *pTileData;

	if(tx<0 || tx>=b->wXTilesNum || ty<0 || ty>=b->wYTilesNum)
		return(TILE_ERROR);

	dwTileOffset = b->dwTileDataOffset [(ty*b->wXTilesNum)+tx];
	pTileData = (unsigned char *) (m_filedata + dwTileOffset);


	d_stream.zalloc = (alloc_func)0;
    d_stream.zfree = (free_func)0;
    d_stream.opaque = (voidpf)0;

    d_stream.next_in  = pTileData;
	d_stream.avail_in=64*64;
    d_stream.next_out = unpacked;
	d_stream.avail_out=64*64;

	inflateInit(&d_stream);
    inflate(&d_stream, Z_NO_FLUSH);
    inflateEnd(&d_stream);

	//decompress(
	up=unpacked;
	for(y=0;y<64;++y)
	{
		kGUIColor *sp=kGUI::GetSurfacePtr(0,63-y);
		for(x=0;x<64;++x)
		{
			c=b->dwColor+*(up++);
			*(sp++)=DrawColor(c->r,c->g,c->b);
		}
	}
	return(TILE_OK);
}


#if 0
void COziMapLoader::ParseDatum (const char * _p, const char * _pNextLine) {
	const char * strDatumEnd = _p;
	while (strDatumEnd < _pNextLine) {
		if (* strDatumEnd == ',' || * strDatumEnd == '\r' || * strDatumEnd == '\n')
			break;
		++ strDatumEnd;
	}
	const string_t strDatum (_p, strDatumEnd);

	}
}

bool COziMapLoader::PrepareData (bool _bCheck) {
	// Check if all required fields are defined.
	if (m_dwPointDefined == 0) {
		if (_bCheck && m_bMMPXY_Defined && m_bMMPLL_Defined) {
			ReportWarning ("The activation key is probably damaged.");
		} else {
			ReportWarning ("No corner points defined (expected in MMPXY and MMPLL lines).");
			ReportText ("Please try to update the .MAP file with newer version of OziExplorer.");
		}
		return false;
	} else if (m_dwPointDefined != 0xF) {
		ReportWarning ("Not enough corner points (4 expected in MMPXY and MMPLL).");
		return false;
	}

	// Convert datum.
	if (m_pDatumTransformation) {
		ReportText ("Performing datum conversion to WGS84...");

		for (size_t cPoint = 0; cPoint < 4; ++ cPoint)
			m_pDatumTransformation->ApplyTo (m_points [cPoint]);
	}

	// Estimate the bounding rectangle.
	for (size_t cPoint = 0; cPoint < 4; ++ cPoint)
		m_rectBound.Extend (m_points [cPoint]);

	return true;
}

bool COziMapLoader::PrepareProjection () {
	ReportText ("Performing projection calculations...");

	return true;
}
#endif
