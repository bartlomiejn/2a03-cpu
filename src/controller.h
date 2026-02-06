#ifndef INC_2A03_CONTROLLER_H
#define INC_2A03_CONTROLLER_H

#include <log.h>

#include <atomic>
#include <cstdint>
#include <format>

namespace NES {

class Controller {
public:
    // Button state - updated by GUI thread
    std::atomic<uint8_t> buttons{0};

    // Button bit positions (matches NES hardware order)
    static constexpr uint8_t BTN_A      = 0x01;
    static constexpr uint8_t BTN_B      = 0x02;
    static constexpr uint8_t BTN_SELECT = 0x04;
    static constexpr uint8_t BTN_START  = 0x08;
    static constexpr uint8_t BTN_UP     = 0x10;
    static constexpr uint8_t BTN_DOWN   = 0x20;
    static constexpr uint8_t BTN_LEFT   = 0x40;
    static constexpr uint8_t BTN_RIGHT  = 0x80;

    Controller() = default;

    void set_button(uint8_t btn, bool pressed) {
        if (pressed)
            buttons.fetch_or(btn);
        else
            buttons.fetch_and(~btn);
    }

    // Called when CPU writes to $4016
    void write(uint8_t val) {
        bool new_strobe = val & 0x01;
        if (strobe && !new_strobe) {
            shift_reg = buttons.load();
        }
        strobe = new_strobe;
    }

    // Called when CPU reads from $4016
    uint8_t read(bool passive) {
        uint8_t result;
        if (strobe) {
            // While strobing, always return A button state
            result = (buttons.load() & BTN_A) ? 0x41 : 0x40;
            NES_LOG("Controller") << std::format(
                "Read passive={:}, strobe active, return {:02X} \n", passive,
                result);
            return result;
        }
        // Return current bit and shift
        result = (shift_reg & 0x01) ? 0x41 : 0x40;
        if (passive) {
            NES_LOG("Controller") << std::format(
                "Read passive, return {:02X}\n", passive, result);
            return result;
        }
        NES_LOG("Controller") << std::format(
            "Read return {:02X}, shift right\n", passive, result);
        shift_reg >>= 1;
        // After 8 reads, shift register returns 1s (open bus behavior)
        shift_reg |= 0x80;
        return result;
    }

private:
    bool strobe = false;
    uint8_t shift_reg = 0;
};

}  // namespace NES

#endif  // INC_2A03_CONTROLLER_H
