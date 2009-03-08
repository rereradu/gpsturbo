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

#ifndef __OSMMAP__
#define __OSMMAP__

#include "map.h"

enum
{
ACCESS_FALSE,
ACCESS_PERMISSIVE,
ACCESS_PRIVATE,
ACCESS_TRUE,
ACCESS_RESTRICTED,
AEROWAY_AERODROME,
AEROWAY_APRON,
AEROWAY_RUNWAY,
AEROWAY_TAXIWAY,
AERIALWAY_UNKNOWN,
AGRICULTURAL_TRUE,
AMENITY_AIRPORTTERMINAL,
AMENITY_AMBULANCESTATION,
AMENITY_ARTGALLERY,
AMENITY_ATTRACTION,
AMENITY_BANK,
AMENITY_BENCH,
AMENITY_BICYCLEPARKING,
AMENITY_BOATLAUNCH,
AMENITY_BUILDING,
AMENITY_BUSSTATION,
AMENITY_CAFE,
AMENITY_CAMPSITE,
AMENITY_CARSHOP,
AMENITY_CASINO,
AMENITY_CEMETERY,
AMENITY_CHAIRLIFT,
AMENITY_CHURCH,
AMENITY_COFFEESHOP,
AMENITY_COLLEGE,
AMENITY_COMMERCIAL,
AMENITY_COMMUNITYCENTER,
AMENITY_CONVENIENCE,
AMENITY_COURTHOUSE,
AMENITY_CULTURALCENTER,
AMENITY_DAM,
AMENITY_DIYSHOP,
AMENITY_DOCK,
AMENITY_DAYCARE,
AMENITY_FARM,
AMENITY_FASTFOOD,
AMENITY_FERRYTERMINAL,
AMENITY_FIRESTATION,
AMENITY_FOUNTAIN,
AMENITY_FUEL,
AMENITY_FUNERALHOME,
AMENITY_GARBAGEDUMP,
AMENITY_GARDEN,
AMENITY_GOLFCOURSE,
AMENITY_GROCERY,
AMENITY_HANGAR,
AMENITY_HOCKEYRINK,
AMENITY_HOSPITAL,
AMENITY_HOTEL,
AMENITY_INDUSTRIAL,
AMENITY_INFORMATION,
AMENITY_LANDFILL,
AMENITY_LIBRARY,
AMENITY_MAILBOX,
AMENITY_MARINA,
AMENITY_MONUMENT,
AMENITY_MOTEL,
AMENITY_MUSEUM,
AMENITY_OFFICE,
AMENITY_PARKING,
AMENITY_PHARMACY,
AMENITY_PIPELINE,
AMENITY_PIER,
AMENITY_PLAYGROUND,
AMENITY_POLICE,
AMENITY_POSTOFFICE,
AMENITY_PRISON,
AMENITY_PUB,
AMENITY_PUBLICBUILDING,
AMENITY_QUARRY,
AMENITY_RECREATIONCENTER,
AMENITY_RECYCLING,
AMENITY_RESIDENTIAL,
AMENITY_RESTRAUNT,
AMENITY_RETAIL,
AMENITY_SCHOOL,
AMENITY_SERVICEYARD,
AMENITY_SHOPPING,
AMENITY_STADIUM,
AMENITY_SUPERMARKET,
AMENITY_SWIMMINGPOOL,
AMENITY_TELEPHONE,
AMENITY_TENNISCOURT,
AMENITY_THEATRE,
AMENITY_TRAINSTATION,
AMENITY_TOILETS,
AMENITY_TOWNHALL,
AMENITY_TOWER,
AMENITY_SPORTSTRACK,
AMENITY_UNDERGROUNDPARKING,
AMENITY_UNIVERSITY,
AMENITY_WATERPARK,
AMENITY_WATERTOWER,
AMENITY_VIEWPOINT,
AMENITY_ZOO,
AREA_TRUE,
AREA_FALSE,
BICYCLE_TRUE,
BICYCLE_FALSE,
BOAT_TRUE,
BOAT_FALSE,
BORDER_COUNTY,
BORDER_PROVINCE,
BORDER_COUNTRY,
BOUNDARY_ADMINISTRATIVE,
BRIDGE_TRUE,
BRIDGE_FALSE,
BUILDING_TRUE,
BUS_TRUE,
BUS_FALSE,
CAR_TRUE,
CAR_FALSE,
CLASS_MINOR,
CLASS_PRIMARY,
CLASS_SECONDARY,
CONDITION_DEFICIENT,
CONDITION_FAIR,
CONDITION_GOOD,
CONDITION_INTOLERABLE,
CONSTRUCTION_PRIMARY,
CONSTRUCTION_RESIDENTIAL,
CROSSING_TRAFFIC_SIGNALS,
CROSSING_UNCONTROLLED,
CYCLEWAY_LANE,
CYCLEWAY_TRACK,
CYCLEWAY_TRUE,
DENOMINATION_ADVENTIST,
DENOMINATION_ALLIANCE,
DENOMINATION_ANGLICAN,
DENOMINATION_BAHAI,
DENOMINATION_BAPTIST,
DENOMINATION_BUDDHIST,
DENOMINATION_CANADIANREFORMED,
DENOMINATION_CATHOLIC,
DENOMINATION_CHRISTIAN,
DENOMINATION_CHRISTIANSCIENCE,
DENOMINATION_COPTIC,
DENOMINATION_EVANGELICAL,
DENOMINATION_EVANGELICALFREE,
DENOMINATION_GOSPEL,
DENOMINATION_GREEKORTHODOX,
DENOMINATION_INDEPENDENT,
DENOMINATION_ISLAM,
DENOMINATION_JEHOVAHSWITNESS,
DENOMINATION_JEWISH,
DENOMINATION_LUTHERAN,
DENOMINATION_MENNONITE,
DENOMINATION_METHODIST,
DENOMINATION_MORMON,
DENOMINATION_NORTHAMERICANBAPTIST,
DENOMINATION_ORTHODOX,
DENOMINATION_PENTECOSTAL,
DENOMINATION_PRESBYTERIAN,
DENOMINATION_PROTESTANT,
DENOMINATION_ROMANCATHOLIC,
DENOMINATION_RUSSIANORTHODOX,
DENOMINATION_SALVATIONARMY,
DENOMINATION_SEVENTHDAYADVENTIST,
DENOMINATION_UKRANIANBAPTIST,
DENOMINATION_UKRANIANCATHOLIC,
DENOMINATION_UKRANIANORTHODOX,
DENOMINATION_UNITED,
DENOMINATION_WESLEYAN,
DIRECTION_BACKWARD,
DIRECTION_FORWARD,
DIRECTION_NORTHEAST,
DIRECTION_SOUTHEAST,
DIRECTION_NORTHWEST,
DIRECTION_SOUTHWEST,
DISTANCE_MARKER_TRUE,
DISPENSING_TRUE,
EMERGENCY_TRUE,
EMERGENCY_FALSE,
FEE_TRUE,
FEE_FALSE,
FOOT_TRUE,
FOOT_FALSE,
FOOT_DESIGNATED,
FOOT_PERMISSIVE,
GOODS_TRUE,
GOODS_FALSE,
HIGHWAY_BRIDLEWAY,
HIGHWAY_BRIDGE,
HIGHWAY_BUS_GUIDEWAY,
HIGHWAY_BUS_STOP,
HIGHWAY_BYWAY,
HIGHWAY_CATTLEGRID,
HIGHWAY_CONSTRUCTION,
HIGHWAY_CROSSING,
HIGHWAY_CYCLEWAY,
HIGHWAY_EMERGENCY_ACCESS_POINT,
HIGHWAY_FOOTWAY,
HIGHWAY_FORD,
HIGHWAY_GATE,
HIGHWAY_INCLINE,
HIGHWAY_INCLINE_STEEP,
HIGHWAY_LIVING_STREET,
HIGHWAY_MINI_ROUNDABOUT,
HIGHWAY_MINOR,
HIGHWAY_MOTORWAY,
HIGHWAY_MOTORWAY_LINK,
HIGHWAY_MOTORWAYJUNCTION,
HIGHWAY_PROPOSED,
HIGHWAY_PATH,
HIGHWAY_PEDESTRIAN,
HIGHWAY_PRIMARY,
HIGHWAY_PRIMARYLINK,
HIGHWAY_RESIDENTIAL,
HIGHWAY_ROAD,
HIGHWAY_SECONDARY,
HIGHWAY_SECONDARYLINK,
HIGHWAY_SERVICE,
HIGHWAY_SERVICES,
HIGHWAY_SPEEDBUMP,
HIGHWAY_STEPS,
HIGHWAY_STILE,
HIGHWAY_STOP,
HIGHWAY_TERTIARY,
HIGHWAY_TOLLBOOTH,
HIGHWAY_TRACK,
HIGHWAY_TRAILHEAD,
HIGHWAY_TRAFFIC_SIGNALS,
HIGHWAY_TRUNK,
HIGHWAY_TRUNK_LINK,
HIGHWAY_TURNING_CIRCLE,
HIGHWAY_UNCLASSIFIED,
HIGHWAY_UNPAVED,
HORSE_TRUE,
HORSE_FALSE,
HGV_TRUE,		//heavy goods vehicle
HGV_FALSE,
JUNCTION_TRUE,
JUNCTION_ROUNDABOUT,
JUNCTION_TRAFFICLIGHT,
LAYER_N5,
LAYER_N4,
LAYER_N3,
LAYER_N2,
LAYER_N1,
LAYER_0,
LAYER_1,
LAYER_2,
LAYER_3,
LAYER_4,
LAYER_5,
LCN_TRUE,		//local cycle route
LEISURE_BENCH,
LEISURE_PARK,
LEISURE_PITCH,
LEISURE_RECREATIONAREA,
LEVEL_1,
LEVEL_2,
LINK_TRUE,
PLACE_AIRPORT,
PLACE_CITY,
PLACE_COUNTY,
PLACE_HAMLET,
PLACE_SUBURB,
PLACE_TOWN,
PLACE_VILLAGE,
POWER_GENERATOR,
POWER_LINE,
POWER_SUBSTATION,
POWER_TOWER,
POWERSOURCE_WIND,
PSV_TRUE,			/* no cars, pedestrians only */
PSV_FALSE,
NATURAL_BEACH,
NATURAL_CANAL,
NATURAL_CAVE,
NATURAL_COASTLINE,
NATURAL_DRAIN,
NATURAL_GRASS,
NATURAL_FIELD,
NATURAL_FOREST,
NATURAL_LAND,
NATURAL_LAKE,
NATURAL_MARSH,
NATURAL_PEAK,
NATURAL_SCRUB,
NATURAL_RESERVOIR,
NATURAL_RIVER,
NATURAL_RIVERBANK,
NATURAL_STREAM,
NATURAL_WATER,
NATURAL_WATERFALL,
NCN_TRUE,		//national cycle route
NOEXIT_TRUE,
NOEXIT_FALSE,
ONEWAY_TRUE,
ONEWAY_FALSE,
ONEWAY_REVERSIBLE,
PASSENGER_TRUE,
PROPOSED_RESIDENTIAL,
PROPOSED_PRIMARYLINK,
LANES_HALF,
LANES_1,
LANES_2,
LANES_3,
LANES_4,
LANES_5,
LANES_6,
MOTORCYCLE_TRUE,
MOTORCYCLE_FALSE,
RAILWAY_ABANDONED,
RAILWAY_CROSSING,
RAILWAY_HALT,
RAILWAY_LAND,
RAILWAY_LEVELCROSSING,
RAILWAY_LIGHTRAIL,
RAILWAY_MONORAIL,
RAILWAY_PROPOSED,
RAILWAY_RAIL,
RAILWAY_SPUR,
RAILWAY_STATION,
RAILWAY_SUBWAY,
RAILWAY_SUBWAYENTRANCE,
RAILWAY_TRAM,
RAILWAY_UNKNOWN,
RAILWAY_YARD,
RCN_TRUE,
ROUTE_BUS,
ROUTE_FERRY,
ROUTE_SKI,
SEPERATED_TRUE,
SEPERATED_FALSE,
SHELTER_TRUE,
SHELTER_FALSE,
SIZE_HUGE,
SIZE_LARGE,
SIZE_MEDIUM,
SPORT_BASKETBALL,
SPORT_BASEBALL,
SPORT_FOOTBALL,
SPORT_SOCCER,
SPORT_TENNIS,
SURFACE_ASPHALT,
SURFACE_DIRT,
SURFACE_GRASS,
SURFACE_GRAVEL,
SURFACE_PAVED,
SURFACE_UNPAVED,
SURFACE_WOOD,
TAXI_TRUE,
TAXI_FALSE,
TOLL_TRUE,
TOLL_FALSE,
TRACKTYPE_GRADE1,
TRACKTYPE_GRADE2,
TRACKTYPE_GRADE3,
TRACKTYPE_GRADE4,
TRACKS_2,
TRUCKROUTE_TRUE,
TUNNEL_TRUE,
TUNNEL_FALSE,
TYPE_CMB,
WHEELCHAIR_FALSE,
WHEELCHAIR_TRUE,
OSM_NUMTAGS
};

#if 1
#define MINOSMZOOM 3
#define MAXOSMZOOM 19

typedef struct
{
	GPXCoord c;
}OSMNODE_DEF;

typedef struct
{
	unsigned int numtags;
	unsigned int tags[6];
	int rendertype;
	int rendersubtype;
	kGUIColor colour;
	kGUIColor colour2;
}OSMRENDER_INFO;

class OSMWAY
{
public:
	OSMWAY() {m_ori=0;}
	unsigned int m_numnodes;
	kGUICorners m_corners;
	GPXCoord m_min;
	GPXCoord m_max;
	OSMNODE_DEF **m_nodes;
	const OSMRENDER_INFO *m_ori;
	kGUIFPoint2 *m_proj;		/* used for pass 2 */
};

#define MAXOPP 8192

/* section node definition */
typedef struct
{
	GPXCoord c;
	char *m_name;
	const OSMRENDER_INFO *m_render;
}OSMSECNODE_DEF;

typedef struct
{
	unsigned int m_numcoords;
	bool m_closed:1;
	char *m_name;
	const OSMRENDER_INFO *m_render;
	GPXCoord *m_coords;

	/* optimize stuff */
	GPXCoord m_min;
	GPXCoord m_max;
	kGUIFPoint2 *m_proj;				/* used for pass 2 */
	int m_pixlen;						/* used for pass 3 drawing street names */
}OSMSECWAY_DEF;


/* smaller area of map */
class OSMSection
{
	friend class OSMMap;
public:
	OSMSection();
	~OSMSection();
	void LoadHeader(DataHandle *dh);
	void Load(DataHandle *dh);
private:
	bool m_loaded;
	bool m_current;
	unsigned int m_numnodes;
	unsigned int m_numways;
	GPXCoord m_nw;
	GPXCoord m_se;
	double m_latscale,m_lonscale;
	unsigned int m_offset;
	unsigned int m_packedlength;
	unsigned int m_unpackedlength;
	OSMSECNODE_DEF *m_node;
	OSMSECWAY_DEF *m_way;
	Heap m_heap;
};

/* openstreetmap.com map ( xml format ) */
class OSMMap : public GPXMap
{
public:
	OSMMap(const char *fn);
	~OSMMap();
	void ToMap(class GPXCoord *c,int *sx,int *sy);
	void FromMap(int sx,int sy,class GPXCoord *c);
	int DrawTile(int tx,int ty);	/* draw the tile to the current display */
	int GetNumCopyrightLines(void) {return 1;}
	const char *GetCopyrightLine(int l) {return "\xa9 www.openstreetmap.org Creative Commons Attribution-ShareAlike 2.0 license";}

private:
	void Translate(unsigned int numpoints,kGUIFPoint2 *in,kGUIFPoint2 *out,float angle,float length);
	void DrawLineLabel(kGUIText *t,int nvert,kGUIFPoint2 *point,double over,bool root);
	void DrawLabel(kGUIText *t,double lx,double ly,double lw,double lh,double heading,bool clipedge);
#if DRAWDEBUG
	void DebugDrawType(OSMSECWAY_DEF *w,kGUICorners *wc);
#endif
	double m_pixelsPerLonDegree[MAXOSMZOOM];
	double m_negpixelsPerLonRadian[MAXOSMZOOM];
	double m_bitmapOrigo[MAXOSMZOOM];

	unsigned int m_numsections;
	ClassArray<OSMSection>m_sections;
	kGUIString m_filename;
	Heap m_drawheap;

	unsigned int m_numdrawways;
	Array<OSMSECWAY_DEF *>m_drawways;
	static kGUIFPoint2 m_ppoints[MAXOPP];
	static kGUIFPoint2 m_ppoints2[MAXOPP];

	int m_txpix;
	int m_typix;
	GPXMapStrings m_lc;
	kGUIText m_t;
	int m_pixlen;
};

#endif

#define MAXTAGS 16

/* coverter node definition */
typedef struct
{
	double m_lat;
	double m_lon;
	unsigned int m_renderindex;
	char *m_name;
	bool m_export;				/* assigned when writing out file */
}OSMCONVNODE_DEF;

typedef struct
{
	unsigned int m_renderindex;
	bool m_closed;
	unsigned int m_numnodes;
	char *m_name;
	OSMCONVNODE_DEF **m_nodes;
}OSMCONVWAY_DEF;

class OSMConvert;

class OSMConvertSection
{
	friend class OSMConvert;
public:
	OSMConvertSection(OSMConvert *parent);
	~OSMConvertSection();
	void Split(void);
	unsigned int WriteHeader(FILE *f,unsigned int);
	void Write(FILE *f);
private:
	OSMConvert *m_parent;
	unsigned int m_numnodes;
	unsigned int m_numexnodes;
	unsigned int m_numways;
	double m_minlat,m_maxlat,m_minlon,m_maxlon;
	double m_latscale,m_lonscale;
	Array<OSMCONVNODE_DEF *>m_nodeptrs;
	Array<OSMCONVWAY_DEF *>m_wayptrs;
	unsigned int m_packedlength;
	unsigned int m_unpackedlength;
	Array<unsigned char>m_packedbuffer;
};

typedef struct
{
	unsigned int id;
	const char *tag;
}OSMTAG_DEF;

class OSMConvert : public kGUIXML
{
	friend class OSMConvertSection;
public:
	OSMConvert(const char *filename);
	~OSMConvert();

	void Init(void);
	void ChildLoaded(kGUIXMLItem *child,kGUIXMLItem *parent);

//	kGUIString *GetOutput(void) {return &m_output;}
	void PrintUnknownPairs(void);
	void PrintUnknownRender(void);

	void AddSection(OSMConvertSection *s) {m_section.SetEntry(m_numsections++,s);}

	void InitRenderLookup(void);
	unsigned int GetRenderIndex(unsigned int numtags,OSMTAG_DEF **tags);
private:
	void WindowEvent(kGUIEvent *event);
	CALLBACKGLUEPTR(OSMConvert,WindowEvent,kGUIEvent);
	void StopEvent(kGUIEvent *event);
	CALLBACKGLUEPTR(OSMConvert,StopEvent,kGUIEvent);
	void Update(void);
	CALLBACKGLUE(OSMConvert,Update);
	void ConvertThread(void);
	CALLBACKGLUE(OSMConvert,ConvertThread);
	void LoadShapefile(FILE *f);
	kGUIWindowObj m_window;
	kGUIBusyRectObj m_busyrect;
	kGUIInputBoxObj m_status;
	kGUIButtonObj m_stop;

	kGUIString m_filename;
	kGUIThread m_thread;
	kGUICommStack<kGUIString *> m_comm;
	volatile bool m_abort;

	static int SortUnknown(const void *v1,const void *v2);
	void AddUnknownPair(const char *s);
	void AddUnknownRender(const char *s);
	unsigned int m_printcount;
	Hash m_taghash;
	Hash m_taggroupspecialhash;
	Hash m_tagpairhash;
	Hash m_unknownpair;		/* used to stop duplicate reporting of unknown tag pairs */
	Hash m_unknownrender;		/* used to stop duplicate reporting of unknown renders */
	Heap m_heap;
	kGUIString m_pair;
	Hash m_nodehash;
	Array<OSMCONVNODE_DEF *>m_tempnodeptrs;
	Array<OSMCONVNODE_DEF *>m_tempnodeptrs2;

	OSMConvertSection *m_rs;
	unsigned int m_numsections;
	Array<OSMConvertSection *>m_section; 

	bool m_hastag[OSM_NUMTAGS];
};


#endif
