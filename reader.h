//
//  reader.h
//  RFIDReader
//
//  Created by MAC on 09.08.17.
//
//

#pragma once

#include "4DPluginAPI.h"
#include "4DPlugin.h"

#define __STDC_WANT_LIB_EXT1__ 1
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <map>

#if VERSIONWIN
	#include <Windows.h>
	#include <WinSCard.h>
	#define MAX_BUFFER_SIZE 264
#else
	#include <PCSC/winscard.h>
	#include <PCSC/wintypes.h>
	#include <PCSC/pcsclite.h>
	typedef LPSTR LPWSTR;
	#define SCARD_PROTOCOL_UNDEFINED 0x0000	/**< protocol not set */ 
#endif

typedef long PA_long32;

class CReader
{
public:
    PA_long32 EstablishContext();
    PA_long32 ReleaseContext();
    PA_long32 Connect(LPSTR, PA_long32, PA_long32*, PA_long32*);
    PA_long32 Transmit(PA_long32, LPBYTE*, PA_long32*, PA_long32);
    PA_long32 Disconnect(PA_long32);
    PA_long32 ValidContext();
    PA_long32 ListReader(LPSTR*, PA_long32*);
    
    CReader();
    ~CReader();
    
private:
    void Clear(); 
    SCARDCONTEXT _context;
    std::map<PA_long32, SCARDHANDLE> _handleCollection;

    char _charBuffer[1024];
    BYTE _byteBuffer[256];
};

