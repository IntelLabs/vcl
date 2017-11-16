# Cleanup
rm -r tdb
rm -r dbs
mkdir dbs # necessary for Descriptors

# Compile and run tests
scons -j16

./main
