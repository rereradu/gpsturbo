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

#ifndef __UPLOADXML__
#define __UPLOADXML__

enum
{
UPNAME_WAYNAME,
UPNAME_TRUNCATE,
UPNAME_DROPVOWELS
};

class UploadXML
{
public:
	UploadXML();
	~UploadXML();
	void Init(int maxpoints,int maxnamelen,bool addchildren,int nameformat);
	void SetMaxLen(unsigned int maxlen) {m_maxlen=maxlen;}
	void SetNameFormat(int nameformat) {m_nameformat=nameformat;}
	void SetMaxPoints(int maxpoints) {m_maxpoints=maxpoints;}
	void SetAddChildren(bool ac) {m_addchildren=ac;}
	void GenName(kGUIString *name);
	void Add(GPXRow *row);
	bool Save(const char *fn);
	int GetNumPoints(void) {return m_numpoints;}
	int GetNumChildPoints(void) {return m_numchildpoints;}
	void Found(const char *name,double lat,double lon);
	int GetNotFound(kGUIString *s);
	kGUIXMLItem *GetXMLRoot(void) {return m_root;}
	void AddToBounds(double lat,double lon) {m_bounds.Add(lat,lon);}
	GPXBounds *GetBoundsObj(void) {return &m_bounds;}
private:
	kGUIXML *m_xml;
	kGUIXMLItem *m_root;
	Hash m_names;				/* names are added to this list to check for duplicates */
	int m_numpoints;
	bool m_first;
	unsigned int m_maxlen;
	int m_maxpoints;
	int m_nameformat;
	bool m_addchildren;
	int m_numchildpoints;		/* number of child points added */
	GPXBounds m_bounds;
};

#endif
