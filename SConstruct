import os

wiic = Environment(CPPDEFINES = [ ("LINUX", 1) ])
libwiic = wiic.StaticLibrary("wiic",
                             Glob("external/wiic-2013-02-12/src/wiic/*.c"))

wiicpp = Environment(CPPDEFINES = [ ("LINUX", 1) ],
                     CPPPATH = [ "external/wiic-2013-02-12/src/wiic",
                                 "external/wiic-2013-02-12/src/wiicpp",
                                 "external/wiic-2013-02-12/src/log" ])
libwiicpp = wiicpp.StaticLibrary("wiicpp",
                                 Glob("external/wiic-2013-02-12/src/wiicpp/*.cpp") +
                                 Glob("external/wiic-2013-02-12/src/log/*.cpp"))

glew = Environment(CPPPATH = [ "external/glew-1.9.0/include/" ])
libglew = glew.StaticLibrary(Glob("external/glew-1.9.0/src/*.c"))

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

env.Append( LIBS = [ "SDL_image", "cwiid", libwiicpp, libwiic, "bluetooth" ])
env.Append( CPPPATH = [ "external/glew-1.9.0/include",
                        "external/wiic-2013-02-12/src/wiic",
                        "external/wiic-2013-02-12/src/wiicpp",
                        "external/wiic-2013-02-12/src/log" ])
env.ParseConfig("pkg-config --libs --cflags bluez | sed 's/-I/-isystem/g'")
env.ParseConfig("pkg-config --cflags --libs gstreamermm-0.10 | sed 's/-I/-isystem/g'")
env.ParseConfig("sdl-config --libs --cflags | sed 's/-I/-isystem/g'")
env.ParseConfig("pkg-config --libs --cflags  gl glu | sed 's/-I/-isystem/g'")
env.ParseConfig("pkg-config --libs --cflags cairomm-1.0 gl glu | sed 's/-I/-isystem/g'")
env.Append( LIBS = [ libglew ])

if False: 
    env.Append(LINKFLAGS=["-pg"], CXXFLAGS="-pg")

# build tests
if True:
    test_env = env.Clone()
    test_env.Append(CPPPATH="src/")
    for filename in Glob("test/*.cpp", strings=True):
        test_env.Program(filename[0:-4], [filename, "src/video_processor.o", "src/texture.o"])

env.Program("viewer", Glob("src/*.cpp"))

# EOF #
