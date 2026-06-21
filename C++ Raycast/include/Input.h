#pragma once
//
// Input.h - Non-blocking keyboard abstraction + terminal lifecycle
// ----------------------------------------------------------------
// The constructor puts the terminal into a game-friendly state (raw / no-echo
// input, virtual-terminal output, hidden cursor) and the destructor restores
// it. poll() refreshes the key-state flags without ever blocking the loop.
//
#ifndef _WIN32
#  include <termios.h>
#endif

class Input {
public:
    Input();
    ~Input();

    Input(const Input&)            = delete;
    Input& operator=(const Input&) = delete;

    // Refresh key states. Call once per frame before reading the accessors.
    void poll();

    bool forward()     const { return fwd_;    }
    bool backward()    const { return back_;   }
    bool turnLeft()    const { return left_;   }
    bool turnRight()   const { return right_;  }
    bool strafeLeft()  const { return sLeft_;  }
    bool strafeRight() const { return sRight_; }
    bool quit()        const { return quit_;   }

private:
    bool fwd_ = false, back_ = false, left_ = false, right_ = false;
    bool sLeft_ = false, sRight_ = false, quit_ = false;

#ifdef _WIN32
    void*         hOut_        = nullptr;  // HANDLE (kept opaque in the header)
    unsigned long prevOutMode_ = 0;        // DWORD console mode to restore
#else
    termios orig_{};       // saved terminal attributes
    int     origFlags_ = 0; // saved stdin fcntl flags
#endif
};
