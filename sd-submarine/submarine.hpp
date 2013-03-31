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

class SubServer;
class SignalHandler;

bool setup_camera(PTP::CHDKCamera& cam, PTP::PTPUSB& proto, int * error);
void setup_motors(Motor * subMotors);
bool compare_states(int8_t * sub_state, int8_t * joy_data);
void wait_for_ready(SubServer& server, SignalHandler& sigHand);
