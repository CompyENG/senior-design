#include <iostream>
#include <unistd.h>
#include <libptp++/libptp++.hpp>

int main(int argc, char * argv[]) {
    PTP::PTPNetwork serverBackend(50000);
    PTP::CameraBase server(&serverBackend);
    
    PTP::PTPContainer in_cmd, in_data;
    server.recv_ptp_message(in_cmd);
    std::cout << "Got command: " << in_cmd.get_param_n(0) << std::endl;
    
    server.recv_ptp_message(in_data);
    std::cout << "Got data: " << in_data.get_param_n(0) << std::endl;
    
    PTP::PTPContainer out_data(PTP::PTPContainer::CONTAINER_TYPE_DATA, 0x1234);
    out_data.add_param(1);
    server.send_ptp_message(out_data);
    
    PTP::PTPContainer out_resp(PTP::PTPContainer::CONTAINER_TYPE_RESPONSE, 0x1234);
    out_resp.add_param(1);
    server.send_ptp_message(out_resp);
    std::cout << "Done" << std::endl;
    //sleep(5);
}
