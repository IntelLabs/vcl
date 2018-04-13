## Compile and Build Library ##

# env = Environment(CPPPATH=['include', 'src'], CXXFLAGS="-std=c++11 -fopenmp -O3")
env_s3 = Environment(CPPPATH=['include', 'src'], CXXFLAGS="-std=c++11 -fopenmp -O3", CPPDEFINES="HAVE_S3")

source_files = ['src/Image.cc', 'src/ImageData.cc', 'src/TDBObject.cc',
    'src/TDBImage.cc', 'src/RemoteConnection.cc',
    'src/Exception.cc',
    'src/utils.cc'
    ]

# env.SharedLibrary('libvcl.so', source_files,
#     LIBS = [ 'tiledb', 'opencv_core', 'opencv_imgproc', 'opencv_imgcodecs', 'gomp'],
#     LIBPATH = ['/usr/local/lib', '/usr/lib'])


env_s3.SharedLibrary('libvcl.so', source_files,
    LIBS = [ 'tiledb', 'opencv_core', 'opencv_imgproc', 'opencv_imgcodecs', 
             'aws-cpp-sdk-core', 'aws-cpp-sdk-s3', 'gomp'],
    LIBPATH = ['/usr/local/lib', '/usr/lib'])

## Compile and Run Tests ##

gtest_source = ['test/unit_tests/main_test.cc'
         , 'test/unit_tests/TDBImage_test.cc'
         , 'test/unit_tests/ImageData_test.cc'
         ,'test/unit_tests/Image_test.cc'
]

# env.Program('test/unit_test', gtest_source,
#         LIBS = ['vcl', 'gtest', 'pthread'
#                 ,'opencv_core'
#                 , 'opencv_imgcodecs'
#                 , 'opencv_highgui'
#                 , 'opencv_imgproc'
#         ],
#         LIBPATH = ['.', '/usr/local/lib', '/usr/lib'])

env_s3.Program('test/unit_test', gtest_source,
        LIBS = ['vcl', 'gtest', 'pthread'
                ,'opencv_core'
                , 'opencv_imgcodecs'
                , 'opencv_highgui'
                , 'opencv_imgproc'
                , 'aws-cpp-sdk-core'
		        , 'aws-cpp-sdk-s3'
        ],
        LIBPATH = ['.', '/usr/local/lib', '/usr/lib'])
