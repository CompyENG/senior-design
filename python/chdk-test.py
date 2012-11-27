#!/usr/bin/python

# Experimenting with PTP with Python
from ptp.PtpUsbTransport import PtpUsbTransport
from ptp.PtpSession import PtpSession, PtpException
from ptp.PtpCHDK import PtpCHDKSession

ptpTransport = PtpUsbTransport(PtpUsbTransport.findptps()[0])
chdkSess = PtpCHDKSession(ptpTransport)
print chdkSess.GetVersion()

chdkSess.ExecuteScript('switch_mode_usb(1)')
chdkSess.ExecuteScript('shoot()')