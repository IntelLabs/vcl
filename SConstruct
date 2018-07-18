## Compile and Build Library ##

env = Environment(CPPPATH=['include', 'src'], CXXFLAGS="-std=c++11 -fopenmp -O3")

source_files = [
    'src/Video.cc',
    'src/Image.cc',
    'src/VideoData.cc',
    'src/ImageData.cc',
    'src/TDBObject.cc',
    'src/TDBImage.cc',
    'src/Exception.cc',
    'src/utils.cc'
    # 'src/VideoUtils.cc',
    # 'src/Globals.cpp'
    ]

env.SharedLibrary('libvcl.so', source_files,
    LIBS = [ 'tiledb', 'opencv_core', 'opencv_imgproc', 'opencv_imgcodecs', 'gomp', 'opencv_videoio', 'opencv_highgui', 'boost_system' ,'boost_filesystem'],
    LIBPATH = ['/usr/lib', '/usr/local/lib'])

## Compile and Run Tests ##

gtest_source = ['test/unit_tests/main_test.cc'
#         , 'test/unit_tests/TDBImage_test.cc'
#         , 'test/unit_tests/ImageData_test.cc'
#         , 'test/unit_tests/Image_test.cc'
	     , 'test/unit_tests/Video_test.cc'
 #       , 'test/unit_tests/VideoData_test.cc'
]

env.ParseConfig('pkg-config --cflags --libs opencv')
env.Program('test/unit_test', gtest_source,
        LIBS = ['vcl', 'gtest', 'pthread'
                ,'opencv_core'
                , 'opencv_imgcodecs'
                , 'opencv_highgui'
                , 'opencv_imgproc'
                , 'opencv_videoio'
        ],
        LIBPATH = ['.', '/usr/local/lib', '/usr/lib'])
