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

struct WebSocketInfo {
	string resource;
	string host;
	string origin;
	string protocol;
	string key;
};

class WebSocket
{

public:
	static WebSocketFrameType checkHandshake(unsigned char* input_frame, int input_len, int& output_len);
	/**
     * 这里并没有返回有效解析长度，确实对于握手而言，我可以把buf直接清空，不会有什么影响
	 * @param input_frame .in. pointer to input frame
	 * @param input_len .in. length of input frame
	 * @return [WS_INCOMPLETE_FRAME, WS_ERROR_FRAME, WS_OPENING_FRAME]
	 */
	static WebSocketFrameType parseHandshake(unsigned char* input_frame, int input_len, int& output_len, WebSocketInfo& wskt_info);

	static string answerHandshake(const WebSocketInfo& wskt_info);

    // 计算创建frame需要的空间
	static int calcMakeFrameSize(int msg_len);
	static int makeFrame(WebSocketFrameType frame_type, unsigned char* msg, int msg_len, unsigned char* buffer, int buffer_len);

	static WebSocketFrameType checkFrame(unsigned char* input_buf, int input_len, int& output_offset, int& output_len);
	static WebSocketFrameType getFrame(unsigned char* input_buf, int input_len, int& output_offset, int& output_len);

private:
    // output_offset: 从 input_buf 的起始偏移量
    // output_len: 从 output_offset开始的长度。所以如果是计算input_buf一共要截取的长度，应该是 output_offset + output_len
    static WebSocketFrameType _unpackFrame(unsigned char* input_buf, int input_len, int& output_offset, int& output_len, bool check);

	static string trim(string str);
	static vector<string> explode(string theString, string theDelimiter, bool theIncludeEmptyStrings = false );
};

#endif	/* WEBSOCKET_H */
