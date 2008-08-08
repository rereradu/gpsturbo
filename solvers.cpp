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
SOLVERTAB_DECODE,
SOLVERTAB_PROJECT,
SOLVERTAB_DISTANCE,
SOLVERTAB_SUNRISESUNSET,
SOLVERTAB_NUMTABS
};

const char *solvertabnames[SOLVERTAB_NUMTABS]={
	"decode",
	"project",
	"distance",
	"sunrise/sunset"};


enum
{
SOLVERDECODE_ROT13,
SOLVERDECODE_ROTN,
SOLVERDECODE_MORSECODE,
SOLVERDECODE_VIGENERE,
SOLVERDECODE_KEYEDCAESAR,
SOLVERDECODE_NUMTYPES};

const char *solverdecodenames[SOLVERDECODE_NUMTYPES]={
	"Rot13","Rot-N","Morse Code","Keyed Vigenére Cipher","Keyed Caesar"};

typedef struct
{
	const char *name;
	double value;
}DIST_DEFS;

/* conversion factor for converting to meters */
DIST_DEFS solverdistances[]={
	{"meters",1.0f},
	{"nautical miles",5.39956803456e-4f},
	{"kilometers",0.001f},
	{"miles",6.21371192237e-4f},
	{"feet",3.28083989501f},
	{"inches",39.3700787402f},
	{"centimeters",100.0f},
	{"yards",1.09361329834f},
	{"millimeters",1000.0f},
	{"mil",39370.0787402f},
	{"fathom",0.5468055555555556f},
	{"chain",4.9709595959596e-2f},
	{"micron",1000000.0f},
	{"rd",0.1988383838383f},
	{"league",6.21371192237e-4f/3.0f}};

typedef struct
{
	const char *name;
	double value1;
	double value2;
}EARTH_DEFS;

EARTH_DEFS solverearthmodels[]={
{"WGS84",			6378.137f/1.852f,	298.257223563f},
{"Sphere",			180.0f*60.0f/PI,	0.0f},
{"NAD27",			6378.2064f/1.852f,	294.9786982138f},
{"International",	6378.388f/1.852f,	297.0f},
{"Krasovsky",		6378.245f/1.852f,	298.3f},
{"Bessel",			6377.397155f/1.852f,299.1528f},
{"WGS72",			6378.135f/1.852f,	298.26f},
{"WGS66",			6378.145f/1.852f,	298.25f},
{"FAI sphere",		6371.0f/1.852f,		1000000000.0f}};

const char *solverheadingnames[SOLVERHEAD_NUMTYPES]={
	"degrees","degrees magnetic"};

typedef struct
{
	const char *name;
	float time;
}ZENITH_DEF;

ZENITH_DEF zeniths[]={
	{"Offical",90.833333333f},
	{"Civil",96.0f},
	{"Nautical",102.0f},
	{"Astronomical",108.0f}};

static const char *monthnames[12]={
	"January",
	"February",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December"};

typedef struct
{
	const char *name;
	float time;
}TIMEZONE_DEF;

TIMEZONE_DEF timezones[]={
          {"Pacific Standard Time (Canada & US)",-8},
          {"Mountain Standard Time (Canada & US)",-7},

          {"Central Standard Time (Canada & US)",-6},
          {"Eastern Time (Canada & US)",-5},
          {"Atlantic Standard Time",-4},

          {"Newfoundland Standard Time",-3.5},
          {"Alaska",-9},
          {"Hawaii, Tahiti",-10},
          {"Samoa, Niue",-11},

          {"Tegucigalpa, El Salvador, Costa Rica",-6},
          {"Bogota, Lima, Quito",-5},
          {"Caracas, La Paz",-4},
          {"Sao Paulo",-3},

          {"Buenos Aires, Montevideo",-3},
          {"Mid-Atlantic",-2},
          {"Azores",-1},
          {"Cape Verde Island",-1},

          {"Casablanca, Monrivia",0},
          {"Dublin, Edinburgh, London",0},
          {"Algiers, Lagos",1},
          {"Brussels, Berlin, Rome, Prague",1},

          {"Harare, Pretoria",2},
          {"Helsinki, Riga, Tallinn, Athens, Cairo, Beirut",2},
          {"Moscow, St. Petersburg, Baghdad",3},
          {"Nairobi, Kuwait",3},

          {"Tehran",3.5},
          {"Muscat, Dubai",4},
          {"Baku, Tbilisi",4},
          {"Kabul",4.5},

          {"Bishkek, Yekaterinburg",5},
          {"Islamabad, Karachi, Tashkent",5},
          {"Calcutta",5.5},
          {"Katmandu",5.75},

		  {"Colombo, Dhaka",6},
          {"Alma-Ata, Novosibirsk",6},
          {"Cocos, Rangoon",6.5},
          {"Bangkok, Hanoi, Jakarta",7},

          {"Beijing, Hong Kong, Perth, Singapore",8},
          {"Ulan Bator, Irkutsk",8},
          {"Osaka, Sapporo, Tokyo, Seoul",9},
          {"Yakutsk",9},

          {"Adelaide",9.5},
          {"Darwin",9.5},
          {"Brisbane, Guam",10},
          {"Canberra, Melbourne, Sydney",10},

          {"Solomon Islands, New Caledonia, Magadan",11},
          {"Auckland, Wellington",12},
		  {"Fiji, Marshall Islands",12}};

void GPX::InitHeadingCombo(kGUIComboBoxObj *box)
{
	unsigned int i;

	box->SetNumEntries(sizeof(solverheadingnames)/sizeof(char *));
	for(i=0;i<sizeof(solverheadingnames)/sizeof(char *);++i)
		box->SetEntry(i,solverheadingnames[i],i);
}

int GPX::GetHeadingIndex(const char *v)
{
	return(GetIndex(v,sizeof(solverheadingnames)/sizeof(char *),solverheadingnames));
}

void GPX::InitDistCombo(kGUIComboBoxObj *box)
{
	unsigned int i;

	box->SetNumEntries(sizeof(solverdistances)/sizeof(DIST_DEFS));
	for(i=0;i<sizeof(solverdistances)/sizeof(DIST_DEFS);++i)
		box->SetEntry(i,solverdistances[i].name,i);
}

int GPX::GetDistIndex(const char *v)
{
	unsigned int i;

	for(i=0;i<sizeof(solverdistances)/sizeof(DIST_DEFS);++i)
	{
		if(!strcmp(solverdistances[i].name,v))
			return(i);
	}
// never should get here!
	return(-1);
}

void GPX::GetDistInfo(int distmode,int modelmode,double *r,double *v1,double *v2)
{
	EARTH_DEFS *ed;

	*r=solverdistances[distmode].value*1000.0f;
	ed=&solverearthmodels[modelmode];
	*(v1)=ed->value1;
	*(v2)=ed->value2;
}



void GPX::InitSolvers(void)
{
	unsigned int i;
	int bw,bh;
	kGUIDate today;
	kGUIString s;
	kGUIText *t;

	/* get area for placing gui items */
	bw=m_tabs.GetChildZoneW();
	bh=m_tabs.GetChildZoneH();
	today.SetToday();

	m_solvertabs.SetPos(0,0);
	m_solvertabs.SetSize(bw,bh-5);
	m_solvertabs.SetNumTabs(SOLVERTAB_NUMTABS);
	for(i=0;i<SOLVERTAB_NUMTABS;++i)
	{
		t=m_solvertabs.GetTabTextPtr(i);

		t->SetFontSize(16);
		t->SetFontID(1);	/* bold */

		m_solvertabs.SetTabName(i,solvertabnames[i]);
	}
	m_tabs.AddObject(&m_solvertabs);

	m_solvertabs.SetCurrentTab(SOLVERTAB_DECODE);

	/* solver / decode text */
	m_solvdecodeincaption.SetPos(0,0);
	m_solvdecodeincaption.SetFontID(1);
	m_solvdecodeincaption.SetString("Plain Text:");
	m_solvdecodein.SetFontID(FONT_COURIER);
	m_solvdecodein.SetFontSize(20);
	m_solvdecodein.SetPos(0,25);
	m_solvdecodein.SetSize(bw-10,100);
	m_solvdecodecontrols.AddObjects(2,&m_solvdecodeincaption,&m_solvdecodein);

	m_solvdecodetype.SetSize(200,20);

	m_solvdecodetype.SetNumEntries(SOLVERDECODE_NUMTYPES);
	for(i=0;i<SOLVERDECODE_NUMTYPES;++i)
		m_solvdecodetype.SetEntry(i,solverdecodenames[i],i);
	m_solvdecodetype.SetEventHandler(this,CALLBACKNAME(ChangeEncode));

	m_solvdecodecontrols.AddObjects(1,&m_solvdecodetype);
	m_solvdecodecontrols.NextLine();

	m_solvdecodekeycaption.SetPos(0,0);
	m_solvdecodekeycaption.SetFontID(1);
	m_solvdecodekeycaption.SetString("Key:");
	m_solvdecodekey.SetFontID(FONT_COURIER);
	m_solvdecodekey.SetFontSize(20);
	m_solvdecodekey.SetPos(0,25);
	m_solvdecodekey.SetSize(bw>>1,25);
	m_solvdecodecontrols.AddObjects(2,&m_solvdecodekeycaption,&m_solvdecodekey);
	m_solvdecodecontrols.NextLine();

	m_solvdecodebuttonenc.SetSize(175,20);
	m_solvdecodebuttonenc.SetString("Encode ( Plain -> Encoded )");
	m_solvdecodebuttonenc.SetEventHandler(this,CALLBACKNAME(SolveEncode));
	m_solvdecodecontrols.AddObject(&m_solvdecodebuttonenc);
	m_solvdecodecontrols.NextLine();

	m_solvdecodebuttondec.SetSize(175,20);
	m_solvdecodebuttondec.SetString("Decode ( Encoded -> Plain )");
	m_solvdecodebuttondec.SetEventHandler(this,CALLBACKNAME(SolveDecode));
	m_solvdecodecontrols.AddObject(&m_solvdecodebuttondec);
	m_solvdecodecontrols.NextLine();

	/* out */

	m_solvdecodeoutcaption.SetPos(0,0);
	m_solvdecodeoutcaption.SetFontID(1);
	m_solvdecodeoutcaption.SetString("Encoded Text:");
	m_solvdecodeout.SetFontID(FONT_COURIER);
	m_solvdecodeout.SetFontSize(20);
	m_solvdecodeout.SetPos(0,25);
	m_solvdecodeout.SetSize(bw-10,100);
	m_solvdecodecontrols.AddObjects(2,&m_solvdecodeoutcaption,&m_solvdecodeout);

	m_solvertabs.AddObject(&m_solvdecodecontrols);

	m_solvertabs.SetCurrentTab(SOLVERTAB_PROJECT);

	/* solver / project - a coordinate */
	m_solvprojfromcaption.SetPos(0,0);
	m_solvprojfromcaption.SetFontID(1);
	m_solvprojfromcaption.SetString("From:");
	m_solvprojfromlat.SetPos(75,0);
	m_solvprojfromlat.SetSize(100,20);
	m_solvprojfromlon.SetPos(200,0);
	m_solvprojfromlon.SetSize(100,20);
	m_solvprojcontrols.AddObjects(3,&m_solvprojfromcaption,&m_solvprojfromlat,&m_solvprojfromlon);
	m_solvprojcontrols.NextLine();

	/* distance */
	m_solvprojdistcaption.SetPos(0,0);
	m_solvprojdistcaption.SetFontID(1);
	m_solvprojdistcaption.SetString("Distance:");
	m_solvprojdistunits.SetPos(75,0);
	m_solvprojdistunits.SetSize(100,20);
	m_solvprojdisttype.SetPos(200,0);
	m_solvprojdisttype.SetSize(100,20);

	InitDistCombo(&m_solvprojdisttype);

	m_solvprojcontrols.AddObjects(3,&m_solvprojdistcaption,&m_solvprojdistunits,&m_solvprojdisttype);
	m_solvprojcontrols.NextLine();

	/* heading */
	m_solvprojheadcaption.SetPos(0,0);
	m_solvprojheadcaption.SetFontID(1);
	m_solvprojheadcaption.SetString("Heading:");
	m_solvprojheadunits.SetPos(75,0);
	m_solvprojheadunits.SetSize(75,20);
	m_solvprojheadtype.SetPos(175,0);
	m_solvprojheadtype.SetSize(125,20);

	m_solvprojheadtype.SetNumEntries(SOLVERHEAD_NUMTYPES);
	for(i=0;i<SOLVERHEAD_NUMTYPES;++i)
		m_solvprojheadtype.SetEntry(i,solverheadingnames[i],i);

	m_solvprojcontrols.AddObjects(3,&m_solvprojheadcaption,&m_solvprojheadunits,&m_solvprojheadtype);
	m_solvprojcontrols.NextLine();

	/* distance */
	m_solvprojearthmodelcaption.SetPos(0,0);
	m_solvprojearthmodelcaption.SetFontID(1);
	m_solvprojearthmodelcaption.SetString("Earth Model:");

	m_solvprojearthmodeltype.SetPos(150,0);
	m_solvprojearthmodeltype.SetSize(150,20);
	m_solvprojearthmodeltype.SetNumEntries(sizeof(solverearthmodels)/sizeof(EARTH_DEFS));
	for(i=0;i<sizeof(solverearthmodels)/sizeof(EARTH_DEFS);++i)
		m_solvprojearthmodeltype.SetEntry(i,solverearthmodels[i].name,i);

	m_solvprojcontrols.AddObjects(2,&m_solvprojearthmodelcaption,&m_solvprojearthmodeltype);
	m_solvprojcontrols.NextLine();

	m_solvprojbutton.SetSize(150,20);
	m_solvprojbutton.SetString("Calculate");
	m_solvprojbutton.SetEventHandler(this,CALLBACKNAME(SolveProject));
	m_solvprojcontrols.AddObject(&m_solvprojbutton);
	m_solvprojcontrols.NextLine();

	/* solver / project - a coordinate */
	m_solvprojtocaption.SetPos(0,0);
	m_solvprojtocaption.SetFontID(1);
	m_solvprojtocaption.SetString("To:");
	m_solvprojtolat.SetPos(75,0);
	m_solvprojtolat.SetSize(100,20);
	m_solvprojtolon.SetPos(200,0);
	m_solvprojtolon.SetSize(100,20);
	m_solvprojcontrols.AddObjects(3,&m_solvprojtocaption,&m_solvprojtolat,&m_solvprojtolon);
	m_solvprojcontrols.NextLine();

	m_solvertabs.AddObject(&m_solvprojcontrols);

	/***********************************************/
	m_solvertabs.SetCurrentTab(SOLVERTAB_DISTANCE);

	/* distance 'from' coordinate */
	m_solvdistfromcaption.SetPos(0,0);
	m_solvdistfromcaption.SetFontID(1);
	m_solvdistfromcaption.SetString("Point A:");
	m_solvdistfromlat.SetPos(100,0);
	m_solvdistfromlat.SetSize(100,20);
	m_solvdistfromlon.SetPos(225,0);
	m_solvdistfromlon.SetSize(100,20);
	m_solvdistcontrols.AddObjects(3,&m_solvdistfromcaption,&m_solvdistfromlat,&m_solvdistfromlon);
	m_solvdistcontrols.NextLine();

	/* distance 'to' coordinate */
	m_solvdisttocaption.SetPos(0,0);
	m_solvdisttocaption.SetFontID(1);
	m_solvdisttocaption.SetString("Point B:");
	m_solvdisttolat.SetPos(100,0);
	m_solvdisttolat.SetSize(100,20);
	m_solvdisttolon.SetPos(225,0);
	m_solvdisttolon.SetSize(100,20);
	m_solvdistcontrols.AddObjects(3,&m_solvdisttocaption,&m_solvdisttolat,&m_solvdisttolon);
	m_solvdistcontrols.NextLine();

	/* distance units */
	m_solvdistdistcaption.SetPos(0,0);
	m_solvdistdistcaption.SetFontID(1);
	m_solvdistdistcaption.SetString("Distance Units:");
	m_solvdistdisttype.SetPos(100,0);
	m_solvdistdisttype.SetSize(100,20);

	m_solvdistdisttype.SetNumEntries(sizeof(solverdistances)/sizeof(DIST_DEFS));
	for(i=0;i<sizeof(solverdistances)/sizeof(DIST_DEFS);++i)
		m_solvdistdisttype.SetEntry(i,solverdistances[i].name,i);

	m_solvdistcontrols.AddObjects(2,&m_solvdistdistcaption,&m_solvdistdisttype);
	m_solvdistcontrols.NextLine();

	/* heading */
	m_solvdistheadcaption.SetPos(0,0);
	m_solvdistheadcaption.SetFontID(1);
	m_solvdistheadcaption.SetString("Heading Type:");
	m_solvdistheadtype.SetPos(100,0);
	m_solvdistheadtype.SetSize(125,20);

	m_solvdistheadtype.SetNumEntries(SOLVERHEAD_NUMTYPES);
	for(i=0;i<SOLVERHEAD_NUMTYPES;++i)
		m_solvdistheadtype.SetEntry(i,solverheadingnames[i],i);

	m_solvdistcontrols.AddObjects(2,&m_solvdistheadcaption,&m_solvdistheadtype);
	m_solvdistcontrols.NextLine();

	m_solvdistearthmodelcaption.SetPos(0,0);
	m_solvdistearthmodelcaption.SetFontID(1);
	m_solvdistearthmodelcaption.SetString("Earth Model:");

	m_solvdistearthmodeltype.SetPos(100,0);
	m_solvdistearthmodeltype.SetSize(150,20);
	m_solvdistearthmodeltype.SetNumEntries(sizeof(solverearthmodels)/sizeof(EARTH_DEFS));
	for(i=0;i<sizeof(solverearthmodels)/sizeof(EARTH_DEFS);++i)
		m_solvdistearthmodeltype.SetEntry(i,solverearthmodels[i].name,i);

	m_solvdistcontrols.AddObjects(2,&m_solvdistearthmodelcaption,&m_solvdistearthmodeltype);
	m_solvdistcontrols.NextLine();

	m_solvdistbutton.SetSize(150,20);
	m_solvdistbutton.SetString("Calculate");
	m_solvdistbutton.SetEventHandler(this,CALLBACKNAME(SolveDistance));
	m_solvdistcontrols.AddObject(&m_solvdistbutton);
	m_solvdistcontrols.NextLine();

	/* results */
	m_solvdistresultdistcaption.SetPos(0,0);
	m_solvdistresultdistcaption.SetFontID(1);
	m_solvdistresultdistcaption.SetString("Distance:");
	m_solvdistdist.SetPos(100,0);
	m_solvdistdist.SetSize(100,20);
	m_solvdistcontrols.AddObjects(2,&m_solvdistresultdistcaption,&m_solvdistdist);
	m_solvdistcontrols.NextLine();

	/* heading a to b */
	m_solvdistresultheadcaption1.SetPos(0,0);
	m_solvdistresultheadcaption1.SetFontID(1);
	m_solvdistresultheadcaption1.SetString("Heading A to B:");
	m_solvdisthead1.SetPos(100,0);
	m_solvdisthead1.SetSize(100,20);
	m_solvdistcontrols.AddObjects(2,&m_solvdistresultheadcaption1,&m_solvdisthead1);
	m_solvdistcontrols.NextLine();

	/* heading a to b */
	m_solvdistresultheadcaption2.SetPos(0,0);
	m_solvdistresultheadcaption2.SetFontID(1);
	m_solvdistresultheadcaption2.SetString("Heading B to A:");
	m_solvdisthead2.SetPos(100,0);
	m_solvdisthead2.SetSize(100,20);
	m_solvdistcontrols.AddObjects(2,&m_solvdistresultheadcaption2,&m_solvdisthead2);

	m_solvertabs.AddObject(&m_solvdistcontrols);

	/***************************************************************/
	m_solvertabs.SetCurrentTab(SOLVERTAB_SUNRISESUNSET);

	/* distance 'from' coordinate */
	m_solvssfromcaption.SetPos(0,0);
	m_solvssfromcaption.SetFontID(1);
	m_solvssfromcaption.SetString("Location:");
	m_solvssfromlat.SetPos(100,0);
	m_solvssfromlat.SetSize(100,20);
	m_solvssfromlon.SetPos(225,0);
	m_solvssfromlon.SetSize(100,20);
	m_solvsscontrols.AddObjects(3,&m_solvssfromcaption,&m_solvssfromlat,&m_solvssfromlon);
	m_solvsscontrols.NextLine();

	m_solvssdaycaption.SetPos(0,0);
	m_solvssdaycaption.SetFontID(1);
	m_solvssdaycaption.SetString("Day:");

	m_solvssday.SetPos(100,0);
	m_solvssday.SetSize(100,20);
	m_solvssday.SetInt(today.GetDay());

	m_solvsscontrols.AddObjects(2,&m_solvssdaycaption,&m_solvssday);
	m_solvsscontrols.NextLine();

	m_solvssmonthcaption.SetPos(0,0);
	m_solvssmonthcaption.SetFontID(1);
	m_solvssmonthcaption.SetString("Month:");

	m_solvssmonth.SetPos(100,0);
	m_solvssmonth.SetSize(100,20);
	m_solvssmonth.SetNumEntries(12);
	for(i=0;i<12;++i)
		m_solvssmonth.SetEntry(i,monthnames[i],i);
	m_solvssmonth.SetSelection(today.GetMonth()-1);
	m_solvsscontrols.AddObjects(2,&m_solvssmonthcaption,&m_solvssmonth);
	m_solvsscontrols.NextLine();

	m_solvssyearcaption.SetPos(0,0);
	m_solvssyearcaption.SetFontID(1);
	m_solvssyearcaption.SetString("Year:");

	m_solvssyear.SetPos(100,0);
	m_solvssyear.SetSize(100,20);
	m_solvssyear.SetInt(today.GetYear());
	m_solvsscontrols.AddObjects(2,&m_solvssyearcaption,&m_solvssyear);
	m_solvsscontrols.NextLine();

	m_solvsstimecaption.SetPos(0,0);
	m_solvsstimecaption.SetFontID(1);
	m_solvsstimecaption.SetString("Time Zone:");

	m_solvsstimetype.SetPos(100,0);
	m_solvsstimetype.SetSize(250,20);
	m_solvsstimetype.SetNumEntries(sizeof(timezones)/sizeof(TIMEZONE_DEF));
	for(i=0;i<sizeof(timezones)/sizeof(TIMEZONE_DEF);++i)
	{
		if(timezones[i].time>=0)
			s.Sprintf("UTC+%.2f %s",timezones[i].time,timezones[i].name);
		else
			s.Sprintf("UTC%.2f %s",timezones[i].time,timezones[i].name);

		m_solvsstimetype.SetEntry(i,s.GetString(),i);
	}

	m_solvsscontrols.AddObjects(2,&m_solvsstimecaption,&m_solvsstimetype);
	m_solvsscontrols.NextLine();

	m_solvsszenithcaption.SetPos(0,0);
	m_solvsszenithcaption.SetFontID(1);
	m_solvsszenithcaption.SetString("Zenith:");

	m_solvsszenithtype.SetPos(100,0);
	m_solvsszenithtype.SetSize(150,20);
	m_solvsszenithtype.SetNumEntries(sizeof(zeniths)/sizeof(ZENITH_DEF));
	for(i=0;i<sizeof(zeniths)/sizeof(ZENITH_DEF);++i)
		m_solvsszenithtype.SetEntry(i,zeniths[i].name,i);

	m_solvsscontrols.AddObjects(2,&m_solvsszenithcaption,&m_solvsszenithtype);
	m_solvsscontrols.NextLine();

	m_solvssbutton.SetSize(150,20);
	m_solvssbutton.SetString("Calculate");
	m_solvssbutton.SetEventHandler(this,CALLBACKNAME(SolveSun));
	m_solvsscontrols.AddObject(&m_solvssbutton);
	m_solvsscontrols.NextLine();

	/* sunrise */
	m_solvssrisecaption.SetPos(0,0);
	m_solvssrisecaption.SetFontID(1);
	m_solvssrisecaption.SetString("Sunrise:");
	m_solvssrise.SetPos(100,0);
	m_solvssrise.SetSize(100,20);
	m_solvsscontrols.AddObjects(2,&m_solvssrisecaption,&m_solvssrise);
	m_solvsscontrols.NextLine();

	/* sunrise */
	m_solvsssetcaption.SetPos(0,0);
	m_solvsssetcaption.SetFontID(1);
	m_solvsssetcaption.SetString("Sunset:");
	m_solvssset.SetPos(100,0);
	m_solvssset.SetSize(100,20);
	m_solvsscontrols.AddObjects(2,&m_solvsssetcaption,&m_solvssset);
	m_solvsscontrols.NextLine();

	m_solvertabs.AddObject(&m_solvsscontrols);

	/* put default back to first tab */
	m_solvertabs.SetCurrentTab(SOLVERTAB_DECODE);
}

void GPX::ChangeEncode(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_AFTERUPDATE)
	{
		switch(m_solvdecodetype.GetSelection())
		{
		case SOLVERDECODE_ROT13:
			m_solvdecodekeycaption.SetString("Key: (not used for Rot13)");
		break;
		case SOLVERDECODE_ROTN:
			m_solvdecodekeycaption.SetString("Key: (Rotate value 0 to 25)");
		break;
		case SOLVERDECODE_MORSECODE:
			m_solvdecodekeycaption.SetString("Key: (not used for Morse Code)");
		break;
		case SOLVERDECODE_VIGENERE:
			m_solvdecodekeycaption.SetString("Key: (Rotate key string)");
		break;
		case SOLVERDECODE_KEYEDCAESAR:
			m_solvdecodekeycaption.SetString("Key: (Substitute key string, letters only, duplicates ignored)");
		break;
		}
	}
}

typedef struct
{
	const char *in;
	const char *out;
}MORSE_DEF;

MORSE_DEF morselist[]={
	{"A",".-"},
	{"B","-..."},
	{"C","-.-."},
	{"D","-.."},
	{"E","."},
	{"F","..-."},
	{"G","--."},
	{"H","...."},
	{"I",".."},
	{"J",".---"},
	{"K",".-.-"},
	{"L",".-.."},
	{"M","--"},
	{"N","-."},
	{"O","---"},
	{"P",".--."},
	{"Q","--.-"},
	{"R",".-."},
	{"S","..."},
	{"T","-"},
	{"U","..-"},
	{"V","...-"},
	{"W",".--"},
	{"X","-..-"},
	{"Y","-.--"},
	{"Z","--.."},
	{" "," "},
	{"0","-----"},
	{"1",".----"},
	{"2","..---"},
	{"3","...--"},
	{"4","....-"},
	{"5","....."},
	{"6","-...."},
	{"7","--..."},
	{"8","---.."},
	{"9","----."},
	{".",".-.-.-"},
	{":","---..."},
	{",","--..--"},
	{";","-.-.-."},
	{"?","..--.."},
	{"=","-...-"},
	{"'",".----."},
	{"+",".-.-."},
	{"!","-.-.--"},
	{"-","-....-"},
	{"/","-..-."},
	{"_","..--.-"},
	{"(","-.--."},
	{"\"",".-..-."},
	{")","-.--.-"},
	{"$","...-..-"},
	{"@",".--.-."}};

void GPX::SolveEncode(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		unsigned int i;
		unsigned char c;

		m_solvdecodeout.Clear();
		switch(m_solvdecodetype.GetSelection())
		{
		case SOLVERDECODE_ROT13:
			for(i=0;i<(unsigned int)m_solvdecodein.GetLen();++i)
			{
				c=m_solvdecodein.GetChar(i);
				if(c>='a' && c<='z')
				{
					c+=13;
					if(c>'z')
						c-=26;
				}
				else if(c>='A' && c<='Z')
				{
					c+=13;
					if(c>'Z')
						c-=26;
				}
				m_solvdecodeout.Append(c);
			}
		break;
		case SOLVERDECODE_ROTN:
		{
			unsigned char n;

			n=(unsigned char)m_solvdecodekey.GetInt();
			if(n>25)
				n=25;
			for(i=0;i<(unsigned int)m_solvdecodein.GetLen();++i)
			{
				c=m_solvdecodein.GetChar(i);
				if(c>='a' && c<='z')
				{
					c+=n;
					if(c>'z')
						c-=26;
				}
				else if(c>='A' && c<='Z')
				{
					c+=n;
					if(c>'Z')
						c-=26;
				}
				m_solvdecodeout.Append(c);
			}
		}
		break;
		case SOLVERDECODE_MORSECODE:
		{
			Hash hash;
			const char **cp;
			char cc[2];

			hash.Init(12,sizeof(cp));
			for(i=0;i<sizeof(morselist)/sizeof(MORSE_DEF);++i)
				hash.Add(morselist[i].in,&morselist[i].out);

			cc[1]=0;
			for(i=0;i<(unsigned int)m_solvdecodein.GetLen();++i)
			{
				cc[0]=m_solvdecodein.GetChar(i);
				cp=(const char **)hash.Find(cc);
				if(!cp)
					m_solvdecodeout.Append("?");
				else
				{
					m_solvdecodeout.Append(cp[0]);
					m_solvdecodeout.Append("/");
				}
			}
		}
		break;
		case SOLVERDECODE_VIGENERE:
		{
			unsigned char off;
			unsigned int k;

			k=0;
			for(i=0;i<(unsigned int)m_solvdecodein.GetLen();++i)
			{
				if(!m_solvdecodekey.GetLen())
					off=0;
				else
				{
					off=(unsigned char)(lc(m_solvdecodekey.GetChar(k++))-'a');
					if(off>25)
						off=25;
					if(k==(unsigned int)m_solvdecodekey.GetLen())
						k=0;
				}
				c=m_solvdecodein.GetChar(i);
				if(c>='a' && c<='z')
				{
					c+=off;
					if(c>'z')
						c-=26;
				}
				else if(c>='A' && c<='Z')
				{
					c+=off;
					if(c>'Z')
						c-=26;
				}
				m_solvdecodeout.Append(c);
			}
		}
		break;
		case SOLVERDECODE_KEYEDCAESAR:
		{
			bool used[26];
			kGUIString key;

			/* generate key string */
			for(i=0;i<26;++i)
				used[i]=false;
			for(i=0;i<(unsigned int)m_solvdecodekey.GetLen();++i)
			{
				c=(unsigned char)lc(m_solvdecodekey.GetChar(i));
				if(c>='a' && c<='z')
				{
					if(used[c-'a']==false)
					{
						used[c-'a']=true;
						key.Append(c);
					}
				}
			}
			for(i=0;i<26;++i)
			{
				if(used[i]==false)
					key.Append(i+'a');
			}

			/* key is now generated, convert string */
			for(i=0;i<(unsigned int)m_solvdecodein.GetLen();++i)
			{
				c=m_solvdecodein.GetChar(i);
				if(c>='a' && c<='z')
					c=key.GetChar(c-'a');
				else if(c>='A' && c<='Z')
					c=uc(key.GetChar(c-'A'));
				m_solvdecodeout.Append(c);
			}
		}
		break;
		}
	}
}

void GPX::SolveDecode(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		unsigned int i;
		unsigned char c;

		m_solvdecodein.Clear();
		switch(m_solvdecodetype.GetSelection())
		{
		case SOLVERDECODE_ROT13:
			for(i=0;i<(unsigned int)m_solvdecodeout.GetLen();++i)
			{
				c=m_solvdecodeout.GetChar(i);
				if(c>='a' && c<='z')
				{
					c+=13;
					if(c>'z')
						c-=26;
				}
				else if(c>='A' && c<='Z')
				{
					c+=13;
					if(c>'Z')
						c-=26;
				}
				m_solvdecodein.Append(c);
			}
		break;
		case SOLVERDECODE_ROTN:
		{
			unsigned char n;

			n=(unsigned char)m_solvdecodekey.GetInt();
			if(n>25)
				n=25;

			for(i=0;i<(unsigned int)m_solvdecodeout.GetLen();++i)
			{
				c=m_solvdecodeout.GetChar(i);
				if(c>='a' && c<='z')
				{
					c-=n;
					if(c<'a')
						c+=26;
				}
				else if(c>='A' && c<='Z')
				{
					c-=n;
					if(c<'A')
						c+=26;
				}
				m_solvdecodein.Append(c);
			}
		}
		break;
		case SOLVERDECODE_MORSECODE:
		{
			Hash hash;
			const char **cp;
			kGUIString cc;

			hash.Init(12,sizeof(cp));
			for(i=0;i<sizeof(morselist)/sizeof(MORSE_DEF);++i)
				hash.Add(morselist[i].out,&morselist[i].in);

			m_solvdecodein.Clear();
			cc.Clear();
			for(i=0;i<(unsigned int)m_solvdecodeout.GetLen();++i)
			{
				c=m_solvdecodeout.GetChar(i);
				if(c=='.' || c=='-')
					cc.Append(c);
				else
				{
					if(cc.GetLen())
					{
						cp=(const char **)hash.Find(cc.GetString());
						if(!cp)
							m_solvdecodein.Append("?");
						else
							m_solvdecodein.Append(cp[0]);
					}
					cc.Clear();
					if(c==' ')
						m_solvdecodein.Append(" ");
				}
			}
			if(cc.GetLen())
			{
				cp=(const char **)hash.Find(cc.GetString());
				if(!cp)
					m_solvdecodein.Append("?");
				else
					m_solvdecodein.Append(cp[0]);
			}
		}
		break;
		case SOLVERDECODE_VIGENERE:
		{
			unsigned char off;
			unsigned int k;

			k=0;
			for(i=0;i<(unsigned int)m_solvdecodeout.GetLen();++i)
			{
				if(!m_solvdecodekey.GetLen())
					off=0;
				else
				{
					off=(unsigned char)(lc(m_solvdecodekey.GetChar(k++))-'a');
					if(off>25)
						off=25;
					if(k==(unsigned int)m_solvdecodekey.GetLen())
						k=0;
				}
				c=m_solvdecodeout.GetChar(i);
				if(c>='a' && c<='z')
				{
					c-=off;
					if(c<'a')
						c+=26;
				}
				else if(c>='A' && c<='Z')
				{
					c-=off;
					if(c<'A')
						c+=26;
				}
				m_solvdecodein.Append(c);
			}
		}
		break;
		case SOLVERDECODE_KEYEDCAESAR:
		{
			bool used[26];
			kGUIString key;
			int xkey[26];

			/* generate key string */
			for(i=0;i<26;++i)
				used[i]=false;
			for(i=0;i<(unsigned int)m_solvdecodekey.GetLen();++i)
			{
				c=lc(m_solvdecodekey.GetChar(i));
				if(c>='a' && c<='z')
				{
					if(used[c-'a']==false)
					{
						used[c-'a']=true;
						key.Append(c);
					}
				}
			}
			for(i=0;i<26;++i)
			{
				if(used[i]==false)
					key.Append(i+'a');
			}

			/* make a reverse table */
			for(i=0;i<26;++i)
				xkey[key.GetChar(i)-'a']='a'+i;

			/* key is now generated, convert string */
			for(i=0;i<(unsigned int)m_solvdecodeout.GetLen();++i)
			{
				c=m_solvdecodeout.GetChar(i);
				if(c>='a' && c<='z')
					c=xkey[c-'a'];
				else if(c>='A' && c<='Z')
					c=uc(xkey[c-'A']);
				m_solvdecodein.Append(c);
			}
		}
		break;
		}
	}
}


double GPX::GetMeters(double distance,int disttype)
{
	double m;
	
	m=distance/solverdistances[disttype].value;
	return(m);
}

void GPX::Project(double distance,int disttype,int model,double heading,int headtype,GPXCoord *from,GPXCoord *to)
{
	double ndist;
	double head;
	EARTH_DEFS *ed;

	ndist=distance/solverdistances[disttype].value;
	ndist*=5.39956803456e-4f;	/* convert from meteres to nautical miles */

	if(headtype==SOLVERHEAD_DEGREESMAGNETIC)
	{
		double dec,dip,ti,gv;

		from->GetMagneticDev(&dec,&dip,&ti,&gv);
		head=heading+dec;
	}
	else
		head=heading;

	ed=&solverearthmodels[model];

	from->Project(ndist,head,ed->value1,ed->value2,to);
}

void GPX::SolveProject(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		GPXCoord from;
		double distance;
		double heading;
		GPXCoord to;
		EARTH_DEFS *ed;

		from.Set(m_solvprojfromlat.GetString(),m_solvprojfromlon.GetString());

		/* get distance in meteres */
		distance=m_solvprojdistunits.GetDouble()/solverdistances[m_solvprojdisttype.GetSelection()].value;
		distance*=5.39956803456e-4f;	/* convert from meteres to nautical miles */

		heading=m_solvprojheadunits.GetDouble();

		if(m_solvprojheadtype.GetSelection()==SOLVERHEAD_DEGREESMAGNETIC)
		{
			double dec,dip,ti,gv;
			from.GetMagneticDev(&dec,&dip,&ti,&gv);
			heading+=dec;
		}

		ed=&solverearthmodels[m_solvprojearthmodeltype.GetSelection()];

		from.Project(distance,heading,ed->value1,ed->value2,&to);
		to.Output(&m_solvprojtolat,&m_solvprojtolon);
	}
}

void GPX::SolveDistance(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		GPXCoord from;
		GPXCoord to;
		EARTH_DEFS *ed;
		double distance;
		double heading12;
		double heading21;

		from.Set(m_solvdistfromlat.GetString(),m_solvdistfromlon.GetString());
		to.Set(m_solvdisttolat.GetString(),m_solvdisttolon.GetString());
		ed=&solverearthmodels[m_solvdistearthmodeltype.GetSelection()];

		/* calculate heading and distances */
		from.DistandHeadings(&to,ed->value1,ed->value2,&distance,&heading12,&heading21);

		/* convert distance to requested distance units */
		distance/=5.39956803456e-4f;	/* convert from nautical miles to meteres */
		distance=distance*solverdistances[m_solvdistdisttype.GetSelection()].value;

		/* if magnetic headings are requested then covert to magnetic */

		if(m_solvdistheadtype.GetSelection()==SOLVERHEAD_DEGREESMAGNETIC)
		{
			double dec,dip,ti,gv;

			from.GetMagneticDev(&dec,&dip,&ti,&gv);
			heading12-=dec;
			to.GetMagneticDev(&dec,&dip,&ti,&gv);
			heading21-=dec;
		}

		m_solvdistdist.SetDouble("%f",distance);
		m_solvdisthead1.SetDouble("%f",heading12);
		m_solvdisthead2.SetDouble("%f",heading21);
	}
}

void GPX::SolveSun(kGUIEvent *event)
{
	if(event->GetEvent()==EVENT_PRESSED)
	{
		GPXCoord c;
		kGUIDate d;
		double timeoff;
		double sunrise,sunset;
		double t;

		d.SetToday();
		c.Set(m_solvssfromlat.GetString(),m_solvssfromlon.GetString());
		c.CalcSun(m_solvssday.GetInt(),m_solvssmonth.GetSelection()+1,m_solvssyear.GetInt(),zeniths[m_solvsszenithtype.GetSelection()].time,&sunrise,&sunset);

		timeoff=timezones[m_solvsstimetype.GetSelection()].time;

		t=sunrise+timeoff;
		if(t<0.0f)
			t+=24.0f;
		else if(t>24.0f)
			t-=24.0f;
		d.SetHour((int)t);
		d.SetMinute((int)((t-(int)t)*60.0f));
		d.SetSecond(0);
		d.Time(&m_solvssrise,false);

		t=sunset+timeoff;
		if(t<0.0f)
			t+=24.0f;
		else if(t>24.0f)
			t-=24.0f;

		d.SetHour((int)t);
		d.SetMinute((int)((t-(int)t)*60.0f));
		d.SetSecond(0);
		d.Time(&m_solvssset,false);
	}
}
