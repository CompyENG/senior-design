int MOTOR_PINS[4][2] = {
    {23, 18}, //P1-12, P1-16 (LEFT)
    {25, 24}, //P1-18, P1-22 (RIGHT)
    { 8,  7}, //P1-24, P1-26 (TOP_FRONT)
    {17, 27} //P1-11, P1-13 (TOP_REAR)
};

enum Submarine_Motors {
    MOTOR_LEFT,
    MOTOR_RIGHT,
    MOTOR_TOP_FRONT,
    MOTOR_TOP_REAR
};

class SubServer;
class SignalHandler;

bool setup_camera(PTP::CHDKCamera& cam, PTP::PTPUSB& proto, int * error);
void setup_motors(Motor * subMotors);
bool compare_states(int8_t * sub_state, int8_t * joy_data);
void update_motors(int8_t * sub_state, int8_t * joy_data, uint32_t joy_data_len, Motor * subMotors, PTP::CHDKCamera& cam);
