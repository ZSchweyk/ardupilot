#pragma once

#include <AP_HAL/AP_HAL.h>
#include <AP_HAL/I2CDevice.h>

class AP_MCPDisplay {
public:
    AP_MCPDisplay() = default;
    bool poll_called = false;

    void init(uint8_t bus = 1);
    void update();

    bool healthy() const { return _healthy; }

    uint8_t left_digit() const { return _left_digit; }     // 0..9 or 0xFF
    uint8_t right_digit() const { return _right_digit; }   // 0..9 or 0xFF
    uint8_t direction() const { return _direction; }       // raw direction bits
    uint32_t raw_led_states() const { return _led_states; }
    uint32_t final_results() const { return _final_results; }

private:
    uint32_t prev_update = 0;

    static constexpr uint8_t ADDR1 = 0x20;
    static constexpr uint8_t ADDR2 = 0x24; // change to match your actual second MCP if used

    // IOCON.BANK = 0 register map
    static constexpr uint8_t REG_IODIRA = 0x00;
    static constexpr uint8_t REG_IODIRB = 0x01;
    static constexpr uint8_t REG_IPOLA  = 0x02;
    static constexpr uint8_t REG_IPOLB  = 0x03;
    static constexpr uint8_t REG_GPPUA  = 0x0C;
    static constexpr uint8_t REG_GPPUB  = 0x0D;
    static constexpr uint8_t REG_GPIOA  = 0x12;
    static constexpr uint8_t REG_GPIOB  = 0x13;

    AP_HAL::OwnPtr<AP_HAL::I2CDevice> _dev1;
    AP_HAL::OwnPtr<AP_HAL::I2CDevice> _dev2;

    volatile bool _new_data = false;
    bool _healthy = false;

    volatile uint32_t _led_states = 1;
    uint32_t _last_led_states = 0;
    uint32_t _final_results = 0;

    uint8_t _left_digit = 0xFF;
    uint8_t _right_digit = 0xFF;
    uint8_t _direction = 0;

    static constexpr uint8_t _ledPins[20] = {
        15, 14, 2, 1, 0, 18, 16, 3, 10, 8,
         6,  5, 4,12,11, 19, 17,13,  9, 7
    };

    void poll();

    bool configure_device(AP_HAL::I2CDevice *dev);
    bool write_reg(AP_HAL::I2CDevice *dev, uint8_t reg, uint8_t val);
    bool read_reg(AP_HAL::I2CDevice *dev, uint8_t reg, uint8_t &val);
    bool read_16(AP_HAL::I2CDevice *dev, uint16_t &value);

    uint8_t getDigitFromMask(uint8_t mask) const;
    void decode_led_states(uint32_t led_states);
    void handle_decoded_value();
};
