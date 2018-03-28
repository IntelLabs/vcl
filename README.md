# The Visual Compute Library (VCL)

The Visual Compute Library was designed to provide an interface through which users can interact with visual data. We currently support images, though we are in the process of developing support for feature vectors and videos. 

The VCL was developed to support the use of an array-based image format, the TDB format that is built on the array data manager [TileDB](https://github.com/TileDB-Inc/TileDB). This results in a new lossless image format that is well suited for visual analytics. In addition, the VCL provides the same interface for PNG and JPEG formats by implementing an abstraction layer over OpenCV.  

More information can be found by running doxygen to get the full API of the code, or by looking at the wiki on github.com/IntelLabs/vcl 
