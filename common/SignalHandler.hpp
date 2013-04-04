class SignalHandler {
protected:
    static bool mbGotExitSignal;
    static bool mbGotUpdateSignal;

public:
    SignalHandler();
    ~SignalHandler();

    static bool gotExitSignal();
    static void setExitSignal(bool _bExitSignal);
    
    static bool gotUpdateSignal();
    static void setUpdateSignal(bool _bUpdateSignal);

    void        setupSignalHandlers();
    static void exitSignalHandler(int _ignored);
    static void updateSignalHandler(int _ignored);

};
