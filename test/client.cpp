#include <iostream>
#include <libptp++/libptp++.hpp>

int main(int argc, char * argv[]) {
    PTP::PTPNetwork clientBackend("127.0.0.1", 50000);
    PTP::CameraBase client(&clientBackend);
    
    PTP::PTPContainer cmd(PTP::PTPContainer::CONTAINER_TYPE_COMMAND, 0x1234);
    cmd.add_param(1);
    PTP::PTPContainer data(PTP::PTPContainer::CONTAINER_TYPE_DATA, 0x1234);
    data.add_param(1);
    data.add_param(2);
    data.add_param(3);
    PTP::PTPContainer out_resp, out_data;
    try {
        client.ptp_transaction(cmd, data, true, out_resp, out_data);
    } catch(PTP::LIBPTP_PP_ERRORS e) {
        std::cout << "Caught error: " << e << std::endl;
        return 0;
    }
    
    std::cout << "Got response: " << out_resp.get_param_n(0) << std::endl;
    std::cout << "Got data: " << out_data.get_param_n(0) << std::endl;
    
    return 0;
}
    
