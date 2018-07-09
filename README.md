# GIGL Examples

This is an anonymised repo for supporting blind-reviewed submissions about GIGL. The content is explained below.


1. The "Example" folder contains examples of projects that uses GIGL (or other versions for comparison).


2. The "include" folder contains the GIGL library code, written in C++.


3. The "able.jar" is the GIGL-C++ translator, which serves in one of the steps in compiling GIGL code.


4. The examples include

  (1) the Monster example (dungeon monster generator),
  
  (2) the Tree example (L-system tree genertor),
  
  (3) the Dungeon example (BSP-dungeon level generator).
  

5. Each of the examples is set up with a Visual Studio project, in which
  
  (1) The ".xc" file is GIGL source code.
  
  (2) The ".generated.h" file is intermediate C++ code in compiling GIGL (i.e. those translated by the GIGL-C++ translator).
  
  (3) The ".manual.h" file is a handwritten counterpart for the GIGL source, created only for the purpose of comparing (i.e. they are not required for compiling the example). It intends to reproduce the semantics of the GIGL code, but uses approaches natural to human (e.g. representing the two possible monster configurations with a boolean) thus being less generalizable than the GIGL version.
  
  (4) Other ".cpp" and ".h" files are project specific C++ source files which can directly interface with the GIGL file (can be compiled together in Visual Studio).
