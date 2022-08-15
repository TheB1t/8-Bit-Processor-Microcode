echo "----[Compile program packer]----"
g++ decoderPacker.cpp -o decoderPacker
echo "----[Pack decoder]----"
./decoderPacker

echo "----[Compile decoder packer]----"
g++ programPacker.cpp -o programPacker
echo "----[Pack program]----"
./programPacker
