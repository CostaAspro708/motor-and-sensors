# invoke SourceDir generated makefile for empty_min.pem4f
empty_min.pem4f: .libraries,empty_min.pem4f
.libraries,empty_min.pem4f: package/cfg/empty_min_pem4f.xdl
	$(MAKE) -f C:\Users\robin\OneDrive\Documents\Cours\sem1_2023\EGH456\UI456\UI_EGH456/src/makefile.libs

clean::
	$(MAKE) -f C:\Users\robin\OneDrive\Documents\Cours\sem1_2023\EGH456\UI456\UI_EGH456/src/makefile.libs clean

