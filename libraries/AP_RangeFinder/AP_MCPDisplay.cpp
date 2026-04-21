#include "AP_MCPDisplay.h"

#include <AP_HAL/AP_HAL.h>
#include <AP_Mission/AP_Mission.h>
#include <GCS_MAVLink/GCS.h>

extern const AP_HAL::HAL& hal;

const uint8_t AP_MCPDisplay::_ledPins[20];

bool AP_MCPDisplay::write_reg(AP_HAL::I2CDevice *dev, uint8_t reg, uint8_t val)
{
    return dev != nullptr && dev->write_register(reg, val);
}

bool AP_MCPDisplay::read_reg(AP_HAL::I2CDevice *dev, uint8_t reg, uint8_t &val)
{
    return dev != nullptr && dev->read_registers(reg, &val, 1);
}

bool AP_MCPDisplay::configure_device(AP_HAL::I2CDevice *dev)
{
    if (dev == nullptr) {
        return false;
    }

    // all pins input
    if (!write_reg(dev, REG_IODIRA, 0xFF)) return false;
    if (!write_reg(dev, REG_IODIRB, 0xFF)) return false;

    // normal polarity
    if (!write_reg(dev, REG_IPOLA, 0x00)) return false;
    if (!write_reg(dev, REG_IPOLB, 0x00)) return false;

    // pullups enabled
    if (!write_reg(dev, REG_GPPUA, 0xFF)) return false;
    if (!write_reg(dev, REG_GPPUB, 0xFF)) return false;

    return true;
}

bool AP_MCPDisplay::read_16(AP_HAL::I2CDevice *dev, uint16_t &value)
{
    uint8_t a = 0;
    uint8_t b = 0;

    if (!read_reg(dev, REG_GPIOA, a)) {
        return false;
    }
    if (!read_reg(dev, REG_GPIOB, b)) {
        return false;
    }

    value = uint16_t(a) | (uint16_t(b) << 8);
    return true;
}

void AP_MCPDisplay::init(uint8_t bus)
{
    GCS_SEND_TEXT(MAV_SEVERITY_EMERGENCY, "MCPD: init ok");

    _dev1 = std::move(hal.i2c_mgr->get_device(bus, ADDR1));
    _dev2 = std::move(hal.i2c_mgr->get_device(bus, ADDR2));

    if (!_dev1) {
        GCS_SEND_TEXT(MAV_SEVERITY_EMERGENCY, "MCPD: dev1 missing");
        return;
    }

    _dev1->set_retries(10);

    if (!configure_device(_dev1.get())) {
        GCS_SEND_TEXT(MAV_SEVERITY_EMERGENCY, "MCPD: dev1 cfg fail");
        return;
    }

    // second chip optional; if missing, only first 16 bits are available
    if (!_dev2) {
        _dev2->set_retries(10);
        if (!configure_device(_dev2.get())) {
            GCS_SEND_TEXT(MAV_SEVERITY_EMERGENCY, "MCPD: dev2 cfg fail");
            _dev2 = nullptr;
        }
    }

    _healthy = true;

    // poll once per second
    _dev1->register_periodic_callback(
        1000000,
        FUNCTOR_BIND_MEMBER(&AP_MCPDisplay::poll, void)
    );

}

void AP_MCPDisplay::poll()
{
    if (!_healthy) {
        GCS_SEND_TEXT(MAV_SEVERITY_EMERGENCY, "not healthy");
        return;
    }

    uint16_t v1 = 0;
    uint16_t v2 = 0;

    if (!read_16(_dev1.get(), v1)) {
        GCS_SEND_TEXT(MAV_SEVERITY_EMERGENCY, "_dev1 read_16 is false");

        return;
    }

    if (_dev2) {
        if (!read_16(_dev2.get(), v2)) {
            GCS_SEND_TEXT(MAV_SEVERITY_EMERGENCY, "_dev2 read_16 is false");
            return;
        }
    }

    // Your Arduino test treated active-low inputs as asserted when digitalRead()==0.
    // Convert to "1 means active" here.
    uint32_t states = 0;

    // mcp1 contributes pins 0..15
    for (uint8_t i = 0; i < 16; i++) {
        const bool active = ((v1 >> i) & 0x1U) == 0;
        if (active) {
            states |= (1UL << i);
        }
    }

    // mcp2 contributes only pins 16..19 in your test code
    for (uint8_t i = 16; i < 20; i++) {
        const bool active = ((v2 >> (i - 16)) & 0x1U) == 0;
        if (active) {
            states |= (1UL << i);
        }
    }

    GCS_SEND_TEXT(MAV_SEVERITY_EMERGENCY, "setting new_data=true");

    _led_states = states;
    _new_data = true;
}

uint8_t AP_MCPDisplay::getDigitFromMask(uint8_t mask) const
{
    // common cathode lookup, same as your Arduino code
    static const uint8_t segmentTable[10] = {
        0x3F, 0x06, 0x5B, 0x4F, 0x66,
        0x6D, 0x7D, 0x07, 0x7F, 0x6F
    };

    for (uint8_t i = 0; i < 10; i++) {
        if (segmentTable[i] == mask) {
            return i;
        }
    }

    return 0xFF;
}

void AP_MCPDisplay::decode_led_states(uint32_t led_states)
{
    _final_results = 0;

    // same bit remap as your Arduino code
    for (uint8_t i = 0; i < 20; i++) {
        _final_results |= ((led_states >> i) & 1UL) << _ledPins[i];
    }

    GCS_SEND_TEXT(MAV_SEVERITY_EMERGENCY, "_final_results %u", (unsigned int) _final_results);


    // Match your existing decode flow, which used ledStates directly:
    // first 7 bits = digit 1
    // skip 1 decimal/interrupt bit
    // next 7 bits = digit 2
    // remaining bits = direction
    uint32_t temp = led_states;
    static constexpr uint8_t digit_mask = 0x7F;

    _left_digit = getDigitFromMask(uint8_t(temp & digit_mask));
    temp >>= 8;  // 7 segments + 1 separator bit
    _right_digit = getDigitFromMask(uint8_t(temp & digit_mask));
    temp >>= 7;
    _direction = uint8_t(temp & 0x1F); // up to 5 remaining bits in your 20-bit scheme
}

void AP_MCPDisplay::handle_decoded_value()
{
    // Example interpretation:
    // distance = two-digit number if both digits decode
    int16_t distance = -1;
    if (distance == -1) {}
    if (_left_digit != 0xFF && _right_digit != 0xFF) {
        distance = int16_t(_left_digit) * 10 + int16_t(_right_digit);
    }

    GCS_SEND_TEXT(MAV_SEVERITY_INFO,
                  "MCPD: raw=0x%05lx map=0x%05lx L=%u R=%u DIR=0x%02x",
                  (unsigned long)_led_states,
                  (unsigned long)_final_results,
                  (unsigned)_left_digit,
                  (unsigned)_right_digit,
                  (unsigned)_direction);

    // Placeholder for waypoint logic.
    // Replace these examples with your actual meaning of direction bits.
    //
    // Example:
    // direction bit0 -> advance to next mission item
    // direction bit1 -> go back one mission item
    // distance -> optional parameter you can use for choosing jump count, radius, etc.

    AP_Mission *mission = AP::mission();
    if (mission == nullptr) {
        return;
    }

    // Example on change only; keep it simple and avoid repeated triggers
    if ((_direction & 0x01U) != 0) {
        const uint16_t current = mission->get_current_nav_index();
        mission->set_current_cmd(current + 1);
    }

    // Another example:
    // if ((_direction & 0x02U) != 0 && distance >= 0) {
    //     const uint16_t current = mission->get_current_nav_index();
    //     mission->set_current_cmd(current + uint16_t(distance));
    // }
}

void AP_MCPDisplay::update()
{
    if (!_new_data) {
        return;
    }
    _new_data = false;

    const uint32_t states = _led_states;
    if (states == _last_led_states) {
        return;
    }
    _last_led_states = states;

    decode_led_states(states);
    handle_decoded_value();
}
