import socket
from optparse import OptionParser

PAYLOAD_SIZE = 4

def main():
    parser = OptionParser()
    parser.add_option("-t", "--target", action="store", type="string", dest="target")
    parser.add_option("-p", "--port", action="store", type="int", dest="port")

    options, args = parser.parse_args()

    con = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) 
    con.connect((options.target, options.port))

    print "Connected to " + options.target + ":" + str(options.port)

    # "Handshake" w/ server for client IP info
    con.sendto(bytearray("HEY IT ME LOL"), (options.target, options.port))

    data, addr = con.recvfrom(PAYLOAD_SIZE)
    print addr, data

    print "Ready"

    while True:
        data, addr = con.recvfrom(PAYLOAD_SIZE)
        print addr, data

    exit(0)

if __name__ == "__main__":
    main()