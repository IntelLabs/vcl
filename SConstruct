## Compile and Build Library ##

env = Environment(CPPPATH=['include', 'src',
                           '/usr/local/include/'],
                  CXXFLAGS="-std=c++11 -O3 -fopenmp")

source_files = [
                'src/utils.cc',
                'src/Exception.cc',
                'src/TDBObject.cc',
                'src/Image.cc',
                'src/ImageData.cc',
                'src/TDBImage.cc',
                'src/DescriptorSet.cc',
                'src/DescriptorSetData.cc',
                'src/FaissDescriptorSet.cc',
                'src/TDBDescriptorSet.cc',
                'src/TDBDenseDescriptorSet.cc',
                'src/TDBSparseDescriptorSet.cc',
                ]

env.SharedLibrary('libvcl.so', source_files,
    LIBS = [ 'tiledb',
             'opencv_core',
             'opencv_imgproc',
             'opencv_imgcodecs',
             'gomp',
             'faiss',
             ],

    LIBPATH = ['/usr/local/lib',
               '/usr/lib',
              ],

    LINKFLAGS="-Wl,--no-as-needed",
    )
