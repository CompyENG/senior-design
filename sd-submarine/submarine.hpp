int MOTOR_PINS[4][2] = {
    {18, 23},
    {24, 25},
    { 8,  7},
    {17, 27}
};

enum Submarine_Motors {
    MOTOR_LEFT,
    MOTOR_RIGHT,
    MOTOR_TOP_FRONT,
    MOTOR_TOP_REAR
};

bool setup_camera(CHDKCamera * cam, int * error);
void setup_motors(Motor * subMotors);
bool compare_states(int * sub_state, int * joy_data);
