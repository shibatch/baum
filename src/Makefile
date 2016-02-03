GLUIINCLUDEDIR=../$(GLUIDIR)/src/include
GLUILIBDIR=../$(GLUIDIR)/src/lib
JDK_HOME=/usr/lib/jvm/java-7-openjdk-amd64

CFLAGS= -fPIC -g -O -Wall `pkg-config --cflags opencv gtk+-2.0` -I../include -I. -I$(GLUIINCLUDEDIR)
LDFLAGS= `pkg-config --libs opencv gtk+-2.0` -L. -L$(GLUILIBDIR)

all : baummarker baumtest baumcreateplan glbaum glbaumui libbaumjni.so libocvimgio.so

glbaumui : glbaumui.cpp libbaum.a
	g++ $(CFLAGS) glbaumui.cpp -o glbaumui $(LDFLAGS) -lbaum -lglui -lGLU -lglut -lGL -lOpenCL

glbaum : glbaum.cpp libbaum.a
	g++ $(CFLAGS) glbaum.cpp -o glbaum $(LDFLAGS) -lbaum -lGLU -lglut -lGL -lOpenCL

baumtest : baumtest.cpp libbaum.a
	g++ $(CFLAGS) baumtest.cpp -o baumtest $(LDFLAGS) -lbaum -lOpenCL

baumcreateplan : baumcreateplan.cpp libbaum.a
	g++ $(CFLAGS) baumcreateplan.cpp -o baumcreateplan $(LDFLAGS) -lbaum -lOpenCL

baummarker : baummarker.cpp libbaum.a
	g++ $(CFLAGS) baummarker.cpp -o baummarker -L. -lbaum -lOpenCL

libbaumjni.so : baumjni.cpp libbaum.a
	g++ $(CFLAGS) baumjni.cpp -o libbaumjni.so -shared -Wl,-soname,libbaumjni.so -I$(JDK_HOME)/include -I$(JDK_HOME)/include/linux -L. -lbaum -lOpenCL

libbaum.a : baum.o poly.o oclhelper.o helper.o
	ar rcs libbaum.a baum.o poly.o oclhelper.o helper.o

baum.o : baum.cpp baum_cl.h
	g++ $(CFLAGS) -c baum.cpp -o baum.o

oclhelper.o : oclhelper.cpp
	g++ $(CFLAGS) -c oclhelper.cpp -o oclhelper.o

helper.o : helper.cpp
	g++ $(CFLAGS) -c helper.cpp -o helper.o

poly.o : poly.cpp
	g++ $(CFLAGS) -c poly.cpp -o poly.o

libocvimgio.so : ocvimgio.cpp
	g++ $(CFLAGS) ocvimgio.cpp -o libocvimgio.so -shared -Wl,-soname,libocvimgio.so -I$(JDK_HOME)/include -I$(JDK_HOME)/include/linux $(LDFLAGS)

clean :
	rm -f *~ *.o *.a .#* glbaumui baumcreateplan glbaum baumtest baummarker libbaumjni.so libocvimgio.so
