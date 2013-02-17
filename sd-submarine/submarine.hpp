int MOTOR_PINS[4][2] = {
    {18, 23},
    {24, 25},
    { 8,  7},
    {17, 27}
};

bool setup_camera(CHDKCamera * cam, int * error);
void setup_motors(Motor * subMotors);
