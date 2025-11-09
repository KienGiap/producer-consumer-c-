#include<iostream>
#include<queue>
#include<condition_variable>
#include<string>
#include<thread>
#include<cstdlib>
#include<ctime>

using namespace std;

#define QUEUE_SIZE 10
#define PRODUCER_SAMPLE_TIME 1000 // ms
#define CONSUMER_SAMPLE_TIME 100  // ms
#define CONSUMER_NUM_THREAD  3		// number of consumer thread

static queue<int> buffer; // QUEUE buffer
static mutex mtx;
static condition_variable produce, consume;

typedef enum {
	SENSOR_ERR_NO_DATA,
	SENSOR_READ_SUCCESS,
	MAX_ERROR_TYPE, // put this end of ENUM
} ERROR_RET_TYPE;
typedef struct {
	ERROR_RET_TYPE (*sensor_read_cb)(int*);
	string sensor_name;
} sensor_data;

#define DEBUG 
#ifdef DEBUG 
int cnt = 0;
/* @brief Simulate sensor value*/
ERROR_RET_TYPE CreateRandom(int *val) {
	
	*val = rand() % 4096; // create random number 0 - 4096
	return SENSOR_READ_SUCCESS;
}
#endif // DEBUG
/* =======================DECLARATION======================*/
class Sensor {
public:
	Sensor();
	Sensor(ERROR_RET_TYPE (*cb)(int*),string name);
	/* concurrent queue*/
	void q_push(int val);
	string GetName();
	/*Private member*/
private:
	ERROR_RET_TYPE (*sensor_read_cb)(int*);	// register callback from user
	void GetInput();	
	bool running = true; // to destroy thread later
	string name;             // sensor type
};
class Consumer {
public:
	Consumer();
	int q_pop();
private:
	bool running  = true;
	void ExeInput();
};
/* =======================IMPLEMENTATION======================*/
//-----------------------SENSOR---------------------
Sensor::Sensor() {
	// Currently reserved due to information of input data
	// Do nothing
	cout << "Reserved, Initialize with construct with parameter return type 'int' "<< endl;
}
Sensor::Sensor(ERROR_RET_TYPE (*cb)(int*),string name){
	sensor_read_cb = cb;
	this->name = name;
    thread Sample(&Sensor::GetInput,this);
    Sample.detach();
}
void Sensor::q_push(int val) {
    unique_lock<mutex> lock(mtx); // init lock mtx, release when unstacking this function
	produce.wait(lock, [] { return buffer.size() < QUEUE_SIZE; } );
    if (buffer.size() < QUEUE_SIZE) {
        buffer.push(val);
    } else {
        /* Equal or greater than */
		cout << "QUEUE FULL, POP THEN PUSH !!" << endl;
        buffer.pop();
        buffer.push(val);
    }
	cout << this->name << " " << "producer: " << val << endl;
    consume.notify_one(); // notify new resource ready 
}
void Sensor::GetInput() { 
	ERROR_RET_TYPE stat;
	int m_val;
    while(running) {
        // sensor had value, push to queue & notify consumer
		if (SENSOR_READ_SUCCESS == sensor_read_cb(&m_val)) {
			q_push(m_val);
		}
		this_thread::sleep_for(chrono::milliseconds(PRODUCER_SAMPLE_TIME));
        /* Take sample of input every PRODUCER_SAMPLE_TIME miliseconds*/
    }
}
string Sensor::GetName(){
	cout << "Sensor name:" << this->name << endl;
	return this->name;
}
//-----------------------CONSUMER---------------------
Consumer::Consumer() {
	thread Sample(&Consumer::ExeInput,this);
	Sample.detach();
}
int Consumer::q_pop() {
	unique_lock<mutex> lock(mtx); // init lock mtx, release when unstacking this function
	 consume.wait(lock);
	int tmp = 0;
	if (!buffer.empty())  {
		tmp = buffer.front();
		buffer.pop();
		cout << "   consumer data: " << tmp << endl;
	} else {
		cout << "QUEUE EMPTY !!" << endl;
	}
	produce.notify_one();
	return tmp;
}

void Consumer::ExeInput() {
	while(running) {
		// take sample 
		q_pop();
		// cout << "   consumer data: " << q_pop() << endl;
		/* Take sample of input every CONSUMER_SAMPLE_TIME miliseconds*/
		this_thread::sleep_for(chrono::milliseconds(CONSUMER_SAMPLE_TIME));
	}
}
sensor_data t[] = {
	{&CreateRandom, "aid1"},
	{&CreateRandom, "aid2"},
	{&CreateRandom, "aid3"},
	{&CreateRandom, "aid4"},
	{&CreateRandom, "aid5"},
	{&CreateRandom, "aid6"},
	{&CreateRandom, "aid7"},
	{&CreateRandom, "aid8"},
	{&CreateRandom, "aid9"},
	{&CreateRandom, "aid10"},
};

/* MAIN */
int main() {
	srand(time(0));
	vector<unique_ptr<Sensor>>	input_sensor;

	for (int i = 0; i < sizeof(t)/sizeof(sensor_data); i++) {
    	input_sensor.emplace_back(make_unique<Sensor>(t[i].sensor_read_cb, t[i].sensor_name));
	}

	Consumer kien[CONSUMER_NUM_THREAD];

	while(1) { this_thread::sleep_for(chrono::seconds(1000));}; // Sleep forever
    return 0;
}