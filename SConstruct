import os

env = Environment(ENV=os.environ,
                  CXX="g++-snapshot",
                  CXXFLAGS= [ "-O3", "-g3",
                              "-std=c++11",
                              # "-ansi",
                              # "-pedantic",
                              "-Wno-unused-variable",
                              "-Wall",
                              "-Wextra",
                              "-Wnon-virtual-dtor",
                              "-Weffc++",
                              # "-Wconversion",
                              "-Wold-style-cast",
                              "-Werror",
                              "-Wshadow",
                              "-Wcast-qual",
                              "-Winit-self", # only works with >= -O1
                              "-Wno-unused-parameter"])

env.Append(LIBS=["SDL_image", "cwiid"])
env.ParseConfig("pkg-config --cflags --libs gstreamermm-0.10 | sed 's/-I/-isystem/g'")
env.ParseConfig("sdl-config --libs --cflags | sed 's/-I/-isystem/g'")
env.ParseConfig("pkg-config --libs --cflags  gl glu glew | sed 's/-I/-isystem/g'")
env.ParseConfig("pkg-config --libs --cflags cairomm-1.0 gl glu | sed 's/-I/-isystem/g'")

if False: 
    env.Append(LINKFLAGS=["-pg"], CXXFLAGS="-pg")

# build tests
if False: 
    test_env = env.Clone()
    test_env.Append(CPPPATH="src/")
    for filename in Glob("test/*.cpp", strings=True):
        test_env.Program(filename[0:-4], [filename, "src/video_processor.o"])

env.Program("viewer", Glob("src/*.cpp"))

# EOF #
