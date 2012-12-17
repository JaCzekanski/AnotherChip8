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
#undef _DEBUG
#include "version.h"
#include "types.h"
#include "logger.h"
#include <cstdio>
#include <cstdlib>

#include <SDL.h>
#undef main

#include "CPU.h"

#define ROM_NAME "rom/pong"
unsigned char FileName[2048];

Logger* log;
CPU* cpu;

uint16_t KeyMatrix[16] =
{ 
	SDL_SCANCODE_X, 
	SDL_SCANCODE_1, 
	SDL_SCANCODE_2, 
	SDL_SCANCODE_3,
	SDL_SCANCODE_Q, 
	SDL_SCANCODE_W, 
	SDL_SCANCODE_E,  
	SDL_SCANCODE_A, 
	SDL_SCANCODE_S, 
	SDL_SCANCODE_D, 
	SDL_SCANCODE_Z, 
	SDL_SCANCODE_C, 
	SDL_SCANCODE_4, 
	SDL_SCANCODE_R, 
	SDL_SCANCODE_F, 
	SDL_SCANCODE_V
};

uint8_t tick = 0;
bool TimerStart;
Uint32 TimerCallback(Uint32 interval, void *param)
{
	if (TimerStart)
	{
		if (tick == 1) TimerStart = false;
		tick--;
	}
	return interval;
}

int main()
{
	log = new Logger("log.txt");
	log->Info("AnotherChip8 version %d.%d", MAJOR_VERSION, MINOR_VERSION);
	log->Info("Jakub Czekanski");

	// Initialization of SDL
	if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		log->Fatal("SDL_Init failed");
		return 1;
	}
	log->Success("SDL_Init successful");

	SDL_Window* MainWindow = SDL_CreateWindow( 
		"AnotherChip8", 
		20, 20,		// x, y
		64*4, 32*4, // w, h
		SDL_WINDOW_SHOWN );

	if ( !MainWindow )
	{
		log->Fatal("Cannot create main window");
		return 1;
	}
	log->Success("Main window created");

	SDL_Surface* screen = SDL_GetWindowSurface( MainWindow ); // Scratched canvas
	SDL_Surface* canvas = SDL_CreateRGBSurface( SDL_SWSURFACE, 64, 32, 32, 0, 0, 0 ,0 );
	if (!canvas) log->Fatal("Cannot create canvas surface!");


	strcpy( (char*)FileName, ROM_NAME );
#ifndef _DEBUG
// Show file selection dialog
	OPENFILENAME ofn = {0};
	ofn.lStructSize = sizeof( ofn );
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = (char*)FileName;
	ofn.lpstrFile[0] = 0;
	ofn.nMaxFile = sizeof(FileName);
	ofn.lpstrFilter = "*.*\0*.*\0";
	ofn.nFilterIndex = 0;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = "./rom/";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	if (!GetOpenFileName( &ofn ))
	{
		log->Debug("GetOpenFileName problem!: %d", GetLastError());
		strcpy( (char*)FileName, ROM_NAME );
	}
#endif

	// Opening rom
	log->Info("Opening %s", FileName);
	FILE* rom = fopen( (const char*)FileName, "rb" );
	if (!rom)
	{
		log->Error("Cannot load %s", FileName);
		return 1;
	}

	fseek( rom, 0, SEEK_END );

	int rom_size = ftell( rom );
	if (rom_size>4096)
	{
		log->Fatal("Rom size > 4096...");
		return 1;
	}
	fseek( rom, 0, SEEK_SET );

	char rom_bin[4*1024];
	fread( rom_bin, 1, rom_size, rom );
	fclose(rom);

	log->Success("%s opened (%dB)", FileName, rom_size);


	log->Info("Creating CPU");
	cpu = new CPU();

	
	if ( cpu->Load( (uint8_t*)rom_bin, rom_size ) )
		log->Info("ROM copied to memory");
	else return 1;

	cpu->Reset();

	srand(time(NULL));
	log->Info("Seed initialized");

	SDL_Event event;
	bool running = true;
	bool CPUrunning = true;

	SDL_AddTimer( 16, TimerCallback, NULL );

	while(running)
	{
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT) running = false;

		uint8_t* keys = SDL_GetKeyboardState( NULL );
		
		if ( keys[SDL_SCANCODE_ESCAPE] ) running = false;
		
		for (int i = 0; i < 16; i++)
		{
			if (keys[KeyMatrix[i]]) cpu->keys[i] = true;
			else cpu->keys[i] = false;
		}


		if (CPUrunning)
		{
			if (!cpu->Step())
			{
				CPUrunning = false;
				log->Info("CPU stopped.");
			}
			if (cpu->ScreenUpdated)
			{
				cpu->ScreenUpdated = false;

				SDL_LockSurface( canvas );
				SDL_Color (*PIXEL)[64] = (SDL_Color(*)[64]) canvas->pixels;
				SDL_Color White = {0xff, 0xff, 0xff, 0xff};
				SDL_Color Black = {0, 0, 0, 0xff};
				for (int y = 0; y<32; y++)
				for (int x = 0; x<64; x++)
				{
					if (cpu->screen[y][x]) PIXEL[y][x] = White;
					else PIXEL[y][x] = Black;
				}
				SDL_UnlockSurface( canvas );
				SDL_SoftStretch( canvas, NULL, screen, NULL );
				SDL_UpdateWindowSurface( MainWindow );
			}
		}
		SDL_Delay(2);
	}

	SDL_FreeSurface( canvas );
	SDL_DestroyWindow( MainWindow );
	SDL_Quit();
	log->Info("Goodbye");

	return 0;
}