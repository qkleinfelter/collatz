echo "Incorrect arg count tests"
./project1
./project1 12 12
./project1 12 12 12
echo 

echo "0 or negative input tests"
./project1 0
./project1 -12
echo 

echo "Too large input test"
./project1 18446744073709551616
echo 

echo "Partial number inputs tests"
./project1 500AD
./project1 5ab8
echo 

echo "Things that should work"
./project1 1
./project1 5
./project1 12
./project1 123456789987654321
echo 