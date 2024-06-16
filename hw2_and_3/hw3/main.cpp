#include<iostream>
#include<queue>
#include<list>
#include<iomanip>
#include<vector>
#include<algorithm>
#include<queue>

#define FCFS 0
#define FR_FCFS 1
#define PARBS 2

using namespace std;

int number_of_process, number_of_bank, queue_size, policy;
int row_hit_latency, row_miss_latency, marking_cap, number_of_following_request;

int finished_request;
int global_cyc;

vector<int> max_load;
vector<int> total_rule;

class request
{
public:
    int serial_number;
    int pid;
    int at_bank;
    int at_row;

    int cur_row;
    bool marked;
    request(int _serial_number, int _pid, int _at_bank, int _at_row){
        serial_number = _serial_number;
        pid = _pid;
        at_bank = _at_bank;
        at_row = _at_row;
        marked = false;
        cur_row = 0;
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
    bool operator<(const request& other) const {
        if(marked && !other.marked) return false;
        else if(!marked && other.marked) return true;

        if(at_row==cur_row && other.at_row!=cur_row) return false;
        else if(at_row!=cur_row && other.at_row==cur_row) return true;

        if(max_load[pid] > max_load[other.pid]) return true;
        else if(max_load[pid] < max_load[other.pid]) return false;

        if(total_rule[pid] > total_rule[other.pid]) return true;
        else if(total_rule[pid] < total_rule[other.pid]) return false;

        if(pid > other.pid) return false;
        else if(pid < other.pid) return true;

        if(serial_number > other.serial_number) return true;
        else return false;
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
        }else if(policy==FR_FCFS){
            if(occupied) return dummy_req;
            for(auto rq=sub_queue.begin();rq!=sub_queue.end();++rq){
                if((*rq).at_row == cur_req.at_row)
                    return *rq;
            }
            for(auto rq=sub_queue.begin();rq!=sub_queue.end();++rq){
                return *rq;
            }
            return dummy_req;
        }else if(policy == PARBS){
            if(occupied) return dummy_req;
            // set cur row
            for(auto rq=sub_queue.begin();rq!=sub_queue.end();++rq){
                (*rq).cur_row = cur_req.at_row;
            }

            auto max_it = std::max_element(sub_queue.begin(), sub_queue.end());

            if(max_it!=sub_queue.end()) return *max_it;

            return dummy_req;
        }else return dummy_req;
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
    void single_bank_batch(){
        vector<int> cap_counter(number_of_process, 0);
        for(auto rq=sub_queue.begin();rq!=sub_queue.end();++rq){
            if(cap_counter[(*rq).pid] < marking_cap){
                cap_counter[(*rq).pid]++;
                (*rq).marked = true;
            }
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

bool should_activate_batch(){
    for(int i=0;i<number_of_bank;i++){
        for(auto rq=my_banks[i].sub_queue.begin();rq!=my_banks[i].sub_queue.end();++rq){
            if((*rq).marked) return false;
        }
    }
    return true;
}

void batch_process(){
    for(int i=0;i<number_of_bank;i++){
        my_banks[i].single_bank_batch();
    }
}

void update_max_load_total_rule(){
    // init max load
    for(int j=0;j<number_of_process;j++)
        max_load[j] = 0;
    // update max load to maintain max rule
    // accumulated marked request to maintain total rule
    for(int i=0;i<number_of_bank;i++){
        vector<int> per_bank_max_load(number_of_process, 0);
        for(auto rq=my_banks[i].sub_queue.begin();rq!=my_banks[i].sub_queue.end();++rq){
            if((*rq).marked){
                per_bank_max_load[(*rq).pid]++;
                total_rule[(*rq).pid]++;
            }
        }
        for(int j=0;j<number_of_process;j++)
            if(per_bank_max_load[j] > max_load[j])
                max_load[j] = per_bank_max_load[j];
    }
}

void dram_schedular(){
    finished_request = 0;
    global_cyc = 0;
    bool use_batch;
    for(int i=0;i<number_of_bank;i++){
        fifo_bank tmp;
        my_banks.push_back(tmp);
    }

    for(int i=0;i<number_of_process;i++){
        max_load.push_back(0);
        total_rule.push_back(0);
    }
    
    while(finished_request<number_of_following_request){
        // if(global_cyc>30) break;
        // init staging request
        for(int i=0;i<number_of_bank;i++) about_to_enter_dram_requests[i] = dummy_req;
        about_to_enter_queue_request = dummy_req;
        use_batch = false;

        // try to put a new request on the queue
        if(get_queue_water_level() < queue_size){
            if(buffer.size()>0){
                about_to_enter_queue_request = buffer.front();
            }
        }
        
        // handle parbs
        if(should_activate_batch()){
            batch_process();
            update_max_load_total_rule();
            use_batch = true;
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
        // cout << endl;
        // if(use_batch) cout << "use batch";
        // else cout << " hold.....";
        // cout << endl;
        // cout << "for   max: ";
        // for(int j=0;j<number_of_process;j++)
        //     cout << max_load[j] << " ";
        // cout << endl;
        // cout << "for total: ";
        // for(int j=0;j<number_of_process;j++)
        //     cout << total_rule[j] << " ";
        cout << endl;
        global_cyc++;
        
        for(int i=0;i<number_of_bank;i++){
            my_banks[i].check_valid();
        }
    }
}

void solve(){
    dram_schedular();
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