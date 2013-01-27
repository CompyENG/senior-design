#ifndef __LIVE_VIEW_H
#define __LIVE_VIEW_H

// Live View protocol version
#define LIVE_VIEW_VERSION_MAJOR 2  // increase only with backwards incompatible changes (and reset minor)
#define LIVE_VIEW_VERSION_MINOR 0  // increase with extensions of functionality

// Control flags for determining which data block to transfer
#define LV_TFR_VIEWPORT     0x01
#define LV_TFR_BITMAP       0x04
#define LV_TFR_PALETTE      0x08

enum lv_aspect_rato {
    LV_ASPECT_4_3,
    LV_ASPECT_16_9,
};

typedef struct {
    /*
    logical screen
    descibes how big the buffer would be in pixels, if it exactly filled the physical screen
    may be larger or smaller than the buffer data, due to letter boxing or unused data
    using lcd_aspect_ratio, you can create a correct representation of the screen
    */
    int logical_width;  
    int logical_height;
    /*
    buffer - describes the actual data sent
    data size is always buffer_width*buffer_height*(buffer bpp implied by type)
    offsets represent the position of the data on the logical screen,
       > 0 for sub images (16:9 on a 4:3 screen, stitch window, etc)
    */
    int buffer_width;

    int buffer_logical_xoffset;
    int buffer_logical_yoffset;

    /*
    visible - describes data within the buffer which contains image data to be displayed
    offsets are relative to buffer
    width must be <= logical_width - buffer_logical_xoffset and width + xoffset must be <= buffer_width 
    */
    int visible_width;
    int visible_height;

    int data_start;    // offset of data
} lv_framebuffer_desc;

typedef struct {
    // TODO not sure we want to put these in every frame
    int version_major;
    int version_minor;
    int lcd_aspect_ratio; // physical aspect ratio of LCD
    int palette_type;
    int palette_data_start;
    lv_framebuffer_desc vp; // viewport
    lv_framebuffer_desc bm; // bitmap
} lv_data_header;

#endif // __LIVE_VIEW_H
