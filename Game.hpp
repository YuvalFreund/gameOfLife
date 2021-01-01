#ifndef __GAMERUN_H
#define __GAMERUN_H
#include "Thread.hpp"
#include "PCQueue.hpp"
#include "Game_Thread.hpp"
/*--------------------------------------------------------------------------------
								  Auxiliary Structures
--------------------------------------------------------------------------------*/
struct game_params {
	// All here are derived from ARGV, the program's input parameters. 
	uint n_gen;
	uint n_thread;
	string filename;
	bool interactive_on; 
	bool print_on; 
};
/*--------------------------------------------------------------------------------
									Class Declaration
--------------------------------------------------------------------------------*/
class Game {

public:

	Game(game_params par);
	~Game();
	void run(); // Runs the game
	const vector<float> gen_hist() const; // Returns the generation timing histogram  
	const vector<float> tile_hist() const; // Returns the tile timing histogram
	uint thread_num() const; //Returns the effective number of running threads = min(thread_num, field_height)


protected: // All members here are protected, instead of private for testing purposes

	// See Game.cpp for details on these three functions
	void _init_game(); 
	void _step(uint curr_gen); 
	void _destroy_game(); 

	uint m_gen_num; 			 // The number of generations to run
	uint m_thread_num; 			 // Effective number of threads = min(thread_num, field_height)
	vector<float> m_tile_hist; 	 // Shared Timing history for tiles: First m_gen_num cells are the calculation durations for tiles in generation 1 and so on. 
							   	 // Note: In your implementation, all m_thread_num threads must write to this structure. 
	vector<float> m_gen_hist;  	 // Timing history for generations: x=m_gen_hist[t] iff generation t was calculated in x microseconds
	vector<Thread*> m_threadpool; // A storage container for your threads. This acts as the threadpool. 

	bool interactive_on; // Controls interactive mode - that means, prints the board as an animation instead of a simple dump to STDOUT 
	bool print_on; // Allows the printing of the board. Turn this off when you are checking performance (Dry 3, last question)
	string **curr_field;//holds current matrix
    string **next_field;//holds next matrix
    int game_height;
	int game_width;
    PCQueue<Life_job> thread_queue;//holds Life_jobs to execute
	Life_job *jobs;//holds jobs to insert the queue
	game_params params;
	int finished;//checks if we finished playing
	pthread_mutex_t hist_lock;

	void print_board(const char* header) {
		if(print_on){ 

			// Clear the screen, to create a running animation 
			if(interactive_on)
				system("clear");

			// Print small header if needed
			if (header != NULL)
				cout << "<------------" << header << "------------>" << endl;
			
			cout << u8"╔";
			for(int i=0;i<game_width;i++){
				cout << u8"═";
			}
			cout << u8"╗" << endl;
			for (int i = 0; i < game_height; ++i) {
				cout << u8"║";
				for (int j = 0; j < game_width; ++j) {
					cout << (curr_field[i][j]=="1" ? u8"█" : u8"░");
				}
				cout << u8"║" << endl;
			}
			cout << u8"╚";
			for(int i=0;i<game_width;i++){
				cout << u8"═";
			}
			cout << u8"╝" << endl; 

			// Display for GEN_SLEEP_USEC micro-seconds on screen 
			if(interactive_on)
				usleep(GEN_SLEEP_USEC);
		}
	}
};
#endif
