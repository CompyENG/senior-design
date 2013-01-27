import logging
import usb

log = logging.getLogger(__name__)

__all__ = ['is_ptp_camera', 'get_ptp_interface', 'list_ptp_cameras', 'find_camera_by_serial']

def is_ptp_camera(usb_device, interfaceClass=6):

    if get_ptp_interface(usb_device, interfaceClass) is not None:
        return True
    else:
        return False

def get_ptp_interface(usb_device, interfaceClass=6):

    for cfg in usb_device:
        intf = usb.util.find_descriptor(cfg, bInterfaceClass=interfaceClass)

        if intf is not None:
            return intf

    return None 

def list_ptp_cameras():
    usb_devices = usb.core.find(find_all=True)
    return filter(get_ptp_interface, usb_devices)


def find_camera_by_serial(serial, partial_ok=False):
    '''
    Attempts to match the provided serial number against PTP cameras connected
    to the USB bus.

    If a match is found, that device is returned.  Otherwise a ValueError is raised.
    '''
    ptp_cameras = list_ptp_cameras()
    log.info('Found %d PTP cameras...', len(ptp_cameras))

    for dev in ptp_cameras:
        try:
            serial_number_str_indx = getattr(dev, 'iSerialNumber')
        except AttributeError:
            log.debug('usb dev %s has no attribute named iSerialNumber, skipping.', str(dev))
            continue

        try:
            dev_serial = usb.util.get_string(dev, 50, serial_number_str_indx)
        except:
            log.debug('Could not retrieve serial number from %s (str indx %d)', str(dev),
                serial_number_str_indx)
            continue

        if not dev_serial.endswith('\x00'):
            log.warning('Device %s serial string is not null terminated:  %s', str(dev), dev_serial)

        else:
            dev_serial = dev_serial.rstrip('\x00')

        if dev_serial.lower() == serial.lower():
            log.info('Found serial number %s in %s', serial, str(dev))
            return dev

        elif partial_ok and serial:
            if len(serial) > len(dev_serial):
                full_str = serial
                sub_str = dev_serial

            else:
                full_str = dev_serial
                sub_str = serial

            if sub_str.lower() in full_str.lower():
                log.info('Partial match:  %s in %s', sub_str, full_str)
                return dev

    raise ValueError('Could not match serial %s to any usb device' %(serial,))