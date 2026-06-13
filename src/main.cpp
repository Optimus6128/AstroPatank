#include <dos.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>

#include "types.h"

#include "game.h"
#include "video.h"
#include "timer.h"
#include "input.h"
#include "sound.h"
#include "musplay.h"
#include "tinyfont.h"

//#define MEM_DEBUG

#ifdef __DJGPP__
	#include <sys/nearptr.h>
	#include <dpmi.h>
	#include <go32.h>
#endif

static Video *video;
static Screen screen;
static bool vsync = true;

uint8 *VGAptr = (uint8*)0xA0000;
uint8 *TXTptr = (uint8*)0xB8000;
uint16 *my_clock = (uint16*)0x046C;


static void interpretArgument(char *arg)
{
	if (!strcmp(arg, "-b")) {
		vsync = false;
	}
}

static void initSystem()
{

	#ifdef __DJGPP__
		__djgpp_nearptr_enable();

		VGAptr += __djgpp_conventional_base;
		TXTptr += __djgpp_conventional_base;
		my_clock = (uint16*)((uint8*)my_clock + __djgpp_conventional_base);
	#endif

	#ifdef SOUND_ON
		initSound();
		loadMusDriver();
		//delay(500);
	#endif

	initTimer();
	initKeyboard();

	initVideoModeInfo();

	#ifndef SCR_UNCHAINED
		video = setVideoMode(SCR_W, SCR_H, SCR_BPP, true);
	#else
		video = setVideoMode(SCR_W, SCR_H, SCR_BPP, false, true);
	#endif

	if (video==0) {
		printf("Video Mode not found\n");
		return;
	}

	screen.width = video->width;
	screen.height = video->height;
	screen.bpp = video->bpp;
	screen.data = getRenderBuffer(video);

	clearFrame(video);

	initTinyFonts();
}

typedef struct FreeMemInfo {
	uint32 largest_available_block;
	uint32 max_unlocked_page;
	uint32 max_locked_page;
	uint32 total_unlocked;
	uint32 total_free;
	uint32 total_pages;
	uint32 free_linear;
	uint32 swap_file_size;
	uint32 dummy[3];
} FreeMemInfo;

#ifdef MEM_DEBUG
uint32 getFreeMem() {
	#ifdef __DJGPP__
		__dpmi_free_mem_info memInfo = {0};

        if (__dpmi_get_free_memory_information(&memInfo) == 0) {
            //return (uint32)memInfo.largest_available_free_block_in_bytes;
			return (uint32)memInfo.total_number_of_free_pages * 4096;
		}
		return 0;
	#else
		FreeMemInfo memInfo;
		memset(&memInfo, 0xFF, sizeof(memInfo));

		union REGS regs = {0};
		struct SREGS sregs = {0};

		sregs.es = FP_SEG(&memInfo);
		regs.x.edi = FP_OFF(&memInfo);
		regs.x.eax = 0x0500;

		int386x(0x31, &regs, &regs, &sregs);

		if (regs.x.cflag == 0) {
			return memInfo.largest_available_block;
		}
		return 0;
	#endif
}
#endif

int main(int argc, char **argv)
{
	#ifdef MEM_DEBUG
	uint32 mem0 = getFreeMem();
	#endif

	for (int i=1; i<argc; ++i) {
		interpretArgument(argv[i]);
	}

	initSystem();

	gameInit();

	#ifdef MEM_DEBUG
	uint32 mem1 = getFreeMem();
	#endif
	
	while(!isGameQuit()) {
		screen.data = getRenderBuffer(video);
		gameRun(&screen, getTime());
		/*if (buttonsHeld.select)*/ drawFps(video);
		updateFrame(video, vsync);
	}

	deinitTimer();
	deinitKeyboard();

	#ifdef SOUND_ON
		shutdownMusPlay();
		deinitSound();
	#endif

	setTextMode();

	#ifdef __DJGPP__
		__djgpp_nearptr_disable();
	#endif

	#ifdef MEM_DEBUG
	uint32 mem2 = getFreeMem();
	printf("%d\n%d\n%d\n", mem0, mem1, mem2);
	#endif


	return 0;
}
