
#include <SDL2/SDL.h>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

vector<string> rom;
vector<uint16_t> ram(32768, 0);

uint16_t a, d, m;
int instrptr = 0;
int amountifInstr = 500000000;
int steps = 0;

enum OP {
    ZERO, ONE, MINUSONE,
    D, A, M,
    NOTD, NOTA, NOTM,
    NEGD, NEGA, NEGM,
    DPLUS1, APLUS1, MPLUS1,
    DMINUS1, AMINUS1, MMINUS1,
    DPLUSA, DPLUSM,
    DMINUSA, DMINUSM,
    AMINUSD, MMINUSD,
    DANDA, DANDM,
    DORA, DORM
};

enum JUMP {
    NOJUMP, JGT, JEQ, JGE,
    JLT, JNE, JLE, JMP
};

unordered_map<string, OP> comp({
    {"0101010", ZERO}, {"0111111", ONE}, {"0111010", MINUSONE},
    {"0001100", D}, {"0110000", A}, {"1110000", M},
    {"0001101", NOTD}, {"0110001", NOTA}, {"1110001", NOTM},
    {"0001111", NEGD}, {"0110011", NEGA}, {"1110011", NEGM},
    {"0011111", DPLUS1}, {"0110111", APLUS1}, {"1110111", MPLUS1},
    {"0001110", DMINUS1}, {"0110010", AMINUS1}, {"1110010", MMINUS1},
    {"0000010", DPLUSA}, {"1000010", DPLUSM}, {"0010011", DMINUSA},
    {"1010011", DMINUSM}, {"0000111", AMINUSD}, {"1000111", MMINUSD},
    {"0000000", DANDA}, {"1000000", DANDM}, {"0010101", DORA},
    {"1010101", DORM}
});

unordered_map<string, JUMP> jump({
    {"000", NOJUMP},
    {"001", JGT},
    {"010", JEQ},
    {"011", JGE},
    {"100", JLT},
    {"101", JNE},
    {"110", JLE},
    {"111", JMP}
});

void performComp(string instr) {

    uint16_t computedVal = 0;
    string compinstr = rom[instrptr].substr(3, 7);
    string jumpinstr = rom[instrptr].substr(13, 3);

    uint16_t currentA = a;

    switch (comp[compinstr]) {

    case ZERO:
        computedVal = 0;
        break;
    case ONE:
        computedVal = 1;
        break;
    case MINUSONE:
        computedVal = -1;
        break;
    case D:
        computedVal = d;
        break;
    case A:
        computedVal = a;
        break;
    case M:
        computedVal = ram[a & 0x7FFF];
        break;
    case NOTD:
        computedVal = ~d;
        break;
    case NOTA:
        computedVal = ~a;
        break;
    case NOTM:
        computedVal = ~ram[a & 0x7FFF];
        break;
    case NEGD:
        computedVal = -d;
        break;
    case NEGA:
        computedVal = -a;
        break;
    case NEGM:
        computedVal = -ram[a & 0x7FFF];
        break;
    case DPLUS1:
        computedVal = d + 1;
        break;
    case APLUS1:
        computedVal = a + 1;
        break;
    case MPLUS1:
        computedVal = ram[a & 0x7FFF] + 1;
        break;
    case DMINUS1:
        computedVal = d - 1;
        break;
    case AMINUS1:
        computedVal = a - 1;
        break;
    case MMINUS1:
        computedVal = ram[a & 0x7FFF] - 1;
        break;
    case DPLUSA:
        computedVal = d + a;
        break;
    case DPLUSM:
        computedVal = d + ram[a & 0x7FFF];
        break;
    case DMINUSA:
        computedVal = d - a;
        break;
    case DMINUSM:
        computedVal = d - ram[a & 0x7FFF];
        break;
    case AMINUSD:
        computedVal = a - d;
        break;
    case MMINUSD:
        computedVal = ram[a & 0x7FFF] - d;
        break;
    case DANDA:
        computedVal = d & a;
        break;
    case DANDM:
        computedVal = d & ram[a & 0x7FFF];
        break;
    case DORA:
        computedVal = d | a;
        break;
    case DORM:
        computedVal = d | ram[a & 0x7FFF];
        break;
    }

    if (instr[10] == '1')
        a = computedVal;

    if (instr[11] == '1')
        d = computedVal;

    if (instr[12] == '1')
        ram[currentA & 0x7FFF] = computedVal;

    bool isZero = (computedVal == 0);
    bool isNegative = (computedVal & 0x8000) != 0;
    bool isPositive = !isZero && !isNegative;

    bool shouldJump = false;

    switch (jump[jumpinstr]) {

    case NOJUMP:
        shouldJump = false;
        break;
    case JGT:
        shouldJump = isPositive;
        break;
    case JEQ:
        shouldJump = isZero;
        break;
    case JGE:
        shouldJump = !isNegative;
        break;
    case JLT:
        shouldJump = isNegative;
        break;
    case JNE:
        shouldJump = !isZero;
        break;
    case JLE:
        shouldJump = isNegative || isZero;
        break;
    case JMP:
        shouldJump = true;
        break;
    }

    if (shouldJump) {
        instrptr = currentA & 0x7FFF;
    }
    else {
        instrptr++;
    }
}

// SCREEN CONFIGURATION

const int SCREEN_WIDTH = 512;
const int SCREEN_HEIGHT = 256;
const int SCREEN_BASE = 16384;

const int WINDOW_WIDTH = SCREEN_WIDTH * 3;
const int WINDOW_HEIGHT = SCREEN_HEIGHT * 3;

const int KBD = 24576;

const int CPU_BATCH_SIZE = 10000;
const Uint64 FRAME_INTERVAL = 1000 / 60;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* screenTexture = nullptr;

vector<uint32_t> pixels(SCREEN_WIDTH * SCREEN_HEIGHT);

bool running = true;

// KEYBOARD

uint16_t getHackKeyCode(SDL_Keycode key, SDL_Keymod mod) {

    if (key >= SDLK_a && key <= SDLK_z) {

        if (mod & KMOD_SHIFT)
            return static_cast<uint16_t>('A' + (key - SDLK_a));

        return static_cast<uint16_t>('a' + (key - SDLK_a));
    }

    if (key >= SDLK_0 && key <= SDLK_9)
        return static_cast<uint16_t>(key);

    if (key >= SDLK_SPACE && key <= SDLK_SLASH)
        return static_cast<uint16_t>(key);

    switch (key) {

    case SDLK_RETURN:
    case SDLK_KP_ENTER:
        return 128;

    case SDLK_BACKSPACE:
        return 129;

    case SDLK_LEFT:
        return 130;

    case SDLK_UP:
        return 131;

    case SDLK_RIGHT:
        return 132;

    case SDLK_DOWN:
        return 133;

    case SDLK_HOME:
        return 134;

    case SDLK_END:
        return 135;

    case SDLK_PAGEUP:
        return 136;

    case SDLK_PAGEDOWN:
        return 137;

    case SDLK_INSERT:
        return 138;

    case SDLK_DELETE:
        return 139;

    case SDLK_ESCAPE:
        return 140;

    case SDLK_F1:
        return 141;

    case SDLK_F2:
        return 142;

    case SDLK_F3:
        return 143;

    case SDLK_F4:
        return 144;

    case SDLK_F5:
        return 145;

    case SDLK_F6:
        return 146;

    case SDLK_F7:
        return 147;

    case SDLK_F8:
        return 148;

    case SDLK_F9:
        return 149;

    case SDLK_F10:
        return 150;

    case SDLK_F11:
        return 151;

    case SDLK_F12:
        return 152;

    default:
        return 0;
    }
}

void handleEvents() {

    SDL_Event event;

    while (SDL_PollEvent(&event)) {

        if (event.type == SDL_QUIT) {
            running = false;
        }

        else if (event.type == SDL_KEYDOWN) {

            uint16_t keyCode =
                getHackKeyCode(
                    event.key.keysym.sym,
                    static_cast<SDL_Keymod>(event.key.keysym.mod)
                );

            if (keyCode != 0) {
                ram[KBD] = keyCode;
            }
        }

        else if (event.type == SDL_KEYUP) {
            ram[KBD] = 0;
        }
    }
}

// SCREEN TEXTURE INITIALIZATION

void initScreenTexture() {

    screenTexture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH,
        SCREEN_HEIGHT
    );

    if (!screenTexture) {
        cerr << "Texture creation failed: "
             << SDL_GetError() << '\n';
        running = false;
    }
}

// SCREEN RENDERING

void renderScreen() {

    // Convert screen RAM into pixel buffer

    for (int y = 0; y < SCREEN_HEIGHT; y++) {

        for (int x = 0; x < SCREEN_WIDTH; x++) {

            int address = SCREEN_BASE + y * 32 + x / 16;
            int bit = x % 16;

            bool isBlack = (ram[address] >> bit) & 1;

            pixels[y * SCREEN_WIDTH + x] =
                isBlack ? 0xFF000000 : 0xFFFFFFFF;
        }
    }

    // Upload pixel buffer to SDL texture

    SDL_UpdateTexture(
        screenTexture,
        nullptr,
        pixels.data(),
        SCREEN_WIDTH * sizeof(uint32_t)
    );

    // Render texture

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    SDL_Rect destination = {
        0, 0,
        SCREEN_WIDTH,
        SCREEN_HEIGHT
    };

    SDL_RenderCopy(
        renderer,
        screenTexture,
        nullptr,
        &destination
    );

    SDL_RenderPresent(renderer);
}

// CPU INSTRUCTION EXECUTION

bool executeInstruction() {

    if (instrptr < 0 || instrptr >= static_cast<int>(rom.size())) {

        cerr << "Program counter out of ROM bounds: "
             << instrptr << '\n';

        return false;
    }

    if (rom[instrptr].empty()) {
        cerr << "Empty instruction at ROM address "
             << instrptr << '\n';
        return false;
    }

    // A instruction

    if (rom[instrptr][0] == '0') {

        string valsubstr = rom[instrptr].substr(1, 15);

        uint16_t val = stoi(valsubstr, nullptr, 2);

        a = val;
        instrptr++;
    }

    // C instruction

    else if (rom[instrptr].substr(0, 3) == "111") {

        performComp(rom[instrptr]);
    }

    else {

        cerr << "Invalid instruction at ROM address "
             << instrptr << ": "
             << rom[instrptr] << '\n';

        return false;
    }

    steps++;

    return true;
}

// MAIN

int main(int argc, char** argv) {

    // Argument validation

    if (argc != 4) {

        cout << "Usage: cpu <filename> <start_address> <end_address>\n";
        return 1;
    }

    string inputfilename = string(argv[1]);

    // Read ROM file

    ifstream inputfile(inputfilename);

    if (!inputfile.is_open()) {

        cout << "Couldn't open file named: "
             << inputfilename << "\n";

        return 1;
    }

    string line;

    while (getline(inputfile, line)) {
        rom.push_back(line);
    }

    cout << "Instructions total: " << rom.size() << "\n";

    // Initialize SDL

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {

        cerr << "SDL initialization failed: "
             << SDL_GetError() << '\n';

        return 1;
    }

    window = SDL_CreateWindow(
        "Hack Computer Screen",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (!window) {

        cerr << "Window creation failed: "
             << SDL_GetError() << '\n';

        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_SOFTWARE
    );

    if (!renderer) {

        cerr << "Renderer creation failed: "
             << SDL_GetError() << '\n';

        SDL_DestroyWindow(window);
        SDL_Quit();

        return 1;
    }

    SDL_RenderSetScale(renderer, 3.0f, 3.0f);

    // Initialize screen texture

    initScreenTexture();

    if (!running) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // CPU EXECUTION LOOP

    bool cpuRunning = true;

    Uint64 lastRender = SDL_GetTicks64();

    while (running && cpuRunning && amountifInstr > 0) {

        // Execute instructions in batches

        for (int i = 0;
             i < CPU_BATCH_SIZE && amountifInstr > 0;
             i++) {

            if (!executeInstruction()) {
                cpuRunning = false;
                break;
            }

            amountifInstr--;
        }

        // Process window and keyboard events

        handleEvents();

        if (!running)
            break;

        // Render at approximately 60 FPS

        Uint64 now = SDL_GetTicks64();

        if (now - lastRender >= FRAME_INTERVAL) {

            renderScreen();

            lastRender = now;
        }
    }

    // PRINT RAM VALUES

    int start = stoi(argv[2]);
    int end = stoi(argv[3]);

    for (int i = start; i <= end; i++) {

        if (i < 0 || i >= static_cast<int>(ram.size())) {
            cerr << "RAM address out of bounds: " << i << '\n';
            break;
        }

        cout << "| " << ram[i] << " | " << i << "\n";
    }

    // Final screen render

    renderScreen();

    // Keep window open after CPU execution finishes

    while (running) {

        handleEvents();

        SDL_Delay(16);
    }

    // CLEANUP

    SDL_DestroyTexture(screenTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
