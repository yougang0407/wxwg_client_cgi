rm -f *.o

#/opt/aarch64_centos7_34_AKII_feiteng_72317/bin/aarch64-linux-gnu-g++ -c -g *.c -I. -I../inc
#/opt/aarch64_centos7_34_AKII_feiteng_72317/bin/aarch64-linux-gnu-g++ -g -o user *.o -L../ftlib -lgsoap -lxml2 -lm -lz

g++ -c -g *.c -I. -I../inc
g++ -g -o user *.o -L../ftlib -lgsoap -lxml2 -lm -lz
