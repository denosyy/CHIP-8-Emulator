#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <SDL/SDL.h>

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

/* Key statuses are defined as registers 0x00-0x0F */
#define CHIP8_REG_DT        0x10
#define CHIP8_REG_ST        0x11

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
uint16_t opcode;
bool drawflag;

const uint8_t font[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,
    0x20, 0x60, 0x20, 0x20, 0x70,
    0xF0, 0x10, 0xF0, 0x80, 0xF0,
    0xF0, 0x10, 0xF0, 0x10, 0xF0,
    0x90, 0x90, 0xF0, 0x10, 0x10,
    0xF0, 0x80, 0xF0, 0x10, 0xF0,
    0xF0, 0x80, 0xF0, 0x90, 0xF0,
    0xF0, 0x10, 0x20, 0x40, 0x40,
    0xF0, 0x90, 0xF0, 0x90, 0xF0,
    0xF0, 0x90, 0xF0, 0x10, 0xF0,
    0xF0, 0x90, 0xF0, 0x90, 0x90,
    0xE0, 0x90, 0xE0, 0x90, 0xE0,
    0xF0, 0x80, 0x80, 0x80, 0xF0,
    0xE0, 0x90, 0x90, 0x90, 0xE0,
    0xF0, 0x80, 0xF0, 0x80, 0xF0,
    0xF0, 0x80, 0xF0, 0x80, 0x80
};




void chip8_init(void) {
    PC = 0x200;
    opcode = 0;
    index_register = 0;
    stack_ptr = 0;
    dt = 0;
    st = 0;
    memset(stack, 0, 16);
    memset(memory, 0, 4096);
    memset(V, 0, 16);
    memset(framebuffer, 0, 2048);
    memset(buttons, 0, 16);
    memcpy(memory, font, 80 * sizeof(int8_t));
}

void chip8_shutdown(void) {

}
void chip8_execute_instruction(void) {
    uint8_t X, Y, kk, n;
    uint16_t nnn;
    uint32_t i, button_pressed;
    opcode = memory[PC] << 8 | memory[PC + 1];
    PC += 2;
    X = (opcode & 0x0F00) >> 8;
    Y = (opcode & 0x00F0) >> 4;
    nnn = (opcode & 0x0FFF);
    kk = (opcode & 0x00FF);
    n = (opcode & 0x000F);
    switch (opcode & 0xF000) {
    case 0x0000:
        switch (opcode & 0x00FF) {
            //00E0 - clear screen
        case 0x00E0:
            memset(framebuffer, 0, 2048);
            break;
            //00EE - return from subroutine
        case 0x00EE:
        {
            --stack_ptr;
            PC = stack[stack_ptr];
            break;
        }
        default: printf("Opcode error 0xxx -> %x\n", opcode);
        }break;
    case 0x1000: //1NNN - jump to address nnn
        PC = nnn;
        break;
    case 0x2000: //2NNN - call subroutine at nnn
        stack[stack_ptr] = PC;
        ++stack_ptr;
        PC = nnn;
        break;
    case 0x3000: //3XNN - skip next instruction if VX == kk
        if (V[X] == kk) PC += 2;
        break;
    case 0x4000: //4XNN - skip next instruction if VX != kk
        if (V[X] != kk) PC += 2;
        break;
    case 0x6000: //6XNN - set register VX
        V[X] = kk;
        break;
    case 0x7000: //7XNN - add value to register VX
        V[X] += kk;
        break;
    case 0xA000: //ANNN - set index register
        index_register = nnn;
        break;
    case 0xD000: //DXYN - draw/display
    {
        uint16_t addr;
        uint8_t x = V[X];
        uint8_t y = V[Y];
        uint8_t height = n;
        chip8_draw_sprite(addr, x, y, height);
        break;
    }
    case 0x5000: //5XY0 - skips next instruction if VX == VY
        if (V[X] == V[Y]) PC += 2;
        break;
    case 0x8000:
        switch (n) {
            //8XY0 - set VX to the value of VY
        case 0x0000:
        {
            V[X] == V[Y];
            break;
        }
        //8XY1 - set VX to VX or VY
        case 0x0001:
        {
            V[X] |= V[Y];
            break;
        }
        //8XY2 - set VY to VX and VY
        case 0x0002:
        {
            V[X] &= V[Y];
            break;
        }
        //8XY3 - set VY to VX xor VY
        case 0x0003:
        {
            V[X] ^= V[Y];
            break;
        }
        //8XY4 - adds VX to VY; VF is set to 1 if there's a carry and 0 if there isn't
        case 0x0004:
        {
            int i = (int)(V[X]) + (int)(V[Y]);
            if (i > 255)
                V[0xF] = 1;
            else
                V[0xF] = 0;
            V[X] = i & 0xFF;
            break;
        }
        //8XY5 - VY is subtracted from VX; VF is set to 0 when there's a borrow, and 1 when there isn't
        case 0x0005:
        {
            if (V[X] > V[Y]) V[0xF] = 1;
            else V(0xF) = 0;
            V[X] -= V[Y];
            break;
        }
        //8XY6 - shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift
        case 0x0006:
        {
            V[0xF] = V[X] & 1;
            V[X] >>= 1;
            break;
        }
        //8XY7 - set VX to VY minus VX. VF is set to 0 when there's a borrow and 1 when there isn't
        case 0x0007:
        {
            if (V[Y] > V[X]) V[0xF] = 1;
            else V[0xF] = 0;
            V[X] = V[Y] - V[X];
            break;
        }
        //8XYE - shift VX left by one. VF is set to the value of the most significant bit of VX before the shift
        case 0x000E:
        {
            V[0xF] = V[X] >> 7;
            V[X] <<= 1;
            break;
        }
        default: printf("Opcode error 8xxx -> % x\n", opcode);
        }break;
    case 0x9000: //9XY0 - skips the next instruction if VX doesn't equal VY
    {
        if (V[X] != V[Y]) PC += 2;
        break;
    }

    }

}
void chip8_reset(void) {
    PC = 0x200;
    opcode = 0;
    index_register = 0;
    stack_ptr = 0;
    dt = 0;
    st = 0;
    memset(stack, 0, 16);
    memset(memory, 0, 4096);
    memset(V, 0, 16);
    memset(framebuffer, 0, 2048);
    memset(buttons, 0, 16);
    memcpy(memory, font, 80 * sizeof(int8_t));
}

int WinMain(int argc, char* argv[]) {
    chip8_init();
    while (PC < 4096) {
        chip8_execute_instruction();
    }
    chip8_shutdown();
    return 0;
}


