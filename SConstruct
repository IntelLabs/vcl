## Compile and Build Library ##
vars = Variables(None, ARGUMENTS)
vars.Add(EnumVariable('ENABLE', 'Which dfs to enable', 'none',  allowed_values=('none', 'S3')))

AddOption('--timing', action='append_const', dest='cflags',
                      const='-DCHRONO_TIMING',
                      help= 'Build server with chronos')

env = Environment(variables = vars, CPPPATH=['include', 'src'], CXXFLAGS="-std=c++11 -fopenmp -O3")

libs = ['tiledb', 'opencv_core', 'opencv_imgproc', 'opencv_imgcodecs', 'gomp']
test_libs = ['vcl', 'gtest', 'pthread'
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

env.MergeFlags(GetOption('cflags'))

if GetOption('cflags'):
    env.Append(CPPPATH=['include', 'src', '/home/crstrong/external_github/vdms/utils/include'])
    libs.append('vdms-utils')
    libpath.append('/home/crstrong/external_github/vdms/utils/')

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
if GetOption('cflags'):
    test_libs.append('vdms-utils')
    libpath.append('/home/crstrong/external_github/vdms/utils/')

env.Program('test/unit_test', gtest_source,
        LIBS = test_libs,
        LIBPATH = libpath)


cputest_source = ['test/benchmark_tests/cpu_tests.cc']
test_libs.append('lz4')
env.Program('test/benchmark_tests/cpu_tests', cputest_source, 
    LIBS=test_libs, LIBPATH=libpath)

iotest_source = ['test/benchmark_tests/io_test.cc']
env.Append(CPPPATH=['include', 'src', '/home/crstrong/external_github/vdms/utils/include'])
test_libs.append('vdms-utils')
libpath.append('/home/crstrong/external_github/vdms/utils/')
env.Program('test/benchmark_tests/io_test', iotest_source, 
    LIBS=test_libs, LIBPATH=libpath)