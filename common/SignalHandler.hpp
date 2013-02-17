class SignalHandler {
protected:
    static bool mbGotExitSignal;

public:
    SignalHandler();
    ~SignalHandler();

    static bool gotExitSignal();
    static void setExitSignal(bool _bExitSignal);

    void        setupSignalHandlers();
    static void exitSignalHandler(int _ignored);

};
