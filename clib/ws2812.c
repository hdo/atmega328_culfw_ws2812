#include <stdint.h>
#include "ws2812.h"
#include "light_ws2812.h"
#include "display.h"
#include "rf_receive.h"

#define LED_COUNT 40
#define COLOR_COUNT 141
#define NO_BUTTON 0xFF


const struct cRGB predefined[COLOR_COUNT] = {
	    { 0xF8, 0xF0, 0xFF }, // 000 -> AliceBlue
	    { 0xEB, 0xFA, 0xD7 }, // 001 -> AntiqueWhite
	    { 0xFF, 0x00, 0xFF }, // 002 -> Aqua
	    { 0xFF, 0x7F, 0xD4 }, // 003 -> Aquamarine
	    { 0xFF, 0xF0, 0xFF }, // 004 -> Azure
	    { 0xF5, 0xF5, 0xDC }, // 005 -> Beige
	    { 0xE4, 0xFF, 0xC4 }, // 006 -> Bisque
	    { 0x00, 0x00, 0x00 }, // 007 -> Black
	    { 0xEB, 0xFF, 0xCD }, // 008 -> BlanchedAlmond
	    { 0x00, 0x00, 0xFF }, // 009 -> Blue
	    { 0x2B, 0x8A, 0xE2 }, // 010 -> BlueViolet
	    { 0x2A, 0xA5, 0x2A }, // 011 -> Brown
	    { 0xB8, 0xDE, 0x87 }, // 012 -> BurlyWood
	    { 0x9E, 0x5F, 0xA0 }, // 013 -> CadetBlue
	    { 0xFF, 0x7F, 0x00 }, // 014 -> Chartreuse
	    { 0x69, 0xD2, 0x1E }, // 015 -> Chocolate
	    { 0x7F, 0xFF, 0x50 }, // 016 -> Coral
	    { 0x95, 0x64, 0xED }, // 017 -> CornflowerBlue
	    { 0xF8, 0xFF, 0xDC }, // 018 -> Cornsilk
	    { 0x14, 0xDC, 0x3C }, // 019 -> Crimson
	    { 0xFF, 0x00, 0xFF }, // 020 -> Cyan
	    { 0x00, 0x00, 0x8B }, // 021 -> DarkBlue
	    { 0x8B, 0x00, 0x8B }, // 022 -> DarkCyan
	    { 0x86, 0xB8, 0x0B }, // 023 -> DarkGoldenRod
	    { 0xA9, 0xA9, 0xA9 }, // 024 -> DarkGray
	    { 0x64, 0x00, 0x00 }, // 025 -> DarkGreen
	    { 0xB7, 0xBD, 0x6B }, // 026 -> DarkKhaki
	    { 0x00, 0x8B, 0x8B }, // 027 -> DarkMagenta
	    { 0x6B, 0x55, 0x2F }, // 028 -> DarkOliveGreen
	    { 0x8C, 0xFF, 0x00 }, // 029 -> DarkOrange
	    { 0x32, 0x99, 0xCC }, // 030 -> DarkOrchid
	    { 0x00, 0x8B, 0x00 }, // 031 -> DarkRed
	    { 0x96, 0xE9, 0x7A }, // 032 -> DarkSalmon
	    { 0xBC, 0x8F, 0x8F }, // 033 -> DarkSeaGreen
	    { 0x3D, 0x48, 0x8B }, // 034 -> DarkSlateBlue
	    { 0x4F, 0x2F, 0x4F }, // 035 -> DarkSlateGray
	    { 0xCE, 0x00, 0xD1 }, // 036 -> DarkTurquoise
	    { 0x00, 0x94, 0xD3 }, // 037 -> DarkViolet
	    { 0x14, 0xFF, 0x93 }, // 038 -> DeepPink
	    { 0xBF, 0x00, 0xFF }, // 039 -> DeepSkyBlue
	    { 0x69, 0x69, 0x69 }, // 040 -> DimGray
	    { 0x90, 0x1E, 0xFF }, // 041 -> DodgerBlue
	    { 0x22, 0xB2, 0x22 }, // 042 -> FireBrick
	    { 0xFA, 0xFF, 0xF0 }, // 043 -> FloralWhite
	    { 0x8B, 0x22, 0x22 }, // 044 -> ForestGreen
	    { 0x00, 0xFF, 0xFF }, // 045 -> Fuchsia
	    { 0xDC, 0xDC, 0xDC }, // 046 -> Gainsboro
	    { 0xF8, 0xF8, 0xFF }, // 047 -> GhostWhite
	    { 0xD7, 0xFF, 0x00 }, // 048 -> Gold
	    { 0xA5, 0xDA, 0x20 }, // 049 -> GoldenRod
	    { 0x80, 0x80, 0x80 }, // 050 -> Gray
	    { 0x80, 0x00, 0x00 }, // 051 -> Green
	    { 0xFF, 0xAD, 0x2F }, // 052 -> GreenYellow
	    { 0xFF, 0xF0, 0xF0 }, // 053 -> HoneyDew
	    { 0x69, 0xFF, 0xB4 }, // 054 -> HotPink
	    { 0x5C, 0xCD, 0x5C }, // 055 -> IndianRed
	    { 0x00, 0x4B, 0x82 }, // 056 -> Indigo
	    { 0xFF, 0xFF, 0xF0 }, // 057 -> Ivory
	    { 0xE6, 0xF0, 0x8C }, // 058 -> Khaki
	    { 0xE6, 0xE6, 0xFA }, // 059 -> Lavender
	    { 0xF0, 0xFF, 0xF5 }, // 060 -> LavenderBlush
	    { 0xFC, 0x7C, 0x00 }, // 061 -> LawnGreen
	    { 0xFA, 0xFF, 0xCD }, // 062 -> LemonChiffon
	    { 0xD8, 0xAD, 0xE6 }, // 063 -> LightBlue
	    { 0x80, 0xF0, 0x80 }, // 064 -> LightCoral
	    { 0xFF, 0xE0, 0xFF }, // 065 -> LightCyan
	    { 0xFA, 0xFA, 0xD2 }, // 066 -> LightGoldenRodYellow
	    { 0xD3, 0xD3, 0xD3 }, // 067 -> LightGray
	    { 0xEE, 0x90, 0x90 }, // 068 -> LightGreen
	    { 0xB6, 0xFF, 0xC1 }, // 069 -> LightPink
	    { 0xA0, 0xFF, 0x7A }, // 070 -> LightSalmon
	    { 0xB2, 0x20, 0xAA }, // 071 -> LightSeaGreen
	    { 0xCE, 0x87, 0xFA }, // 072 -> LightSkyBlue
	    { 0x88, 0x77, 0x99 }, // 073 -> LightSlateGray
	    { 0xC4, 0xB0, 0xDE }, // 074 -> LightSteelBlue
	    { 0xFF, 0xFF, 0xE0 }, // 075 -> LightYellow
	    { 0xFF, 0x00, 0x00 }, // 076 -> Lime
	    { 0xCD, 0x32, 0x32 }, // 077 -> LimeGreen
	    { 0xF0, 0xFA, 0xE6 }, // 078 -> Linen
	    { 0x00, 0xFF, 0xFF }, // 079 -> Magenta
	    { 0x00, 0x80, 0x00 }, // 080 -> Maroon
	    { 0xCD, 0x66, 0xAA }, // 081 -> MediumAquaMarine
	    { 0x00, 0x00, 0xCD }, // 082 -> MediumBlue
	    { 0x55, 0xBA, 0xD3 }, // 083 -> MediumOrchid
	    { 0x70, 0x93, 0xDB }, // 084 -> MediumPurple
	    { 0xB3, 0x3C, 0x71 }, // 085 -> MediumSeaGreen
	    { 0x68, 0x7B, 0xEE }, // 086 -> MediumSlateBlue
	    { 0xFA, 0x00, 0x9A }, // 087 -> MediumSpringGreen
	    { 0xD1, 0x48, 0xCC }, // 088 -> MediumTurquoise
	    { 0x15, 0xC7, 0x85 }, // 089 -> MediumVioletRed
	    { 0x19, 0x19, 0x70 }, // 090 -> MidnightBlue
	    { 0xFF, 0xF5, 0xFA }, // 091 -> MintCream
	    { 0xE4, 0xFF, 0xE1 }, // 092 -> MistyRose
	    { 0xE4, 0xFF, 0xB5 }, // 093 -> Moccasin
	    { 0xDE, 0xFF, 0xAD }, // 094 -> NavajoWhite
	    { 0x00, 0x00, 0x80 }, // 095 -> Navy
	    { 0xF5, 0xFD, 0xE6 }, // 096 -> OldLace
	    { 0x80, 0x80, 0x00 }, // 097 -> Olive
	    { 0x8E, 0x6B, 0x23 }, // 098 -> OliveDrab
	    { 0xA5, 0xFF, 0x00 }, // 099 -> Orange
	    { 0x45, 0xFF, 0x00 }, // 100 -> OrangeRed
	    { 0x70, 0xDA, 0xD6 }, // 101 -> Orchid
	    { 0xE8, 0xEE, 0xAA }, // 102 -> PaleGoldenRod
	    { 0xFB, 0x98, 0x98 }, // 103 -> PaleGreen
	    { 0xEE, 0xAF, 0xEE }, // 104 -> PaleTurquoise
	    { 0x70, 0xDB, 0x93 }, // 105 -> PaleVioletRed
	    { 0xEF, 0xFF, 0xD5 }, // 106 -> PapayaWhip
	    { 0xDA, 0xFF, 0xB9 }, // 107 -> PeachPuff
	    { 0x85, 0xCD, 0x3F }, // 108 -> Peru
	    { 0xC0, 0xFF, 0xCB }, // 109 -> Pink
	    { 0xA0, 0xDD, 0xDD }, // 110 -> Plum
	    { 0xE0, 0xB0, 0xE6 }, // 111 -> PowderBlue
	    { 0x00, 0x80, 0x80 }, // 112 -> Purple
	    { 0x33, 0x66, 0x99 }, // 113 -> RebeccaPurple
	    { 0x00, 0xFF, 0x00 }, // 114 -> Red
	    { 0x8F, 0xBC, 0x8F }, // 115 -> RosyBrown
	    { 0x69, 0x41, 0xE1 }, // 116 -> RoyalBlue
	    { 0x45, 0x8B, 0x13 }, // 117 -> SaddleBrown
	    { 0x80, 0xFA, 0x72 }, // 118 -> Salmon
	    { 0xA4, 0xF4, 0x60 }, // 119 -> SandyBrown
	    { 0x8B, 0x2E, 0x57 }, // 120 -> SeaGreen
	    { 0xF5, 0xFF, 0xEE }, // 121 -> SeaShell
	    { 0x52, 0xA0, 0x2D }, // 122 -> Sienna
	    { 0xC0, 0xC0, 0xC0 }, // 123 -> Silver
	    { 0xCE, 0x87, 0xEB }, // 124 -> SkyBlue
	    { 0x5A, 0x6A, 0xCD }, // 125 -> SlateBlue
	    { 0x80, 0x70, 0x90 }, // 126 -> SlateGray
	    { 0xFA, 0xFF, 0xFA }, // 127 -> Snow
	    { 0xFF, 0x00, 0x7F }, // 128 -> SpringGreen
	    { 0x82, 0x46, 0xB4 }, // 129 -> SteelBlue
	    { 0xB4, 0xD2, 0x8C }, // 130 -> Tan
	    { 0x80, 0x00, 0x80 }, // 131 -> Teal
	    { 0xBF, 0xD8, 0xD8 }, // 132 -> Thistle
	    { 0x63, 0xFF, 0x47 }, // 133 -> Tomato
	    { 0xE0, 0x40, 0xD0 }, // 134 -> Turquoise
	    { 0x82, 0xEE, 0xEE }, // 135 -> Violet
	    { 0xDE, 0xF5, 0xB3 }, // 136 -> Wheat
	    { 0xFF, 0xFF, 0xFF }, // 137 -> White
	    { 0xF5, 0xF5, 0xF5 }, // 138 -> WhiteSmoke
	    { 0xFF, 0xFF, 0x00 }, // 139 -> Yellow
	    { 0xCD, 0x9A, 0x32 }, // 140 -> YellowGreen
};

uint8_t last_button = NO_BUTTON;
uint8_t current_color_index = 9; // BLUE
struct cRGB led[LED_COUNT];
uint32_t myTicks = 0;
uint32_t lastEventTicks = 0;
uint8_t pendingUpdate = 0;

uint32_t math_calc_diff(uint32_t value1, uint32_t value2) {
	if (value1 == value2) {
		return 0;
	}
	if (value1 > value2) {
		return (value1 - value2);
	}
	else {
		// check for overflow
		return (0xffffffff - value2 + value1);
	}
}

void ws2812_update(void) {
	pendingUpdate = 0;
	for(uint8_t i=0; i < LED_COUNT; i++) {
		led[i] = predefined[current_color_index];
	}
	ws2812_setleds(led, LED_COUNT);
}

void ws2812_init(void) {
	current_color_index = 9; // BLUE
	ws2812_update();
}

void ws2812_task(uint32_t currentTicks) {
	myTicks = currentTicks;
	if (last_button != NO_BUTTON) {
		DS("BUTTON: ");
		DH2(last_button);
		DNL();
		switch(last_button) {
		case 0: current_color_index = 9;    break; // blue
		case 1: current_color_index = 114;  break; // red
		case 2: current_color_index = 51;   break; // green
		case 3: current_color_index = 139;  break; // yellow
		case 4: current_color_index = 19;   break; // crimson
		case 5: current_color_index = 2;    break; // aqua
		case 6: current_color_index = (current_color_index-1) % COLOR_COUNT; break;
		case 7: current_color_index = (current_color_index+1) % COLOR_COUNT; break;
		}
		DS("COLOR: ");
		DH2(current_color_index);
		DNL();
		last_button = NO_BUTTON;
		lastEventTicks = currentTicks;
		pendingUpdate = 1;
		//ws2812_update();
	}
	if (pendingUpdate && math_calc_diff(currentTicks, lastEventTicks) > 15) {
		lastEventTicks = currentTicks; // reset
		ws2812_update();
	}
	return;
}

void ws2812_register_remote_button(uint8_t button) {
	last_button = button;
	return;
}
