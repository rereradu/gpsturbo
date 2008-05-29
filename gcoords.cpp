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

#include "kgui.h"
#include "gcoords.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2 (M_PI*2.0f)
#endif

//Hash GPXCoord::m_hash;

void GPXUnit::Set(const char *a)
{
	bool neg;
	int h,m,s;
	double c;

	m_value=0;
	if(!a[0])
		return;
	if((a[0]=='W') || (a[0]=='S'))
		neg=true;
	else
		neg=false;
	++a;
	while(a[0]==' ')
		++a;

	if(!(a[0]>='0' && a[0]<='9'))
		return;
	h=a[0]-'0';
	++a;
	while(a[0]>='0' && a[0]<='9')
	{
		h=h*10+(a[0]-'0');
		++a;
	}
	if(a[0]!=' ' && a[0]!='.')
		return;
	++a;
	if(!(a[0]>='0' && a[0]<='9'))
		return;
	m=a[0]-'0';
	++a;
	while(a[0]>='0' && a[0]<='9')
	{
		m=m*10+(a[0]-'0');
		++a;
	}
	if(a[0]!='.' && a[0]!=' ')
		return;
	++a;
	if(!(a[0]>='0' && a[0]<='9'))
		return;
	s=a[0]-'0';
	++a;
	while(a[0]>='0' && a[0]<='9')
	{

		s=s*10+(a[0]-'0');
		++a;
	}

	/* got h,m,s */
	c=(double)h+((double)m/60.0f)+((double)s/60000.0f);
	if(neg==true)
		m_value=-c;
	else
		m_value=c;
}

void GPXUnit::ToString(const char *axis,kGUIString *str,bool noseconds)
{
	int h,m,s;
	char e;
	double coord=m_value;

	if(coord>=0.0f)
		e=axis[0];
	else
	{
		e=axis[1];
		coord=-coord;
	}

	h=(int)coord;
	coord-=(double)h;
	coord*=60.0f;

	if(noseconds==true)
	{
		m=int(coord+0.5f);
		if(m==60)
		{
			++h;
			m=0;
		}
		str->Sprintf("%c %d %02d",e,h,m);		/* deg/min only */
	}
	else
	{
		m=int(coord);
		coord-=(double)m;
		coord*=1000;
		s=int(coord+0.5f);
		if(s==1000)
		{
			s=0;
			if(++m==60)
			{
				m=0;
				++h;
			}
		}
		str->Sprintf("%c %d %02d.%03d",e,h,m,s);		/* deg/min/sec */
	}
}

/* calculate distance between 2 lon/lat points in km */

//3963.1 statute miles
//3443.9 nautical miles
//6378 km

double GPXCoord::Dist(class GPXCoord *c2)
{
	double a,b,c,d;
	double angle;
	double result;
#if 0
	double h[4];
	double *cd;

	/* if this already in the cache? */
	h[0]=m_lat.m_value;
	h[1]=m_lon.m_value;
	h[2]=c2->m_lat.m_value;
	h[3]=c2->m_lon.m_value;
	cd=(double *)m_hash.Find((const char *)h);
	if(cd)
		return(*cd);
#endif

	a=GetLat()/57.29577951f;	/* convert to radians */
	b=GetLon()/57.29577951f;	/* convert to radians */
	c=c2->GetLat()/57.29577951f;	/* convert to radians */
	d=c2->GetLon()/57.29577951f;	/* convert to radians */

	if(a==c && b==d)
		result=0.0f;				/* same! */
	else
	{
		angle=sin(a)*sin(c)+cos(a)*cos(c)*cos(b-d);
		if(angle>1.0f)
			result=6378.0f*acos(1.0f);
		else
			result=6378.0f*acos(angle);
	}
#if 0
	m_hash.Add((const char *)h,&result);
#endif
	return(result);
}

/*******************************************************************************/

/*
 * Peter Daly
 * MIT Ocean Acoustics
 * pmd@mit.edu
 * 25-MAY-1998
 * 
 * These routines convert UTM to Lat/Longitude and vice-versa,
 * using the WGS-84 (GPS standard) or Clarke 1866 Datums.
 * 
 * The formulae for these routines were originally taken from
 * Chapter 10 of "GPS: Theory and Practice," by B. Hofmann-Wellenhof,
 * H. Lictenegger, and J. Collins. (3rd ed) ISBN: 3-211-82591-6,
 * however, several errors were present in the text which
 * made their formulae incorrect.
 *
 * Instead, the formulae for these routines was taken from
 * "Map Projections: A Working Manual," by John P. Snyder
 * (US Geological Survey Professional Paper 1395)
 *
 * Copyright (C) 1998 Massachusetts Institute of Technology
 *               All Rights Reserved
 *
 * RCS ID: $Id: convert_datum.c,v 1.3 1999/03/25 17:26:22 pmd Exp pmd $
 */

void GPXLL::get_grid_zone(char grid_zone[GRID_ZONE_LENGTH], double *lambda0)
{
  unsigned int zone_long;
  unsigned char zone_lat;

  /* Solve for the grid zone, returns the central meridian */

  /* First, let's take care of the polar regions */

  if (m_latitude < -80) {
    if (m_longitude < 0) {
      sprintf (grid_zone, "30A");
      *lambda0 = 0 * M_PI / 180.0;
    } else {
      sprintf (grid_zone, "31B");
      *lambda0 = 0 * M_PI / 180.0;
    }
    return;

  } else if (m_latitude > 84) {
    if (m_longitude < 0) {
      sprintf (grid_zone, "30Y");
      *lambda0 = 0 * M_PI / 180.0;
    } else {
      sprintf (grid_zone, "31Z");
      *lambda0 = 0 * M_PI / 180.0;
    }
    return;
  }

  /* Now the special "X" grid */

  if (m_latitude > 72 && m_longitude > 0 && m_longitude < 42) {
    if (m_longitude < 9) {
      *lambda0 = 4.5;
      sprintf (grid_zone, "31X");
    } else if (m_longitude < 21) {
      *lambda0 = 15 * M_PI / 180.0;
      sprintf (grid_zone, "33X");
    } else if (m_longitude < 33) {
      *lambda0 = 27 * M_PI / 180.0;
      sprintf (grid_zone, "35X");
    } else if (m_longitude < 42) {
      *lambda0 = 37.5 * M_PI / 180.0;
      sprintf (grid_zone, "37X");
    }
  
    return;
  }

  /* Handle the special "V" grid */

  if (m_latitude > 56 && m_latitude < 64 &&
      m_longitude > 0 && m_longitude < 12) {
    if (m_longitude < 3) {
      *lambda0 = 1.5 * M_PI / 180.0;
      sprintf (grid_zone, "31V");
    } else if (m_longitude < 12) {
      *lambda0 = 7.5 * M_PI / 180.0;
      sprintf (grid_zone, "32V");
    }
      
    return;
  }

  /* The remainder of the grids follow the standard rule */

  zone_long = (unsigned int) ((m_longitude - (-180.0)) / 6.0) + 1;
  *lambda0 = ((zone_long - 1) * 6.0 + (-180.0) + 3.0) * M_PI / 180.0;

  zone_lat = (unsigned char) ((m_latitude - (-80.0)) / 8.0) + 'C';
  if (zone_lat > 'H')
    zone_lat++;
  if (zone_lat > 'N')
    zone_lat++;

  if (m_latitude > 80)
    zone_lat = 'X';

  grid_zone[0] = ((unsigned char) (zone_long / 10)) + '0';
  grid_zone[1] = (zone_long % 10) + '0';
  grid_zone[2] = zone_lat;
  grid_zone[3] = 0;

  /* All done */
}

static bool cisdigit(char c)
{
	if(c>='0' && c<='9')
		return true;
	return false;
}

int GPXLL::get_lambda0(char grid_zone[GRID_ZONE_LENGTH], double *lambda0)
{
  unsigned int zone_long;
  unsigned char zone_lat;

  /* given the grid zone, sets the central meridian, lambda0 */

  /* Check the grid zone format */

  if (!cisdigit(grid_zone[0]) || !cisdigit(grid_zone[1]))
  {
	assert(false,"Invalid grid zone format");
	return(0);
  }

  zone_long = (grid_zone[0] - '0') * 10 + (grid_zone[1] - '0');
  zone_lat = grid_zone[2];

  /* Take care of special cases */

  switch(zone_lat) {
  case 'A': case 'B': case 'Y': case 'Z':
    *lambda0 = 0;
    return(0);
    break;
  case 'V':
    switch (zone_long) {
    case 31:
      *lambda0 = 1.5 * M_PI / 180.0;
      return(0);
      break;
    case 32:
      *lambda0 = 7.5 * M_PI / 180.0;
      return(0);
      break;
    break;
    }
  case 'X':
    switch (zone_long) {
    case 31:
      *lambda0 = 4.5 * M_PI / 180.0;
      return(0);
      break;
    case 33:
      *lambda0 = 15 * M_PI / 180.0;
      return(0);
      break;
    case 35:
      *lambda0 = 27 * M_PI / 180.0;
      return(0);
      break;
    case 37:
      *lambda0 = 37.5 * M_PI / 180.0;
      return(0);
      break;
    case 32: case 34: case 36:
      fprintf (stderr, "Zone %02d%c does not exist!\n",
	       zone_long,
	       zone_lat);
      return(-1);
      break;
    }
    break;
  }

  /* Now handle standard cases */

  *lambda0 = ((zone_long - 1) * 6.0 + (-180.0) + 3.0) * M_PI / 180.0;

  /* All done */

  return(0);
}


void GPXLL::ToUTM(GPXUTM *utm, int datum)
{
  double a=0, b=0, f, e, e2, e4, e6;
  double phi, lambda, lambda0, phi0, k0;
  double t, rho, m, x, y, k, mm, mm0;
  double aa, aa2, aa3, aa4, aa5, aa6;
  double ep2, nn, tt, cc;
  char grid_zone[GRID_ZONE_LENGTH];

  /* Converts lat/long to UTM, using the specified datum */

  switch (datum) {
  case CLARKE_1866_DATUM:
    a = 6378206.4;
    b = 6356583.8;
    break;
  case GRS_80_DATUM:
    a = 6378137;
    b = 6356752.3;
    break;
  case WGS_84_DATUM:
    a = 6378137.0;		/* semimajor axis of ellipsoid (meters) */
    b = 6356752.31425;		/* semiminor axis of ellipsoid (meters) */
    break;
  default:
		assert(false,"Unknown datum!");
	break;
  }

  /* Calculate flatness and eccentricity */

  f = 1 - (b/a);
  e2 = 2*f - f*f;
  e = sqrt(e2);
  e4 = e2*e2;
  e6 = e4*e2;

  /* Convert latitude/longitude to radians */
  
  phi = (m_latitude) * M_PI / 180.0;
  lambda = (m_longitude) * M_PI / 180.0;

  /* Figure out the UTM zone, as well as lambda0 */

  get_grid_zone (grid_zone, &lambda0);
  phi0 = 0.0;

  /* See if this will use UTM or UPS */

  if (m_latitude > 84.0) {

    /* use Universal Polar Stereographic Projection (north polar aspect) */

    k0 = 0.994;

    t = sqrt(((1-sin(phi))/(1+sin(phi))) *
	     pow((1+e*sin(phi))/(1-e*sin(phi)), e));
    rho = 2 * a * k0 * t / sqrt(pow(1+e,1+e) * pow(1-e,1-e));
    m = cos(phi) / sqrt (1 - e2 * sin(phi)*sin(phi));

    x = rho * sin(lambda - lambda0);
    y = -rho * cos(lambda - lambda0);
    k = rho * a * m;

    /* Apply false easting/northing */

    x += 2000000;
    y += 2000000;

  } else if (m_latitude < -80.0) {

    /* use Universal Polar Stereographic Projection (south polar aspect) */

    phi = -phi;
    lambda = -lambda;
    lambda0 = -lambda0;

    k0 = 0.994;

    t = sqrt(((1-sin(phi))/(1+sin(phi))) *
	     pow((1+e*sin(phi))/(1-e*sin(phi)), e));
    rho = 2 * a * k0 * t / sqrt(pow(1+e,1+e) * pow(1-e,1-e));
    m = cos(phi) / sqrt (1 - e2 * sin(phi)*sin(phi));

    x = rho * sin(lambda - lambda0);
    y = -rho * cos(lambda - lambda0);
    k = rho * a * m;

    x = -x;
    y = -y;

    /* Apply false easting/northing */

    x += 2000000;
    y += 2000000;

  } else {

    /* Use GPXUTM */

    /* set scale on central median (0.9996 for UTM) */
    
    k0 = 0.9996;

    mm = a * ((1-e2/4 - 3*e4/64 - 5*e6/256) * phi -
	      (3*e2/8 + 3*e4/32 + 45*e6/1024) * sin(2*phi) +
	      (15*e4/256 + 45*e6/1024) * sin(4*phi) -
	      (35*e6/3072) * sin(6*phi));

    mm0 = a * ((1-e2/4 - 3*e4/64 - 5*e6/256) * phi0 -
	       (3*e2/8 + 3*e4/32 + 45*e6/1024) * sin(2*phi0) +
	       (15*e4/256 + 45*e6/1024) * sin(4*phi0) -
	       (35*e6/3072) * sin(6*phi0));

    aa = (lambda - lambda0) * cos(phi);
    aa2 = aa*aa;
    aa3 = aa2*aa;
    aa4 = aa2*aa2;
    aa5 = aa4*aa;
    aa6 = aa3*aa3;

    ep2 = e2 / (1 - e2);
    nn = a / sqrt(1 - e2*sin(phi)*sin(phi));
    tt = tan(phi) * tan(phi);
    cc = ep2 * cos(phi) * cos(phi);

    k = k0 * (1 + (1+cc)*aa2/2 + (5-4*tt+42*cc+13*cc*cc-28*ep2)*aa4/24.0
	      + (61-148*tt+16*tt*tt)*aa6/720.0);
    x = k0 * nn * (aa + (1-tt+cc)*aa3/6 +
		   (5-18*tt+tt*tt+72*cc-58*ep2)*aa5/120.0);
    y = k0 * (mm - mm0 + nn * tan(phi) * 
	      (aa2/2 + (5-tt+9*cc+4*cc*cc)*aa4/24.0
	       + (61 - 58*tt + tt*tt + 600*cc - 330*ep2)*aa6/720));

    /* Apply false easting and northing */

    x += 500000.0;
    if (y < 0.0)
      y += 10000000;
  }

  /* Set entries in UTM structure */

  memcpy (utm->m_grid_zone, grid_zone, GRID_ZONE_LENGTH);
  utm->m_x = x;
  utm->m_y = y;
}

void GPXLL::FromUTM(GPXUTM *utm, int datum)
{
  double a=0, b=0, f, e, e2, e4, e6, e8;
  double lambda0, x, y, k0, rho, t, chi, phi, phi1, phit;
  double lambda, phi0, e1, e12, e13, e14;
  double mm, mm0, mu, ep2, cc1, tt1, nn1, rr1;
  double dd, dd2, dd3, dd4, dd5, dd6;

  unsigned int zone_long;
  unsigned char zone_lat;

  /* Converts UTM to lat/long, using the specified datum */

  switch (datum) {
  case CLARKE_1866_DATUM:
    a = 6378206.4;
    b = 6356583.8;
    break;
  case GRS_80_DATUM:
    a = 6378137;
    b = 6356752.3;
    break;
  case WGS_84_DATUM:
    a = 6378137.0;		/* semimajor axis of ellipsoid (meters) */
    b = 6356752.31425;		/* semiminor axis of ellipsoid (meters) */
    break;
  default:
		assert(false,"Unknown datum!");
  break;
  }

  /* Calculate flatness and eccentricity */

  f = 1 - (b/a);
  e2 = 2*f - f*f;
  e = sqrt(e2);
  e4 = e2*e2;
  e6 = e4*e2;
  e8 = e4*e4;

  /* Given the UTM grid zone, generate a baseline lambda0 */

  if (get_lambda0 (utm->m_grid_zone, &lambda0) < 0)
  {
		assert(false,"unable to translate UTM to LL");
		return;
  }

  zone_long = (utm->m_grid_zone[0] - '0') * 10 + (utm->m_grid_zone[1] - '0');
  zone_lat = utm->m_grid_zone[2];

  /* Take care of the polar regions first. */

  switch(zone_lat) {
  case 'Y': case 'Z':		/* north polar aspect */

    /* Subtract the false easting/northing */

    x = utm->m_x - 2000000;
    y = utm->m_y - 2000000;

    /* Solve for inverse equations */

    k0 = 0.994;
    rho = sqrt (x*x + y*y);
    t = rho * sqrt(pow(1+e,1+e) * pow(1-e,1-e)) / (2*a*k0);

    /* Solve for latitude and longitude */

    chi = M_PI_2 - 2 * atan(t);
    phit = chi + (e2/2 + 5*e4/24 + e6/12 + 13*e8/360) * sin(2*chi) +
      (7*e4/48 + 29*e6/240 + 811*e8/11520) * sin(4*chi) +
      (7*e6/120 + 81*e8/1120) * sin(6*chi) +
      (4279*e8/161280) * sin(8*chi);

    do {
      phi = phit;
      phit = M_PI_2 - 2*atan(t*pow((1-e*sin(phi))/(1+e*sin(phi)),e/2));
    } while (fabs(phi-phit) > LOWER_EPS_LIMIT);

    lambda = lambda0 + atan2(x,-y);
    break;

  case 'A': case 'B':		/* south polar aspect */

    /* Subtract the false easting/northing */

    x = -(utm->m_x - 2000000);
    y = -(utm->m_y - 2000000);

    /* Solve for inverse equations */

    k0 = 0.994;
    rho = sqrt (x*x + y*y);
    t = rho * sqrt(pow(1+e,1+e) * pow(1-e,1-e)) / (2*a*k0);

    /* Solve for latitude and longitude */

    chi = M_PI_2 - 2 * atan(t);
    phit = chi + (e2/2 + 5*e4/24 + e6/12 + 13*e8/360) * sin(2*chi) +
      (7*e4/48 + 29*e6/240 + 811*e8/11520) * sin(4*chi) +
      (7*e6/120 + 81*e8/1120) * sin(6*chi) +
      (4279*e8/161280) * sin(8*chi);

    do {
      phi = phit;
      phit = M_PI_2 - 2*atan(t*pow((1-e*sin(phi))/(1+e*sin(phi)),e/2));
    } while (fabs(phi-phit) > LOWER_EPS_LIMIT);

    phi = -phi;
    lambda = -(-lambda0 + atan2(x,-y));

    break;

  default:

    /* Now take care of the UTM locations */

    k0 = 0.9996;

    /* Remove false eastings/northings */

    x = utm->m_x - 500000;
    y = utm->m_y;

    if (zone_lat > 'B' && zone_lat < 'N')   /* southern hemisphere */
      y -= 10000000;

    /* Calculate the footpoint latitude */

    phi0 = 0.0;
    e1 = (1 - sqrt(1-e2))/(1 + sqrt(1-e2));
    e12 = e1*e1;
    e13 = e1*e12;
    e14 = e12*e12;

    mm0 = a * ((1-e2/4 - 3*e4/64 - 5*e6/256) * phi0 -
	       (3*e2/8 + 3*e4/32 + 45*e6/1024) * sin(2*phi0) +
	       (15*e4/256 + 45*e6/1024) * sin(4*phi0) -
	       (35*e6/3072) * sin(6*phi0));
    mm = mm0 + y/k0;
    mu = mm/(a*(1-e2/4-3*e4/64-5*e6/256));

    phi1 = mu + (3*e1/2 - 27*e13/32) * sin(2*mu) +
      (21*e12/16 - 55*e14/32) * sin(4*mu) +
      (151*e13/96) * sin(6*mu) +
      (1097*e14/512) * sin(8*mu);

    /* Now calculate lambda and phi */

    ep2 = e2/(1-e2);
    cc1 = ep2*cos(phi1)*cos(phi1);
    tt1 = tan(phi1)*tan(phi1);
    nn1 = a / sqrt(1-e2*sin(phi1)*sin(phi1));
    rr1 = a * (1-e2)/pow(1-e2*sin(phi1)*sin(phi1), 1.5);
    dd = x / (nn1 * k0);

    dd2 = dd*dd;
    dd3 = dd*dd2;
    dd4 = dd2*dd2;
    dd5 = dd3*dd2;
    dd6 = dd4*dd2;

    phi = phi1 - (nn1*tan(phi1)/rr1) *
      (dd2/2 - (5+3*tt1+10*cc1-4*cc1*cc1-9*ep2)*dd4/24 +
       (61+90*tt1+298*cc1+45*tt1*tt1-252*ep2-3*cc1*cc1)*dd6/720);
    lambda = lambda0 +
      (dd - (1+2*tt1+cc1)*dd3/6 +
       (5-2*cc1+28*tt1-3*cc1*cc1+8*ep2+24*tt1*tt1)*dd5/120)/ cos(phi1);
  }

  /* Convert phi/lambda to degrees */
  
  m_latitude = phi * 180.0 / M_PI;
  m_longitude = lambda * 180.0 / M_PI;
}

static double mod(double x,double y)
{
  return x-y*floor(x/y);
}

static double modlon(double x)
{
  return mod(x+M_PI,2.0f*M_PI)-M_PI;
}

static double modcrs(double x)
{
  return mod(x,2*M_PI);
}

static double modlat(double x)
{
  return mod(x+M_PI/2,2*M_PI)-M_PI/2;
}

void GPXCoord::DistandHeadings(class GPXCoord *c2,double earth1,double earth2,double *dist,double *heading12,double *heading21)
{
	double lat1=GetLat()*(M_PI/180.0f);
	double lon1=GetLon()*(M_PI/180.0f);
	double lat2=c2->GetLat()*(M_PI/180.0f);
	double lon2=c2->GetLon()*(M_PI/180.0f);
	double d,crs12,crs21;

	if(earth2==0.0f)
	{
		double argacos;

		lon1=-lon1;
		lon2=-lon2;
		/* sphere model */
		if ((lat1+lat2==0.0f) && (fabs(lon1-lon2)==M_PI) && (fabs(lat1) != (M_PI/180.0f)*90.0f))
		{
		//	alert("Course between antipodal points is undefined")
		}

		d=acos(sin(lat1)*sin(lat2)+cos(lat1)*cos(lat2)*cos(lon1-lon2));
		if ((d==0.0f) || (lat1==-(M_PI/180.0f)*90.0f))
		{
			crs12=2*M_PI;
		}
		else if (lat1==(M_PI/180.0f)*90.0f)
		{
			crs12=M_PI;
		}
		else
		{
			argacos=(sin(lat2)-sin(lat1)*cos(d))/(sin(d)*cos(lat1));
			if (sin(lon2-lon1) < 0)
			{       
				crs12=acosf((float)argacos);
			}
			else
			{
				crs12=2.0f*M_PI-acosf((float)argacos);
			}
		}
		if ((d==0.0f) || (lat2==-(M_PI/180.0f)*90.0f))
		{
			crs21=0.0f;
		}
		else if (lat2==(M_PI/180.0f)*90.0f)
		{
			crs21=M_PI;
		}
		else
		{
			argacos=(sin(lat1)-sin(lat2)*cos(d))/(sin(d)*cos(lat2));
			if (sin(lon1-lon2)<0.0f)
			{       
				crs21=acosf((float)argacos);
			}
			else
			{
				crs21=2.0f*M_PI-acosf((float)argacos);
			}
		}
		d=d*(180.0f/M_PI)*60.0f;
	}
	else
	{
		double glat1,glon1,glat2,glon2;
		double f,a;
		double r, tu1, tu2, cu1, su1, cu2, s1, b1, f1;
		double x, sx=0, cx=0, sy=0, cy=0,y=0, sa, c2a=0, cz=0, e=0, c;
		double EPS= 0.00000000005f;
		double faz, baz, s;
		int iter=1;
		int MAXITER=100;

		glat1=lat1;
		glat2=lat2;
		glon1=lon1;
		glon2=lon2;

		a=earth1;
		f=1.0f/earth2;

		if ((glat1+glat2==0.0f) && (fabs(glon1-glon2)==M_PI))
		{
		//	alert("Course and distance between antipodal points is undefined")
			glat1=glat1+0.00001f; // allow algorithm to complete
		}
		if (glat1==glat2 && (glon1==glon2 || fabs(fabs(glon1-glon2)-2*M_PI) <  EPS))
		{
		//	alert("Points 1 and 2 are identical- course undefined")
			crs12=0.0f;
			crs21=M_PI;
			d=0.0f;
		}
		else
		{
			r = 1.0f-f;
			tu1 = r * tan (glat1);
			tu2 = r * tan (glat2);
			cu1 = 1.0f / sqrt (1.0f + tu1 * tu1);
			su1 = cu1 * tu1;
			cu2 = 1.0f / sqrt (1.0f + tu2 * tu2);
			s1 = cu1 * cu2;
			b1 = s1 * tu2;
			f1 = b1 * tu1;
			x = glon2 - glon1;
			d = x + 1; // force one pass
			while ((fabs(d - x) > EPS) && (iter < MAXITER))
			{
				iter=iter+1;
				sx = sin (x);
				cx = cos (x);
				tu1 = cu2 * sx;
				tu2 = b1 - su1 * cu2 * cx;
				sy = sqrt(tu1 * tu1 + tu2 * tu2);
				cy = s1 * cx + f1;
				y = atan2 (sy, cy);
				sa = s1 * sx / sy;
				c2a = 1 - sa * sa;
				cz = f1 + f1;
				if (c2a > 0.0f)
					cz = cy - cz / c2a;
				e = cz * cz * 2.0f - 1.0f;
				c = ((-3.0f * c2a + 4.0f) * f + 4.0f) * c2a * f / 16.0f;
				d = x;
				x = ((e * cy * c + cz) * sy * c + y) * sa;
				x = (1. - c) * x * f + glon2 - glon1;
			}
			faz = modcrs(atan2(tu1, tu2));
			baz = modcrs(atan2(cu1 * sx, b1 * cx - su1 * cu2) + M_PI);
			x = sqrt ((1 / (r * r) - 1) * c2a + 1);
			x +=1;
			x = (x - 2.0f) / x;
			c = 1.0f - x;
			c = (x * x / 4.0f + 1.0f) / c;
			d = (0.375f * x * x - 1.0f) * x;
			x = e * cy;
			s = ((((sy*sy*4.0f-3.0f)*(1.0f-e-e)*cz*d/6.0f-x)*d/4.0f+cz)*sy*d+y)*c*a*r;
			crs12=faz;
			crs21=baz;
			d=s;
			if (abs(iter-MAXITER)<EPS)
			{
			//	alert("Algorithm did not converge")
			}
		}
	}
	heading12[0]=crs12*(180.0f/M_PI);
	heading21[0]=crs21*(180.0f/M_PI);
	dist[0]=d;
}

static void direct(double lat1,double lon1,double crs12,double d12,double *olat,double *olon)
{
  double EPS=0.00000000005f;
  double dlon,lat,lon;

// 5/16 changed to "long-range" algorithm
  if ((fabs(cos(lat1))<EPS) && !(fabs(sin(crs12))<EPS))
  {
	  olat[0]=0.0f;
	  olon[0]=0.0f;
		return;
	  //  alert("Only N-S courses are meaningful, starting at a pole!")
  }

  lat=asin(sin(lat1)*cos(d12)+cos(lat1)*sin(d12)*cos(crs12));
  if (fabs(cos(lat))<EPS)
  {
    lon=0.0f; //endpoint a pole
  }
  else
  {
    dlon=atan2(sin(crs12)*sin(d12)*cos(lat1),cos(d12)-sin(lat1)*sin(lat));
    lon=mod( lon1-dlon+M_PI,2*M_PI )-M_PI;
  }
  olat[0]=lat;
  olon[0]=lon;
}

static void direct_ell(double glat1,double glon1,double faz,double s,double earth1,double earth2,double *olat,double *olon)
{
// glat1 initial geodetic latitude in radians N positive 
// glon1 initial geodetic longitude in radians E positive 
// faz forward azimuth in radians
// s distance in units of a (=nm)
	double EPS= 0.00000000005f;
	double a,r, tu, sf, cf, b, cu, su, sa, c2a, x, c, d, y, sy=0, cy=0, cz=0, e=0.0f;
	double glat2,glon2,baz,f;

	if ((fabs(cos(glat1))<EPS) && !(fabs(sin(faz))<EPS))
	{
		olat[0]=0.0f;
		olon[0]=0.0f;
		return;
		//alert("Only N-S courses are meaningful, starting at a pole!")
	}

	a=earth1;
	f=1.0f/earth2;
	r = 1.0f - f;
	tu = r * tan (glat1);
	sf = sin (faz);
	cf = cos (faz);
	if (cf==0.0f)
	{
		b=0.0f;
	}
	else
	{
		b=2.0f*atan2(tu,cf);
	}
	cu = 1.0f/sqrt (1.0f + tu * tu);
	su = tu * cu;
	sa = cu * sf;
	c2a = 1 - sa * sa;
	x = 1.0f+ sqrt (1.0f+ c2a * (1.0f/ (r * r) - 1.0f));
	x = (x - 2.) / x;
	c = 1.0f- x;
	c = (x * x / 4.0f+ 1.) / c;
	d = (0.375f * x * x - 1.0f) * x;
	tu = s / (r * a * c);
	y = tu;
	c = y + 1;
	while (fabs (y - c) > EPS)
	{
		sy = sin (y);
		cy = cos (y);
		cz = cos (b + y);
		e = 2.0f* cz * cz - 1.0f;
		c = y;
		x = e * cy;
		y = e + e - 1.0f;
		y = (((sy * sy * 4.0f- 3.) * y * cz * d / 6.0f+ x) *
				d / 4.0f- cz) * sy * d + tu;
	}

	b = cu * cy * cf - su * sy;
	c = r * sqrt (sa * sa + b * b);
	d = su * cy + cu * sy * cf;
	glat2 = modlat(atan2 (d, c));
	c = cu * cy - su * sy * cf;
	x = atan2 (sy * sf, c);
	c = ((-3.0f* c2a + 4.) * f + 4.) * c2a * f / 16.0f;
	d = ((e * cy * c + cz) * sy * c + y) * sa;
	glon2 = modlon(glon1 + x - (1.0f- c) * d * f);	// fix date line problems 
	baz = modcrs(atan2 (sa, b) + M_PI);

	olat[0]=glat2;
	olon[0]=glon2;

//  out=new MakeArray(0)
//  out.lat=glat2
//  out.lon=glon2
//  out.crs21=baz
//  return out
}


/* project a distance in nautical miles and a true heading using the earth model values */

void GPXCoord::Project(double distnm,double heading,double earth1,double earth2,GPXCoord *dest)
{
	double crs12=heading*M_PI/180.0f;	/* convert heading to radians */
	double lat1,lon1;
	double lat2,lon2;

	lat1=GetLat()*(M_PI/180);
	lon1=GetLon()*(M_PI/180);

	if(earth2==0.0f)	/* sphere */
	{
		distnm/=(180*60/M_PI);
		direct(lat1,-lon1,crs12,distnm,&lat2,&lon2);
		lat2*=(180.0f/M_PI);
		lon2*=-(180.0f/M_PI);
	}
	else
	{
		direct_ell(lat1,lon1,crs12,distnm,earth1,earth2,&lat2,&lon2);
		lat2*=(180.0f/M_PI);
		lon2*=(180.0f/M_PI);
	}
	dest->Set(lat2,lon2);
}

//  E0000(1,NULL,alt,glat,glon,time,dec,dip,ti,gv);

void GPXCoord::GetMagneticDev(double *dec, double *dip, double *ti, double *gv)
{
	static int dataloaded=0;
  static int maxord,n,m,j,D1,D2,D3,D4;
  static float c[13][13],cd[13][13],tc[13][13],dp[13][13],snorm[169],
    sp[13],cp[13],fn[13],fm[13],pp[13],k[13][13],pi,dtr,a,b,re,
    a2,b2,c2,a4,b4,c4,epoch,gnm,hnm,dgnm,dhnm,flnmj,otime,oalt,
    olat,olon,dt,rlon,rlat,srlon,srlat,crlon,crlat,srlat2,
    crlat2,q,q1,q2,ct,st,r2,r,d,ca,sa,aor,ar,br,bt,bp,bpp,
    par,temp1,temp2,parp,bx,by,bz,bh;
//  char model[20], c_str[81], c_new[5];
  float *p = snorm;
	/* changed to get these from the class instead of passing in */
	kGUIDate today;
	float glat=(float)GetLat();
	float glon=(float)GetLon();
	float alt=0.0f;
	float time;

	today.SetToday();
	time=(float)today.GetYear();

	if(dataloaded==0)
	{
		dataloaded=1;
		DataHandle dh;
		kGUIString line;
		kGUIStringSplit fields;
		int numfields;
		bool ok;
		
//		  FILE *wmmdat;

		dh.SetFilename("WMM.COF");
		ok=dh.Open();
		assert(ok==true,"Error opening data file for magnetic deviation!");
//		wmmdat = fopen("WMM.COF","r");
	  
	/* INITIALIZE CONSTANTS */
	maxord = 12;						/* changed to constant from a variable */
	sp[0] = 0.0;
	cp[0] = *p = pp[0] = 1.0;
	dp[0][0] = 0.0;
	a = (float)6378.137;
	b = (float)6356.7523142;
	re = (float)6371.2;
	a2 = a*a;
	b2 = b*b;
	c2 = a2-b2;
	a4 = a2*a2;
	b4 = b2*b2;
	c4 = a4 - b4;
	  
	/* READ WORLD MAGNETIC MODEL SPHERICAL HARMONIC COEFFICIENTS */
	c[0][0] = 0.0;
	cd[0][0] = 0.0;
	  
#if 1
	dh.ReadLine(&line);
	fields.Split(&line," ");
	line.Trim();
	epoch=(float)fields.GetWord(0)->GetDouble();
#else
	fgets(c_str, 80, wmmdat);
	sscanf(c_str,"%f%s",&epoch,model);
#endif

S3:
#if 1
	dh.ReadLine(&line);
	while(line.Replace("  "," ")>0);
	line.Trim();
	numfields=fields.Split(&line," ");
	if(numfields==1)
	{
		goto S4;
	}
	assert(numfields==6,"Wrong number of fields!");
	n=fields.GetWord(0)->GetInt();
	m=fields.GetWord(1)->GetInt();
	gnm=(float)fields.GetWord(2)->GetDouble();
	hnm=(float)fields.GetWord(3)->GetDouble();
	dgnm=(float)fields.GetWord(4)->GetDouble();
	dhnm=(float)fields.GetWord(5)->GetDouble();
#else
	fgets(c_str, 80, wmmdat);
	/* CHECK FOR LAST LINE IN FILE */
	for (i=0; i<4 && (c_str[i] != '\0'); i++)
		{
		c_new[i] = c_str[i];
		c_new[i+1] = '\0';
		}
	icomp = strcmp("9999", c_new);
	if (icomp == 0) goto S4;
	/* END OF FILE NOT ENCOUNTERED, GET VALUES */
	sscanf(c_str,"%d%d%f%f%f%f",&n,&m,&gnm,&hnm,&dgnm,&dhnm);
#endif
	if (m <= n)
		{
		c[m][n] = gnm;
		cd[m][n] = dgnm;
		if (m != 0)
			{
			c[n][m-1] = hnm;
			cd[n][m-1] = dhnm;
			}
		}
	goto S3;

	/* CONVERT SCHMIDT NORMALIZED GAUSS COEFFICIENTS TO UNNORMALIZED */
	S4:
	*snorm = 1.0;
	for (n=1; n<=maxord; n++)
		{
		*(snorm+n) = (float)(*(snorm+n-1)*(double)(2*n-1)/(double)n);
		j = 2;
		for (m=0,D1=1,D2=(n-m+D1)/D1; D2>0; D2--,m+=D1)
			{
			k[m][n] =(float)( (double)(((n-1)*(n-1))-(m*m))/(double)((2*n-1)*(2*n-3)));
			if (m > 0)
				{
				flnmj = (float)((double)((n-m+1)*j)/(double)(n+m));
				*(snorm+n+m*13) = *(snorm+n+(m-1)*13)*sqrt(flnmj);
				j = 1;
				c[n][m-1] = *(snorm+n+m*13)*c[n][m-1];
				cd[n][m-1] = *(snorm+n+m*13)*cd[n][m-1];
				}
			c[m][n] = *(snorm+n+m*13)*c[m][n];
			cd[m][n] = *(snorm+n+m*13)*cd[m][n];
			}
		fn[n] = (float)(n+1);
		fm[n] = (float)n;
		}
	k[1][1] = 0.0;
	  
	otime = oalt = olat = olon = -1000.0;
	//fclose(wmmdat);
	dh.Close();
	}

  dt = time - epoch;
  if (otime < 0.0 && (dt < 0.0 || dt > 5.0))
    {      
#if 0
		printf("\n\n WARNING - TIME EXTENDS BEYOND MODEL 5-YEAR LIFE SPAN");
      printf("\n CONTACT NGDC FOR PRODUCT UPDATES:");
      printf("\n         National Geophysical Data Center");
      printf("\n         NOAA EGC/2");
      printf("\n         325 Broadway");
      printf("\n         Boulder, CO 80303 USA");
      printf("\n         Attn: Susan McLean or Stefan Maus");
      printf("\n         Phone:  (303) 497-6478 or -6522");
      printf("\n         Email:  Susan.McLean@noaa.gov");
      printf("\n         or");
      printf("\n         Stefan.Maus@noaa.gov");
      printf("\n         Web: http://www.ngdc.noaa.gov/seg/WMM/");
      printf("\n\n EPOCH  = %.3lf",epoch);
      printf("\n TIME   = %.3lf",time);
      printf("\n Do you wish to continue? (y or n) ");
      scanf("%c%*[^\n]",&answer);
      getchar();
      if ((answer == 'n') || (answer == 'N'))
        {
          printf("\n Do you wish to enter more point data? (y or n) ");
          scanf("%c%*[^\n]",&answer);
          getchar();
          if ((answer == 'y')||(answer == 'Y')) goto GEOMG1;
          else exit (0);
        }
#endif
	}
  pi = (float)3.14159265359;
  dtr = (float)(pi/180.0);
  rlon = glon*dtr;
  rlat = glat*dtr;
  srlon = sin(rlon);
  srlat = sin(rlat);
  crlon = cos(rlon);
  crlat = cos(rlat);
  srlat2 = srlat*srlat;
  crlat2 = crlat*crlat;
  sp[1] = srlon;
  cp[1] = crlon;

/* CONVERT FROM GEODETIC COORDS. TO SPHERICAL COORDS. */
  if (alt != oalt || glat != olat)
    {
      q = sqrt(a2-c2*srlat2);
      q1 = (float)(alt*q);
      q2 = ((q1+a2)/(q1+b2))*((q1+a2)/(q1+b2));
      ct = srlat/sqrt(q2*crlat2+srlat2);
      st = (float)(sqrt(1.0-(ct*ct)));
      r2 = (float)((alt*alt)+2.0*q1+(a4-c4*srlat2)/(q*q));
      r = sqrt(r2);
      d = sqrt(a2*crlat2+b2*srlat2);
      ca = (float)((alt+d)/r);
      sa = c2*crlat*srlat/(r*d);
    }
  if (glon != olon)
    {
      for (m=2; m<=maxord; m++)
        {
          sp[m] = sp[1]*cp[m-1]+cp[1]*sp[m-1];
          cp[m] = cp[1]*cp[m-1]-sp[1]*sp[m-1];
        }
    }
  aor = re/r;
  ar = aor*aor;
  br = bt = bp = bpp = 0.0;
  for (n=1; n<=maxord; n++)
    {
      ar = ar*aor;
      for (m=0,D3=1,D4=(n+m+D3)/D3; D4>0; D4--,m+=D3)
        {
/*
   COMPUTE UNNORMALIZED ASSOCIATED LEGENDRE POLYNOMIALS
   AND DERIVATIVES VIA RECURSION RELATIONS
*/
          if (alt != oalt || glat != olat)
            {
              if (n == m)
                {
                  *(p+n+m*13) = st**(p+n-1+(m-1)*13);
                  dp[m][n] = st*dp[m-1][n-1]+ct**(p+n-1+(m-1)*13);
                  goto S50;
                }
              if (n == 1 && m == 0)
                {
                  *(p+n+m*13) = ct**(p+n-1+m*13);
                  dp[m][n] = ct*dp[m][n-1]-st**(p+n-1+m*13);
                  goto S50;
                }
              if (n > 1 && n != m)
                {
                  if (m > n-2) *(p+n-2+m*13) = 0.0;
                  if (m > n-2) dp[m][n-2] = 0.0;
                  *(p+n+m*13) = ct**(p+n-1+m*13)-k[m][n]**(p+n-2+m*13);
                  dp[m][n] = ct*dp[m][n-1] - st**(p+n-1+m*13)-k[m][n]*dp[m][n-2];
                }
            }
        S50:
/*
    TIME ADJUST THE GAUSS COEFFICIENTS
*/
          if (time != otime)
            {
              tc[m][n] = c[m][n]+dt*cd[m][n];
              if (m != 0) tc[n][m-1] = c[n][m-1]+dt*cd[n][m-1];
            }
/*
    ACCUMULATE TERMS OF THE SPHERICAL HARMONIC EXPANSIONS
*/
          par = ar**(p+n+m*13);
          if (m == 0)
            {
              temp1 = tc[m][n]*cp[m];
              temp2 = tc[m][n]*sp[m];
            }
          else
            {
              temp1 = tc[m][n]*cp[m]+tc[n][m-1]*sp[m];
              temp2 = tc[m][n]*sp[m]-tc[n][m-1]*cp[m];
            }
          bt = bt-ar*temp1*dp[m][n];
          bp += (fm[m]*temp2*par);
          br += (fn[n]*temp1*par);
/*
    SPECIAL CASE:  NORTH/SOUTH GEOGRAPHIC POLES
*/
          if (st == 0.0 && m == 1)
            {
              if (n == 1) pp[n] = pp[n-1];
              else pp[n] = ct*pp[n-1]-k[m][n]*pp[n-2];
              parp = ar*pp[n];
              bpp += (fm[m]*temp2*parp);
            }
        }
    }
  if (st == 0.0) bp = bpp;
  else bp /= st;
/*
    ROTATE MAGNETIC VECTOR COMPONENTS FROM SPHERICAL TO
    GEODETIC COORDINATES
*/
  bx = -bt*ca-br*sa;
  by = bp;
  bz = bt*sa-br*ca;
/*
    COMPUTE DECLINATION (DEC), INCLINATION (DIP) AND
    TOTAL INTENSITY (TI)
*/
  bh = sqrt((bx*bx)+(by*by));
  *ti = sqrt((bh*bh)+(bz*bz));
  *dec = atan2(by,bx)/dtr;
  *dip = atan2(bz,bh)/dtr;
/*
    COMPUTE MAGNETIC GRID VARIATION IF THE CURRENT
    GEODETIC POSITION IS IN THE ARCTIC OR ANTARCTIC
    (I.E. GLAT > +55 DEGREES OR GLAT < -55 DEGREES)

    OTHERWISE, SET MAGNETIC GRID VARIATION TO -999.0
*/
  *gv = -999.0;
  if (fabs(glat) >= 55.)
    {
      if (glat > 0.0 && glon >= 0.0) *gv = *dec-glon;
      if (glat > 0.0 && glon < 0.0) *gv = *dec+fabs(glon);
      if (glat < 0.0 && glon >= 0.0) *gv = *dec+glon;
      if (glat < 0.0 && glon < 0.0) *gv = *dec-fabs(glon);
      if (*gv > +180.0) *gv -= 360.0;
      if (*gv < -180.0) *gv += 360.0;
    }
  otime = time;
  oalt = alt;
  olat = glat;
  olon = glon;
}

/*************************************************************************************/

static double clip360(double a)
{
	while(a<0.0f)
		a+=360.0f;
	while(a>360.0f)
		a-=360.0f;
	return(a);
}

#define deg2rad(x) ((x)*(M_PI/180.0f))
#define rad2deg(x) ((x)*(180.0f/M_PI))

void GPXCoord::CalcSun(int day,int month,int year,double zenith,double *sunrise,double *sunset)
{
	int pass;
	double N,N1,N2,N3;
	double lngHour,t,L,M,RA;
	double LQ,RAQ;
//	double zenith=90.83f;	/* 96.0f =civil twilight */
	double sinDec,cosDec,cosH,sinM,sin2M,tanL;
	double H,UT;

	N1 = floor(275.0f * month / 9.0f);
	N2 = floor((month + 9.0f) / 12.0f);
	N3 = (1.0f + floor((year - 4.0f * floor(year / 4.0f) + 2.0f) / 3.0f));
	N = N1 - (N2 * N3) + day - 30.0f;

	/* convert the longitude to hour value and calculate an approximate time */

	lngHour = GetLon() / 15.0f;
	
	/* pass0=runrise, pass1=sunset */

	for(pass=0;pass<2;++pass)
	{
//Convert the longitude to hour value and calculate and
//Approximate time.
		if(pass==1)
			t=N+((18.0f-lngHour)/24.0f);
		else
			t=N+((6.0f-lngHour)/24.0f);

	//Calculate the Sun's mean anomaly
		M = (0.9856f*t)-3.289f;

	//Calculate the Sun's true longitude
		sinM = sin(deg2rad(M));
		sin2M = sin(2*deg2rad(M));
		
		L=M +(1.916f*sinM) + (0.02f*sin2M) + 282.634f;
		L=clip360(L);

	//Calculate the Sun's right ascension(RA)
		tanL = 0.91764f * tan(deg2rad(L));
		RA = rad2deg(atan(tanL));
		RA=clip360(RA);

	//Putting the RA value into the same quadrant as L
		LQ = (floor(L/90.0f))*90.0f;
		RAQ = (floor(RA/90.0f))*90.0f;
		RA = RA + (LQ - RAQ);

	//Convert RA values to hours
		RA /= 15.0f;

	//calculate the Sun's declination
		sinDec = 0.39782f * sin(deg2rad(L));
		cosDec = cos(asin(sinDec));

	//calculate the Sun's local hour angle
	//if cosH > 1 the sun never rises on this date at this location
	//if cosH < -1 the sun never sets on this date at this location
		cosH = (cos(deg2rad(zenith)) - (sinDec * sin(deg2rad(GetLat())))) / (cosDec * cos(deg2rad(GetLat())));

	//finish calculating H and convert into hours
		if(pass==1)
			H = rad2deg(acos(cosH));
		else
			H = 360.0f - rad2deg(acos(cosH));
		H /= 15.0f;

	//Calculate local mean time of rising/setting
		UT = H + RA - (0.06571f * t) - 6.622f;

	//Adjust back to UTC
		UT -= lngHour;
		if(!pass)
			sunrise[0]=UT;
		else
			sunset[0]=UT;
	}
}

bool GPXCoordBox::Inside(GPXCoord *c)
{
	/* todo, check for box spanning world seam at 0.0? */

	if(c->GetLat()<m_uplat)
		return(false);					/* above */

	if(c->GetLat()>m_downlat)
		return(false);					/* below */

	if(c->GetLon()<m_leftlon)
		return(false);					/* left */

	if(c->GetLon()>m_rightlon)
		return(false);					/* left */

	return(true);	/* point is inside box */
}

void GPXBounds::Add(double lat,double lon)
{
	if(m_first)
	{
		m_first=false;
		m_maxlat=m_minlat=lat;
		m_minlon=m_maxlon=lon;
	}
	else
	{
		if(lat<m_minlat)
			m_minlat=lat;
		if(lon<m_minlon)
			m_minlon=lon;
		if(lat>m_maxlat)
			m_maxlat=lat;
		if(lon>m_maxlon)
			m_maxlon=lon;
	}
}
