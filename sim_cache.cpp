#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <iterator>
#include <vector>
#include <map>
#include <math.h>
#include <iomanip>
#include <bitset>
#include <algorithm>

using namespace std;

int replacement_policy = 0; 	/*0:LRU, 	1:LFU*/
#define LRU  0
#define LFU  1
int write_policy = 0; 		/*0:WBWA, 	1:WTNA*/
#define WBWA 0
#define WTNA 1

// type of request
#define READ 0
#define WRITE 1

//content is valid or not
#define INVALID 0
#define VALID 1

// cache type
#define NORMAL_CACHE 0
#define VICTIM_CACHE 1
/*---------------------Utils---------------*/
string hex_to_binary(string hex) {
  string bin_str = "";
  for (unsigned int i = 0; i < hex.length(); ++i) {
    switch (hex[i]) {
      case '0':
         bin_str.append("0000");
         break;
      case '1':
         bin_str.append("0001");
         break;
      case '2':
         bin_str.append("0010");
         break;
      case '3':
         bin_str.append("0011");
         break;
      case '4':
         bin_str.append("0100");
         break;
      case '5':
         bin_str.append("0101");
         break;
      case '6':
         bin_str.append("0110");
         break;
      case '7':
         bin_str.append("0111");
         break;
      case '8':
         bin_str.append("1000");
         break;
      case '9':
         bin_str.append("1001");
         break;
      case 'a':
         bin_str.append("1010");
         break;
      case 'b':
         bin_str.append("1011");
         break;
      case 'c':
         bin_str.append("1100");
         break;
      case 'd':
         bin_str.append("1101");
         break;
      case 'e':
         bin_str.append("1110");
         break;
      case 'f':
         bin_str.append("1111");
         break;
    }
  }
  return bin_str;
}
string binary_to_hex(string binary) {
  string bin(binary);
  int result = 0;
  for (size_t count = 0; count < bin.length(); ++count) {
    result *= 2;
    result += bin[count] == '1' ? 1 : 0;
  }
  stringstream ss;
  ss << hex << setw(8) << result;
  string hexVal(ss.str());
  return hexVal;
}

string get_tag(string binary_address, int index, int offset) {
  string tag;
  tag = binary_address.substr(0, binary_address.length() - (offset + index)); 
  return tag;
}

int get_index(string binary_address, int index, int offset) {
  int index1 = 0;
  string substr = binary_address.substr(32 - (offset + index), index);
  bitset<32> bs(substr);
  index1 = (int) bs.to_ulong();
  return index1;
}

/*-----------------------------------------*/
class Block {
public:
   int counter_block;
   int counter_set;
   int size;
   int valid;
   int dirty;
   string tag;
};

   
class Cache{
public:
   Cache * victim_cache;
   Cache * next_level;
   int cache_type;
   int size;
   int assoc;
   int block_size;
   int set;
   vector<vector<Block> > cache_content_vec;
   	
	int reads_hits;
	int reads_misses;
	int writes_hits;
	int writes_misses;
	int write_backs;
	int total_memory_traffic;
 	int swap_counter; 
public:
  Cache(int _cache_type, int _size, int _assoc, int _block_size, Cache* _victim_cache, Cache* _next_level) {
    cache_type = _cache_type;
    size = _size;
    assoc = _assoc;
    block_size = _block_size;
    set = size / (assoc * block_size );
	
        next_level = _next_level;	
	victim_cache = _victim_cache;
	
        reads_hits =0;
	reads_misses = 0;
	writes_hits = 0;
	writes_misses = 0;
	write_backs = 0;
	total_memory_traffic = 0;
 	swap_counter = 0; 
    for (int i = 0; i < set; i++) {
        vector<Block> temp_set;
        for (int j = 0; j < assoc; j++) {
			Block* temp_block = new Block();

			if (replacement_policy == LRU)
				temp_block->counter_block = j;

			if (replacement_policy == LFU)
				temp_block->counter_block = 1;

           temp_block->counter_set = 0; //TODO: for LFU?
        temp_block->size = block_size;
        temp_block->valid = INVALID;
        temp_block->dirty = 0;
        temp_block->tag = "";
        temp_set.push_back(*temp_block);
		}
        cache_content_vec.push_back(temp_set);
    }
  }
  /*---------------------------------------------*/
  int getSet(){return set;}
  /*---------------------------------------------*/
  void update_counters(int index, const string& TAG, int i) {
    //cout<<"index: "<<index<<" TAG "<<TAG<<"i"<<i<<"\n";
    //LRU: counters less than this counter are incremented by one
    if (replacement_policy == LRU){
      
      for (int j = 0; j < assoc; j++) {
        if ((cache_content_vec.at(index)[j].tag != TAG)
          && (cache_content_vec.at(index)[j].counter_block < cache_content_vec.at(index)[i].counter_block)) {
          //cout<<"increment counter j"<<j<<"\n";
		  // on a write hit
          cache_content_vec.at(index)[j].counter_block++; // increment LRU counter by one
        }
       }
	   cache_content_vec.at(index)[i].counter_block = 0; // most recently used
    }else if (replacement_policy == LFU){
      cache_content_vec.at(index)[i].counter_block ++; 
    }
  }
  /*----------------------------------------------*/
   int find_victim(int index){
    int victim_block_index = 0;
    int i = 0;
    int candid = cache_content_vec.at(index)[0].counter_block;
	//cout<<"index"<<index<<"candid"<<candid<<"\n";
    if (replacement_policy == LRU){		//LRU: find max
      for (i = 0; i < assoc; i++) {
	     //cout<< "vs "<< cache_content_vec.at(index)[i].counter_block<<"\n";
        if (candid < cache_content_vec.at(index)[i].counter_block ) {
	       //victim found
	       victim_block_index = i;
	       //cout<<"victim block index"<<victim_block_index<<"\n";
	       candid = cache_content_vec.at(index)[i].counter_block;
        }
      }
      for (int j = 0; j < assoc; j++) {
           cache_content_vec.at(index)[j].counter_block++; // increment LRU counter by one
      }
      cache_content_vec.at(index)[victim_block_index].counter_block = 0; // most recently used

    }else if (replacement_policy == LFU){	//LFU: find min
      for (i = 0; i < assoc; i++) {
        if (candid > cache_content_vec.at(index)[i].counter_block ) {
	      //victim found
	       victim_block_index = i;
	       candid = cache_content_vec.at(index)[i].counter_block;
        }
      }
      cache_content_vec.at(index)[0].counter_set = cache_content_vec.at(index)[victim_block_index].counter_block;	
    }		
    return victim_block_index;	 
   }
   /*------------------------------------------------*/
  void allocate(int access_type, int index, const string& TAG, string addr)
  {
	/////////////////////////////// L1 //////////////////////////
        bool swap_flag = false; 
        if (cache_type == NORMAL_CACHE && victim_cache != NULL){                      /* I am L1  and I have a victim cache*/
            int hit_victim_cache = victim_cache->cache_access(READ, addr,false);

	    if (hit_victim_cache == 1) /* swap it with victim cache if L1 victim block is dirty*/
		swap_flag = true;

            if (hit_victim_cache == 0 &&  next_level != NULL){ /* L1 & L2 & victim_cache */
               int hit_l2 = next_level->cache_access(READ, addr, false);
            }
        }

        if (cache_type == NORMAL_CACHE && victim_cache == NULL && next_level != NULL){ /* L1 & L2 only*/
            int hit_l2 = next_level->cache_access(READ, addr,false);
        }
	//////////////////////////////////////////////////////////////
	bool found = false;
	int i;
	int tmp_index =0;
	for (i = 0; i < assoc; i++) {
          if (cache_content_vec.at(index)[i].valid == INVALID) {
		tmp_index = i; 
		int j, tmp = cache_content_vec.at(index)[i].counter_block;
		for(j = i + 1; j < assoc; j++){
			   if (replacement_policy == LRU){ 
			        if(cache_content_vec.at(index)[j].valid == INVALID &&
					  tmp < cache_content_vec.at(index)[j].counter_block ){
					  tmp =  cache_content_vec.at(index)[j].counter_block;
					  tmp_index = j;
				    }
				}
		}
                cache_content_vec.at(index)[tmp_index].valid = VALID;
	        cache_content_vec.at(index)[tmp_index].tag = TAG;
		cache_content_vec.at(index)[tmp_index].dirty = 0;
	        found = true;
	        break;
	     }
        }

	if (found == true){
    	   if(replacement_policy == LFU)
	      cache_content_vec.at(index)[tmp_index].counter_block = (cache_content_vec.at(index)[0].counter_set) +1;
	     if (replacement_policy == LRU)
	       update_counters(index, TAG, tmp_index);
	   if (access_type == WRITE)
                cache_content_vec.at(index)[tmp_index].dirty = 1;
        }

	if (found == false){

	  //find_victim
	  //cout<<"calling find victim\n";
	  int victim_block_index = find_victim(index);
          string victim_addr = get_address( victim_block_index, cache_content_vec.at(index)[victim_block_index].tag);

  	  if (write_policy == WBWA && cache_content_vec.at(index)[victim_block_index].dirty){ //Means WBWA
		write_backs++;
		total_memory_traffic++;
		cache_content_vec.at(index)[victim_block_index].dirty = 0;

              // do a write back
              ///////////////////////////////////////
		if (cache_type == VICTIM_CACHE&& next_level != NULL){ 				/*I am a victim cache*/
                   next_level->cache_access(WRITE, victim_addr, true);
                }
				#if 0
                if (cache_type == NORMAL_CACHE && victim_cache != NULL){ 	/* L1 & victim cache (may have L2) */
                   victim_cache->cache_access(WRITE, victim_addr);
                }
				#endif
                if (cache_type == NORMAL_CACHE && victim_cache == NULL && next_level != NULL){ /* L1 & L2 */
                   next_level->cache_access(WRITE, victim_addr, true);
		}
              ///////////////////////////////////////
          }

          cache_content_vec.at(index)[victim_block_index].tag = TAG;
          cache_content_vec.at(index)[victim_block_index].valid = VALID;


          if(replacement_policy == LFU)
	         cache_content_vec.at(index)[victim_block_index].counter_block = (cache_content_vec.at(index)[0].counter_set) +1;
	  if (replacement_policy == LRU)
	         update_counters(index, TAG, victim_block_index);
	 
	  if (access_type == WRITE)	
		cache_content_vec.at(index)[victim_block_index].dirty = 1;

          if (swap_flag == true && cache_type == NORMAL_CACHE && victim_cache != NULL){   /* I am L1  and I have a victim cache*/
                swap_counter++;
                victim_cache->swap_access(victim_addr, addr);
          }

          if (swap_flag == false && cache_type == NORMAL_CACHE && victim_cache != NULL){ /* I am L1  and I DONT have a victim cache*/
                   victim_cache->cache_access(WRITE, victim_addr, true);
          }
 
       }//end of found == false

}
  string get_address( int index, const string& TAG)
  {
         int index_bits= (int) ceil(log2(set));
         int offset_bits= (int) ceil(log2(block_size));
         /*create the addr for victim*/
         string victim_addr = TAG;

         string tmp_index = std::bitset< 32 >( index ).to_string();
         tmp_index= tmp_index.substr(32 - index_bits, 31);
         victim_addr = victim_addr + tmp_index;

         for (int i=0;i < offset_bits; i++) 
                victim_addr = victim_addr + '0';
         //cout << "index_bits: "<<index_bits<< "offset_bits: "<<offset_bits<<"TAG: "<< TAG<<"index:" <<index<<" victim_addr: "<<victim_addr<<"  tmp_index:  "<<tmp_index<<"\n";
         return victim_addr;

  }
  int swap_access(string victim_addr, string dest_addr)
  {
    //cout<<"swap_access-----------------------------------------";
    int index_bits= (int) ceil(log2(set));
    int offset_bits= (int) ceil(log2(block_size));
	
    int victim_index = get_index(victim_addr, index_bits, offset_bits);
    const string victim_TAG = get_tag(victim_addr, index_bits, offset_bits);

    int dest_index = get_index(dest_addr, index_bits, offset_bits);
    const string dest_TAG = get_tag(dest_addr, index_bits, offset_bits);
	//cout << "set: "<< set<< "block_size: "<<block_size<<"dest_TAG: "<< dest_TAG<<"   victim_TAG: "<< victim_TAG << "\n";
    for (int i = 0; i < assoc; i++) {
      if ((cache_content_vec.at(dest_index)[i].tag == dest_TAG) && cache_content_vec.at(dest_index)[i].valid == VALID) {
    	  /*update_counters(dest_index, dest_TAG, i);*/
	  cache_content_vec.at(dest_index)[i].dirty = 1;
          cache_content_vec.at(dest_index)[i].tag = victim_TAG;

	    //cout <<"HIT at swap_access\n";
	    break;
      }
    }
    return 0;
  }
   /////////////////////////////////////
#if 0
   int read_victim_cache(int access_type, string addr /*, int index, const string& TAG*/) { /*access_type:0 read. access_type:1 write */
    int index_bits= (int) ceil(log2(set));
    int offset_bits= (int) ceil(log2(block_size));
	
    int index = get_index(addr, index_bits, offset_bits);
    const string TAG = get_tag(addr, index_bits, offset_bits);

    int hit = 0;
    int i;
    for (i = 0; i < assoc; i++) {
      if ((cache_content_vec.at(index)[i].tag == TAG) && cache_content_vec.at(index)[i].valid == VALID) {
        /* HIT */
		//cout<<"calling update_counter"<<"\n";
        update_counters(index, TAG, i);
        //end update for the counters

	    if (access_type == READ)
              reads_hits++;
	    if (access_type == WRITE){
	      writes_hits++;
	      if(write_policy == WBWA)
	        cache_content_vec.at(index)[i].dirty = 1;
	      else if (write_policy == WTNA){
	        //propagate to other levels
	      }
	    }
	    //cout <<"HIT\n";
        hit = 1; // set a read hit no read request from lower level cache or Memory is needed
	    break;
      }
    }
#if 0
    /* MISS */
    if (hit == 0) {
	    //cout<<"MISS\n";
        if (access_type == READ) {/*READ miss-----------------------------------*/
	      reads_misses++;
	      total_memory_traffic++;
	      //allocate(access_type, index, TAG, addr);
        }// end of READ Miss
      else { /* WRITE miss----------------------------------------------*/
	      writes_misses++;
          total_memory_traffic++;
	      if (write_policy == WBWA ){
	         //allocate(access_type, index, TAG, addr);
              }else if (write_policy == WTNA){
	        // propagate to next level
          }
      }
	  
    }
#endif
    return hit;
  }
#endif
  /////////////////////////////////////////////  
  int cache_access(int access_type, string addr /*, int index, const string& TAG*/, bool is_write_back) { /*access_type:0 read. access_type:1 write */
    int index_bits= (int) ceil(log2(set));
    int offset_bits= (int) ceil(log2(block_size));
	
    int index = get_index(addr, index_bits, offset_bits);
    const string TAG = get_tag(addr, index_bits, offset_bits);

    int hit = 0;
    int i;
    for (i = 0; i < assoc; i++) {
      if ((cache_content_vec.at(index)[i].tag == TAG) && cache_content_vec.at(index)[i].valid == VALID) {
        /* HIT */
		//cout<<"calling update_counter"<<"\n";
        update_counters(index, TAG, i);
        //end update for the counters

	    if (access_type == READ)
              reads_hits++;
	    if (access_type == WRITE){
	      writes_hits++;
	      if(write_policy == WBWA)
	        cache_content_vec.at(index)[i].dirty = 1;
	      else if (write_policy == WTNA){
	        //propagate to other levels
	      }
	    }
	    //cout <<"HIT\n";
        hit = 1; // set a read hit no read request from lower level cache or Memory is needed
	    break;
      }
    }
    /* MISS */
    if (hit == 0) {
	    //cout<<"MISS\n";
        if (access_type == READ) {/*READ miss-----------------------------------*/
	      reads_misses++;
	      total_memory_traffic++;
	      allocate(access_type, index, TAG, addr);
        }// end of READ Miss
        else { /* WRITE miss----------------------------------------------*/
	      if (is_write_back == false)writes_misses++;
              total_memory_traffic++;
	      if (write_policy == WBWA ){
	         allocate(access_type, index, TAG, addr);
              }else if (write_policy == WTNA){
	        // propagate to next level
              }
        }
	  
    }
    return hit;
  }
    void print(int level) { /*1: L1, 2: L2, 0:victim_cache*/
    for (int i = 0; i < set; i++) {
                cout << setw(4) << left << "Set" << setw(4) << right << i << ":";
		sort(cache_content_vec.at(i).begin(), cache_content_vec.at(i).end(), compare);
                for (int j = 0; j < assoc; j++) {
                        cout << setw(16) << right << binary_to_hex(cache_content_vec.at(i)[j].tag);
                        if (cache_content_vec.at(i).at(j).dirty)
                                cout << setw(2) << right << "D";
                        else
                                cout << setw(2) << right << " ";
                }
                cout << endl;
    }
    double miss_rate =   ( reads_misses + writes_misses + 0.0 ) / 
	    (reads_misses + reads_hits + writes_hits + writes_misses );

    cout << "======== Simulation results (raw) ======" <<"\n";
    cout << /*setw(24) << left <<*/ "a. number of L"<<level<<" reads: " /*<< setw(24) << left */<<reads_misses + reads_hits <<"\n";
    cout << /*setw(24) << left <<*/ "b. number of L"<<level<<" read misses: " /*<<setw(24) << left */<< reads_misses <<"\n";
    cout << /*setw(24) << left <<*/ "c. number of L"<<level<<" writes: " /*<< setw(24) << left */<< writes_misses + writes_hits <<"\n";
    cout << /*setw(24) << left <<*/ "d. number of L"<<level<<" write misses: " /*<< setw(24) << left */<< writes_misses <<"\n";
    cout << /*setw(24) << left <<*/ "e. L"<<level<<" miss rate: " /*<< setw(24) << left */<< miss_rate << "\n";
    if (level == 1)
       cout << /*setw(24) << left <<*/ "f. number of swaps " << /*setw(24) << left <<*/ swap_counter <<"\n";

    if (level == 0) /* this is victim cache*/
       cout << /*setw(24) << left <<*/ "f. number of victim cache writebacks " /*<< setw(24) << left*/ << write_backs <<"\n";
    
    else if (level == 2)
       cout << /*setw(24) << left <<*/ "f. number of L"<<level<<" writebacks: " /*<< setw(24) << left */<< write_backs <<"\n";

	//cout << setw(24) << left << "g. total memory traffic: " << setw(24) << left << total_memory_traffic <<"\n";
    cout << "======================================" << "\n";
    cout << "\n";
    
  }

  static bool compare(const Block& l, const Block& r) {
     return (l.counter_block < r.counter_block);
  }
};

int main(int argc, char ** argv) {
  int block_size;
  int L1_size, L2_size; 
  int L1_assoc, L2_assoc;
  int victim_size;
  if(argc < 8){
	cout << "Usage: ./ [block_size] [L1_size] [L1_assoc][victim_cache_size] [L2_size][L2_assoc][trace_filename]\n";
	exit(-1);
  }

  block_size = atoi(argv[1]);
  L1_size = atoi(argv[2]);
  L1_assoc = atoi(argv[3]);

  victim_size = atoi(argv[4]);

  L2_size = atoi(argv[5]);  
  L2_assoc = atoi(argv[6]);

  string buffer; // for reads
  const char * trace_file = argv[7];
  //read a trace file
  ifstream infile(trace_file);

  if (!infile.is_open()) {
    cout << "Could not open file: " << trace_file << "\n";
    exit(-1);
  }
  Cache *C1, *C2, *Cvictim;

  if (L2_size != 0 && victim_size != 0){ /*L1 & L2 & victim cache*/
    cout <<"L1 & L2 & victim cache\n";
    C2 = new Cache(NORMAL_CACHE, L2_size, L2_assoc, block_size, NULL, NULL);
    Cvictim = new Cache(VICTIM_CACHE, victim_size,victim_size/block_size, block_size, NULL, C2);
    C1 = new Cache(NORMAL_CACHE, L1_size, L1_assoc, block_size, Cvictim, C2);
  }
  else if (L2_size != 0 && victim_size == 0){ /*L1 & L2*/
    cout <<"L1 & L2\n";
    C2 = new Cache(NORMAL_CACHE, L2_size, L2_assoc, block_size, NULL, NULL);
    C1 = new Cache(NORMAL_CACHE, L1_size, L1_assoc, block_size, NULL, C2);
  }
  else if (L2_size == 0 && victim_size != 0){ /*L1 & victim*/
    cout << "L1 & victim\n";
    Cvictim = new Cache(VICTIM_CACHE, victim_size,victim_size/block_size, block_size, NULL, NULL);
    C1 = new Cache(NORMAL_CACHE, L1_size, L1_assoc, block_size, Cvictim, NULL);
  }

  int set = C1->getSet();
  int index_bits= (int) ceil(log2(set));
  int offset_bits= (int) ceil(log2(block_size));

  while (infile.good()) {
    getline(infile, buffer);
    istringstream buf(buffer);
    ostringstream padded_address;
    istream_iterator<string> beg(buf), end;
    vector<string> tokens(beg, end); // done!
    if (buffer != "") {
      string index_str(tokens.at(1));
      padded_address << setw(8) << setfill('0') << index_str;
#if 0
      cout << "ACCESS_TYPE:" << tokens.at(0) << " ";
      cout << "HEX: " << padded_address.str() << " ";
      cout << "Binary: " << hex_to_binary(padded_address.str());
      cout << "\n";
#endif
       int access_type = (tokens.at(0)=="w")?WRITE:READ;  
      C1->cache_access(access_type, hex_to_binary(padded_address.str()), false);	
    }
  }
  infile.close();
  cout << "===== Simulator configuration ===== " << "\n";
  cout << setw(24) << left << "BLOCKSIZE:" << setw(24) << left << block_size<< "\n";
  cout << setw(24) << left << "L1_SIZE:" << setw(24) << left << L1_size << "\n";
  cout << setw(24) << left << "L1_ASSOC:" << setw(24) << left << L1_assoc << "\n";
  cout << setw(24) << left << "L1_REPLACEMENT_POLICY:" << setw(24) << left << replacement_policy << "\n"; 
  cout << setw(24) << left << "L1_WRITE_POLICY:" << setw(24) << left << write_policy << "\n";
  cout << setw(24) << left << "trace_file:" << setw(24) << left << trace_file << "\n";                      
  cout << "=================================== " << "\n";  

  if (L2_size != 0 && victim_size != 0){ /*L1 & L2 & victim cache*/
    cout <<"Cache 1\n";
    C1->print(1);
    cout <<"Cache 2\n";
    C2->print(2);
    //cout<<"Victim Cache\n";
    //Cvictim->print(0); 
  }
  else if (L2_size > 0 && victim_size == 0){ /*L1 & L2*/
    cout <<"Cache 1\n";
    C1->print(1);
    cout <<"Cache 2\n";
    C2->print(2);
  }
 else if (L2_size == 0 && victim_size > 0){ /*L1 & victim*/
    cout <<"Cache 1\n";
    C1->print(1);
    cout<<"Victim Cache\n";
    Cvictim->print(0);
 }
  if (L1_size != 0 && L2_size != 0){
    double miss_rate_l1 =   ( C1->reads_misses + C1->writes_misses + 0.0 ) / 
	    (C1->reads_misses + C1->reads_hits + C1->writes_hits + C1->writes_misses );

    double miss_rate_l2 =   ( C2->reads_misses + C2->writes_misses + 0.0 ) /
            (C2->reads_misses + C2->reads_hits + C2->writes_hits + C2->writes_misses );

    double miss_penalty_2 = 20.0 + 0.5*(block_size / 16.0 );
    
    double hit_time_l1 = 0.25 + 2.5 * (L1_size / (512.0*1024.0)) + 0.025 * (block_size / 16.0) + 0.025 * L1_assoc;
    double hit_time_l2 = 2.5 + 2.5 * (L2_size / (512.0*1024.0)) + 0.025 * (block_size / 16.0) + 0.025 * L2_assoc;

    double miss_penalty = hit_time_l2 + miss_rate_l2 * miss_penalty_2;

    double averageAccessTime = hit_time_l1 + (miss_rate_l1 * miss_penalty);
   
    cout <<"==== Simulation results (performance) ===="<<"\n";
    cout << setw(24) << left << "1. average access time:" << setw(24) << left << averageAccessTime <<" ns\n";
  }
  return 0;
}
