class Motor {
    public:
        enum MotorDirection {
            OFF = 0,
            FORWARD = 1,
            REVERSE = 2,
            BACKWARD = 2,
            UNKNOWN
        };
        static bool setup_gpio();
        Motor();
        Motor(int * GPIO);
        bool setup(int * GPIO);
        bool spinForward();
        bool spinBackward();
        bool spinReverse();
        bool stop();
        bool spin(MotorDirection direction);
        Motor::MotorDirection getState();
        
    private:
        int pins[2];
        static bool gpio_setup;
        enum MotorDirection state;
};
