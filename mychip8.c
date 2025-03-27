#include <stdint.h>
#include <stdio.h>

#define MEMORY_SIZE 4096
#define MEMORY_MASK (MEMORY_SIZE - 1)

extern uint8_t chip8_mem_read(uint16_t addr);
extern void chip8_mem_write(uint16_t addr, uint8_t val);
extern uint8_t chip8_register_read(uint8_t reg);
extern void chip8_register_write(uint8_t reg, uint8_t val);
extern void chip8_clear_frame(void);
extern void chip8_mem_clear(void);
extern int chip8_draw_sprite(uint16_t addr, uint8_t x, uint8_t y, uint8_t height);
extern void chip8_mem_reset(void);

typedef struct {
	uint8_t memory[MEMORY_SIZE];
	uint32_t framebuffer[64 * 32];
	uint8_t dt, st; //delay timer, sound timer
	uint8_t buttons[16];
	uint8_t keymap[16];
	uint16_t V[16]; //general registers
	uint16_t PC; //program counter
	uint8_t SP; //stack pointer
	uint16_t stack[16]; //stack
	uint16_t stack_ptr; //index pointer
	uint16_t index_register;

} chip8;

/* Key statuses are defined as registers 0x00-0x0F */
#define CHIP8_REG_DT        0x10
#define CHIP8_REG_ST        0x11

int main() {
	return 0;
}

uint16_t fetch(chip8* chip) {

}

extern void chip8_init(void);
extern void chip8_shutdown(void);
extern void chip8_execute_instruction(void);
extern void chip8_reset(void);


