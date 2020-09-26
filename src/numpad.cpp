#include "numpad.h"
#include "binary.h"

#define LAYOUT_US_ENGLISH
#define HID_REPORTID_KEYBOARD
#define HID_REPORTID_CONSUMERCONTROL
#include "HID-Project.h"

// Key matrix logical layout
#define ROW_0 D,7
#define ROW_1 C,6
#define ROW_2 D,4
#define ROW_3 D,0
#define ROW_4 D,1
#define ROW_5 D,2
#define ROW_6 D,3
#define COL_01 B,6
#define COL_09 F,6
#define COL_11 B,1
#define COL_12 B,3
#define COL_13 B,2
#define COL_14 F,7
#define COL_16 F,4
#define COL_17 F,5

// ===== Keys physical layout =====
// [ 0] R0+C09 | [ 1] R1+C01 | [ 2] R0+C16 | [ 3] R6+C17
// [ 4] R4+C11 | [ 5] R4+C12 | [ 6] R4+C13 | [ 7] R5+C13
// [ 8] R0+C11 | [ 9] R0+C12 | [10] R0+C13 | [11] R0+C14
// [12] R1+C11 | [13] R1+C12 | [14] R1+C13 | [15] R5+C17
// [16] R2+C11 | [17] R2+C12 | [18] R2+C13 | [19] R1+C09
// [20] R3+C12 | [21] R6+C09 | [22] R3+C13 | [23] R2+C14

uint32_t keys; // Key pressed state, inverted
uint32_t prevKeys; // inverted
uint32_t pressedKeys; // Key pressed event flags
uint32_t releasedKeys; // Key released event flags
char symbols[26] = "0123456789ABCDEFGHIJKLMN.";

enum KeyState : uint8_t {
	KEY_PRESSED = 0,
	KEY_RELEASED = 1,
};

void initScan() {
	// Configure columns as inputs with pull-up
	PORTB |= bfi(COL_01) | bfi(COL_11) | bfi(COL_12) | bfi(COL_13);
	PORTF |= bfi(COL_09) | bfi(COL_14) | bfi(COL_16) | bfi(COL_17);
	// Keep rows in Hi-Z state
	// Doing the scan we drive down (open drain state) each row separately and test which columns became low
}

// Delay to eliminate switching transients
#define SCAN_DELAY delayMicroseconds(4)

//    ======== Scan layout ==========
//    C01 C09 C11 C12 C13 C14 C16 C17
// R0  .. [ 0][ 8][ 9][10][11][ 2] ..
// R1 [01][19][12][13][14] ..  ..  ..
// R2  ..  .. [16][17][18][23] ..  ..
// R3  ..  ..  .. [20][22] ..  ..  ..
// R4  ..  .. [ 4][ 5][ 6] ..  ..  ..
// R5  ..  ..  ..  .. [ 7] ..  .. [15]
// R6  .. [21] ..  ..  ..  ..  .. [ 3]
void scan() {
	spi_ddr(ROW_0); SCAN_DELAY;
	wrtbitsif(rpi(COL_09), keys, bit(0));
	wrtbitsif(rpi(COL_11), keys, bit(8));
	wrtbitsif(rpi(COL_12), keys, bit(9));
	wrtbitsif(rpi(COL_13), keys, bit(10));
	wrtbitsif(rpi(COL_14), keys, bit(11));
	wrtbitsif(rpi(COL_16), keys, bit(2));
	cpi_ddr(ROW_0);
	spi_ddr(ROW_1); SCAN_DELAY;
	wrtbitsif(rpi(COL_01), keys, bit(1));
	wrtbitsif(rpi(COL_09), keys, bit(19));
	wrtbitsif(rpi(COL_11), keys, bit(12));
	wrtbitsif(rpi(COL_12), keys, bit(13));
	wrtbitsif(rpi(COL_13), keys, bit(14));
	cpi_ddr(ROW_1);
	spi_ddr(ROW_2); SCAN_DELAY;
	wrtbitsif(rpi(COL_11), keys, bit(16));
	wrtbitsif(rpi(COL_12), keys, bit(17));
	wrtbitsif(rpi(COL_13), keys, bit(18));
	wrtbitsif(rpi(COL_14), keys, bit(23));
	cpi_ddr(ROW_2);
	spi_ddr(ROW_3); SCAN_DELAY;
	wrtbitsif(rpi(COL_12), keys, bit(20));
	wrtbitsif(rpi(COL_13), keys, bit(22));
	cpi_ddr(ROW_3);
	spi_ddr(ROW_4); SCAN_DELAY;
	wrtbitsif(rpi(COL_11), keys, bit(4));
	wrtbitsif(rpi(COL_12), keys, bit(5));
	wrtbitsif(rpi(COL_13), keys, bit(6));
	cpi_ddr(ROW_4);
	spi_ddr(ROW_5); SCAN_DELAY;
	wrtbitsif(rpi(COL_13), keys, bit(7));
	wrtbitsif(rpi(COL_17), keys, bit(15));
	cpi_ddr(ROW_5);
	spi_ddr(ROW_6); SCAN_DELAY;
	wrtbitsif(rpi(COL_09), keys, bit(21));
	wrtbitsif(rpi(COL_17), keys, bit(3));
	cpi_ddr(ROW_6);
}

KeyState getKeyState(uint8_t i) {
	return getbits(keys, bit(i)) == 0UL ? KeyState::KEY_PRESSED : KeyState::KEY_RELEASED;
}

bool isKeyPressed(uint8_t i) {
	return getbits(pressedKeys, bit(i)) != 0UL;
}

bool isKeyReleased(uint8_t i) {
	return getbits(releasedKeys, bit(i)) != 0UL;
}

uint8_t scanChanges() {
	uint8_t numChanges = (prevKeys != keys) ? 1 : 0;
	prevKeys = keys;
	return numChanges;
}

void printKeys() {
	for (uint8_t i = 0; i < 24; ++i) {
		Serial.print(getKeyState(i) == KeyState::KEY_PRESSED ? symbols[i] : symbols[24]);
	}
	Serial.println();
}

uint8_t scanPressed() {
	pressedKeys = (~keys & prevKeys);
	releasedKeys = (keys & ~prevKeys);
	prevKeys = keys;
	uint8_t numChanges = (pressedKeys | releasedKeys) ? 1 : 0;
	return numChanges;
}

void Keyboard_write(const char *str) {
	if (str == NULL)
		return;
	size_t size = strlen(str);
	while (size--) {
		if (!Keyboard.write(*str++))
			break;
	}
}

void printPressed() {
	if (isKeyPressed(0)) {
		// Shutdown (Win+X, U, U)
		Keyboard.press(KEY_RIGHT_WINDOWS);
		Keyboard.press(KEY_X);
		Keyboard.releaseAll();
		Keyboard.write(KEY_U);
		Keyboard.write(KEY_U);
	}
	if (isKeyPressed(1)) {
		// Sleep (Win+X, U, S)
		Keyboard.press(KEY_RIGHT_WINDOWS);
		Keyboard.press(KEY_X);
		Keyboard.releaseAll();
		Keyboard.write(KEY_U);
		Keyboard.write(KEY_S);
	}
	if (isKeyPressed(2)) {
		// Open Calc
		Consumer.write(ConsumerKeycode::CONSUMER_CALCULATOR);
	}
	if (isKeyPressed(3)) {
		// Toogle Desktop (Win+D)
		Keyboard.press(KEY_RIGHT_WINDOWS);
		Keyboard.press(KEY_D);
		Keyboard.releaseAll();
	}
	if (isKeyReleased(4)) {
		// Check Mark (U+2714)
		/*Keyboard.press(KEY_LEFT_ALT);
		Keyboard.write(KEYPAD_ADD);
		Keyboard.write(KEY_2);
		Keyboard.write(KEY_7);
		Keyboard.write(KEY_1);
		Keyboard.write(KEY_4);
		Keyboard.release(KEY_LEFT_ALT);*/
		Keyboard_write(":heavy_check_mark:");
	}
	if (isKeyReleased(5)) {
		// Red Heart (U+2764)
		/*Keyboard.press(KEY_LEFT_ALT);
		Keyboard.write(KEYPAD_ADD);
		Keyboard.write(KEY_2);
		Keyboard.write(KEY_7);
		Keyboard.write(KEY_6);
		Keyboard.write(KEY_4);
		Keyboard.release(KEY_LEFT_ALT);*/
		Keyboard_write(":heart:");
	}
	if (isKeyReleased(6)) {
		// Anger Symbol (U+1F4A2)
		Keyboard_write(":anger:");
	}
	if (isKeyReleased(7)) {
		// Sparkles (U+2728)
		/*Keyboard.press(KEY_LEFT_ALT);
		Keyboard.write(KEYPAD_ADD);
		Keyboard.write(KEY_2);
		Keyboard.write(KEY_7);
		Keyboard.write(KEY_2);
		Keyboard.write(KEY_8);
		Keyboard.release(KEY_LEFT_ALT);*/
		Keyboard_write(":sparkles:");
	}
	if (isKeyReleased(8)) {
		// Wheelchair (U+267F)
		/*Keyboard.press(KEY_LEFT_ALT);
		Keyboard.write(KEYPAD_ADD);
		Keyboard.write(KEY_2);
		Keyboard.write(KEY_6);
		Keyboard.write(KEY_7);
		Keyboard.write(KEY_F);
		Keyboard.release(KEY_LEFT_ALT);*/
		Keyboard_write(":wheelchair:");
	}
	if (isKeyReleased(9)) {
		// Biohazard (U+2623)
		/*Keyboard.press(KEY_LEFT_ALT);
		Keyboard.write(KEYPAD_ADD);
		Keyboard.write(KEY_2);
		Keyboard.write(KEY_6);
		Keyboard.write(KEY_2);
		Keyboard.write(KEY_3);
		Keyboard.release(KEY_LEFT_ALT);*/
		Keyboard_write(":biohazard:");
	}
	if (isKeyReleased(10)) {
		// Star (U+2B50)
		/*Keyboard.press(KEY_LEFT_ALT);
		Keyboard.write(KEYPAD_ADD);
		Keyboard.write(KEY_2);
		Keyboard.write(KEY_B);
		Keyboard.write(KEY_5);
		Keyboard.write(KEY_0);
		Keyboard.release(KEY_LEFT_ALT);*/
		Keyboard_write(":star:");
	}
	if (isKeyReleased(11)) {
		// Woman Dancing (U+1F483)
		Keyboard_write(":dancer:");
	}
	if (isKeyReleased(12)) {
		// Turtle (U+1F422)
		Keyboard_write(":turtle:");
	}
	if (isKeyReleased(13)) {
		// Lady Beetle (U+1F41E)
		Keyboard_write(":beetle:");
	}
	if (isKeyReleased(14)) {
		// Butterfly (U+1F98B)
		Keyboard_write(":butterfly:");
	}
	if (isKeyReleased(15)) {
		// Fairy (U+1F9DA)
		Keyboard_write(":fairy:");
	}
	if (isKeyReleased(16)) {
		// Snail (U+1F40C)
		Keyboard_write(":snail:");
	}
	if (isKeyReleased(17)) {
		// Bug (U+1F41B)
		Keyboard_write(":bug:");
	}
	if (isKeyPressed(18)) {
		Keyboard_write(":cherry_blossom:");
	}
	if (isKeyPressed(19)) {
		Keyboard_write(":ribbon:");
	}
	if (isKeyPressed(20)) {
		// Play/Pause
		Consumer.write(ConsumerKeycode::MEDIA_PLAY_PAUSE);
	}
	if (isKeyPressed(21)) {
		// Previous track
		Consumer.write(ConsumerKeycode::MEDIA_PREV);
	}
	if (isKeyPressed(22)) {
		// Next track
		Consumer.write(ConsumerKeycode::MEDIA_NEXT);
	}
	if (isKeyPressed(23)) {
		// Mute/Unmute
		Consumer.write(ConsumerKeycode::MEDIA_VOL_MUTE);
	}
	int err = Keyboard.getWriteError();
	if (err != 0) {
		Serial.println("Error: ");
		Serial.println(err);
		Keyboard.clearWriteError();
	}
	pressedKeys = 0UL;
}

void setup() {
	Serial.begin(9600);
	//Serial.println("Multimedia numpad");
	Keyboard.begin();
	Consumer.begin();
	initScan();
}

void loop() {
	scan();
	if (scanPressed() != 0) {
		printPressed();
	} else {
		delay(10); // 100 Hz
	}
}
