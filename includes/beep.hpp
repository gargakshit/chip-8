#ifdef _WIN32
#include <Windows.h>
#endif

namespace chip8 {
inline void beep() {
#ifdef _WIN32
  Beep(3750, 64);
#endif
}
} // namespace chip8
