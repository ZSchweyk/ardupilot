#include "Copter.h"
#include "AP_MCPDisplay.h"

float beacon_bitmap_dir_to_rad(const uint8_t dir_bitmap) {
    float pi = 3.1415926536;

    if (dir_bitmap & 0b10000) return -pi / 2;
    if ((dir_bitmap & 0b11000) == 0b11000) return -67.5 * (pi / 180);
    if (dir_bitmap & 0b01000) return -pi / 4;
    if ((dir_bitmap & 0b01100) == 0b01100) return -22.5 * (pi / 180);
    if (dir_bitmap & 0b00100) return 0;  // up, 0 deg
    if ((dir_bitmap & 0b00110) == 0b00110) return 22.5 * (pi / 180);
    if (dir_bitmap & 0b00010) return pi / 4;
    if ((dir_bitmap & 0b00011) == 0b00011) return 67.5 * (pi / 180);
    return pi / 2;
}

void Copter::update_sensor_guided_target(void) {
    snowbat_search_algo_count++;
    // if (!mcp_disp.healthy()) {
    //     return;
    // }

    GCS_SEND_TEXT(MAV_SEVERITY_EMERGENCY, "Run %u", (unsigned) snowbat_search_algo_count);
    return;
    mcp_disp.update();

    // const uint8_t left = mcp_disp.left_digit();
    // const uint8_t right = mcp_disp.right_digit();
    // const uint8_t dir_bitmap = mcp_disp.direction();
    // float dir_angle_rad = beacon_bitmap_dir_to_rad(dir_bitmap);

    // if (left == 0xFF || right == 0xFF) {
    //     return;
    // }

    // // add our averaging filter here...

    // const float distance_m = (float) left + (float) right / 10;

    // Vector3p curr = pos_control->get_pos_estimate_NED_m();
    // Vector3p target = curr;
    // // target: convert distance + direction to NED coords

    // if (flightmode != &mode_guided) {
    //     set_mode(Mode::Number::GUIDED, ModeReason::SCRIPTING);
    // }

    // mode_guided.set_pos_NED_m(target, false, 0.0f, false, 0.0f, false, false);
}










    // if (dir & 0x01) {
    //     target.x += distance_m;   // North, example
    // }
    // if (dir & 0x02) {
    //     target.x -= distance_m;   // South
    // }
    // if (dir & 0x04) {
    //     target.y += distance_m;   // East
    // }
    // if (dir & 0x08) {
    //     target.y -= distance_m;   // West
    // }