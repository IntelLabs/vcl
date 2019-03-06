# Cleanup
rm -rf tdb
rm -rf dbs
rm -rf temp
rm -rf videos_tests
mkdir dbs  # necessary for Descriptors
mkdir temp # necessary for Videos
mkdir videos_tests

# Compile and run tests
n=`getconf _NPROCESSORS_ONLN`
scons -j$n

rm -f log.log
./main 2> log.log
