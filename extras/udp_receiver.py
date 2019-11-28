from socket import *

s=socket(AF_INET, SOCK_DGRAM)
s.bind(('',1666))
while(1):
    m=s.recvfrom(4096)
    print m[0].strip()


