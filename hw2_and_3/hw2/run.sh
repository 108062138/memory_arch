make
input_file="t4.txt"
answer_file="a4.txt"
./main < "$input_file" > tmp.log
# diff t1.txt  tmp.txt
diff <(sed 's/\r$//' "$answer_file") <(sed 's/\r$//' tmp.log)