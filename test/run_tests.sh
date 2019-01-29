# Cleanup
rm -r tdb
rm -r dbs
rm -r temp
rm -r videos_tests
mkdir dbs  # necessary for Descriptors
mkdir temp # necessary for Videos
mkdir videos_tests

# Compile and run tests
scons -j16

rm log.log
./main 2> log.log
