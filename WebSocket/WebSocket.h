// WebSocket, v1.00 2012-09-13
//
// Description: WebSocket FRC6544 codec, written in C++.
// Homepage: http://katzarsky.github.com/WebSocket
// Author: katzarsky@gmail.com

#ifndef WEBSOCKET_H
#define	WEBSOCKET_H

#include <assert.h>
#include <stdint.h> /* uint8_t */
#include <stdio.h> /* sscanf */
#include <ctype.h> /* isdigit */
#include <stddef.h> /* int */

// std c++
#include <vector> 
#include <string> 

using namespace std;

enum WebSocketFrameType {
	ERROR_FRAME=0xFF00,
	INCOMPLETE_FRAME=0xFE00,

	OPENING_FRAME=0x3300,
	CLOSING_FRAME=0x3400,

	INCOMPLETE_TEXT_FRAME=0x01,
	INCOMPLETE_BINARY_FRAME=0x02,

	TEXT_FRAME=0x81,
	BINARY_FRAME=0x82,

	PING_FRAME=0x19,
	PONG_FRAME=0x1A
};

class WebSocket
{
	public:

	string resource;
	string host;
	string origin;
	string protocol;
	string key;

	WebSocket();

	/**
	 * @param input_frame .in. pointer to input frame
	 * @param input_len .in. length of input frame
	 * @return [WS_INCOMPLETE_FRAME, WS_ERROR_FRAME, WS_OPENING_FRAME]
	 */
	WebSocketFrameType parseHandshake(unsigned char* input_frame, int input_len);
	string answerHandshake();

    // 计算创建frame需要的空间
	int calcMakeFrameSize(int msg_length);
	int makeFrame(WebSocketFrameType frame_type, unsigned char* msg, int msg_len, unsigned char* buffer, int buffer_len);
	WebSocketFrameType checkFrame(unsigned char* in_buffer, int in_length, int* out_offset, int* out_length);
	WebSocketFrameType getFrame(unsigned char* in_buffer, int in_length, int* out_offset, int* out_length);

	string trim(string str);
	vector<string> explode(string theString, string theDelimiter, bool theIncludeEmptyStrings = false );

private:
    // out_offset: 从 in_buffer 的起始偏移量
    // out_length: 从 out_offset开始的长度。所以如果是计算in_buffer一共要截取的长度，应该是 out_offset + out_length
    WebSocketFrameType _unpackFrame(unsigned char* in_buffer, int in_length, int* out_offset, int* out_length, bool check);
};

#endif	/* WEBSOCKET_H */
