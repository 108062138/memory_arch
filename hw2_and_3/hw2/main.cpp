#include<iostream>
#include<queue>
#include<list>
#include<iomanip>
#include<vector>

#define FCFS 0
#define FR_FCFS 1
#define PARBS 2

using namespace std;

int number_of_process, number_of_bank, queue_size, policy;
int row_hit_latency, row_miss_latency, marking_cap, number_of_following_request;

int finished_request;
int global_cyc;

class request
{
public:
    int serial_number;
    int pid;
    int at_bank;
    int at_row;
    request(int _serial_number, int _pid, int _at_bank, int _at_row){
        serial_number = _serial_number;
        pid = _pid;
        at_bank = _at_bank;
        at_row = _at_row;
    }
    void demo_request(){
        cout << "t"<<std::left << std::setw(5) << serial_number;
        cout << "P"<<std::left << std::setw(2) << pid;
        cout << "B"<<std::left << std::setw(2) << at_bank;
        cout << "("<<std::left << std::setw(2) << at_row <<")";
    }
    bool operator==(const request& other) const {
        return (serial_number == other.serial_number &&
                pid == other.pid &&
                at_bank == other.at_bank &&
                at_row == other.at_row);
    }
    bool operator!=(const request& other) const {
        return (serial_number != other.serial_number ||
                pid != other.pid ||
                at_bank != other.at_bank ||
                at_row != other.at_row);
    }
};

request dummy_req(-11, -22, -33, -44);

class fifo_bank{
public:
    request cur_req;
    int max_latency;
    int cur_latency;
    bool occupied;
    list<request> sub_queue;
    fifo_bank() : cur_req(dummy_req), max_latency(row_hit_latency), cur_latency(row_hit_latency), occupied(false) {}
    void demo_bank(){
        if(occupied){
            if(cur_latency==0){
                cur_req.demo_request();
            }else if(cur_latency==max_latency-1){
                cout << " -------------- ";
            }else{
                cout << "|              |";
            }
        }else{
            cout << "                ";
        }
    }
    request pick_request_from_subqueue(){
        if(policy==FCFS){
            if(occupied) return dummy_req;
            for(auto rq=sub_queue.begin();rq!=sub_queue.end();++rq){
                return *rq;
            }
            return dummy_req;
        }else{
            if(occupied) return dummy_req;
            for(auto rq=sub_queue.begin();rq!=sub_queue.end();++rq){
                if((*rq).at_row == cur_req.at_row)
                    return *rq;
            }
            for(auto rq=sub_queue.begin();rq!=sub_queue.end();++rq){
                return *rq;
            }
            return dummy_req;
        }
        
    }
    void check_valid(){
        if(occupied){
            if(cur_latency==max_latency-1){
                occupied = false;
                finished_request++;
            }
        }
    }
    void update_bank(request& target){
        if(target==dummy_req){
            if(occupied){
                if(cur_latency<max_latency-1)
                    cur_latency++;
            }
        }else{
            for(auto rq=sub_queue.begin();rq!=sub_queue.end();++rq){
                if(*rq==target){
                    sub_queue.erase(rq);
                    break;
                }else{
                    cout << "wtfffffffff";
                }
            }
            if(target.at_row==cur_req.at_row)
                max_latency = row_hit_latency;
            else
                max_latency = row_miss_latency;
            cur_latency = 0;
            occupied = true;
            cur_req = target;
        }
    }
};

list<request> buffer;
vector<fifo_bank> my_banks;
vector<request> about_to_enter_dram_requests(4, dummy_req);
request about_to_enter_queue_request = dummy_req;

int get_queue_water_level(){
    int tmp = 0;
    for(int i=0;i<number_of_bank;i++) tmp+= my_banks[i].sub_queue.size();
    return tmp;
}

void fcfs_solver(){
    finished_request = 0;
    global_cyc = 0;
    for(int i=0;i<number_of_bank;i++){
        fifo_bank tmp;
        my_banks.push_back(tmp);
    }
    
    while(finished_request<number_of_following_request){
        // init staging request
        for(int i=0;i<number_of_bank;i++) about_to_enter_dram_requests[i] = dummy_req;
        about_to_enter_queue_request = dummy_req;

        // try to put a new request on the queue
        if(get_queue_water_level() < queue_size){
            if(buffer.size()>0)
                about_to_enter_queue_request = buffer.front();
        }

        // try to put bank sub queue's request over the bank
        for(int i=0;i<number_of_bank;i++){
            my_banks[i].check_valid();
            request tmp = my_banks[i].pick_request_from_subqueue();
            if(tmp!=dummy_req){
                about_to_enter_dram_requests[i] = tmp;
            }
        }

        // update each bank's subqueue
        for(int i=0;i<number_of_bank;i++){
            my_banks[i].update_bank(about_to_enter_dram_requests[i]);
        }

        // update the queue and buffer
        if(about_to_enter_queue_request!=dummy_req){
            if(buffer.size()>0){
                buffer.pop_front();
                int at_bank = about_to_enter_queue_request.at_bank;
                my_banks[at_bank].sub_queue.push_back(about_to_enter_queue_request);
            }
        }

        // print the request that enters the queue, print the bank's current status
        cout << std::left << setw(7) << global_cyc;
        if(about_to_enter_queue_request!=dummy_req){
            about_to_enter_queue_request.demo_request();
        }else{
            cout << "                ";
        }
        for(int i=0;i<number_of_bank;i++){
            cout << "   ";
            my_banks[i].demo_bank();
        }
        cout << endl;
        global_cyc++;
        for(int i=0;i<number_of_bank;i++){
            my_banks[i].check_valid();
        }
    }
}

void solve(){
    if(policy==FCFS){
        fcfs_solver();
    }
}

void handle_input(){
    cin >> number_of_process >> number_of_bank >> queue_size >> policy;
    cin >> row_hit_latency >> row_miss_latency >> marking_cap >> number_of_following_request;
    int req_serial_number, req_pid, req_at_bank, req_at_row;
    for(int i=0;i<number_of_following_request;i++){
        cin >> req_serial_number >> req_pid >> req_at_bank >> req_at_row;
        request tmp_req(req_serial_number, req_pid, req_at_bank, req_at_row);
        buffer.emplace_back(tmp_req);
        // cout << "ddd: " <<buffer.size() << endl;
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
}

int main(){
    handle_input();
    demo_input();
    solve();
}