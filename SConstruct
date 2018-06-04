## Compile and Build Library ##
vars = Variables(None, ARGUMENTS)
vars.Add(EnumVariable('ENABLE', 'Which dfs to enable', 'none',  allowed_values=('none', 'S3')))

env = Environment(variables = vars, CPPPATH=['include', 'src'], CXXFLAGS="-std=c++11 -fopenmp -O3")

libs = ['tiledb', 'opencv_core', 'opencv_imgproc', 'opencv_imgcodecs', 'gomp']
test_libs = ['vcl', 'gtest', 'pthread', 'gomp', 'tbb'
            ,'opencv_core', 'opencv_imgcodecs', 'opencv_highgui', 'opencv_imgproc']
libpath = ['/usr/local/lib', '/usr/lib']

if env['ENABLE'] == 'S3':
    print '*** Enabling AWS S3 ***'
    env.Append( CPPDEFINES="S3_SUPPORT" )
    libs.append('aws-cpp-sdk-core')
    libs.append('aws-cpp-sdk-s3')
    test_libs.append('aws-cpp-sdk-core')
    test_libs.append('aws-cpp-sdk-s3')

source_files = ['src/Image.cc', 
                'src/ImageData.cc', 
                'src/TDBObject.cc',
                'src/TDBImage.cc', 
                'src/RemoteConnection.cc',
                'src/Exception.cc',
                'src/utils.cc'
    ]

env.SharedLibrary('libvcl.so', source_files,
    LIBS = libs,
    LIBPATH = libpath)

## Compile and Run Tests ##

gtest_source = ['test/unit_tests/main_test.cc'
         , 'test/unit_tests/TDBImage_test.cc'
         , 'test/unit_tests/ImageData_test.cc'
         , 'test/unit_tests/Image_test.cc'
         , 'test/unit_tests/RemoteConnection_test.cc'
    ]

libpath.append('.')

env.Program('test/unit_test', gtest_source,
        LIBS = test_libs,
        LIBPATH = libpath)


cputest_source = ['test/benchmark_tests/cpu_tests.cc']
test_libs.append('lz4')
env.Program('test/benchmark_tests/cpu_tests', cputest_source, 
    LIBS=test_libs, LIBPATH=libpath)

iotest_source = ['test/benchmark_tests/aws_io_tests.cc', 'src/chrono/Chrono.cc']
test_libs.append('tiledb')
env.Append(CPPPATH=['include', 'src', 'include/chrono'])
env.Program('test/benchmark_tests/io_test', iotest_source, 
    LIBS=test_libs, LIBPATH=libpath)
