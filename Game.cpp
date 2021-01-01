
#include "Game.hpp"
#include "utils.hpp"
#include <cmath>
#include <vector>

using std::cout;
using std::vector;
using std::string;

Game::Game(game_params par):m_gen_num(par.n_gen),params(par),finished(0){
	//parsing input
    const string& filename = params.filename;
	vector <string> raw_lines = utils::read_lines(filename);
	vector <string> columns_lines = utils::split(raw_lines[0],' ');
    this->game_height = (int)raw_lines.size();
	this->game_width = (int)columns_lines.size();
	this->curr_field = new string*[game_height];
	this->next_field = new string*[game_height];
	for(int i=0;i<game_height;i++){
		this->curr_field[i] = new string[game_width];
		this->next_field[i] = new string[game_width];
	}

	for(int i=0;i<game_height;i++){
		columns_lines = utils::split(raw_lines[i],' ');
		for(int j=0;j<game_width;j++){
			this->curr_field[i][j] = columns_lines[j];
		}
	} 
    //finished parsing. curr has input file values;
    this->m_thread_num = std::min(game_height, (int)params.n_thread);//true number of threads needed
	this->jobs = new Life_job[m_thread_num];
	for(uint i=0;i<m_thread_num;i++){
		this->jobs[i] = new life_job;
	}
	this->print_on = par.print_on;
	this->interactive_on = par.interactive_on;
	//initializing hist_lock mutex
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
		pthread_mutex_init(&hist_lock, &attr);
		pthread_mutexattr_destroy(&attr);
}

Game::~Game()=default;
/*--------------------------------------------------------------------------------
								
--------------------------------------------------------------------------------*/

void Game::run() {
	this->finished = 0;
	_init_game(); // Starts the threads and all other variables you need
	print_board("Initial Board");
	for (uint i = 0; i < this->m_gen_num; ++i) {
		auto gen_start = std::chrono::system_clock::now();
		_step(i); // Iterates a single generation 
		auto gen_end = std::chrono::system_clock::now();
		this->m_gen_hist.push_back((float)std::chrono::duration_cast<std::chrono::microseconds>(gen_end - gen_start).count());
		print_board(NULL);
	} // generation loop
	this->finished = 1;
	for (uint i=0;i<this->m_thread_num;i++){
        this->thread_queue.push(NULL);
    }
	print_board("Final Board");
	for(uint i=0;i<this->m_thread_num;i++){
		this->m_threadpool[i]->join();
	}
	_destroy_game();
}

void Game::_init_game() {
	for (uint i = 0; i < this->m_thread_num; i++) {
		Game_Thread *temp = new Game_Thread(i,&thread_queue,&m_tile_hist,&finished,&hist_lock);
		if(temp->start()){
        	this->m_threadpool.push_back((Thread*)temp);//allocating all the new threads
		}
		else{
			delete temp;
			i--;//try again
		}
    }
		// Testing of your implementation will presume all threads are started here
}

void Game::_step(uint curr_gen) {
    int NTH = this->game_height/this->m_thread_num; //NTH = Normal tile height
    int LTH = this->game_height/this->m_thread_num + this->game_height % this->m_thread_num;//LTH = last tile height
    int line_count=0;
    for (uint i = 0; i < this->m_thread_num; i++ ){
		jobs[i]->job_game_width = this->game_width;
		jobs[i]->job_curr = &this->curr_field;
		jobs[i]->job_game_height = this->game_height;
		jobs[i]->job_next = &this->next_field;
		jobs[i]->job_line_count = line_count;
		jobs[i]->job_TH = NTH;
        if(i<this->m_thread_num-1) {
			this->thread_queue.increase_size();
			this->thread_queue.push(jobs[i]);
			line_count += NTH;
        }
        else{//we are last iteration meaning tile is bigger
			jobs[i]->job_TH = LTH;
			this->thread_queue.increase_size();
			this->thread_queue.push(jobs[i]);	
        }
    }
	while(!this->thread_queue.is_empty());//wait till all writing threads finish running
	// Wait for the workers to finish calculating 
	// Swap pointers between current and next field
	for(int i=0;i<this->game_height;i++){
		for(int j=0;j<this->game_width;j++){
			this->curr_field[i][j] = this->next_field[i][j];
		}
	} 
}

void Game::_destroy_game(){
	// Destroys board and frees all threads and resources
	//destroy threads:
	for (uint i = 0; i < this->m_thread_num; i++){
		delete this->m_threadpool[i];
	}
	//delete matrixs
	for(int i=0;i<this->game_height;i++){
		delete []this->curr_field[i];
		this->curr_field[i] = NULL;
		delete []this->next_field[i];
		this->next_field[i] = NULL;
	}
	delete []this->curr_field;
	delete []this->next_field;
	//delete job array
	for(uint i=0;i < this->m_thread_num;i++){
		delete this->jobs[i];
	}
	delete[] this->jobs;
	// Not implemented in the Game's destructor for testing purposes. 
	// Testing of your implementation will presume all threads are joined here
}

const vector<float> Game::gen_hist() const{// Returns the generation timing histogram  
	return this->m_gen_hist;
}
const vector<float> Game::tile_hist() const{// Returns the tile timing histogram
	return this->m_tile_hist;
} 
uint Game::thread_num() const{//Returns the effective number of running threads = min(thread_num, field_height)
	return this->m_thread_num;
} 

/*--------------------------------------------------------------------------------
								
--------------------------------------------------------------------------------*/



/* Function sketch to use for printing the board. You will need to decide its placement and how exactly 
	to bring in the field's parameters. 

		cout << u8"╔" << string(u8"═") * field_width << u8"╗" << endl;
		for (uint i = 0; i < field_height ++i) {
			cout << u8"║";
			for (uint j = 0; j < field_width; ++j) {
				cout << (field[i][j] ? u8"█" : u8"░");
			}
			cout << u8"║" << endl;
		}
		cout << u8"╚" << string(u8"═") * field_width << u8"╝" << endl;
*/
/*bool is_bound(int i, int j, int x, int y){
    return i >= 0 && j >= 0 && i < x && j < y ;
}*/
