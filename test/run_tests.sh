# Cleanup
rm -r tdb
rm -r dbs
rm -r temp
rm -r videos_test
mkdir dbs  # necessary for Descriptors
mkdir temp # necessary for Videos
mkdir videos_test

# Compile and run tests
scons -j16

./main
