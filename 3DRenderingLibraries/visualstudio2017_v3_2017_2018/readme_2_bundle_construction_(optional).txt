For details on how this bundle was constructed in case you do NOT want to use the given bundle (Why god why?)
or want to upgrade/downgrade to a different version of a library, see below.

Bundle construction:
--------------------

1. Create basic folder vs_extended, with 3 folders in: bin, lib, include

2. Overlay GLM
--------------------------------------
Download the latest GLM from https://glm.g-truc.net/
Copy the GLM folder at glm\glm (so the deepest one) to vs_extended\include

3. Overlay GLEW
--------------------------------------
Download GLEW from http://glew.sourceforge.net/
Copy include/bin/lib to the corresponding folders with the same name in the vs_extended folder.
(Or build it from source)

4. Overlay SFML
---------------
Download the latest SFML for Visual C++ xx (xxxx) 32 bit (depends on your VS version).
Copy include/bin/lib to the corresponding folders with the same name in the vs_extended folder

