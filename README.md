# gmtk game jam
thing

## How To Build
I just built this with manual G++, sadly.

### Without Statically-Linked Libraries

`g++ src/main.cpp src/misc/glad.c src/misc/shaders.cpp src/game.cpp src/scenes.cpp src/misc/collision.cpp src/misc/utility.cpp src/globals.cpp src/misc/audiomanager.cpp src/entity.cpp src/misc/camera2D.cpp src/misc/particles.cpp -o bin/main.exe -I include -L lib -lglfw3dll -lOpenAL32`

`g++ src/main.cpp src/misc/glad.c src/misc/shaders.cpp src/game.cpp src/scenes.cpp src/misc/collision.cpp src/misc/utility.cpp src/globals.cpp src/misc/audiomanager.cpp src/entity.cpp src/misc/camera2D.cpp src/misc/particles.cpp -o bin/main.exe -I include -L lib -static -static-libgcc -static-libstdc++ -lglfw3 -lopengl32 -lgdi32 -lOpenAL32`
(you still need to keep the `openal32.dll` file though)