#include "Copter.h"
#include <AP_Math/AP_Math.h>

#include "AP_MCPDisplay.h"



float min(double x, double y) {
    if (x < y) return x;
    return y;
}


Vector3f make_ned_target_body_relative(const Vector3f& current_ned,
                                       float drone_yaw_rad,
                                       float direction_rad,
                                       float remaining_distance_m,
                                       float max_step_m)
{
    const float step_m = min(remaining_distance_m, max_step_m);

    // Convert body-relative direction to world/NED direction
    const float world_direction_rad = drone_yaw_rad + direction_rad;

    Vector3f target;
    target.x = current_ned.x + step_m * cosf(world_direction_rad); // North
    target.y = current_ned.y + step_m * sinf(world_direction_rad); // East
    target.z = current_ned.z;                                      // Down unchanged

    return target;
}

void Copter::update_sensor_guided_target(void) {
    snowbat_search_algo_count++;
    // if (!mcp_disp.healthy()) {
    //     return;
    // }

    // GCS_SEND_TEXT(MAV_SEVERITY_EMERGENCY, "Run %u", (unsigned) snowbat_search_algo_count);
    // return;
    // mcp_disp.update();

    Vector3f current = pos_control->get_pos_estimate_NED_m();

    float drone_yaw_rad = ahrs_view->yaw;       // current drone heading
    float direction_rad = 0;         // command says "right"
    float distance_m = 5.0f;
    float max_step_m = 0.5f;

    Vector3f target = make_ned_target_body_relative(
        current,
        drone_yaw_rad,
        direction_rad,
        distance_m,
        max_step_m
    );

    GCS_SEND_TEXT(MAV_SEVERITY_EMERGENCY, "Drone yaw %f, North %f, East %f, Down %f", drone_yaw_rad * 180/3.14, target.x, target.y, target.z);



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