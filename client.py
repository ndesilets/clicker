import socket
from optparse import OptionParser

PAYLOAD_SIZE = 4

def main():
    parser = OptionParser()
    parser.add_option("-t", "--target", action="store", type="string", dest="target")
    parser.add_option("-p", "--port", action="store", type="int", dest="port")
    history = {}
    answers = [0, 0, 0, 0, 0, 0, 0, 0, 0]

    options, args = parser.parse_args()

    con = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) 
    con.connect((options.target, options.port))

    # "Handshake" w/ server for client IP info
    con.sendto(bytearray("HEY IT ME LOL"), (options.target, options.port))

    print "Connected to " + options.target + ":" + str(options.port)

    while True:
        data, addr = con.recvfrom(PAYLOAD_SIZE)

        deviceKey = str(ord(data[0])) + str(ord(data[1])) + str(ord(data[2]))

        #print ord(data[0]), ord(data[1]), ord(data[2]), ord(data[3])

        if ord(data[3]) < 1 or ord(data[3]) > 9:
            continue

        # If device not registered
        if deviceKey not in history:
            history[deviceKey] = {
                "1": False,
                "2": False,
                "3": False,
                "4": False,
                "5": False,
                "6": False,
                "7": False,
                "8": False,
                "9": False
            }

            history[deviceKey][str(ord(data[3]))] = True
            answers[ord(data[3]) - 1] += 1

            printAnswers(answers)

        # If device registered but answer is new
        elif history[deviceKey][str(ord(data[3]))] is False:
            history[deviceKey][str(ord(data[3]))] = True
            answers[ord(data[3]) - 1] += 1

            printAnswers(answers)

        # Else ignore (duplicate device/answer combo)
        else:
            pass
        
    exit(0)

def printAnswers(answers):
    print "Answers - ",
    for i in range(len(answers)):
        print "[" + str(i + 1) + "]: " + str(answers[i]) + ", ",
    print ""

if __name__ == "__main__":
    main()