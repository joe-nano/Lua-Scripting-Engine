GLEW
----------------------------------------
Download GLEW src from the GLEW site.

Right-click and open build/vc12/glew.sln with Visual Studio.
Try to build glew_shared by selecting "Debug" and "Win32" in the toolbar.
If that doesn't work, retarget your project, by right click glew_shared and selecting retarget.

After that works:
Build glew_shared - debug - win32 
Result:
	bin\Debug\glew32d.dll .pdb
	lib\Debug\glew32d.lib .exp
	
Build glew_shared - release - win32
Result:
	bin\Debug\glew32.dll .pdb
	lib\Release\glew32.lib .exp

Build glew_static - debug - win32 
Result:
	lib\Debug\glew32sd.lib
	
Build glew_shared - release - win32
Result:
	lib\Release\glew32s.lib
	
Copy all dlls/pdb to bin folder (4 in total).
Copy all libs/exp to lib folder (6 in total).


Build and enjoy! 