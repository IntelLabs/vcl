# Cleanup
rm -r tdb
rm -r dbs
rm -r temp
mkdir dbs  # necessary for Descriptors
mkdir temp # necessary for Videos

# Compile and run tests
scons -j16

./main
