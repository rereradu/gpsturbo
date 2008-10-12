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

#include "kguithread.h"
#include "kguicsv.h"

enum
{
BABELTYPE_WAYPOINTS=1,
BABELTYPE_TRACKS=2,
BABELTYPE_REALTIME=4
};

enum
{
BABELFILTER_SIMPLIFYTRACK=1
};

class BabelGlue
{
public:
	BabelGlue();
	~BabelGlue();
	void SetInput(const char *format,const char *name,const char *baud=0) {m_informat.SetString(format);m_inname.SetString(name);m_inbaud.Clear();if(baud)m_inbaud.SetString(baud);}
	void SetOutput(const char *format,const char *name,const char *baud=0) {m_outformat.SetString(format);m_outname.SetString(name);m_outbaud.Clear();if(baud)m_outbaud.SetString(baud);}
	bool Call(bool build,kGUIBusy *busy);
	void StartTracking(void);								/* used for realtime 'tracking' mode */
	void StopTracking(void);								/* used for realtime 'tracking' mode */
	bool IsTracking(void) {return m_thread.GetActive();}	/* is in tracking mode */
	bool GetPos(GPXCoord *pos);								/* true if a new point is ready */
	void ClearTypes(void) {m_types=0;}
	void AddType(int type) {m_types|=type;}
	void SimplifyTrack(int num) {m_filter=BABELFILTER_SIMPLIFYTRACK;m_stnum=num;}
	bool GetInputFormats(unsigned int index,const char **name,const char **desc,const char **ext);
	kGUIString *GetOutputString(void) {return m_ct.GetString();}
	CALLBACKGLUE(BabelGlue,OutputChanged)
	void GetVersion(kGUIString *s);
private:
	void Purge(void);
	void BuildParms(void);
	void CallThread(void);
	void OutputChanged(void);
	bool GetChunk(const char *tag,const char *file,kGUIString *s);

	kGUICallThread m_ct;
	kGUIString m_output;
	kGUIBusy *m_busy;

	kGUIString m_informat;
	kGUIString m_inname;
	kGUIString m_inbaud;
	kGUIString m_outformat;
	kGUIString m_outname;
	kGUIString m_outbaud;
	int m_types;
	int m_filter;
	int m_stnum;

	int m_argc;
	ClassArray<kGUIString>m_strings;

	/* tracking mode stuff */
	volatile bool m_asyncactive;
	kGUIThread m_thread;

	long m_crc;
	kGUIString m_created;
	double m_lat,m_lon;

	kGUICSV m_filetypes;

	CALLBACKGLUE(BabelGlue,CallThread);
};
