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

#include "CPU.h"

#undef _DEBUG
#ifdef _DEBUG
#define OPCODE(x, ...) sprintf(buffer+strlen(buffer), x, __VA_ARGS__)
#define OUTPUT() log->Debug("%s", buffer);
#else
#define OPCODE(x, ...) 
#define OUTPUT() 
#endif

extern uint8_t tick;
extern bool TimerStart;

CPU::CPU(void)
{
	ScreenUpdated = true;
	memset( memory, 0, 0x1000 );
	memset( screen, 0, 64*32 );
	memset( S, 0, 16*2 );

	memcpy( memory , font, 16*5 );

	this->Reset();
	log->Debug("CPU created");
}

CPU::~CPU(void)
{
	log->Debug("CPU destroyed");
}

int CPU::Load( uint8_t* bin, uint16_t size )
{
	if (size>0x1000-0x200)
	{
		log->Fatal("ROM size is too big!");
		return 0;
	}
	memcpy( memory+0x200, bin, size );
}

void CPU::Reset()
{
	PC = 0x200;
	SP = 0;
	I = 0;
	for (int i = 0; i<=0xf; i++) V[i] = keys[i] = 0;
	log->Debug("CPU reseted");
}

void CPU::DelayTimer( uint8_t t )
{
	tick = t;
	TimerStart = true;
}

uint16_t CPU::Pop()
{
	if (SP == 0) log->Info("SP<0");
	SP--;
	return S[SP];
}

void CPU::Push( uint16_t d )
{
	S[SP] = d;
	SP++;
	if (SP>15) log->Info("SP>15");
}


int CPU::Step()
{
	// Temporary variables
	uint16_t previousPC = PC;
	uint16_t address;
	uint16_t out;
	uint8_t x, y;
	uint8_t _x, _y;
	uint8_t pixel;

	// Fetch
	uint8_t opcode = memory[PC];
	uint8_t data = memory[PC+1];

#ifdef _DEBUG
	char buffer[512];
	sprintf(buffer, "%.4X:  %.2X%.2X   ", PC, opcode, data);
	
#endif

	PC+=2;

	// Decode && execute
	switch( (opcode>>4)&0xf ) // High nibble
	{
	case 0x0:
		// 00E0 - CLS
		// Clears the screen
		if (data == 0xE0)
		{
			memset( screen, 0, 64*32 );
			ScreenUpdated = true;
			OPCODE("CLS");
		}

		// 00EE - RET
		// Return from a subroutine
		else if (data == 0xEE)
		{
			PC = Pop();
			OPCODE("RET");
		}

		// 0NNN - SYS addr
		// (obsolete) Calls program
		else
		{
			OPCODE("SYS");
		}
		break;

	// 1nnn - JP addr
	// Jump to location nnn
	case 0x1:
		address = (((uint16_t)opcode&0xf)<<8 | data);
		PC = address;

		OPCODE("JP %x", address);
		break;

	// 2nnn - CALL addr
	// Call subroutine at nnn
	case 0x2:
		Push(PC);
		address = (((uint16_t)opcode&0xf)<<8 | data);
		PC = address;

		OPCODE("CALL %x", address);
		break;

	// 3xkk - SE Vx, byte
	// Skip next instruction if Vx = kk
	case 0x3:
		if ( V[ opcode&0xf ] == data ) PC+=2; 
		OPCODE("SE V%x, %x", opcode&0xf, data );
		break;

	// 4xkk - SNE Vx, byte
	// Skip next instruction if Vx != kk
	case 0x4:
		if ( V[ opcode&0xf ] != data ) PC+=2; 
		OPCODE("SNE V%x, %x", opcode&0xf, data );
		break;

	// 5xy0 - SE Vx, Vy
	// Skip next instruction if Vx = Vy
	case 0x5:
		if ( V[ opcode&0xf ] == V[ ((data&0xf0)>>4) ] ) PC+=2; 
		OPCODE("SE V%x, V%x", opcode&0xf, ((data&0xf0)>>4) );
		break;

	// 6xkk - LD Vx, byte
	// Set Vx = kk
	case 0x6:
		V[ opcode&0xf ] = data; 
		OPCODE("LD V%x, %x", opcode&0xf, data );
		break;

	// 7xkk - ADD Vx, byte
	// Set Vx += kk
	case 0x7:
		V[ opcode&0xf ] += data; 
		OPCODE("ADD V%x, %x", opcode&0xf, data );
		break;

	case 0x8:
		switch (data&0xf)
		{
			// 8xy0 - SE Vx, Vy
			// Set Vx = Vy
			case 0x0:
				V[ opcode&0xf ] = V[ ((data&0xf0)>>4) ]; 
				OPCODE("LD V%x, V%x", opcode&0xf, ((data&0xf0)>>4) );
				break;

			// 8xy1 - OR Vx, Vy
			// Set Vx |= Vy
			case 0x1:
				V[ opcode&0xf ] |= V[ ((data&0xf0)>>4) ]; 
				OPCODE("OR V%x, V%x", opcode&0xf, ((data&0xf0)>>4) );
				break;

			// 8xy2 - AND Vx, Vy
			// Set Vx &= Vy
			case 0x2:
				V[ opcode&0xf ] &= V[ ((data&0xf0)>>4) ]; 
				OPCODE("AND V%x, V%x", opcode&0xf, ((data&0xf0)>>4) );
				break;

			// 8xy3 - XOR Vx, Vy
			// Set Vx ^= Vy
			case 0x3:
				V[ opcode&0xf ] ^= V[ ((data&0xf0)>>4) ]; 
				OPCODE("XOR V%x, V%x", opcode&0xf, ((data&0xf0)>>4) );
				break;

			// 8xy4 - ADD Vx, Vy
			// Set Vx += Vy
			case 0x4:
				x = V[ opcode&0xf ];
				y = V[ ((data&0xf0)>>4) ];
				out = x+y; // To check the carry
				if (out & 0x100) V[0xf] = 1;
				V[ opcode&0xf ] = out & 0xff; 
				OPCODE("ADD V%x, V%x", opcode&0xf, ((data&0xf0)>>4) );
				break;

			// 8xy5 - SUB Vx, Vy
			// Set Vx = Vx - Vy, set VF = NOT borrow.
			case 0x5:
				x = V[ opcode&0xf ];
				y = V[ ((data&0xf0)>>4) ];
				out = x - y; // To check the carry

				if ( x > y ) V[0xf] = 1;
				else V[0xf] = 0;

				V[ opcode&0xf ] = out & 0xff;
				
				OPCODE("SUB V%x, V%x", opcode&0xf, ((data&0xf0)>>4) );
				break;

			// 8xy6 - SHR Vx
			// Set Vx = Vx SHR 1.
			case 0x6:
				x = V[ opcode&0xf ];

				if (x&1) V[0xf] = 1;
				else V[0xf] = 0;

				V[ opcode&0xf] >>= 1;
				
				OPCODE("SHR Vx" );
				break;

			// 8xy7 - SUBN Vx, Vy
			// Set Vx = Vy - Vx, set VF = NOT borrow.
			case 0x7:
				x = V[ opcode&0xf ];
				y = V[ ((data&0xf0)>>4) ];
				out = y - x; // To check the carry

				if ( y > x ) V[0xf] = 1;
				else V[0xf] = 0;

				V[ opcode&0xf ] = out & 0xff;
				
				OPCODE("SUBN V%x, V%x", opcode&0xf, ((data&0xf0)>>4) );
				break;

			// 8xyE - SHL Vx
			// Set Vx = Vx SHL 1.
			case 0xE:
				x = V[ opcode&0xf ];

				if (x&80) V[0xf] = 1;
				else V[0xf] = 0;

				V[ opcode&0xf ] <<= 1;
				
				OPCODE("SHL Vx" );
				break;
		}
		break;

	// 9xy0 - SNE Vx, Vy
	// Skip next instruction if Vx != Vy
	case 0x9:
		if (V[ opcode&0xf ] != V[ ((data&0xf0)>>4) ]) PC+=2;
		OPCODE("SNE V%x, V%x", opcode&0xf, ((data&0xf0)>>4) );
		break;

	// Annn - LD I, addr
	// Set I = nnn
	case 0xA:
		address = (((uint16_t)opcode&0xf)<<8 | data);
		I = address;
		OPCODE("LD I, %x", address );
		break;

	// Bnnn - JP V0, addr
	// Jump to location nnn + V0
	case 0xB:
		address = (((uint16_t)opcode&0xf)<<8 | data);
		PC = V[0] + address;
		OPCODE("JP V0, %x", address );
		break;

	// Cxkk - RND Vx, byte
	// Vx = random AND kk
	case 0xC:
		V[opcode&0xf] = (rand()&0xff)&data;
		OPCODE("RND V%x, %x", opcode&0xf, data);
		break;

	// Dxyn - DRW Vx, Vy, height
	// Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
	//
	case 0xD:
		x = opcode&0xf;
		y = ((data&0xf0)>>4);
		V[0xf] = 0;

		for ( int YL = 0; YL < (data&0xf); YL++ ) // For every line in sprite
		{
			pixel = memory[ I + YL ];
			_y = (YL + V[y])%32;

			for ( int XL = 0; XL <8; XL++ ) // For every pixel
			{
				if ( pixel & (0x80>>XL) )
				{
					_x = (XL + V[x])%64;
					if ( screen[_y][_x] ) V[0xf] = 1;
					screen[_y][_x] ^= 1;
				}
			}
		}

		OPCODE("DRW V%x, V%x, %x", x, y, data&0xf );
		ScreenUpdated = true;
		break;

	case 0xE:
		// Ex9E - SKP Vx
		// Skip next instruction if key with the value of Vx is pressed.
		if (data == 0x9E)
		{
			if (keys[V[opcode&0xf]]) PC+=2;
			OPCODE("SPK V%x", opcode&0xf );
		}

		// ExA1 - SKNP Vx
		// Skip next instruction if key with the value of Vx is not pressed.
		else if (data == 0xA1)
		{
			if (!keys[V[opcode&0xf]]) PC+=2;
			OPCODE("SKNP V%x", opcode&0xf );
		}
		break;


	// Loads
	case 0xF:
		switch ( data )
		{
			// 0x07 - LD Vx, DT
			// Vx = DT (Delay timer)
		case 0x07:
			V[opcode&0xf] = tick;
			OPCODE("LD V%x, DT", opcode&0xf);
			break;
			
			// 0x0A - LD Vx, K
			// Wait for key press, store value in Vx
		case 0x0A:
			for (int i = 0; i< 16; i++) if (keys[i]) V[opcode&0xf] = i;
			OPCODE("LD V%x, K", opcode&0xf);
			break;
			
			// 0x15 - LD DT, Vx
			// DT = Vx
		case 0x15:
			DelayTimer( V[opcode&0xf] );
			//tick = V[opcode&0xf];
			OPCODE("LD DT, V%x", opcode&0xf);
			break;
			
			// 0x18 - LD ST, Vx
			// ST = Vx (Sound timer)
		case 0x18:
			OPCODE("LD ST, V%x", opcode&0xf);
			break;
			
			// 0x1E - ADD I, Vx
		case 0x1E:
			I += V[opcode&0xf];
			OPCODE("ADD I, V%x", opcode&0xf);
			break;
			
			// 0x29 - LD F, Vx
			// Set I = location of sprite for digit Vx.
		case 0x29:
			I = V[opcode&0xf]*5;
			OPCODE("LD F, V%x", opcode&0xf);
			break;

			// 0x33 - LD B, Vx
			// Store BCD representation of Vx in memory locations I (S00), I+1 (D0), and I+2 (J).
		case 0x33:
			memory[I] = V[opcode&0xf]/100;
			memory[I+1] = (V[opcode&0xf]/10)%10;
			memory[I+2] = V[opcode&0xf]%10;
			OPCODE("LD B, V%x", opcode&0xf);
			break;
			
			// 0x55 - LD [I], Vall
			// Store registers V0 through Vx in memory starting at location I.
		case 0x55:
			for (int i = 0; i<=(opcode&0xf); i++) 
			{
				memory[I+i] = V[i];
			}
			I += (opcode&0xf) + 1;
			OPCODE("LD [I], Vall");
			break;
			
			// 0x65 - LD Vall, [I]
			// Read registers V0 through Vx from memory starting at location I.
		case 0x65:
			for (int i = 0; i<=(opcode&0xf); i++) 
			{
				V[i] = memory[I+i];
			}
			I += (opcode&0xf) + 1;
			OPCODE("LD Vall, [I]");
			break;
		}
		break;


	default:
		OUTPUT();
		log->Error("Undefined instruction (%x%x)! Execution stopped", opcode, data);
		return 0;

	}

	OUTPUT();
	return 1;
}
