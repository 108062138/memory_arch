make
input_file="t8.txt"
./main < "$input_file" > tmp.log
# diff t1.txt  tmp.txt
diff <(sed 's/\r$//' "$input_file") <(sed 's/\r$//' tmp.log)