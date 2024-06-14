#include<iostream>
#include<queue>
#include<list>
#include<iomanip>

#define FCFS 0
#define FR_FCFS 1
#define PARBS 2

using namespace std;

int number_of_process, number_of_bank, queue_size, policy;
int row_hit_latency, row_miss_latency, marking_cap, number_of_following_request;
int global_cycle;
int num_finish_requests;
int queue_valid_request;
int num_on_queue_requests;

class request{
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
    // void demo_request(){
    //     cout << serial_number << " " << pid << " " << at_bank << " " << at_row << endl;
    // }
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

request dummy_request(-1,-2,-3,-4);

class bank {
private:
    list<request> bank_ls;
public:
    request cur_request;
    int max_latency;
    int cur_latency;
    int bank_id;
    bool used;
    bank(request& _cur_request, int _bank_id)
        : cur_request(_cur_request), max_latency(-1), cur_latency(-1), bank_id(_bank_id),used(false) {
    }
    void update_bank_cur_latency_used() {
        if(used){
            if (cur_latency < max_latency-1) {
                cur_latency++;
            }
        } 
    }
    void update_bank_request_max_latency_cur_latency_used(request& target){
        if(used){
            cout << "fuck up used~~~~~~" << endl;
            return;
        }
        if(target.at_bank != bank_id){
            cout << "fuck up bank id~~~~~~" << endl;
            return;
        }
        if(cur_request.at_row==target.at_row)
            max_latency = row_hit_latency;
        else
            max_latency = row_miss_latency;
        cur_latency = 0;// since we will step 
        // update bank's current request by target
        cur_request = target;
        used = true;
    }
    void show_bank(){
        cout << "==============================================" << endl;
        cout << "at cycle: "<< global_cycle << endl;
        cout << cur_latency << "/" << max_latency << endl;
        if(used){
            cout << "used" << endl;
        }else{
            cout << "empty" << endl;
        }
        cout << "cur request: ";
        cur_request.demo_request();
        cout << endl;
        cout << "subqueue size: "<< bank_ls.size() << endl;
        demo_subqueue();
        cout << endl;
        cout << "==============================================" << endl;
    }
    void demo_subqueue(){
        for(auto rq=bank_ls.begin();rq!=bank_ls.end();++rq){
            (*rq).demo_request();
            cout << "   ";
        }
    }
    request fcfs_pick_request(){
        // if(global_cycle==5){
        //     show_bank();
        // }
        // if(global_cycle==6){
        //     show_bank();
        // }

        if(used) return dummy_request;
        for(auto rq=bank_ls.begin();rq!=bank_ls.end();++rq){
            return *rq;
        }
        return dummy_request;
    }
    int remove_bank_ls(request& target){
        if(target==dummy_request) return -1;
        for(auto rq=bank_ls.begin();rq!=bank_ls.end();++rq){
            if(*rq==target){
                bank_ls.erase(rq);
                return 1;
            }
        }
        return -1;
    }
    void append_bank_ls(request& target){
        bank_ls.emplace_back(target);
    }
    int get_bank_ls_size(){
        return bank_ls.size();
    }
    void demo_bank(){
        if(used){
            if(cur_latency==0){
                cur_request.demo_request();
            }else if(cur_latency>0 && cur_latency<max_latency-1){
                cout << "|              |";
            }else if(cur_latency==max_latency-1){
                cout << " -------------- ";
                used = false;
                num_finish_requests++;
            }else{
                cout << " ?????????????? ";
            }
        }else{
            cout << "                ";
        }
    }
    void update_bank(request& target){
        if(target!=dummy_request){
            update_bank_request_max_latency_cur_latency_used(target);
        }else{
            update_bank_cur_latency_used();
        }
    }
};

list<request> buffer;
vector<bank> my_banks;
vector<request> on_queue_possible_request(4, dummy_request);
request from_buffer_request = dummy_request;

void see_queue_possible_request(){
    cout << "$$:  ";
    for(int i=0;i<number_of_bank;i++){
        on_queue_possible_request[i].demo_request();
        cout << ">>>";
    }
}

void initer(){
    // init. from buffer request and on queue possible request by dummy request
    from_buffer_request = dummy_request;
    for(int i=0;i<number_of_bank;i++)
        on_queue_possible_request[i] = dummy_request;
}

void get_queue_size(){
    // collect the number of request on the queue
    num_on_queue_requests = 0;
    for(int i=0;i<number_of_bank;i++)
        num_on_queue_requests += my_banks[i].get_bank_ls_size();
}

void my_printer(){
    cout << std::left << std::setw(7) << global_cycle;
    if(from_buffer_request!=dummy_request)
        from_buffer_request.demo_request();
    else
        cout << "                ";
    for(int i=0;i<number_of_bank;i++){
        cout << "   ";
        my_banks[i].demo_bank();
    }
    // get_queue_size();
    // cout << "#on queue" << num_on_queue_requests;
    cout << endl;
}


void handle_fcfs(){
    while(num_finish_requests!=number_of_following_request){
        initer();
        get_queue_size();
        
        // for each bank, find a good request on queue to serve the dram
        queue_valid_request = 0;
        for(int i=0;i<number_of_bank;i++){
            request tmp_req = my_banks[i].fcfs_pick_request();
            if(tmp_req==dummy_request)continue;
            queue_valid_request++;
            on_queue_possible_request[i] = tmp_req;
        }

        // if we find good request on queue to serve the dram, 
        // then take one content from buffer into the queue.
        // if we can't find good request on queue, 
        // we check if the number of request on queue exceeds the max,
        // if so, grab a request from the buffer
        if(num_on_queue_requests<queue_size){
            if(buffer.size()>0)
                from_buffer_request = buffer.front();
        }
        // if(queue_valid_request>0){
        //     if(buffer.size()>0)
        //         from_buffer_request = buffer.front();
        // }

        // update bank's cycle
        for(int i=0;i<number_of_bank;i++){
            my_banks[i].update_bank(on_queue_possible_request[i]);
            my_banks[i].remove_bank_ls(on_queue_possible_request[i]);
        }

        if(from_buffer_request!=dummy_request){
            if(num_on_queue_requests<queue_size){
                // first enque
                my_banks[from_buffer_request.at_bank].append_bank_ls(from_buffer_request);
                for(auto rq=buffer.begin();rq!=buffer.end();++rq){
                    if(*rq==from_buffer_request){
                        buffer.erase(rq);
                        break;
                    }
                }
            }
        }
        // printing
        my_printer();
        global_cycle++;
    }
}

void solve(){
    // init global cycle
    global_cycle = 0;
    num_finish_requests = 0;
    // init bank
    for(int i=0;i<number_of_bank;i++){
        bank tmp(dummy_request, i);
        my_banks.push_back(tmp);
    }
    if(policy==FCFS){
        handle_fcfs();
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