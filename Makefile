GLUI=glui-2.36

all : $(GLUI)/src/LICENSE.txt
	(cd $(GLUI)/src; make)
	(cd src; make GLUIDIR=$(GLUI))
	cp src/glbaumui src/baumcreateplan src/glbaum src/baumtest src/baummarker bin
	cp src/libbaum.a lib
	cp src/libbaumjni.so src/libocvimgio.so java

$(GLUI)/src/LICENSE.txt : $(GLUI).zip
	unzip $(GLUI).zip
	touch $(GLUI)/src/LICENSE.txt

javadoc :
	rm -r javadoc; mkdir javadoc; javadoc -encoding UTF-8 -classpath ./java -d ./javadoc -subpackages org.naokishibata

clean :
	(cd src; make clean)
	rm -f *~ *.o *.a .#* output.png baum_plan.txt bin/baum_plan.txt java/libbaumjni.so java/libocvimgio.so java/BaumTestPlan.txt
	rm -f markers/*.html markers/*.svg
	rm -f *.svg
	rm -f java/output.png java/BaumTestPlan.txt
	rm -rf $(GLUI)
