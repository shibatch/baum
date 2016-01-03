GLUI=glui-2.36

all : $(GLUI)/src/LICENSE.txt
	(cd $(GLUI)/src; make)
	(cd src; make GLUIDIR=$(GLUI))
	cp src/glbaumui src/baumcreateplan src/glbaum src/baumtest src/baummarker bin
	cp src/libbaum.a lib

$(GLUI)/src/LICENSE.txt : $(GLUI).zip
	unzip $(GLUI).zip
	touch $(GLUI)/src/LICENSE.txt

clean :
	(cd src; make clean)
	rm -f *~ *.o *.a .#* output.png baum_plan.txt bin/baum_plan.txt
	rm -f markers/*.html
	rm -rf $(GLUI)
