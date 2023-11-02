from time import sleep
import serial
import serial.tools.list_ports
import winsound

if __name__ == "__main__":
    ports = serial.tools.list_ports.comports()
    for port, desc, hwid in sorted(ports):
        print("{}: {} [{}]".format(port, desc, hwid))
        lastport = port
    

    try:
        serial_conn = serial.Serial(lastport, 115200)
    except:
        print('\nCant connect to port {}'.format(port))
        exit(0)

    count = 0
    while not serial_conn.is_open:
        sleep(0.1)
        if count == 10:
            print('\nTimed out')
            exit(0)

    print('\nconnection established') 

    while serial_conn.is_open:
        try:
            line = serial_conn.readline()
            print(line)
            if "on" in str(line):
                winsound.PlaySound("buzzer.wav", winsound.SND_FILENAME)
        except:
            break

    print('\nconnection lost')
    exit(0) 