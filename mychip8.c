#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <SDL/SDL.h>

#define MEMORY_SIZE 4096
#define MEMORY_MASK (MEMORY_SIZE - 1)

void chip8_init(void);
void chip8_execute_instruction(void);
void chip8_shutdown(void);
void chip8_reset(void);
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

uint8_t buttons[16];
uint8_t V[16]; //general registers
uint16_t PC; //program counter
uint16_t I; //index ptr
uint8_t SP; //stack pointer
uint16_t stack[16]; //stack
uint16_t opcode;

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
    chip8_mem_reset();
    PC = 0x200;
    opcode = 0;
    I = 0;
    SP = 0;
    chip8_register_write(CHIP8_REG_DT, 0);
    chip8_register_write(CHIP8_REG_ST, 0);
    for (int i = 0; i < 16; i++) { V[i] = 0; }
    //for (int i = 0; i < 16; i++) { buttons[i] = 0; }
    //chip8_clear_frame();
}

void chip8_shutdown(void) {
    chip8_clear_frame();
    chip8_mem_clear();
}
void chip8_execute_instruction(void) {
    uint8_t X, Y, kk, n;
    uint16_t nnn;
    uint32_t button_pressed;
    opcode = chip8_mem_read(PC) << 8 | chip8_mem_read(PC + 1);
    PC += 2;
    X = (opcode >> 8) & 0x0F;
    Y = (opcode >> 4) & 0x0F;
    nnn = (opcode & 0x0FFF);
    kk = (opcode & 0x00FF);
    n = (opcode & 0x000F);
    switch (opcode & 0xF000) {
    case 0x0000:
        switch (opcode & 0x00FF) {
            //00E0 - clear screen
        case 0x00E0:
            chip8_clear_frame();
            break;
            //00EE - return from subroutine
        case 0x00EE:
        {
            SP--;
            PC = stack[SP];
            break;
        }
        }break;
    case 0x1000: //1NNN - jump to address nnn
        PC = nnn;
        break;
    case 0x2000: //2NNN - call subroutine at nnn
        stack[SP] = PC;
        SP++;
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
        I = nnn;
        break;
    case 0xD000: //DXYN - draw/display
    {
        chip8_draw_sprite(I, V[X], V[Y], n);
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
            V[X] = V[Y];
            break;
        }
        //8XY1 - set VX to VX or VY
        case 0x0001:
        {
            V[X] = V[X] | V[Y];
            break;
        }
        //8XY2 - set VY to VX and VY
        case 0x0002:
        {
            V[X] = V[X] & V[Y];
            break;
        }
        //8XY3 - set VY to VX xor VY
        case 0x0003:
        {
            V[X] = V[X] ^ V[Y];
            break;
        }
        //8XY4 - adds VX to VY; VF is set to 1 if there's a carry and 0 if there isn't
        case 0x0004:
        {
            if ((V[X] + V[Y]) > 255)
                V[15] = 1;
            else
                V[15] = 0;

            V[X] += V[Y];
            break;
        }
        //8XY5 - VY is subtracted from VX; VF is set to 0 when there's a borrow, and 1 when there isn't
        case 0x0005:
        {
            if (V[X] > V[Y]) V[15] = 1;
            else V[15] = 0;
            V[X] -= V[Y];
            break;
        }
        //8XY6 - shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift
        case 0x0006:
        {
            V[15] = V[X] & 0x1;
            V[X] = V[X] >> 1;
            break;
        }
        //8XY7 - set VX to VY minus VX. VF is set to 0 when there's a borrow and 1 when there isn't
        case 0x0007:
        {
            if (V[Y] > V[X]) V[15] = 1;
            else V[15] = 0;
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
        default: break;
        }
    case 0x9000: //9XY0 - skips the next instruction if VX doesn't equal VY
    {
        if (V[X] != V[Y]) { PC += 2; }
        break;
    }
    case 0xB000: //BNNN - jumps to address NNN plus V0
    {
        PC = (nnn)+V[0x0];
        break;
    }
    case 0xC000: //CXNN - set VX to a random number, masked by NN
    {
        V[X] = (rand() % 256) & (kk);
        break;
    }
    case 0xE000:
        switch (kk) {
            //EX9E - skips next instruction if the key stored in VX is pressed
        case 0x009E:
        {
            if (chip8_register_read(V[X])) PC += 2;
            break;
        }
        //EXA1 - skips next instruction if the key stored in VX isn't pressed
        case 0x00A1:
        {
            if (!chip8_register_read(V[X])) PC += 2;
            break;
        }
        }break;
    case 0xF000:
        switch (kk) {
            //FX07 - sets VX to value of the delay timer
        case 0x0007:
        {
            V[X] = chip8_register_read(CHIP8_REG_DT);
            break;
        }
        //FX0A - a key press is expected, then stored in VX
        case 0x000A:
        {
            V[X] = 0;
            int value;
            for (int i = 0; i < 16; i++) {
                value = chip8_register_read(i);
                if (value != 0) { V[X] = value; }
            }
            if (value == 0) { PC += 2; }
        }
        //FX15 - sets delay timer to VX
        case 0x0015:
        {
            chip8_register_write(CHIP8_REG_DT, V[X]);
            break;
        }
        //FX18 - sets sound timer to VX
        case 0x0018:
        {
            chip8_register_write(CHIP8_REG_ST, V[X]);
            break;
        }
        //FX1E - add VX to I
        case 0x001E:
        {
            I = I + V[X];
            break;
        }
        //FX29 - set I to the location of the sprite for the character in VX
        case 0x0029:
        {
            I = V[X] * 5;
            break;
        }
        //FX33 - stores the binary-coded representation of VX at the addresses I, I + 1, I + 2
        case 0x0033:
        {
            uint8_t num = V[X];
            chip8_mem_write((I + 2), num % 10);
            num /= 10;
            chip8_mem_write((I + 1), num % 10);
            num /= 10;
            chip8_mem_write((I), num % 10);
            break;

        }
        //FX55 - stores V0 to VX in memory starting at address I
        case 0x0055:
        {
            for (uint8_t i = 0; i <= X; ++i) {
                chip8_mem_write(I + i, V[i]);
            }
            break;
        }
        //FX65
        case 0x0065:
        {
            for (uint8_t i = 0; i <= X; ++i) {
                V[i] = chip8_mem_read(I + i);
            }
            break;
        }
        break;
        }
    default: break;
    }
}

void chip8_reset(void) {
    chip8_mem_reset();
    PC = 0x200;
    opcode = 0;
    I = 0;
    SP = 0;
    chip8_register_write(CHIP8_REG_DT, 0);
    chip8_register_write(CHIP8_REG_ST, 0);
    //memset(stack, 0, 16);
    for (int i = 0; i < 16; i++) { V[i] = 0; }
    for (int i = 0; i < 16; i++) { buttons[i] = 0; }
    //chip8_mem_reset();
    //chip8_clear_frame();
    //memset(buttons, 0, 16);
}




