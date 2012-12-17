// Copyright (C) 2012 AnotherChip8.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Jakub Czekanski
// 00kubx@gmail.com

#pragma once
#include "types.h"
#include "logger.h"

extern Logger* log;

const static uint8_t font[16*5] = 
{
0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
0x20, 0x60, 0x20, 0x20, 0x70, // 1
0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
0x90, 0x90, 0xF0, 0x10, 0x10, // 4
0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
0xF0, 0x10, 0x20, 0x40, 0x40, // 7
0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
0xF0, 0x90, 0xF0, 0x90, 0x90, // A
0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
0xF0, 0x80, 0x80, 0x80, 0xF0, // C
0xE0, 0x90, 0x90, 0x90, 0xE0, // D
0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

class CPU
{
private:
	// Memory
	uint8_t memory[0x1000]; // 4K

	// Registers
	uint8_t V[16];  // registers V0-VF
	uint16_t I;		// Address register
	uint16_t PC;	// Program counter
	uint16_t S[16];	// Stack (?)
	uint8_t SP;

	// Timers
	void DelayTimer( uint8_t t );

public:
	// Screen
	bool screen[32][64];
	bool keys[0xf];
	bool ScreenUpdated;

	CPU(void);
	~CPU(void);

	uint16_t Pop();
	void Push( uint16_t d );
	
	int Load( uint8_t* bin, uint16_t size ); // Loads ROM to memory

	void Reset();
	int Step();
};
