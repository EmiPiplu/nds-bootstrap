#pragma GCC optimize ("Os")

#include "platinum_hud.h"

#include <nds/ndstypes.h>
#include <nds/arm9/background.h>
#include <nds/arm9/video.h>

#include "locations.h"
#include "tonccpy.h"


/*
 * We use tile numbers equal to ASCII character codes.
 *
 * ' ' = 0x20
 * ...
 * 'Z' = 0x5A
 *
 * 0x3B tiles * 32 bytes = 0x760 bytes.
 */
#define PLAT_HUD_TILE_FIRST 0x20
#define PLAT_HUD_TILE_LAST  0x5A
#define PLAT_HUD_TILE_COUNT \
	(PLAT_HUD_TILE_LAST - PLAT_HUD_TILE_FIRST + 1)

#define PLAT_HUD_TILE_BACKUP_SIZE \
	(PLAT_HUD_TILE_COUNT * 32)

/*
 * Visible part of a 32x32 text map:
 *
 * 32 columns * 24 rows * 2 bytes = 0x600.
 */
#define PLAT_HUD_MAP_WORDS (32 * 24)
#define PLAT_HUD_MAP_BACKUP_SIZE \
	(PLAT_HUD_MAP_WORDS * sizeof(u16))


/*
 * Borrow the same extended IGM scratch area used by the IGM's
 * VRAM backup.
 *
 * The normal IGM hotkey is suppressed while the Platinum
 * debugger is BLOCKED, so the two users cannot run at once.
 */
#define PLAT_HUD_SCRATCH \
	((u8 *)(INGAME_MENU_EXT_LOCATION + 0x18200))

#define PLAT_HUD_TILE_BACKUP \
	((u8 *)PLAT_HUD_SCRATCH)

#define PLAT_HUD_MAP_BACKUP \
	((u16 *)(PLAT_HUD_SCRATCH + PLAT_HUD_TILE_BACKUP_SIZE))


typedef struct {
	bool active;

	u32 dispcnt;

	u16 bg0cnt;
	u16 bg1cnt;
	u16 bg2cnt;
	u16 bg3cnt;

	u16 bg3hofs;
	u16 bg3vofs;

	u8 vramCCr;
	u8 vramHCr;

	u16 palette0;
	u16 palette1;

	u16 masterBright;
} PlatinumHudState;


static PlatinumHudState platinumHudState;


/*
 * Tiny 5x7 font.
 *
 * Each byte contains one five-pixel row in bits 4..0.
 */

static const u8 platinumHudDigits[10][7] = {
	{0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E}, // 0
	{0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E}, // 1
	{0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F}, // 2
	{0x1E, 0x01, 0x01, 0x0E, 0x01, 0x01, 0x1E}, // 3
	{0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}, // 4
	{0x1F, 0x10, 0x10, 0x1E, 0x01, 0x01, 0x1E}, // 5
	{0x0E, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x0E}, // 6
	{0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08}, // 7
	{0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}, // 8
	{0x0E, 0x11, 0x11, 0x0F, 0x01, 0x01, 0x0E}, // 9
};


static const u8 platinumHudLetters[26][7] = {
	{0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}, // A
	{0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E}, // B
	{0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E}, // C
	{0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E}, // D
	{0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F}, // E
	{0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10}, // F
	{0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F}, // G
	{0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}, // H
	{0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E}, // I
	{0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C}, // J
	{0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11}, // K
	{0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F}, // L
	{0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11}, // M
	{0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11}, // N
	{0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}, // O
	{0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10}, // P
	{0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D}, // Q
	{0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11}, // R
	{0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E}, // S
	{0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}, // T
	{0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}, // U
	{0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04}, // V
	{0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A}, // W
	{0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11}, // X
	{0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04}, // Y
	{0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F}, // Z
};


static const u8 platinumHudBlank[7] = {
	0, 0, 0, 0, 0, 0, 0
};


static const u8 *platinumHudGlyph(unsigned char c) {
	if (c >= '0' && c <= '9') {
		return platinumHudDigits[c - '0'];
	}

	if (c >= 'A' && c <= 'Z') {
		return platinumHudLetters[c - 'A'];
	}

	return platinumHudBlank;
}


static void platinumHudBuildGlyph(unsigned char c) {
	if (c < PLAT_HUD_TILE_FIRST ||
	    c > PLAT_HUD_TILE_LAST) {
		return;
	}

	/*
	 * DS VRAM must be written using 16/32-bit accesses.
	 *
	 * One 4bpp 8x8 tile:
	 *   64 pixels * 4 bits = 32 bytes = 16 u16s.
	 */
	u16 *tile =
		BG_GFX_SUB + (c * 16);

	toncset16(
		tile,
		0,
		16
	);

	const u8 *rows =
		platinumHudGlyph(c);

	for (int y = 0; y < 7; y++) {
		u8 row = rows[y];

		for (int x = 0; x < 5; x++) {
			if (!(row & (1 << (4 - x)))) {
				continue;
			}

			/*
			 * Centre our 5-pixel glyph inside the
			 * 8-pixel-wide tile.
			 */
			int px = x + 1;

			/*
			 * Four 4bpp pixels per u16.
			 */
			int pixel = y * 8 + px;
			int word = pixel >> 2;
			int shift = (pixel & 3) * 4;

			tile[word] |=
				(u16)(1u << shift);
		}
	}
}


static void platinumHudBuildFont(void) {
	for (unsigned int c = PLAT_HUD_TILE_FIRST;
	     c <= PLAT_HUD_TILE_LAST;
	     c++) {

		platinumHudBuildGlyph((unsigned char)c);
	}
}


static void platinumHudPutChar(
	int x,
	int y,
	unsigned char c
) {
	if (x < 0 || x >= 32 ||
	    y < 0 || y >= 24) {
		return;
	}

	if (c < PLAT_HUD_TILE_FIRST ||
	    c > PLAT_HUD_TILE_LAST) {
		c = ' ';
	}

	BG_MAP_RAM_SUB(15)[(y * 32) + x] = c;
}


static void platinumHudPrint(
	int x,
	int y,
	const char *text
) {
	while (*text && x < 32) {
		platinumHudPutChar(
			x++,
			y,
			(unsigned char)*text++
		);
	}
}


static void platinumHudPrintHex(
	int x,
	int y,
	u32 value
) {
	for (int i = 7; i >= 0; i--) {
		u8 digit = value & 0xF;

		platinumHudPutChar(
			x + i,
			y,
			digit < 10
				? '0' + digit
				: 'A' + digit - 10
		);

		value >>= 4;
	}
}

void platinumHudDrawTracking(
	u32 currentRng,
	bool haveInitialSeed,
	u32 initialSeed,
	u32 advances
) {
	toncset16(
		BG_MAP_RAM_SUB(15),
		0,
		PLAT_HUD_MAP_WORDS
	);

	platinumHudPrint(
		9,
		2,
		"PLATINUM RNG"
	);

	platinumHudPrint(
		4,
		6,
		"RNG"
	);

	platinumHudPrintHex(
		10,
		6,
		currentRng
	);

	if (haveInitialSeed) {
		platinumHudPrint(
			4,
			9,
			"SEED"
		);

		platinumHudPrintHex(
			10,
			9,
			initialSeed
		);

		platinumHudPrint(
			4,
			12,
			"ADV"
		);

		platinumHudPrintHex(
			10,
			12,
			advances
		);
	} else {
		platinumHudPrint(
			4,
			9,
			"SEED NOT READY"
		);
	}

	platinumHudPrint(
		4,
		19,
		"R STEP"
	);

	platinumHudPrint(
		4,
		21,
		"START RUN"
	);
}

void platinumHudDrawSetup(
	u8 selection,
	const char *methodName
) {
	toncset16(
		BG_MAP_RAM_SUB(15),
		0,
		PLAT_HUD_MAP_WORDS
	);

	platinumHudPrint(9, 2, "PLATINUM RNG");

	static const char *labels[] = {
		"POKEMON",
		"METHOD",
		"CONDITIONS",
		"CONFIRM",
	};

	for (u8 i = 0; i < 4; i++) {
		int y = 6 + (i * 3);

		platinumHudPutChar(
			2,
			y,
			selection == i ? 'X' : ' '
		);

		platinumHudPrint(
			4,
			y,
			labels[i]
		);
	}

	platinumHudPrint(
		16,
		6,
		speciesName
	);

	platinumHudPrint(
		16,
		9,
		methodName
	);

	platinumHudPrint(
		16,
		12,
		"ANY"
	);

	platinumHudPrint(4, 20, "A SELECT");
	platinumHudPrint(4, 22, "START RUN");
}

void platinumHudDrawPokemon(
	u8 selection,
	const char *const *items,
	u8 count
) {
	toncset16(
		BG_MAP_RAM_SUB(15),
		0,
		PLAT_HUD_MAP_WORDS
	);

	platinumHudPrint(
		9,
		2,
		"POKEMON"
	);

	for (u8 i = 0; i < count; i++) {
		int y = 5 + (i * 3);

		platinumHudPutChar(
			2,
			y,
			selection == i ? 'X' : ' '
		);

		platinumHudPrint(
			4,
			y,
			items[i]
		);
	}

	platinumHudPrint(4, 20, "A CONFIRM");
	platinumHudPrint(4, 22, "B CANCEL");
}

void platinumHudDrawMethod(u8 selection)
{
	toncset16(
		BG_MAP_RAM_SUB(15),
		0,
		PLAT_HUD_MAP_WORDS
	);

	platinumHudPrint(
		10,
		2,
		"METHOD"
	);

	static const char *items[] = {
		"STARTER",
		"WILD",
		"STATIONARY",
	};

	for (u8 i = 0; i < 3; i++) {
		int y = 7 + (i * 3);

		platinumHudPutChar(
			4,
			y,
			selection == i ? 'X' : ' '
		);

		platinumHudPrint(
			6,
			y,
			items[i]
		);
	}

	platinumHudPrint(
		4,
		19,
		"A CONFIRM"
	);

	platinumHudPrint(
		4,
		21,
		"B CANCEL"
	);
}

void platinumHudEnter(void) {
	if (platinumHudState.active) {
		return;
	}

	/*
	 * Save Platinum's sub-screen configuration.
	 */
	platinumHudState.dispcnt =
		REG_DISPCNT_SUB;

	platinumHudState.bg0cnt =
		REG_BG0CNT_SUB;

	platinumHudState.bg1cnt =
		REG_BG1CNT_SUB;

	platinumHudState.bg2cnt =
		REG_BG2CNT_SUB;

	platinumHudState.bg3cnt =
		REG_BG3CNT_SUB;

	platinumHudState.bg3hofs =
		REG_BG3HOFS_SUB;

	platinumHudState.bg3vofs =
		REG_BG3VOFS_SUB;

	platinumHudState.vramCCr =
		VRAM_C_CR;

	platinumHudState.vramHCr =
		VRAM_H_CR;

	platinumHudState.masterBright =
    *(vu16 *)0x0400106C;

	/*
	 * Match the IGM's known-working VRAM arrangement.
	 *
	 * If C is currently mapped as sub BG memory, temporarily
	 * remove it so H is the bank backing 0x06200000.
	 */
	if (VRAM_C_CR & 4) {
		VRAM_C_CR = VRAM_C_LCD;
	}

	VRAM_H_CR =
		VRAM_ENABLE |
		VRAM_H_SUB_BG;

	/*
	 * Now H is accessible through BG_GFX_SUB /
	 * BG_MAP_RAM_SUB, so save only the regions we're
	 * actually going to overwrite.
	 */
	tonccpy(
		PLAT_HUD_TILE_BACKUP,
		((u8 *)BG_GFX_SUB) +
			(PLAT_HUD_TILE_FIRST * 32),
		PLAT_HUD_TILE_BACKUP_SIZE
	);

	tonccpy(
		PLAT_HUD_MAP_BACKUP,
		BG_MAP_RAM_SUB(15),
		PLAT_HUD_MAP_BACKUP_SIZE
	);

	platinumHudState.palette0 =
		BG_PALETTE_SUB[0];

	platinumHudState.palette1 =
		BG_PALETTE_SUB[1];

	/*
	 * Configure a simple 4bpp text BG.
	 */
	REG_DISPCNT_SUB =
		MODE_0_2D |
		DISPLAY_BG3_ACTIVE;

	REG_BG0CNT_SUB = 0;
	REG_BG1CNT_SUB = 0;
	REG_BG2CNT_SUB = 0;

	REG_BG3CNT_SUB =
		(u16)(
			BG_MAP_BASE(15) |
			BG_TILE_BASE(0) |
			BgSize_T_256x256
		);

	REG_BG3HOFS_SUB = 0;
	REG_BG3VOFS_SUB = 0;

	/*
	 * Palette index 0 = black background.
	 * Palette index 1 = white font.
	 */
	BG_PALETTE_SUB[0] = 0x0000;
	BG_PALETTE_SUB[1] = 0x7FFF;

	/*
	* Match the real nds-bootstrap IGM display setup.
	*
	* Platinum may have blending/fade state active on the sub engine.
	* If we leave that state intact, our BG3 text can be rendered
	* correctly but then blended completely to black.
	*/

	*(vu16 *)0x0400106C = 0;

	REG_MOSAIC_SUB = 0;
	REG_BLDCNT_SUB = 0;
	REG_BLDALPHA_SUB = 0;
	REG_BLDY_SUB = 0;

	platinumHudBuildFont();

	platinumHudState.active = true;

}


void platinumHudLeave(void) {
	if (!platinumHudState.active) {
		return;
	}

	/*
	 * H is still mapped as sub BG memory here, so restore
	 * everything we changed before restoring its mapping.
	 */
	tonccpy(
		((u8 *)BG_GFX_SUB) +
			(PLAT_HUD_TILE_FIRST * 32),
		PLAT_HUD_TILE_BACKUP,
		PLAT_HUD_TILE_BACKUP_SIZE
	);

	tonccpy(
		BG_MAP_RAM_SUB(15),
		PLAT_HUD_MAP_BACKUP,
		PLAT_HUD_MAP_BACKUP_SIZE
	);

	BG_PALETTE_SUB[0] =
		platinumHudState.palette0;

	BG_PALETTE_SUB[1] =
		platinumHudState.palette1;

	/*
	 * Restore the game's VRAM mappings first.
	 */
	VRAM_H_CR =
		platinumHudState.vramHCr;

	VRAM_C_CR =
		platinumHudState.vramCCr;

	/*
	 * Then restore the sub-screen registers.
	 */
	REG_BG0CNT_SUB =
		platinumHudState.bg0cnt;

	REG_BG1CNT_SUB =
		platinumHudState.bg1cnt;

	REG_BG2CNT_SUB =
		platinumHudState.bg2cnt;

	REG_BG3CNT_SUB =
		platinumHudState.bg3cnt;

	REG_BG3HOFS_SUB =
		platinumHudState.bg3hofs;

	REG_BG3VOFS_SUB =
		platinumHudState.bg3vofs;

	REG_DISPCNT_SUB =
		platinumHudState.dispcnt;

	*(vu16 *)0x0400106C =
		platinumHudState.masterBright;


	platinumHudState.active = false;
}