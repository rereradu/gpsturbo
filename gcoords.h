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

#ifndef __GCOORDS__
#define __GCOORDS__

#define GRID_ZONE_LENGTH 4
#define LOWER_EPS_LIMIT 1e-14	/* kind of arbitrary... */

#define CLARKE_1866_DATUM 0
#define GRS_80_DATUM      1
#define WGS_84_DATUM      2

class GPXUTM
{
public:
	double m_x;
	double m_y;
	char m_grid_zone[GRID_ZONE_LENGTH];
};

class GPXLL
{
public:
	void ToUTM(GPXUTM *utm, int datum);
	void FromUTM(GPXUTM *utm, int datum);

	double m_longitude;
	double m_latitude;
private:
	void get_grid_zone(char grid_zone[GRID_ZONE_LENGTH], double *lambda0);
	int get_lambda0(char grid_zone[GRID_ZONE_LENGTH], double *lambda0);
};

/* lat or lon */
class GPXUnit
{
public:
	GPXUnit() {m_value=0.0f;}
	void ToString(const char *axis,kGUIString *str,bool noseconds);
	void Set(double v) {m_value=v;}
	void Set(const char *a);
	double GetValue(void) {return m_value;}
private:
	double m_value;
};

class GPXCoordBox
{
public:
	void Set(double uplat,double downlat,double leftlon,double rightlon) {m_uplat=uplat;m_downlat=downlat;m_leftlon=leftlon;m_rightlon=rightlon;}
	bool Inside(class GPXCoord *c);
private:
	double m_uplat;
	double m_downlat;
	double m_leftlon;
	double m_rightlon;
};

class GPXCoord
{
public:
	void Set(const char *lat,const char *lon) {m_lat.Set(lat);m_lon.Set(lon);}
	void Set(double lat,double lon) {m_lat.Set(lat);m_lon.Set(lon);}
	void Set(GPXCoord *c) {Set(c->GetLat(),c->GetLon());}
	double Dist(class GPXCoord *c2);	/* quick distance (sphere) in kms */
	void DistandHeadings(class GPXCoord *c2,double earth1,double earth2,double *dist,double *heading12,double *heading21);
	void Project(double distnm,double heading,double earth1,double earth2,GPXCoord *dest);
	void GetMagneticDev(double *dec, double *dip, double *ti, double *gv);
	void CalcSun(int day,int month,int year,double zenith,double *sunrise,double *sunset);
	void SetLat(double lat) {m_lat.Set(lat);}
	void SetLon(double lon) {m_lon.Set(lon);}
	double GetLat(void) {return m_lat.GetValue();}
	double GetLon(void) {return m_lon.GetValue();}

	/* convert lat/lon to string N49.000.000 or W122.000.000 etc. */
	void OutputLat(kGUIString *lat,bool noseconds=false) {m_lat.ToString("NS",lat,noseconds);}
	void OutputLon(kGUIString *lon,bool noseconds=false) {m_lon.ToString("EW",lon,noseconds);}
	void Output(kGUIString *lat,kGUIString *lon) {OutputLat(lat);OutputLon(lon);}
private:
	GPXUnit m_lon;
	GPXUnit m_lat;
};

/* add point to a bound box */
class GPXBounds
{
public:
	GPXBounds() {Init();}
	~GPXBounds() {}
	void Init(void) {m_first=true;m_minlat=0.0f;m_maxlat=0.0f;m_minlon=0.0f;m_maxlon=0.0f;}
	void Add(double lat,double lon);
	void Add(GPXCoord *c) {Add(c->GetLat(),c->GetLon());}
	void Add(GPXBounds *b) {Add(b->m_minlat,b->m_minlon);Add(b->m_maxlat,b->m_maxlon);}
	double GetMinLat(void) {return m_minlat;}
	double GetMaxLat(void) {return m_maxlat;}
	double GetMinLon(void) {return m_minlon;}
	double GetMaxLon(void) {return m_maxlon;}
private:
	bool m_first;
	double m_minlat;
	double m_maxlat;
	double m_minlon;
	double m_maxlon;
};

#endif
