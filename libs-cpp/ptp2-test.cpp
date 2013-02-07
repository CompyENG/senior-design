#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libusb-1.0/libusb.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "libptp2.hpp"
#include "live_view.h"

using namespace std;

int main(int argc, char * argv[]) {
    int r;
    
    r = libusb_init(NULL);
    if (r < 0)
        return r;
        
    libusb_device * dev = CHDKCamera::find_first_camera();
    
    if(dev == NULL) {
        fprintf(stderr, "No camera found.\n");
        return -1;
    }
    
    struct libusb_device_descriptor desc;
    r = libusb_get_device_descriptor(dev, &desc);
    if (r < 0) {
        fprintf(stderr, "failed to get device descriptor");
        return -1;
    }

    printf("%04x:%04x (bus %d, device %d)\n",
        desc.idVendor, desc.idProduct,
        libusb_get_bus_number(dev), libusb_get_device_address(dev));
        
    CHDKCamera cam;
    try {
        cam.open(dev);
    } catch(int e) {
        printf("Error occured opening device: %d\n", e);
        return -1;
    }
    
    cout << "CHDK Version: " << cam.get_chdk_version() << endl;
        
    cam.execute_lua("switch_mode_usb(1)", NULL);
    
    char recv_data[1024];
    int sock, connected, bytes_received;
    struct sockaddr_in server_addr,client_addr;
    socklen_t sin_size;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(50000);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_addr.sin_zero), 8);
    
    bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
    
    listen(sock, 5);
    
    int count = 0;
    
    while(1) {
        sin_size = sizeof(struct sockaddr_in);
        
        connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);
        
        printf("\n I got a connection from (%s , %d)", inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
        ++count;
        
        bytes_received = recv(connected, recv_data, 1024, 0);
        recv_data[bytes_received] = '\0';
        cout << "Received: " << bytes_received << endl << recv_data << endl;
        
        // Now, send live view data
        LVData lv;
        cam.get_live_view_data(&lv, true);
        uint8_t * lv_rgb;
        int size, width, height;
        lv_rgb = lv.get_rgb(&size, &width, &height, true);
        
        send(connected, "1", 1, 0);
        
        cout << "Sending data of size: " << size << endl;
        
        send(connected, lv_rgb, size, 0);
        
        free(lv_rgb);
        
        close(connected); // I hope I can do this...
    }
    
    close(sock);
    
    //cam.execute_lua("shoot()", NULL);
    
    //sleep(3);
    /*
    LVData lv;
    cam.get_live_view_data(&lv, true);
    
    cout << "Live view version: " << lv.get_lv_version() << endl;
    
    uint8_t * lv_rgb;
    int size, width, height;
    lv_rgb = lv.get_rgb(&size, &width, &height, true);
    
    ofstream img_file;
    img_file.open("lv-test-cpp.ppm", ios::out | ios::binary);
    img_file << "P6 " << width << " " << height << " 255" << endl;
    img_file.write((char *)lv_rgb, size);
    img_file.close();
    
    free(lv_rgb);
    */
    
    libusb_exit(NULL);
    
    return 0;
}
