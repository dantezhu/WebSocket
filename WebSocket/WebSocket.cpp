// WebSocket, v1.00 2012-09-13
//
// Description: WebSocket FRC6544 codec, written in C++.
// Homepage: http://katzarsky.github.com/WebSocket
// Author: katzarsky@gmail.com

#include "WebSocket.h"

//#include "md5/md5.h"
#include "base64/base64.h"
#include "sha1/sha1.h"

#include <iostream>
#include <string>
#include <vector>
#include <string.h>

using namespace std;

WebSocketFrameType WebSocket::checkHandshake(unsigned char* input_frame, int input_len, int& output_len)
{
	// 1. copy char*/len into string
	// 2. try to parse headers until \r\n occurs
    char end_str[] = "\r\n\r\n";
	string headers((char*)input_frame, input_len); 
	int header_end = headers.find(end_str);

	if(header_end == string::npos) { // end-of-headers not found - do not parse
		return INCOMPLETE_FRAME;
	}

	//return FrameType::OPENING_FRAME;
	printf("HANDSHAKE-PARSED\n");

    output_len = header_end + strlen(end_str);

	return OPENING_FRAME;
}

WebSocketFrameType WebSocket::parseHandshake(unsigned char* input_frame, int input_len, int& output_len, WebSocketInfo& wskt_info)
{
	// 1. copy char*/len into string
	// 2. try to parse headers until \r\n occurs
    char end_str[] = "\r\n\r\n";
	string headers((char*)input_frame, input_len); 
	int header_end = headers.find(end_str);

	if(header_end == string::npos) { // end-of-headers not found - do not parse
		return INCOMPLETE_FRAME;
	}

	headers.resize(header_end); // trim off any data we don't need after the headers
    // 这里打印出来会差4，就是差的\r\n\r\n
    // printf("header_end: %d, input_len: %d\n", header_end, input_len);
	vector<string> headers_rows = explode(headers, string("\r\n"));
	for(int i=0; i<headers_rows.size(); i++) {
		string& header = headers_rows[i];
		if(header.find("GET") == 0) {
			vector<string> get_tokens = explode(header, string(" "));
			if(get_tokens.size() >= 2) {
				wskt_info.resource = get_tokens[1];
			}
		}
		else {
			int pos = header.find(":");
			if(pos != string::npos) {
				string header_key(header, 0, pos);
				string header_value(header, pos+1);
				header_value = trim(header_value);
				if(header_key == "Host") wskt_info.host = header_value;
				else if(header_key == "Origin") wskt_info.origin = header_value;
				else if(header_key == "Sec-WebSocket-Key") wskt_info.key = header_value;
				else if(header_key == "Sec-WebSocket-Protocol") wskt_info.protocol = header_value;
			}
		}
	}

	//this->key = "dGhlIHNhbXBsZSBub25jZQ==";
	//printf("PARSED_KEY:%s \n", this->key.data());

	//return FrameType::OPENING_FRAME;
	printf("HANDSHAKE-PARSED\n");

    output_len = header_end + strlen(end_str);

	return OPENING_FRAME;
}

string WebSocket::answerHandshake(const WebSocketInfo& wskt_info) 
{
    unsigned char digest[20]; // 160 bit sha1 digest

	string answer;
	answer += "HTTP/1.1 101 Switching Protocols\r\n";
	answer += "Upgrade: WebSocket\r\n";
	answer += "Connection: Upgrade\r\n";
	if(wskt_info.key.length() > 0) {
		string accept_key;
		accept_key += wskt_info.key;
		accept_key += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"; //RFC6544_MAGIC_KEY

		//printf("INTERMEDIATE_KEY:(%s)\n", accept_key.data());

		SHA1 sha;
		sha.Input(accept_key.data(), accept_key.size());
		sha.Result((unsigned*)digest);
		
		//printf("DIGEST:"); for(int i=0; i<20; i++) printf("%02x ",digest[i]); printf("\n");

		//little endian to big endian
		for(int i=0; i<20; i+=4) {
			unsigned char c;

			c = digest[i];
			digest[i] = digest[i+3];
			digest[i+3] = c;

			c = digest[i+1];
			digest[i+1] = digest[i+2];
			digest[i+2] = c;
		}

		//printf("DIGEST:"); for(int i=0; i<20; i++) printf("%02x ",digest[i]); printf("\n");

		accept_key = base64_encode((const unsigned char *)digest, 20); //160bit = 20 bytes/chars

		answer += "Sec-WebSocket-Accept: "+(accept_key)+"\r\n";
	}
	if(wskt_info.protocol.length() > 0) {
		answer += "Sec-WebSocket-Protocol: "+(wskt_info.protocol)+"\r\n";
	}
	answer += "\r\n";
	return answer;

	//return WS_OPENING_FRAME;
}

int WebSocket::calcMakeFrameSize(int msg_len) {
    int pos = 0;
    int size = msg_len; 
    pos++; // text frame

    if(size <= 125) {
        pos++;
    }
    else if(size <= 65535) {
        pos += 3;
    }
    else { // >2^16-1 (65535)
        pos ++; //64 bit length follows
        
        // write 8 bytes length (significant first)
        
        // since msg_len is int it can be no longer than 4 bytes = 2^32-1
        // padd zeroes for the first 4 bytes
        for(int i=3; i>=0; i--) {
            pos ++;
        }
        // write the actual 32bit msg_len in the next 4 bytes
        for(int i=3; i>=0; i--) {
            pos ++;
        }
    }
    return (size+pos);
}
int WebSocket::makeFrame(WebSocketFrameType frame_type, unsigned char* msg, int msg_len, unsigned char* buffer, int buffer_size)
{
	int pos = 0;
	int size = msg_len; 
	buffer[pos++] = (unsigned char)frame_type; // text frame

	if(size <= 125) {
		buffer[pos++] = size;
	}
	else if(size <= 65535) {
		buffer[pos++] = 126; //16 bit length follows
		
		buffer[pos++] = (size >> 8) & 0xFF; // leftmost first
		buffer[pos++] = size & 0xFF;
	}
	else { // >2^16-1 (65535)
		buffer[pos++] = 127; //64 bit length follows
		
		// write 8 bytes length (significant first)
		
		// since msg_len is int it can be no longer than 4 bytes = 2^32-1
		// padd zeroes for the first 4 bytes
		for(int i=3; i>=0; i--) {
			buffer[pos++] = 0;
		}
		// write the actual 32bit msg_len in the next 4 bytes
		for(int i=3; i>=0; i--) {
			buffer[pos++] = ((size >> 8*i) & 0xFF);
		}
	}
	memcpy((void*)(buffer+pos), msg, size);
	return (size+pos);
}

WebSocketFrameType WebSocket::checkFrame(unsigned char* input_buf, int input_len, int& output_offset, int& output_len) {
    return _unpackFrame(input_buf, input_len, output_offset, output_len, true);
}

WebSocketFrameType WebSocket::getFrame(unsigned char* input_buf, int input_len, int& output_offset, int& output_len) {
    return _unpackFrame(input_buf, input_len, output_offset, output_len, false);
}

// 为了减少内存copy，当需要unmask的时候，原数据会被修改
WebSocketFrameType WebSocket::_unpackFrame(unsigned char* input_buf, int input_len, int& output_offset, int& output_len, bool check)
{
	//printf("getTextFrame()\n");
	if(input_len < 3) return INCOMPLETE_FRAME;

	unsigned char msg_opcode = input_buf[0] & 0x0F;
	unsigned char msg_fin = (input_buf[0] >> 7) & 0x01;
	unsigned char msg_masked = (input_buf[1] >> 7) & 0x01;

	// *** message decoding 

	int payload_length = 0;
	int pos = 2;
	int length_field = input_buf[1] & (~0x80);
	unsigned int mask = 0;

	//printf("IN:"); for(int i=0; i<20; i++) printf("%02x ",buffer[i]); printf("\n");

	if(length_field <= 125) {
		payload_length = length_field;
	}
	else if(length_field == 126) { //msglen is 16bit!
		payload_length = input_buf[2] + (input_buf[3]<<8);
		pos += 2;
	}
	else if(length_field == 127) { //msglen is 64bit!
		payload_length = input_buf[2] + (input_buf[3]<<8); 
		pos += 8;
	}
	//printf("PAYLOAD_LEN: %08x\n", payload_length);
	if(input_len < payload_length+pos) {
		return INCOMPLETE_FRAME;
	}

	if(msg_masked) {
		mask = *((unsigned int*)(input_buf+pos));
		//printf("MASK: %08x\n", mask);
		pos += 4;

        if (!check) {
            // unmask data:
            unsigned char* c = input_buf+pos;
            for(int i=0; i<payload_length; i++) {
                c[i] = c[i] ^ ((unsigned char*)(&mask))[i%4];
            }
        }
	}
	
    output_offset = pos;
	output_len = payload_length;
	
	if(msg_opcode == 0x0) return (msg_fin)?TEXT_FRAME:INCOMPLETE_TEXT_FRAME; // continuation frame ?
	if(msg_opcode == 0x1) return (msg_fin)?TEXT_FRAME:INCOMPLETE_TEXT_FRAME;
	if(msg_opcode == 0x2) return (msg_fin)?BINARY_FRAME:INCOMPLETE_BINARY_FRAME;
	if(msg_opcode == 0x9) return PING_FRAME;
	if(msg_opcode == 0xA) return PONG_FRAME;

	return ERROR_FRAME;
}

string WebSocket::trim(string str) 
{
	//printf("TRIM\n");
	char whitespace[] = " \t\r\n";
	string::size_type pos = str.find_last_not_of(whitespace);
	if(pos != string::npos) {
		str.erase(pos + 1);
		pos = str.find_first_not_of(whitespace);
		if(pos != string::npos) str.erase(0, pos);
	}
	else {
		return string();
	}
	return str;
}

vector<string> WebSocket::explode(	
	string  theString,
    string  theDelimiter,
    bool    theIncludeEmptyStrings)
{
	//printf("EXPLODE\n");
	//UASSERT( theDelimiter.size(), >, 0 );
	
	vector<string> theStringVector;
	int  start = 0, end = 0, length = 0;

	while ( end != string::npos )
	{
		end = theString.find( theDelimiter, start );

		// If at end, use length=maxLength.  Else use length=end-start.
		length = (end == string::npos) ? string::npos : end - start;

		if (theIncludeEmptyStrings
			|| (   ( length > 0 ) /* At end, end == length == string::npos */
            && ( start  < theString.size() ) ) )
		theStringVector.push_back( theString.substr( start, length ) );

		// If at end, use start=maxSize.  Else use start=end+delimiter.
		start = (   ( end > (string::npos - theDelimiter.size()) )
              ?  string::npos  :  end + theDelimiter.size()     );
	}
	return theStringVector;
}

