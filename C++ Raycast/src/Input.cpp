#include "Input.h"

#include <cstdio>

// ===========================================================================
//  Windows implementation: GetAsyncKeyState gives true held-key state, which
//  is ideal for smooth continuous movement.
// ===========================================================================
#ifdef _WIN32
#include <windows.h>

Input::Input() {
    hOut_ = GetStdHandle(STD_OUTPUT_HANDLE);

    // Enable ANSI escape sequences (\x1b[H etc.) on the output handle.
    DWORD mode = 0;
    if (GetConsoleMode(static_cast<HANDLE>(hOut_), &mode)) {
        prevOutMode_ = mode;
        SetConsoleMode(static_cast<HANDLE>(hOut_),
                       mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }

    std::fputs("\x1b[2J", stdout);    // one-time clear
    std::fputs("\x1b[?25l", stdout);  // hide cursor
    std::fflush(stdout);
}

Input::~Input() {
    std::fputs("\x1b[?25h", stdout);  // show cursor
    std::fflush(stdout);
    if (hOut_) {
        SetConsoleMode(static_cast<HANDLE>(hOut_), prevOutMode_);
    }
}

void Input::poll() {
    auto down = [](int vk) { return (GetAsyncKeyState(vk) & 0x8000) != 0; };

    fwd_    = down('W') || down(VK_UP);
    back_   = down('S') || down(VK_DOWN);
    left_   = down('A') || down(VK_LEFT);
    right_  = down('D') || down(VK_RIGHT);
    sLeft_  = down('Q');
    sRight_ = down('E');
    if (down(VK_ESCAPE) || down('X')) quit_ = true;
}

// ===========================================================================
//  POSIX implementation: raw, non-blocking termios. There are no key-up
//  events, so each frame we reset the flags and set them from the bytes that
//  arrived this frame. Key auto-repeat keeps held movement going.
// ===========================================================================
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

Input::Input() {
    tcgetattr(STDIN_FILENO, &orig_);

    termios raw = orig_;
    raw.c_lflag &= ~(ICANON | ECHO);   // unbuffered, no echo
    raw.c_cc[VMIN]  = 0;               // read() returns immediately...
    raw.c_cc[VTIME] = 0;               // ...even with no data
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);

    origFlags_ = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, origFlags_ | O_NONBLOCK);

    std::fputs("\x1b[2J", stdout);     // one-time clear
    std::fputs("\x1b[?25l", stdout);   // hide cursor
    std::fflush(stdout);
}

Input::~Input() {
    std::fputs("\x1b[?25h", stdout);   // show cursor
    std::fflush(stdout);
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_);
    fcntl(STDIN_FILENO, F_SETFL, origFlags_);
}

void Input::poll() {
    // Forget last frame's transient keys; quit_ latches once set.
    fwd_ = back_ = left_ = right_ = sLeft_ = sRight_ = false;

    char buf[64];
    const int n = static_cast<int>(::read(STDIN_FILENO, buf, sizeof(buf)));

    for (int i = 0; i < n; ++i) {
        const unsigned char c = static_cast<unsigned char>(buf[i]);

        if (c == 27) { // ESC: either a lone quit key or an arrow-key sequence
            if (i + 2 < n && buf[i + 1] == '[') {
                switch (buf[i + 2]) {
                    case 'A': fwd_   = true; break; // up
                    case 'B': back_  = true; break; // down
                    case 'C': right_ = true; break; // right
                    case 'D': left_  = true; break; // left
                }
                i += 2;
            } else {
                quit_ = true;
            }
            continue;
        }

        switch (c) {
            case 'w': case 'W': fwd_    = true; break;
            case 's': case 'S': back_   = true; break;
            case 'a': case 'A': left_   = true; break;
            case 'd': case 'D': right_  = true; break;
            case 'q': case 'Q': sLeft_  = true; break;
            case 'e': case 'E': sRight_ = true; break;
            case 'x': case 'X': quit_   = true; break;
        }
    }
}

#endif
