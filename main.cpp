#include<iostream>
#include<queue>
#include<condition_variable>
#include<string>
#include<thread>

using namespace std;

#define QUEUE_SIZE 10
#define SAMPLE_TIME 1000

static queue<int> buffer; // QUEUE buffer
static mutex mtx;
static condition_variable produce, consume;

#define DEBUG 
#ifdef DEBUG 
int cnt = 0;
#endif // DEBUG

class Sensor {
public:
	Sensor();
	void hook() { q_push(++cnt); }
	/* concurrent queue*/
	void q_push(int val);
	/*Private member*/
private:
	void GetInput();	
	atomic<bool> running{1}; // to destroy thread later
	int m_val;               // input raw data
	string type;             // sensor type

};

class Consumer {
public:
	Consumer();
	int q_pop();
private:
	atomic<bool> running{1};
	void ExeInput();
};
//-----------------------SENSOR---------------------
Sensor::Sensor() {
    thread Sample(&Sensor::GetInput,this);
    Sample.join();
}
void Sensor::q_push(int val) {
    unique_lock<mutex> lock(mtx); // init lock mtx, release when unstacking this function
    produce.wait(lock);
    if (buffer.size() < QUEUE_SIZE) {
        buffer.push(val);
    } else {
        /* Equal or greater than */
        buffer.pop();
        buffer.push(val);
    }
    consume.notify_one(); // notify new resource ready 
}
void Sensor::GetInput() { 
    while(running) {
        // take sample 
		hook(); // hook function
        cout << "producer: " << cnt << endl;
        /* Take sample of input every SAMPLE_TIME miliseconds*/
        this_thread::sleep_for(chrono::milliseconds(SAMPLE_TIME));
    }
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
		tmp = buffer.back();
		buffer.pop();
	}
	produce.notify_one();
	return tmp;
}
void Consumer::ExeInput() {
	while(running) {
		// take sample 
		cout << "received data" << q_pop() << endl;

		/* Take sample of input every SAMPLE_TIME miliseconds*/
		this_thread::sleep_for(chrono::milliseconds(SAMPLE_TIME));
	}
}
/* MAIN */
int main() {
    Sensor aid1;
    Consumer kien;

    return 0;
}