import os

wiic_env = Environment(CPPDEFINES = [ ("LINUX", 1) ])
libwiic = wiic_env.StaticLibrary("wiic",
                                 Glob("external/wiic-2013-02-12/src/wiic/*.c"))

wiicpp_env = Environment(CPPDEFINES = [ ("LINUX", 1) ],
                         CPPPATH = [ "external/wiic-2013-02-12/src/wiic",
                                     "external/wiic-2013-02-12/src/wiicpp",
                                     "external/wiic-2013-02-12/src/log" ])
libwiicpp = wiicpp_env.StaticLibrary("wiicpp",
                                     Glob("external/wiic-2013-02-12/src/wiicpp/*.cpp") +
                                     Glob("external/wiic-2013-02-12/src/log/*.cpp"))

glew = Environment(CPPPATH = [ "external/glew-1.9.0/include/" ])
libglew = glew.StaticLibrary(Glob("external/glew-1.9.0/src/*.c"))

yaml = Environment(CPPPATH = [ "external/yaml-cpp-0.5.0/include/" ])
libyaml = yaml.StaticLibrary(Glob("external/yaml-cpp-0.5.0/src/*.cpp"))

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

env.Append( LIBS = [ "cwiid", libwiicpp, libwiic, "bluetooth" ])
env.Append( LIBS = [ "boost_system", "boost_filesystem" ])
env.Append( CXXFLAGS = [ "-isystemexternal/glm-0.9.4.2",
                         "-isystemexternal/yaml-cpp-0.5.0/include/",
                         "-isystemexternal/glew-1.9.0/include",
                         "-isystemexternal/wiic-2013-02-12/src/wiic",
                         "-isystemexternal/wiic-2013-02-12/src/wiicpp",
                         "-isystemexternal/wiic-2013-02-12/src/log" ])
env.ParseConfig("pkg-config --libs --cflags bluez | sed 's/-I/-isystem/g'")
env.ParseConfig("pkg-config --cflags --libs gstreamermm-0.10 | sed 's/-I/-isystem/g'")
env.ParseConfig("pkg-config --libs --cflags sdl2 SDL2_image | sed 's/-I/-isystem/g'")
env.ParseConfig("pkg-config --libs --cflags  gl glu | sed 's/-I/-isystem/g'")
env.ParseConfig("pkg-config --libs --cflags cairomm-1.0 gl glu | sed 's/-I/-isystem/g'")
env.Append( LIBS = [ libglew ])
env.Append( LIBS = [ libyaml ])

if False: 
    env.Append(LINKFLAGS=["-pg"], CXXFLAGS="-pg")

# build tests
if True:
    test_env = env.Clone()
    test_env.Append(CPPPATH="src/")
    for filename in Glob("test/*.cpp", strings=True):
        test_env.Program(filename[0:-4], [filename, "src/video_processor.o", "src/texture.o", "src/tokenize.o", "src/wiimote_manager.o", "src/opengl_state.o"])

env.Program("viewer", Glob("src/*.cpp"))

# EOF #
