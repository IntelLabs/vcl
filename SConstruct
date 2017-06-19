## Compile and Build Library ##

env = Environment(CPPPATH=['include', 'src'], CXXFLAGS="-std=c++11 -fopenmp -O3")

source_files = ['src/Image.cc', 'src/ImageData.cc', 'src/TDBObject.cc',
    'src/TDBImage.cc',
    'src/Exception.cc',
    'src/utils.cc'
    ]

env.SharedLibrary('libvcl.so', source_files,
    LIBS = [ 'tiledb', 'opencv_core', 'opencv_imgproc', 'opencv_imgcodecs', 'gomp'],
    LIBPATH = ['/usr/lib', '/usr/local/lib'])

## Compile and Run Tests ##

gtest_source = ['test/unit_tests/main_test.cc'
         , 'test/unit_tests/TDBImage_test.cc'
         , 'test/unit_tests/ImageData_test.cc'
         ,'test/unit_tests/Image_test.cc'
]

env.Program('test/unit_test', gtest_source,
        LIBS = ['vcl', 'gtest', 'pthread'
                ,'opencv_core'
                , 'opencv_imgcodecs'
                , 'opencv_highgui'
                , 'opencv_imgproc'
        ],
        LIBPATH = ['.', '/usr/lib', '/usr/local/lib'])
