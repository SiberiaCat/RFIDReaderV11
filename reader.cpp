//
//  reader.cpp
//  RFIDReader
//
//  Created by MAC on 09.08.17.
//
//

#include "reader.h"


PA_long32 CReader::EstablishContext()
{
    Clear();
	PA_long32 returnValue = (PA_long32)SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &_context); // SCARD_SCOPE_USER // SCARD_SCOPE_SYSTEM
	if (_context == 0) return SCARD_E_INVALID_HANDLE;
    return returnValue;
}


PA_long32 CReader::ReleaseContext()
{
	Clear();
	return SCARD_S_SUCCESS;
}


PA_long32 CReader::ListReader(LPSTR* readername, PA_long32* readername_len)
{
    DWORD length = sizeof(_charBuffer);
    memset(_charBuffer, 0, sizeof(_charBuffer));
    *readername = 0;
    *readername_len = 0;
    
    if (_context == 0) return SCARD_E_INVALID_HANDLE;
    /************************************************/
    
	PA_long32 returnValue = SCardListReaders(_context, NULL,  NULL, &length);//misst Groesse von mszReaders
    
    if((length<MAX_BUFFER_SIZE) && (length>0) && (returnValue == SCARD_S_SUCCESS))
    {
        returnValue = SCardListReaders(_context, NULL, _charBuffer, &length); // befuellt mszReaders
        if (returnValue == SCARD_S_SUCCESS) {
            if (length <= sizeof(_charBuffer)) {
                *readername = _charBuffer;
                *readername_len = (PA_long32)length;
            } else {
                returnValue = SCARD_E_NO_MEMORY;
            }
        }
    }
    return returnValue;
}



PA_long32 CReader::Connect(LPSTR readername, PA_long32 readername_len, PA_long32* protocol, PA_long32* handle)
{
    *protocol = 0;
    *handle = 0;
    
    if (_context == 0) return SCARD_E_INVALID_HANDLE;
    /************************************************/
    PA_long32 returnValue = SCARD_S_SUCCESS;

    DWORD dwProtocol = (DWORD)(*protocol);
    SCARDHANDLE hCard = 0;
    
    switch (dwProtocol) {
        case SCARD_PROTOCOL_T0:
            break;
        case SCARD_PROTOCOL_T1:
            break;
        case SCARD_PROTOCOL_UNDEFINED:
            break;
        default:
            returnValue = SCARD_E_INVALID_VALUE;
    }
    
    if (returnValue == SCARD_S_SUCCESS)
    {
        if (dwProtocol== SCARD_PROTOCOL_UNDEFINED) {
            returnValue = SCardConnect(_context, readername, SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &hCard, &dwProtocol);
        }else {
            returnValue = SCardConnect(_context, readername, SCARD_SHARE_SHARED, dwProtocol, &hCard, &dwProtocol);
        }
        if (returnValue == SCARD_S_SUCCESS) {
            PA_long32 index = 1;
            while (_handleCollection.find(index) != _handleCollection.end()) 
			{
				index++;
            }
            _handleCollection.insert(std::map<PA_long32, SCARDHANDLE>::value_type(index, hCard));
            *handle = index;
            *protocol = dwProtocol;
        }
    }
    return returnValue;
}


PA_long32 CReader::Transmit(PA_long32 index, LPBYTE* buffer, PA_long32* length, PA_long32 protocol)
{
    DWORD returnBufferLength = sizeof(_byteBuffer);
    memset(_byteBuffer, 0, sizeof(_byteBuffer));
    
    *buffer = 0;
    *length = 0;
    
    if (_context == 0) return SCARD_E_INVALID_HANDLE;
    /************************************************/
    PA_long32 returnValue = SCARD_E_INVALID_HANDLE;
    
    LPCSCARD_IO_REQUEST hRequest = NULL;
    BYTE cmd[] = {0xFF,0xCA,0x00,0x00,0x00};
    
    std::map<PA_long32,SCARDHANDLE>::iterator pos = _handleCollection.find(index);
    if (pos != _handleCollection.end()) {
        if (pos->second != 0) {
			returnValue = SCARD_S_SUCCESS;

			switch (protocol)
			{
			case SCARD_PROTOCOL_T0:
				hRequest = SCARD_PCI_T0;
				break;
			case SCARD_PROTOCOL_T1:
				hRequest = SCARD_PCI_T1;
				break;
			default:
				returnValue = SCARD_E_INVALID_VALUE;
			}

			if (returnValue == SCARD_S_SUCCESS)
			{
				returnValue = SCardTransmit(pos->second, hRequest, cmd, sizeof(cmd), NULL, _byteBuffer, &returnBufferLength);
				if (returnValue == SCARD_S_SUCCESS) {
                    if (returnBufferLength <= sizeof(_byteBuffer) && (returnBufferLength > 0)) {
                        *buffer = _byteBuffer;
                        *length = returnBufferLength;
                    } else {
                        returnValue = SCARD_E_NO_MEMORY;
                    }
				}
			}     
        }
    } 
    return returnValue;
}



PA_long32 CReader::Disconnect(PA_long32 index)
{
	if (_context == 0) return SCARD_E_INVALID_HANDLE;
    /************************************************/
    PA_long32 returnValue = SCARD_S_SUCCESS;
    
    std::map<PA_long32,SCARDHANDLE>::iterator pos = _handleCollection.find(index);
    if (pos != _handleCollection.end()) {
        if (pos->second != 0) {
            returnValue = (PA_long32)SCardDisconnect(pos->second, SCARD_UNPOWER_CARD);
            if (returnValue == SCARD_S_SUCCESS) {
                pos->second = 0;
                _handleCollection.erase(pos);
            }
        }
    }
    return returnValue;
}


void CReader::Clear()
{
	if (_context != 0) 
	{
		std::map<PA_long32, SCARDHANDLE>::iterator pos;
		for (pos = _handleCollection.begin(); pos != _handleCollection.end(); ++pos) {
			if (pos->second != 0) {
				SCardDisconnect(pos->second, SCARD_UNPOWER_CARD);
				pos->second = 0;
			}
		}
		SCardReleaseContext(_context);  
    }
	_handleCollection.clear();
	_context = 0;
    memset(_charBuffer, 0, sizeof(_charBuffer));
    memset(_byteBuffer, 0, sizeof(_byteBuffer));
}


PA_long32 CReader::ValidContext()
{
    PA_long32 returnValue = SCARD_E_INVALID_HANDLE;
    if (_context != 0) {
        returnValue = SCardIsValidContext(_context);
    }
    return returnValue;
}


CReader::CReader()
{
    Clear();
}


CReader::~CReader()
{
    Clear();
}