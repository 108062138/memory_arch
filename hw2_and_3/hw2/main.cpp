#include<iostream>
#include<queue>
#include<list>

#define FCFS 0
#define FR_FCFS 1
#define PARBS 2

using namespace std;

int number_of_process, number_of_bank, queue_size, policy;
int row_hit_latency, row_miss_latency, marking_cap, number_of_following_request;

class request
{
private:
    int serial_number;
    int pid;
    int at_bank;
    int at_row;
public:
    request(int _serial_number, int _pid, int _at_bank, int _at_row);
    void demo_request();
};

request::request(int _serial_number, int _pid, int _at_bank, int _at_row){
    serial_number = _serial_number;
    pid = _pid;
    at_bank = _at_bank;
    at_row = _at_row;
}

void request::demo_request(){
    cout << serial_number << " " << pid << " " << at_bank << " " << at_row << endl;
}

list<request> fcfs_q;

void handle_input(){
    cin >> number_of_process >> number_of_bank >> queue_size >> policy;
    cin >> row_hit_latency >> row_miss_latency >> marking_cap >> number_of_following_request;
    int req_serial_number, req_pid, req_at_bank, req_at_row;
    for(int i=0;i<number_of_following_request;i++){
        cin >> req_serial_number >> req_pid >> req_at_bank >> req_at_row;
        request tmp_req(req_serial_number, req_pid, req_at_bank, req_at_row);
        fcfs_q.emplace_back(tmp_req);
        // cout << "ddd: " <<fcfs_q.size() << endl;
    }
}

void demo_input(){
    cout << number_of_process << endl;
    cout << number_of_bank << endl;
    cout << queue_size << endl;
    cout << policy << endl;
    cout << row_hit_latency << endl;
    cout << row_miss_latency << endl;
    cout << marking_cap << endl;
    cout << number_of_following_request << endl;
    for(auto iter = fcfs_q.begin();iter!=fcfs_q.end();++iter){
        iter->demo_request();
    }
}

int main(){
    handle_input();
    demo_input();
}