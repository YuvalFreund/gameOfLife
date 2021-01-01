#ifndef __GAME_THREAD_H
#define __GAME_THREAD_H
#include "Headers.hpp"
#include "Thread.hpp"
#include "PCQueue.hpp"

typedef struct life_job{
    int job_TH;
    int job_game_height;
    int job_game_width;
    int job_line_count;
    string ***job_curr;
    string ***job_next;    
}*Life_job;

class Game_Thread : public Thread{

    PCQueue<Life_job> *thread_queue;
	vector<float> *m_tile_hist;
    int *finished;
	pthread_mutex_t *hist_lock;

	//bool is_bound(int i, int j, int x, int y){
    //	return i >= 0 && j >= 0 && i < x && j < y ;
	//}

	int live_neighbors(string **mat, int i, int j, int x, int y){
		// i,j are received indexes. we count the neighbors of this pixel
		int count = 0;

		for(int k = -1;k < 2; k++){
			for (int p = -1 ; p < 2; p++){
				if(k == 0 && p == 0) continue;
				if (i+k >= 0 && j+p >=0 && i+k < x && j+p < y && mat[i+k][p+j] == "1"){
					count++;
				}
			}
		}
		return count;
	}
	public:

	Game_Thread(uint thread_id,PCQueue<Life_job> *queue,vector<float> *m_tile,int *finish,pthread_mutex_t *hist_lock){
		this->thread_queue = queue;
		this->m_tile_hist = m_tile;
        this->finished = finish;
        this->m_thread_id = thread_id;
		this->hist_lock = hist_lock;
	}

	void thread_workload() override{
		int LN;
		while(1){
            Life_job to_do = this->thread_queue->pop();
			if(*(this->finished)) break;
			auto tile_start = std::chrono::system_clock::now();
            for(int i=to_do->job_line_count; i < to_do->job_TH + to_do->job_line_count; i++){
                for(int j = 0; j<to_do->job_game_width; j++ ){
                    LN = live_neighbors(*(to_do->job_curr),i,j,to_do->job_game_height,to_do->job_game_width);
                    if (LN < 2 || LN > 3){
                        (*(to_do->job_next))[i][j] = "0"; //cell dies of overcrowding or loneliness
                    }
                    if ((LN == 2 && (*(to_do->job_curr))[i][j] == "1") || (LN ==3)){
                        (*(to_do->job_next))[i][j] = "1"; // cell survives or born
                    }
                }
            }
			auto tile_end = std::chrono::system_clock::now();
			this->thread_queue->reduce_size();
			pthread_mutex_lock(hist_lock);
			this->m_tile_hist->push_back((float)std::chrono::duration_cast<std::chrono::microseconds>(tile_end - tile_start).count());
			pthread_mutex_unlock(hist_lock);

		}
        
	}
	
};

#endif
