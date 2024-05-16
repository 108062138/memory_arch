#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <cassert>
#include <cstdint> // Include for uint32_t
#include <iomanip> // Include for std::setw

#define LINE_STATE int32_t
#define UINT32 int32_t
#define INT32 int32_t
#define Addr_t uint32_t
#define old_enough (3)

using namespace std;

struct Operation {
    int serialNumber;
    int programCounter;
    int accessedSetNumber;
    int hitOrMiss;
    int wayHit;
    bool hasWayHit; // to check if wayHit field is present
};

int numSets, numWays, policyIndicator, numCounters, numOperations;
bool last_operation;
vector<Operation> operations;

void parse_input() {
    string line;
    int lineNumber = 0;

    last_operation = false;

    while (getline(cin, line)) {
        stringstream ss(line);
        if (lineNumber == 0) {
            ss >> numSets;
        } else if (lineNumber == 1) {
            ss >> numWays;
        } else if (lineNumber == 2) {
            ss >> policyIndicator;
        } else if (lineNumber == 3) {
            ss >> numCounters;
        } else if (lineNumber == 4) {
            ss >> numOperations;
        } else {
            Operation op;
            ss >> op.serialNumber >> op.programCounter >> op.accessedSetNumber >> op.hitOrMiss;
            if (op.hitOrMiss == 1) {
                ss >> op.wayHit;
                op.hasWayHit = true;
            } else {
                op.hasWayHit = false;
            }
            operations.push_back(op);
        }
        lineNumber++;
    }

    // // Output the read values for verification
    // cout << "Number of sets: " << numSets << endl;
    // cout << "Number of ways: " << numWays << endl;
    // cout << "Policy indicator: " << policyIndicator << endl;
    // cout << "Number of counters: " << numCounters << endl;
    // cout << "Number of operations: " << numOperations << endl;

    // for (const auto& op : operations) {
    //     cout << "Operation " << op.serialNumber
    //          << ": PC=" << op.programCounter
    //          << ", Set=" << op.accessedSetNumber
    //          << ", Hit/Miss=" << op.hitOrMiss;
    //     if (op.hasWayHit) {
    //         cout << ", WayHit=" << op.wayHit;
    //     }
    //     cout << endl;
    // }
}


#ifndef REPL_STATE_H
#define REPL_STATE_H

// Replacement Policies Supported
typedef enum 
{
    CRC_REPL_LRU        = 0,
    CRC_REPL_RANDOM     = 1,
    CRC_REPL_CONTESTANT = 2
} ReplacemntPolicy;

// Replacement State Per Cache Line
typedef struct
{
    UINT32  LRUstackposition;

    // CONTESTANTS: Add extra state per cache line here
    UINT32 RRPV;
    bool outcome;
    UINT32 signature;
} LINE_REPLACEMENT_STATE;


// The implementation for the cache replacement policy
class CACHE_REPLACEMENT_STATE
{

  private:
    UINT32 numsets;
    UINT32 assoc;
    UINT32 replPolicy;
    
    LINE_REPLACEMENT_STATE   **repl;

    // COUNTER mytimer;  // tracks # of references to the cache

    // CONTESTANTS:  Add extra state for cache here

  public:
    vector<int> SHCT;
    

    // The constructor CAN NOT be changed
    CACHE_REPLACEMENT_STATE( UINT32 _sets, UINT32 _assoc, UINT32 _pol );

    INT32  GetVictimInSet( UINT32 tid, UINT32 setIndex, const LINE_STATE *vicSet, UINT32 assoc, Addr_t PC, Addr_t paddr, UINT32 accessType );
    void   UpdateReplacementState( UINT32 setIndex, INT32 updateWayID );

    void   SetReplacementPolicy( UINT32 _pol ) { replPolicy = _pol; } 
    // void   IncrementTimer() { mytimer++; } 

    void   UpdateReplacementState( UINT32 setIndex, INT32 updateWayID, const LINE_STATE *currLine, 
                                   UINT32 tid, Addr_t PC, UINT32 accessType, bool cacheHit );

    ostream&   PrintStats( ostream &out);

  private:
    
    void   InitReplacementState();
    INT32  Get_Random_Victim( UINT32 setIndex );

    INT32  Get_LRU_Victim( UINT32 setIndex );

    INT32  Get_SHiP_Victim(UINT32 setIndex );
    INT32  Exist_Old_Enough(UINT32 setIndex);
    void   Make_SetIndex_Older(UINT32 setIndex);
    
    void   UpdateLRU( UINT32 setIndex, INT32 updateWayID );
    
    void   Update_SHiP( INT32 setIndex, INT32 updateWayID, INT32 tid, 
                        Addr_t PC, UINT32 accessType, bool cacheHit);
    void   Handle_SHiP_Cache_Hit(UINT32 setIndex, INT32 updateWayID);
    void   Handle_SHiP_Cache_Miss(UINT32 setIndex, UINT32 tid, Addr_t PC, UINT32 accessType);
};
#endif

CACHE_REPLACEMENT_STATE::CACHE_REPLACEMENT_STATE( UINT32 _sets, UINT32 _assoc, UINT32 _pol )
{

    numsets    = _sets;
    assoc      = _assoc;
    replPolicy = _pol;

    // mytimer    = 0;

    InitReplacementState();
}

void CACHE_REPLACEMENT_STATE::InitReplacementState()
{
    // Create the state for sets, then create the state for the ways
    repl  = new LINE_REPLACEMENT_STATE* [ numsets ];

    vector<int> tmp(numCounters, 1);
    SHCT = tmp;

    // ensure that we were able to create replacement state
    assert(repl);

    // Contestants:  ADD INITIALIZATION FOR YOUR HARDWARE HERE
    // Create the state for the sets
    for(UINT32 setIndex=0; setIndex<numsets; setIndex++) 
    {
        repl[ setIndex ]  = new LINE_REPLACEMENT_STATE[ assoc ];
        for(UINT32 way=0; way<assoc; way++) 
        {
            // initialize stack position (for true LRU)
            repl[ setIndex ][ way ].LRUstackposition = way;
            repl[ setIndex ][ way ].RRPV = old_enough;
            repl[ setIndex ][ way ].outcome = false;
            repl[ setIndex ][ way ].signature = 0;
        }
    }
}

INT32 CACHE_REPLACEMENT_STATE::Get_LRU_Victim( UINT32 setIndex )
{
    // Get pointer to replacement state of current set
    LINE_REPLACEMENT_STATE *replSet = repl[ setIndex ];

    INT32   lruWay   = 0;

    // Search for victim whose stack position is assoc-1
    for(UINT32 way=0; way<assoc; way++) 
    {
        if( replSet[way].LRUstackposition == (assoc-1) ) 
        {
            lruWay = way;
            break;
        }
    }

    // return lru way
    return lruWay;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function finds a random victim in the cache set                       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::Get_Random_Victim( UINT32 setIndex )
{
    INT32 way = (rand() % assoc);
    
    return way;
}

/*
    check if there exists an old line to be swapped out
    if so, return its way number
    else, return -1 to indicate that there is no valid candidate
*/
INT32 CACHE_REPLACEMENT_STATE::Exist_Old_Enough(UINT32 setIndex){
    // Get pointer to replacement state of current set
    LINE_REPLACEMENT_STATE *replSet = repl[setIndex];

    INT32 ship_way = -1;

    // Search for victim whose RRPV==3
    for(UINT32 way=0; way<assoc; way++){
        if(replSet[way].RRPV == old_enough){
            ship_way = way;
            break;
        }
    }
    return ship_way;
}

/*
    since no line is old enough, 
    increment the setIndex's each line by 1 s.t. we can select a good candidate
*/
void CACHE_REPLACEMENT_STATE::Make_SetIndex_Older(UINT32 setIndex){
     // Get pointer to replacement state of current set
    LINE_REPLACEMENT_STATE *replSet = repl[ setIndex ];

    // make each line older
    for(UINT32 way=0; way<assoc; way++){
        if(replSet[way].RRPV< old_enough)
            replSet[way].RRPV++;
    }
}


INT32 CACHE_REPLACEMENT_STATE::Get_SHiP_Victim(UINT32 setIndex){
    int candidate;
    while(1){
        candidate = Exist_Old_Enough(setIndex);
        if(candidate==-1){
            Make_SetIndex_Older(setIndex);
            continue;
        }else break;
    }

    return candidate;
}


INT32 CACHE_REPLACEMENT_STATE::GetVictimInSet( UINT32 tid, UINT32 setIndex, const LINE_STATE *vicSet, UINT32 assoc,
                                               Addr_t PC, Addr_t paddr, UINT32 accessType )
{
    // If no invalid lines, then replace based on replacement policy
    if( replPolicy == CRC_REPL_LRU ) 
    {
        return Get_LRU_Victim( setIndex );
    }
    else if( replPolicy == CRC_REPL_RANDOM )
    {
        return Get_Random_Victim( setIndex );
    }
    else if( replPolicy == CRC_REPL_CONTESTANT )
    {
        // Contestants:  ADD YOUR VICTIM SELECTION FUNCTION HERE
        return Get_SHiP_Victim(setIndex);
    }

    // We should never get here
    assert(0);

    return -1; // Returning -1 bypasses the LLC
}

void CACHE_REPLACEMENT_STATE::UpdateLRU( UINT32 setIndex, INT32 updateWayID )
{
    // Determine current LRU stack position
    UINT32 currLRUstackposition = repl[ setIndex ][ updateWayID ].LRUstackposition;

    // Update the stack position of all lines before the current line
    // Update implies incremeting their stack positions by one
    for(UINT32 way=0; way<assoc; way++) 
    {
        if( repl[setIndex][way].LRUstackposition < currLRUstackposition ) 
        {
            repl[setIndex][way].LRUstackposition++;
        }
    }

    // Set the LRU stack position of new line to be zero
    repl[ setIndex ][ updateWayID ].LRUstackposition = 0;
}

/*
    handle ship's hit
*/
void CACHE_REPLACEMENT_STATE :: Handle_SHiP_Cache_Hit(UINT32 setIndex, INT32 updateWayID){
    cout << "Hit" << " " << updateWayID << endl;
    repl[ setIndex ][ updateWayID ].outcome = true;
    if(SHCT[repl[ setIndex ][ updateWayID ].signature] <3)
        SHCT[repl[ setIndex ][ updateWayID ].signature]++;
    repl[ setIndex ][ updateWayID ].RRPV = 0;

    cout << std::left << std::setw(11) << "RRPV:";
    for(int i=0; i<assoc; i++){
        if(i == updateWayID){
            cout << "(" << repl[ setIndex ][ i ].RRPV << ")";
        }else{
            cout << repl[ setIndex ][ i ].RRPV;   
        }
        if(i==assoc-1) cout << endl;
        else cout << " ";
    }

    cout << std::left << std::setw(11) << "Signature:";
    
    for(int i=0; i<assoc; i++){
        cout << repl[ setIndex ][ i ].signature;
        if(i==assoc-1) cout << endl;
        else cout << " ";
    }

    cout << std::left << std::setw(11) << "outcome:";
    
    for(int i=0; i<assoc; i++){
        if(i == updateWayID){
            cout << "(" << repl[ setIndex ][ i ].outcome << ")";
        }else{
            cout << repl[ setIndex ][ i ].outcome;   
        }
        if(i==assoc-1) cout << endl;
        else cout << " ";
    }

    cout << std::left << std::setw(11) << "SHCT:";
    for(int i=0; i<numCounters; i++){
        if(i == repl[ setIndex ][ updateWayID ].signature){
            cout << "(" << SHCT[i] << ")";
        }else{
            cout << SHCT[i];
        }
        if(i==numCounters-1){
            if(!last_operation)
                cout << endl;
        }else cout << " ";
    }
}

/*
    handle ship miss
*/
void CACHE_REPLACEMENT_STATE :: Handle_SHiP_Cache_Miss(UINT32 setIndex, UINT32 tid, Addr_t PC, UINT32 accessType){
    INT32 victim = GetVictimInSet(tid, setIndex, nullptr, assoc, PC, 0, accessType);
    int last_signature = repl[setIndex][victim].signature;
    bool modify_shct = false;
    if(repl[setIndex][victim].outcome == false){
        modify_shct = true;
        if(SHCT[repl[setIndex][victim].signature] > 0){
            SHCT[repl[setIndex][victim].signature]--;
        }
    }
    // insert the new line

    repl[setIndex][victim].outcome = false;
    repl[setIndex][victim].signature = PC % numCounters;
    if(SHCT[repl[setIndex][victim].signature] == 0){
        repl[setIndex][victim].RRPV = 3;
    }else{
        repl[setIndex][victim].RRPV = 2;
    }

    cout << "Replace" << " " << victim << endl;
    cout << std::left << std::setw(11) << "RRPV:";
    
    for(int i=0; i<assoc; i++){
        if(i == victim){
            cout << "(" << repl[ setIndex ][ i ].RRPV << ")";
        }else{
            cout << repl[ setIndex ][ i ].RRPV;   
        }
        if(i==assoc-1) cout << endl;
        else cout << " ";
    }
    
    cout << std::left << std::setw(11) << "Signature:";
    
    for(int i=0;i<assoc;i++){
        if(i == victim){
            cout << "(" << repl[ setIndex ][ i ].signature << ")";
        }else{
            cout << repl[ setIndex ][ i ].signature;
        }
        if(i==assoc-1) cout << endl;
        else cout << " ";
    }
    
    cout << std::left << std::setw(11) << "outcome:";
    
    for(int i=0;i<assoc;i++){
        if(i == victim){
            cout << "(" << repl[ setIndex ][ i ].outcome << ")";
        }else{
            cout << repl[ setIndex ][ i ].outcome;
        }
        if(i==assoc-1) cout << endl;
        else cout << " ";
    }

    cout << std::left << std::setw(11) << "SHCT:";

    for(int i=0; i<numCounters; i++){
        if(i == last_signature && modify_shct){
            cout << "(" << SHCT[i] << ")";
        }else{
            cout << SHCT[i];
        }
        if(i==numCounters-1){
            if(!last_operation)
                cout << endl;
        }else cout << " ";
    }
}

/*
    according to the cache_hit, we have to update SHCT differently
*/

void CACHE_REPLACEMENT_STATE :: Update_SHiP(INT32 setIndex, INT32 updateWayID, INT32 tid, Addr_t PC, UINT32 accessType, bool cacheHit){
    if(cacheHit) Handle_SHiP_Cache_Hit(setIndex, updateWayID);
    else Handle_SHiP_Cache_Miss(setIndex, tid, PC, accessType);
}

void CACHE_REPLACEMENT_STATE::UpdateReplacementState( 
    UINT32 setIndex, INT32 updateWayID, const LINE_STATE *currLine, 
    UINT32 tid, Addr_t PC, UINT32 accessType, bool cacheHit )
{
    // What replacement policy?
    if( replPolicy == CRC_REPL_LRU ) 
    {
        UpdateLRU( setIndex, updateWayID );
    }
    else if( replPolicy == CRC_REPL_RANDOM )
    {
        // Random replacement requires no replacement state update
    }
    else if( replPolicy == CRC_REPL_CONTESTANT )
    {
        // Contestants:  ADD YOUR UPDATE REPLACEMENT STATE FUNCTION HERE
        // Feel free to use any of the input parameters to make
        // updates to your replacement policy
        Update_SHiP(setIndex, updateWayID, tid, PC, accessType, cacheHit);
    }
}

ostream & CACHE_REPLACEMENT_STATE::PrintStats(ostream &out)
{

    out<<"=========================================================="<<endl;
    out<<"=========== Replacement Policy Statistics ================"<<endl;
    out<<"=========================================================="<<endl;

    // CONTESTANTS:  Insert your statistics printing here

    return out;
    
}

int main() {
    parse_input();

    cout << numSets << endl;
    cout << numWays << endl;
    cout << policyIndicator << endl;
    cout << numCounters << endl;
    cout << numOperations << endl;
    CACHE_REPLACEMENT_STATE cs(numSets, numWays, policyIndicator);

    for(int i=0;i<numOperations;i++){
        int hitway = -1;
        if(i==numOperations-1) last_operation = true;
        const auto op = operations[i];
        
        cout << op.serialNumber << " " << op.programCounter << " " << op.accessedSetNumber << " ";
        if(op.hasWayHit)
            hitway = op.wayHit;
        cs.UpdateReplacementState(op.accessedSetNumber,hitway, nullptr, 
                                    op.programCounter, op.programCounter, 0, op.hasWayHit);
        cout << endl;
    }
    return 0;
}