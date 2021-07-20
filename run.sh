rm -rf build
mkdir build
cd build
cmake ../
make
cd ..
python3 ./tests/lab1/test_lexer.py
