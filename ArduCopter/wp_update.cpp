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

float compare_neds(Vector3f current_ned, Vector3f target_ned, bool use_down) {
    float dN = target_ned.x - current_ned.x;
    float dE = target_ned.y - current_ned.y;
    float dD = target_ned.z - current_ned.z;

    if (use_down) return dN*dN + dE*dE + dD*dD;
    return dN*dN + dE*dE;
}

void Copter::update_sensor_guided_target(void) {
    snowbat_search_algo_count++;
    // if (!mcp_disp.healthy()) {
    //     return;
    // }

    // GCS_SEND_TEXT(MAV_SEVERITY_EMERGENCY, "Run %u", (unsigned) snowbat_search_algo_count);
    // return;
    // mcp_disp.update();
    Vector3f current_ned = pos_control->get_pos_estimate_NED_m();
    float ned_compare_tol = .05;
    if (!is_target_set) {

        const float pi = 3.1415926536;

        float drone_yaw_rad = ahrs_view->yaw;  // current drone heading
        float direction_rad = rand_float() * pi - pi/2;
        float distance_m = 1;
        float max_step_m = 1;

        target_ned = make_ned_target_body_relative(
            current_ned,
            drone_yaw_rad,
            direction_rad,
            distance_m,
            max_step_m
        );
        
        is_target_set = true;

        GCS_SEND_TEXT(MAV_SEVERITY_EMERGENCY, "Moving from NEDi = (%f, %f) to NEDt = (%f, %f)", current_ned.x, current_ned.y, target_ned.x, target_ned.y);
    } else {
        if (snowbat_search_algo_count % 20 == 0) {
            float ned_error = compare_neds(current_ned, target_ned, false);
            if (ned_error <= ned_compare_tol * ned_compare_tol) {
                is_target_set = false;
                GCS_SEND_TEXT(MAV_SEVERITY_EMERGENCY, "Reached target, setting new one");
            } else {
                GCS_SEND_TEXT(MAV_SEVERITY_EMERGENCY, "NED current: (%f, %f) NED target: (%f, %f). Error is %f", current_ned.x, current_ned.y, target_ned.x, target_ned.y, ned_error);
            }
        }
    }

    

    


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