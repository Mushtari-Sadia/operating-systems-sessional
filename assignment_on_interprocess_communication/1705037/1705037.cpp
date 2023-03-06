#include<cstdio>
#include<pthread.h>
#include<semaphore.h>
#include<queue>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <random>
#include <ctime>
#include <chrono>
#include<queue>
#include <math.h>
#include <stdlib.h>

using namespace std;
using std::chrono::steady_clock;

int m,N,P,w,x,y,z;
sem_t empty_kiosk;
sem_t special_kiosk;
sem_t *belts;
pthread_mutex_t mtx;
pthread_mutex_t mtx_boarding;
pthread_mutex_t mtx_vip;
steady_clock::time_point begin_time;
steady_clock::time_point end_time;

queue<int> kiosk_free;
pthread_cond_t cond_lr;
pthread_cond_t cond_rl;
bool vip_direction_left_right = true;
int lrcount=0;
int rcount=0;

class Passenger {
	public :
	string id;
	bool vip;
	bool boarding_pass_lost;
	Passenger(int i,bool vip)
	{
		this->vip = vip;

		id = to_string(i);
		if(vip)
		{
			id += " (VIP)";
		}

		boarding_pass_lost = false;
	}

};

void assignKiosk(Passenger *passenger);
void security_check(Passenger *passenger);
void left_right_vip_channel(Passenger *passenger);
void right_left_vip_channel(Passenger *passenger);
void board_plane(Passenger *passenger);
void process_after_losing_boarding_pass(Passenger *passenger);


void assignKiosk(Passenger *passenger)
{
	int value;

	sem_wait(&empty_kiosk);

	pthread_mutex_lock(&mtx);
	value = kiosk_free.front();
	kiosk_free.pop();
	end_time= steady_clock::now();
	cout << "Passenger " << passenger->id << " has started self-check in at kiosk " << value << " at time " << chrono::duration_cast<std::chrono::seconds>(end_time- begin_time).count() << endl;
	pthread_mutex_unlock(&mtx);

	sleep(w);

	pthread_mutex_lock(&mtx);
	end_time= steady_clock::now();
	cout << "Passenger " << passenger->id << " has finished check in at time " << chrono::duration_cast<std::chrono::seconds>(end_time- begin_time).count() << endl;
	kiosk_free.push(value);
	pthread_mutex_unlock(&mtx);

	sem_post(&empty_kiosk);
}

void security_check(Passenger *passenger)
{

	int value = rand() % N;
	pthread_mutex_lock(&mtx);
	end_time= steady_clock::now();
	cout << "Passenger " << passenger->id << " has started waiting for security check in belt " << value << " from time " << chrono::duration_cast<std::chrono::seconds>(end_time- begin_time).count() << endl;
	pthread_mutex_unlock(&mtx);

	sem_wait(&belts[value]);
	pthread_mutex_lock(&mtx);
	end_time= steady_clock::now();
	cout << "Passenger " << passenger->id << " has started the security check at time " << chrono::duration_cast<std::chrono::seconds>(end_time- begin_time).count() << endl;
	pthread_mutex_unlock(&mtx);
	sleep(x);
	pthread_mutex_lock(&mtx);
	end_time= steady_clock::now();
	cout << "Passenger " << passenger->id << " has crossed the security check at time " << chrono::duration_cast<std::chrono::seconds>(end_time- begin_time).count() << endl;
	pthread_mutex_unlock(&mtx);
	sem_post(&belts[value]);

}


void left_right_vip_channel(Passenger *passenger)
{
	lrcount++;
	pthread_mutex_lock(&mtx_vip);
	if(!vip_direction_left_right)
	{
		pthread_cond_wait(&cond_lr,&mtx_vip);
	}

	pthread_mutex_lock(&mtx);
	end_time= steady_clock::now();
	cout << "Passenger " << passenger->id << " has started crossing vip channel(left-right) at time " << chrono::duration_cast<std::chrono::seconds>(end_time- begin_time).count() << endl;
	pthread_mutex_unlock(&mtx);
	pthread_mutex_unlock(&mtx_vip);
	sleep(z);

	pthread_mutex_lock(&mtx_vip);

	pthread_mutex_lock(&mtx);
	end_time= steady_clock::now();
	cout << "Passenger " << passenger->id << " has crossed the vip channel(left-right) at time " << chrono::duration_cast<std::chrono::seconds>(end_time- begin_time).count() << endl;
	pthread_mutex_unlock(&mtx);

	lrcount--;
	if(lrcount==0)
	{
		pthread_cond_broadcast(&cond_rl);
	}
	

	pthread_mutex_unlock(&mtx_vip);

}

void right_left_vip_channel(Passenger *passenger)
{
	pthread_mutex_lock(&mtx_vip);
	if(lrcount==0)
	{
		pthread_cond_broadcast(&cond_rl);
	}
	else if(lrcount>0)
	{
		pthread_cond_wait(&cond_rl,&mtx_vip);
	}

	vip_direction_left_right = false;

	pthread_mutex_lock(&mtx);
	end_time= steady_clock::now();
	cout << "Passenger " << passenger->id << " has started crossing vip channel(right-left) at time " << chrono::duration_cast<std::chrono::seconds>(end_time- begin_time).count() << endl;
	rcount++;
	pthread_mutex_unlock(&mtx);
	
	pthread_mutex_unlock(&mtx_vip);
	
	sleep(z);

	pthread_mutex_lock(&mtx_vip);

	pthread_mutex_lock(&mtx);
	end_time= steady_clock::now();
	cout << "Passenger " << passenger->id << " has crossed the vip channel(right-left) at time " << chrono::duration_cast<std::chrono::seconds>(end_time- begin_time).count() << endl;
	rcount--;
	pthread_mutex_unlock(&mtx);

	if(rcount==0)
	{
		vip_direction_left_right=true;
		pthread_cond_broadcast(&cond_lr);

	}

	pthread_mutex_unlock(&mtx_vip);

}



void board_plane(Passenger *passenger)
{
	int v = rand() % 6;
	if(v<3)
	{
		passenger->boarding_pass_lost = true;
	}

	pthread_mutex_lock(&mtx);
	end_time= steady_clock::now();
	cout << "Passenger " << passenger->id << " has started waiting to be boarded at time " << chrono::duration_cast<std::chrono::seconds>(end_time- begin_time).count() << endl;
	pthread_mutex_unlock(&mtx);

	pthread_mutex_lock(&mtx_boarding);

	pthread_mutex_lock(&mtx);
	end_time= steady_clock::now();
	cout << "Passenger " << passenger->id << " has started boarding the plane at time " << chrono::duration_cast<std::chrono::seconds>(end_time- begin_time).count() << endl;
	pthread_mutex_unlock(&mtx);

	if(passenger->boarding_pass_lost)
	{
		pthread_mutex_lock(&mtx);
		cout << "Passenger " << passenger->id << " has lost his/her boarding pass "<< endl;
		pthread_mutex_unlock(&mtx);
		pthread_mutex_unlock(&mtx_boarding);
		process_after_losing_boarding_pass(passenger);
		
		return;
	}

	sleep(y);

	pthread_mutex_lock(&mtx);
	end_time= steady_clock::now();
	cout << "Passenger " << passenger->id << " has boarded the plane at time " << chrono::duration_cast<std::chrono::seconds>(end_time- begin_time).count() << endl;
	pthread_mutex_unlock(&mtx);

	pthread_mutex_unlock(&mtx_boarding);

}

void process_after_losing_boarding_pass(Passenger *passenger)
{
	right_left_vip_channel(passenger);

	pthread_mutex_lock(&mtx);
	end_time= steady_clock::now();
	cout << "Passenger " << passenger->id << " has started waiting at special kiosk at time " << chrono::duration_cast<std::chrono::seconds>(end_time- begin_time).count() << endl;
	pthread_mutex_unlock(&mtx);

	sem_wait(&special_kiosk);

	pthread_mutex_lock(&mtx);
	end_time= steady_clock::now();
	cout << "Passenger " << passenger->id << " has started self-check in at special kiosk at time " << chrono::duration_cast<std::chrono::seconds>(end_time- begin_time).count() << endl;
	pthread_mutex_unlock(&mtx);

	sleep(w);
	passenger->boarding_pass_lost = false;

	pthread_mutex_lock(&mtx);
	end_time= steady_clock::now();
	cout << "Passenger " << passenger->id << " has finished check in at special kiosk at time " << chrono::duration_cast<std::chrono::seconds>(end_time- begin_time).count() << endl;
	pthread_mutex_unlock(&mtx);

	sem_post(&special_kiosk);
	left_right_vip_channel(passenger);
	board_plane(passenger);


}


void * passengerProcess(void * passengerObj)
{
	Passenger *passengerObject = static_cast<Passenger *>(passengerObj);
	assignKiosk(passengerObject);

	if(passengerObject->vip)
	{
		left_right_vip_channel(passengerObject);
	}
	else
	{
		security_check(passengerObject);
	}

	board_plane(passengerObject);

	return 0;
}

int main(void)
{
	ofstream out("out.txt");
    streambuf *coutbuf = std::cout.rdbuf(); //save old buf
    cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt
	// random_device rd;
 	// mt19937 rng (rd ()); 

 	// double lamda = 0.04;
 	// exponential_distribution<double> exp(lamda);

	srand(time(NULL));
	ifstream inFile;
	inFile.open("input.txt");
	begin_time = steady_clock::now();

	if (!inFile) {
		cerr << "Unable to open file input.txt";
		exit(1);   // call system to stop
	}

	inFile >> m;
	inFile >> N;
	inFile >> P;

	inFile >> w;
	inFile >> x;
	inFile >> y;
	inFile >> z;

	inFile.close();

	pthread_t passenger;

	sem_init(&empty_kiosk,0,m);

	belts = new sem_t[N];
	for(int i=1;i<=m;i++)
	{
		kiosk_free.push(i);
	}


	for(int i=0;i<N;i++)
	{
		sem_init(&belts[i],0,P);
	}

	sem_init(&special_kiosk,0,1);
	pthread_mutex_init(&mtx,NULL);
	pthread_mutex_init(&mtx_boarding,NULL);
	pthread_mutex_init(&mtx_vip,NULL);
	cond_lr = PTHREAD_COND_INITIALIZER;
	cond_rl = PTHREAD_COND_INITIALIZER;

	int z=0;
	
	while(1)
	{
		default_random_engine generator(time(0));
		poisson_distribution<int> distribution(4.1);
		int p = distribution(generator);


		for(int i=z;i<z+p;i++)
		{
			bool vip = false;

			int v = rand() % 6;
			if(v<3)
			{
				vip = true;
			}
			Passenger *passengerObj = new Passenger(i+1,vip);
			pthread_mutex_lock(&mtx);
			end_time= steady_clock::now();
			cout << "Passenger " << passengerObj->id << " has arrived at the airport at time " << chrono::duration_cast<std::chrono::seconds>(end_time- begin_time).count()  << endl;
			pthread_mutex_unlock(&mtx);

			pthread_create(&passenger,NULL,passengerProcess,(void*)passengerObj);
		}
		z+=p;
		// int sleeptime = (int)exp.operator() (rng);
		pthread_mutex_lock(&mtx);
		// cout << "Next arrival time of passengers: " << sleeptime << endl;
		pthread_mutex_unlock(&mtx);
		sleep(10);
		// sleep(sleeptime);
	}
	return 0;
}
