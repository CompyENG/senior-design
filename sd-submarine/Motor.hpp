class Motor {
    public:
        enum MotorDirection {
            OFF = 0,
            FORWARD = 1,
            REVERSE = 2,
            BACKWARD = 2,
            UNKNOWN
        };
        static bool setup_gpio(bool debug);
        Motor(bool debug=false);
        Motor(int * GPIO, bool debug=false);
        ~Motor();
        bool setup(int * GPIO);
        bool spinForward();
        bool spinBackward();
        bool spinReverse();
        bool stop();
        bool spin(MotorDirection direction);
        Motor::MotorDirection getState();
        
    private:
        static int instances;
        int pins[2];
        static bool gpio_setup;
        enum MotorDirection state;
};
