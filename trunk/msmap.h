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

#ifndef __MSMAP__
#define __MSMAP__

#include "map.h"

#define MAXMSZOOM 20
#define MINMSZOOM 0
#define UNIT_TO_DEG .00002145767211914062L	// 360/(2^24)
#define UNIT_TO_DEG32 0.00000008381903171539306640625L // 360/(2^32)

/* used for sorting polygons into draw order */
typedef struct
{
	class MSGPXMap *map;
	const char *rstart;
	unsigned int polytype;
	unsigned int numpoints;
	double thickness;
	int pixlen;
	kGUIDPoint2 *points;
	kGUICorners corners;
	int numlabels;				/* 0 to 4 */
	const char *curlabels[4];	/* pointer to current label */
}POLYSORT_DEF;

typedef struct
{
	int x,y;
	int icon;
}ICON_POS;

#if 0
typedef struct
{
	int dlong;
	int dlat;
}MSCOORD;

typedef struct
{
	MSCOORD nw;
	MSCOORD se;
}MSBOUND;
#else

class MSCOORD
{
public:
	MSCOORD() {dlong=0;dlat=0;}
	int dlong;
	int dlat;
};

class MSBOUND
{
public:
	MSBOUND() {}
	MSCOORD nw;
	MSCOORD se;
};


#endif

class MSFAT
{
public:
	MSFAT() {size=0;blockstart=0;blockend=0;}
	kGUIString filename;
	kGUIString filetype;
	long size;
	int blockstart;
	int blockend;
};

class MSLEVEL
{
public:
	MSLEVEL() {zoom=0;inherit=0;cbits=0;nsubdivisions=0;}
	unsigned char zoom;
	unsigned char inherit;
	unsigned char cbits;
	unsigned short nsubdivisions;
	bool drawlevel;	/* level contains draw primitives */
};

class MSSUBDIV
{
public:
	MSSUBDIV() {elements=0;last=0;next_level_idx=0;shiftby=0;}
	const char *from;	/* used for resizing when locked */
	int level;
	int elements;
	int rgn_offset;
	int rgn_end;
	MSCOORD center;
	MSBOUND boundary;
	int last;
	int next_level_idx;
	int shiftby;
//	const char *rgnoffsets[5];

	const char *m_pntstart;
	const char *m_pntend;

	const char *m_idxstart;
	const char *m_idxend;

	const char *m_linestart;
	const char *m_lineend;

	const char *m_polystart;
	const char *m_polyend;
};

// max in city select v7 found was 0x0e80
#define MAXPP 8192

class MSGPXChild
{
public:
	MSGPXChild() {m_map=0;}
	~MSGPXChild();
	int m_mapnum;
	class MSGPXMap *m_map;
	GPXCoord m_nw;
	GPXCoord m_se;
};

enum
{
ROADGROUP_LINE,
ROADGROUP_CREEK,
ROADGROUP_NARROWSTREET,
ROADGROUP_STREET,
ROADGROUP_RAMPS,
ROADGROUP_COLLECTOR,
ROADGROUP_HIGHWAY,
ROADGROUP_NUM};

#define MAXLABELS 4

/* Garmin mapsource map */
class MSGPXMap : public GPXMap
{
public:
	MSGPXMap(const char *fn);
	~MSGPXMap();
	void ToMap(class GPXCoord *c,int *sx,int *sy);
	void ToMap(class GPXCoord *c,double *sx,double *sy);
	void FromMap(int sx,int sy,class GPXCoord *c);
	int DrawTile(int tx,int ty);			/* draw the tile to the current display */
	bool DrawTile(int level);
	int GetNumCopyrightLines(void) {return m_numcopyrightlines;}
	const char *GetCopyrightLine(int l) {return m_copyrights.GetEntry(l)->GetString();}

	static void Init(void);
	static void Purge(void);
	static void GetLongName(const char *fn,kGUIString *longname);
	static unsigned char ReadU8(const char *fp);
	static unsigned short ReadU16(const char *fp);
	static unsigned int ReadU24(const char *fp);
	static unsigned int ReadU32(const char *fp);
	static int Read32(const char *fp);
	static int Read24(const char *fp);
	static short Read16(const char *fp);

	//this func is also used by the openstreetmap renderer
	static void DrawTrainTracks(int nvert,kGUIDPoint2 *point);
private:
	MSFAT *LocateFile(const char *name,const char *type);
	MSFAT *LocateType(const char *type);
	void LoadSub(MSSUBDIV *sub,const char *fp,int shiftby);
	void ReLoadSub(MSSUBDIV *sub,int shiftby);
	void SetLevelShift(int l,int b);
	void DrawSub(MSSUBDIV *sub);
	void AddSubPolys(MSSUBDIV *sub);
	void AddSubPolyLines(MSSUBDIV *sub);
	void DrawPoly(POLYSORT_DEF *ps);
	void DrawRoadGroupLabels(POLYSORT_DEF *ps);
	void DrawPolyLabel(POLYSORT_DEF *ps);
	void CalcSubRegions(MSSUBDIV *sub);
	double ToDegrees(int mapunit) {return (double) mapunit * UNIT_TO_DEG;}
	const char *ReadPoint(MSSUBDIV *sub,const char *rstart);
	const char *ReadPoly(MSSUBDIV *sub,const char *rstart,int type);
	int GetPoint(kGUIBitStream *bs,int nbits,int sign);
	
	kGUIText m_t[MAXLABELS];	/* temp used in drawing */
	void ReadLabel(const char *enc,kGUIString *s);
	static void DrawLabel(kGUIText *t,double lx,double ly,double lw,double lh,double heading,bool clipedge);
	void DrawLineLabel(kGUIText *s,int nvert,kGUIDPoint2 *point,double over,bool root);
//	void DrawPolyLabel(kGUIText *s,int nvert,kGUIPoint2 *point);

	const char *m_filedata;
	int m_blocksize;	/* bytes per block */
	int m_rgnoffset;
	int m_rgnsize;

	double m_pixelsPerLonDegree[MAXMSZOOM];
	double m_negpixelsPerLonRadian[MAXMSZOOM];
	double m_bitmapOrigo[MAXMSZOOM];
	int m_numTiles[MAXMSZOOM];
	
	MSBOUND m_boundary;

	int m_numfats;
	Array<MSFAT *>m_fats;

	int m_numdrawlevels;
	Array<int>m_drawlevels;

	int m_numlevels;
	Array<MSLEVEL *>m_levels;

	int m_numsubdivs;
	Array<MSSUBDIV *>m_subdivs;

	int m_numpoints;	/* numpoints in last read poly */
	unsigned int m_polytype;		/* type of last read poly */
	int m_pixlen;		/* length of last read polygons perimiter */
	kGUICorners m_polycorners;

	static unsigned int m_numsortpolys;
	static Array<POLYSORT_DEF>m_sortpolys;

	static unsigned int m_roadgroupspolys[ROADGROUP_NUM];
	static Array<POLYSORT_DEF>m_roadgroups[ROADGROUP_NUM];
//	static PolyGroup m_roadgroups[ROADGROUP_NUM];

	static Heap m_sortpolysheap;
	static int SortPolygonsType(const void *v1,const void *v2);
	static int SortPolygonsLeft(const void *v1,const void *v2);

	static int m_numiconsdrawn;
	static Array<ICON_POS> m_iconpos;

	int m_numchildren;
	MSGPXChild *m_children;	/* array of children classes */

	/* long map name extracted from tdb file */
	kGUIString m_name;

	/* label stuff */
	const  char *m_labelstart;
	int m_labelsize;
	int m_labelencoding;
	int m_labelshift;

	/* net stuff */
	const char *m_netstart;
	int m_netsize;
	int m_netshift;

	/* copyright messages */
	int m_numcopyrightlines;
	Array<kGUIString *>m_copyrights;

	/* list of subs over current tile */
	static unsigned int m_numdrawsubs;
	static Array<MSGPXMap *>m_drawmaps;
	static Array<MSSUBDIV *>m_drawsubdivs;
	static kGUICorners m_tilecorners;
	static int m_tx;
	static int m_ty;
	
	int m_numlabels;			/* 0 to 4 */
	const char *m_curlabels[4];	/* pointer to current label */
	static kGUIDrawSurface *m_lcwindow;	/* label collision window */
	static kGUICorners m_lcbounds;

	static MSCOORD m_points[MAXPP];
	static kGUIDPoint2 m_ppoints[MAXPP];

	static int m_labelicon;				/* icon for last read label */
	static Array<kGUIImage *>m_icons;
};

/* used to get the filename for a given mapnumber */
class MSGPXFName
{
public:
	static void Load(const char *path);
	static void Purge(void);
	static const char *GetFName(int mapnum);
private:
	static Hash *m_hash;
};

#endif
