class Motor {
    int pins[2];
    static bool gpio_setup;
    
    public:
        const static int OFF = 0;
        const static int FORWARD = 1;
        const static int REVERSE = 2;
        const static int BACKWARD = 3;
        static bool setup_gpio();
        Motor();
        Motor(int * GPIO);
        bool setup(int * GPIO);
        bool spinForward();
        bool spinBackward();
        bool spinReverse();
        bool stop();
        bool spin(int direction);
};
