// Tiny headless GBA driver for smoke-testing ROM hacks with mGBA's core.
// Usage: gbadrive ROM SCRIPT
// Script commands (one per line):
//   load STATEFILE          load a savestate
//   save STATEFILE          write a savestate
//   wait N                  run N frames with no keys
//   press KEYS [N]          hold keys (e.g. A, A+B, UP) for N frames (default 6), then release 6 frames
//   hold KEYS N             hold keys for N frames, no release frames
//   mash KEY TIMES          press KEY TIMES times (6 on / 10 off)
//   shot FILE.ppm           save the current frame
//   peek ADDR               print the byte at hex ADDR
#include <mgba/flags.h>
#include <mgba/core/core.h>
#include <mgba/core/serialize.h>
#include <mgba/internal/gba/input.h>
#include <mgba-util/vfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct mCore* core;
static mColor* buffer;

static int keyBit(const char* k) {
    if (!strcmp(k, "A")) return 1 << GBA_KEY_A;
    if (!strcmp(k, "B")) return 1 << GBA_KEY_B;
    if (!strcmp(k, "SELECT")) return 1 << GBA_KEY_SELECT;
    if (!strcmp(k, "START")) return 1 << GBA_KEY_START;
    if (!strcmp(k, "RIGHT")) return 1 << GBA_KEY_RIGHT;
    if (!strcmp(k, "LEFT")) return 1 << GBA_KEY_LEFT;
    if (!strcmp(k, "UP")) return 1 << GBA_KEY_UP;
    if (!strcmp(k, "DOWN")) return 1 << GBA_KEY_DOWN;
    if (!strcmp(k, "R")) return 1 << GBA_KEY_R;
    if (!strcmp(k, "L")) return 1 << GBA_KEY_L;
    fprintf(stderr, "unknown key %s\n", k);
    return 0;
}

static int parseKeys(char* s) {
    int keys = 0;
    for (char* t = strtok(s, "+"); t; t = strtok(NULL, "+"))
        keys |= keyBit(t);
    return keys;
}

static void run(int keys, int frames) {
    core->setKeys(core, keys);
    for (int i = 0; i < frames; i++)
        core->runFrame(core);
}

static void shot(const char* path) {
    unsigned w, h;
    core->currentVideoSize(core, &w, &h);
    FILE* f = fopen(path, "wb");
    fprintf(f, "P6\n%u %u\n255\n", w, h);
    for (unsigned i = 0; i < w * h; i++) {
        uint32_t c = buffer[i];
        unsigned char rgb[3] = { c & 0xFF, (c >> 8) & 0xFF, (c >> 16) & 0xFF };
        fwrite(rgb, 1, 3, f);
    }
    fclose(f);
}

int main(int argc, char** argv) {
    if (argc < 3) { fprintf(stderr, "usage: %s ROM SCRIPT\n", argv[0]); return 1; }
    core = mCoreFind(argv[1]);
    if (!core || !core->init(core)) { fprintf(stderr, "no core\n"); return 1; }
    mCoreInitConfig(core, NULL);
    unsigned w, h;
    core->baseVideoSize(core, &w, &h);
    buffer = malloc(w * h * sizeof(mColor));
    core->setVideoBuffer(core, buffer, w);
    if (!mCoreLoadFile(core, argv[1])) { fprintf(stderr, "load failed\n"); return 1; }
    core->reset(core);

    FILE* script = fopen(argv[2], "r");
    char line[256];
    while (fgets(line, sizeof(line), script)) {
        char cmd[32] = {0}, a[200] = {0};
        int n = -1;
        if (line[0] == '#' || sscanf(line, "%31s", cmd) != 1) continue;
        sscanf(line, "%31s %199s %d", cmd, a, &n);
        if (!strcmp(cmd, "wait")) run(0, atoi(a));
        else if (!strcmp(cmd, "press")) { run(parseKeys(a), n > 0 ? n : 6); run(0, 6); }
        else if (!strcmp(cmd, "hold")) run(parseKeys(a), n);
        else if (!strcmp(cmd, "mash")) { int k = parseKeys(a); for (int i = 0; i < n; i++) { run(k, 6); run(0, 10); } }
        else if (!strcmp(cmd, "shot")) shot(a);
        else if (!strcmp(cmd, "peek")) {  // peek ADDR(hex) -> prints the byte
            unsigned addr = strtoul(a, NULL, 16);
            printf("PEEK %08x %u\n", addr, (unsigned) core->busRead8(core, addr));
            fflush(stdout);
        }
        else if (!strcmp(cmd, "save")) {
            struct VFile* vf = VFileOpen(a, O_CREAT | O_TRUNC | O_RDWR);
            mCoreSaveStateNamed(core, vf, SAVESTATE_ALL);
            vf->close(vf);
        } else if (!strcmp(cmd, "load")) {
            struct VFile* vf = VFileOpen(a, O_RDONLY);
            if (!vf || !mCoreLoadStateNamed(core, vf, SAVESTATE_ALL)) fprintf(stderr, "loadstate failed %s\n", a);
            if (vf) vf->close(vf);
        } else fprintf(stderr, "unknown cmd %s\n", cmd);
    }
    core->deinit(core);
    return 0;
}
