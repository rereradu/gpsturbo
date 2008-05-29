#include "gpsturbo.h"

#ifndef USE_EXTERNAL_DATA
#include "data.cpp"
#endif

extern BigFile *g_bf;

void GPX::InitArt(void)
{
	g_bf=new BigFile();
	//code was changed from using local bigfile to having the data included right into the code
#ifdef USE_EXTERNAL_DATA
	g_bf->SetFilename("data.big");
#else
	g_bf->SetMemory(bin_data,sizeof(bin_data));
#endif
	g_bf->Load();
	DataHandle::AddBig(g_bf);
}
