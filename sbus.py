import serial
import binascii
import time

SBUS_SIGNAL_OK         = 0x00
SBUS_SIGNAL_LOST       = 0x01
SBUS_SIGNAL_FAILSAFE   = 0x03

sbus_date = [0x0f,0x01,0x04,0x20,0x00,0xff,0x07,0x40,0x00,0x02,0x10,0x80,0x2c,0x64,0x21,0x0b,0x59,0x08,0x40,0x00,0x02,0x10,0x80,0x00,0x00]
channels = [1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,0,0]
servos = [1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,1023,0,0]

failsafe_status = SBUS_SIGNAL_FAILSAFE
sbus_passthrough = 1
byte_in_sbus= 0
bit_in_sbus= 0
ch = 0
bit_in_channel = 0
bit_in_servo = 0
inBuffer = [0]*100
#bufferindex=0
inData = ''
toChannels = 0


def bus_read(s):
    return int(binascii.b2a_hex(s.read()), 16)

def feedLine(s):
    bufferindex=0
    while(s.inWaiting()):
        inData = binascii.b2a_hex(s.read())
        if inData != '':
            if(inData == '0f'):
            #if(inData == '00'):
                #print inBuffer
                bufferindex = 0
                inBuffer[bufferindex] = int(inData,16)
                #inBuffer[24] = 0xff
                #print "hi"
                #toChannels = 1

                ch1 = ((inBuffer[2 ]&0b111) << 8) | inBuffer[1]
                ch2 = ((inBuffer[3 ]&0b00111111) << 5) | ((inBuffer[2 ]&0b11111000)>>3)
                ch3 = ((inBuffer[4 ]&0b11111111) << 2) | ((inBuffer[3 ]&0b11000000)>>6) | ((inBuffer[5]&0b00000001)<<10)
                ch4 = ((inBuffer[6 ]&0b00001111) << 7) | ((inBuffer[5 ]&0b11111110)>>1)
                ch5 = ((inBuffer[7 ]&0b01111111) << 4) | ((inBuffer[6 ]&0b11110000)>>4)
                ch6 = ((inBuffer[8 ]&0b11111111) << 1) | ((inBuffer[7 ]&0b10000000)>>7) | ((inBuffer[9]&0b00000011)<<9)
                ch7 = ((inBuffer[10]&0b00011111) << 6) | ((inBuffer[9 ]&0b11111100)>>2)
                ch8 = ((inBuffer[11]&0b11111111) << 3) | ((inBuffer[10]&0b11100000)>>5)
                
                print "ch1:%5d  ch2:%5d  ch3:%5d  ch4:%5d  ch5:%5d  ch6:%5d  ch7:%5d  ch8:%5d"%(ch1,ch2,ch3,ch4,ch5,ch6,ch7,ch8)
                '''
                res =  update_channels(inBuffer)
                
                ch1 = (res[0]&0b011111111000) | ((res[1]>>8)&0b0111)

                print ch1
                '''
                '''
                for i in range(15):
                    print  format(res[i], '012b'),
                    print " ",
                print ""
                '''
                '''
                #if inBuffer[1] == 0xbc:
                if 1:
                    for i in range(1,16):

                        print  format(inBuffer[i], '08b'),
                        print " ",
                    print ""
                '''
                break
            else:
                bufferindex += 1
                inBuffer[bufferindex] = int(inData,16)
            if (inBuffer[0] == 0x0f) and (inBuffer[24] == 0x00):
                toChannels = 1
            #if inData == '00':
            #    print "00"
            #    break
        else:
            print "No!"
            break


def update_channels(sbus_date):
    sbus_pointer = 0

    # clear channels[]
    for i in range(16):
        channels[i] = 0

    # reset counters
    byte_in_sbus = 2
    bit_in_sbus = 0
    ch = 0
    bit_in_channel = 0

    # process actual sbus data
    for i in range(176):
        if sbus_date[byte_in_sbus] & (1<<bit_in_sbus):
            channels[ch] |= (1<<bit_in_channel)

        bit_in_sbus += 1
        bit_in_channel += 1

        if bit_in_sbus == 8:
         bit_in_sbus = 0
         byte_in_sbus += 1

        if bit_in_channel == 11:
            bit_in_channel = 0
            ch += 1
    # DigitalChannel 1
    if sbus_date[23] & (1 << 0):
        channels[16] = 1
    else:
        channels[16] = 0

    # DigitalChannel 2
    if sbus_date[23] & (1 << 1):
        channels[17] = 1
    else:
        channels[17] = 0

    # failsafe
    failsafe_status = SBUS_SIGNAL_OK
    if sbus_date[23] & (1<<2):
        failsafe_status = SBUS_SIGNAL_LOST

    if sbus_date[23] & (1<<3):
        failsafe_status = SBUS_SIGNAL_FAILSAFE

    return channels
    
def update_servos():
    if sbus_passthrough == 0:
        for i in range(24):
            sbus_date[i] = 0
                
        # reset counters
        ch = 0
        bit_in_servo = 0
        byte_in_sbus = 1
        bit_in_sbus = 0

        # store servo data
        for i in range(176):
            if servos[ch] & (1<<bit_in_servo):
                sbus_date[byte_in_sbus] |=(1<<bit_in_sbus)

            bit_in_sbus += 1
            bit_in_servo += 1

            if bit_in_sbus == 0:
                bit_in_sbus = 0
                byte_in_sbus += 1
            if bit_in_servo == 11:
                bit_in_servo = 0
                ch += 1
            
        # DigitalChannel 1
        if channels[16] == 1:
            sbus_date[23] |= 1 << 0

        # DigitalChannel 2
        if channels[17] == 1:
            sbus_date[23] |= 1 << 1

        # failsafe
        if failsafe_status == SBUS_SIGNAL_LOST:
            sbus_date[23] |= 1 << 2

        if failsafe_status == SBUS_SIGNAL_FAILSAFE:
            sbus_date[23] |= 1 << 2
            sbus_date[23] |= 1 << 3

    # send data out
    #for i in range(25):

    
    
if __name__ == '__main__':
    s=serial.Serial("/dev/ttyS1",100000,serial.EIGHTBITS,serial.PARITY_EVEN, serial.STOPBITS_TWO)
    while 1:
        feedLine(s)
        if toChannels == 1:
            update_channels()
            update_servos()
            toChannels = 0
            print "toChannels"
            print servos
        #print "out"
'''      
    s.read(s.inWaiting())
  
    bus = []
    ch_val = [0] * 8
    cnt = 0
    flag = 0


    while 1:
        res = bus_read(s)
        if res == 120:
            if len(bus) > 5:
              ch_val[0] = ((bus[1]&0b111)<<8) | ((bus[2]&0b11111000)>>1) | ((bus[3]&0b01110000)>>4)
              print ch_val[0]
            
            bus=[]
            flag = 1

        
        if flag:
            bus.append(res)

        #if cnt == 11:
            #ch_val[0] = ((bus[1]&0b111)<<8) | ((bus[2]&0b11111000)>>1) | ((bus[3]&0b01110000)>>4)
            #ch_val[1]
            #ch_val[2]
            #ch_val[3]
            #ch_val[4]
            #ch_val[5]
            #ch_val[6]
            #ch_val[7]

            #print ch_val[0]
            #cnt = 0
'''