@if not exist bin\baum_plan.txt @(
    @echo Creating task execution plan
    @call createPlan.bat
)
@cls
@echo Launching demo
@echo Instruction
@echo 1. Print out exampleMarkers.pdf.
@echo 2. Select a camera from "Choose Capture" list box.
@echo 3. Choose a GPU device from "Choose Device" list box.
@echo 4. Point your camera at the circular barcodes that you printed out.
@bin\glbaumui.exe
