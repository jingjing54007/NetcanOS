/*************************************************************************
	> File Name: drivers/video_terminal.c
	  > Author: Netcan
	  > Blog: http://www.netcan666.com
	  > Mail: 1469709759@qq.com
	  > Created Time: 2018-07-08 Sun 11:57:39 CST
 ************************************************************************/

#include <video_terminal.h>
#include <io.h>
#include <string.h>

unsigned char *VRAM = (unsigned char *)VRAM_ADDRESS;

// 根据row和col计算偏移量
static int get_vt_offset(int row, int col) {
	return (row * MAX_COL + col) * 2;
}

// 获取当前光标的偏移位置
static int get_vt_cursor(void) {
// REG_SCREEN_CTRL 写入14，获取偏移量的高字节
	port_byte_out(REG_SCREEN_CTRL, 14);
	int offset = port_byte_in(REG_SCREEN_DATA) << 8;
// REG_SCREEN_CTRL 写入15，获取偏移量的低字节
	port_byte_out(REG_SCREEN_CTRL, 15);
	offset |= port_byte_in(REG_SCREEN_DATA);
	return offset * 2;
}

// 设置光标位置
static void set_vt_cursor(int offset) {
	offset /= 2;
	port_byte_out(REG_SCREEN_CTRL, 14);
	port_byte_out(REG_SCREEN_DATA, (unsigned char)(offset >> 8));
	port_byte_out(REG_SCREEN_CTRL, 15);
	port_byte_out(REG_SCREEN_DATA, (unsigned char)(offset & 0xff));
}

// 处理滚屏
static int handle_vt_scrolling(int offset) {
	if(offset < MAX_COL * MAX_ROW * 2) return offset;
	memcpy(VRAM + get_vt_offset(0, 0), VRAM + get_vt_offset(1, 0), MAX_COL * (MAX_ROW - 1) * 2);
	unsigned char *last_line = VRAM + get_vt_offset(MAX_ROW - 1, 0);
	for(int i = 0; i < MAX_COL; ++i) last_line[i * 2] = 0;
	// 滚到上一行
	offset -= 2 * MAX_COL;
	return offset;
}

// 在第row行，第col列打印属性为attribute_byte的字符character
void print_char(char character, int row, int col, char attribute_byte) {
	if(! attribute_byte) attribute_byte = WHITE_ON_BLACK;

	int offset;
	if(row >= 0 && row < MAX_ROW && col >= 0 && col < MAX_COL) {
		offset = get_vt_offset(row, col);
		set_vt_cursor(offset);
	}
	else offset = get_vt_cursor();

	if(character == '\n') { // 换行
		int row = offset / MAX_COL / 2;
		offset = get_vt_offset(row, MAX_COL - 1);
	} else {
		VRAM[offset] = character;
		VRAM[offset + 1] = attribute_byte;
	}

	offset += 2;
	offset = handle_vt_scrolling(offset);
	set_vt_cursor(offset);
}

// 在第row行，第col列打印字符串
void print_at(const char* string, int row, int col) {
	const char *c = string;
	while(*c) print_char(*c++, row, col, 0);
}

// 在当前行打印字符串
void print(const char* string) {
	print_at(string, -1, -1);
}


// 清屏
void clear_vt(void) {
	for(int i = 0; i < MAX_ROW * MAX_COL; ++i)
		VRAM[i * 2] = 0;
	set_vt_cursor(0);
}

