/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# include <iostream>

using namespace std;

# include <queue.H>

using namespace Designar;

void send(ConcurrentQueue<string> & q, nat_t num_msgs, nat_t ms)
{
  for (int i = 0; i < num_msgs; ++i)
    {
      this_thread::sleep_for(chrono::milliseconds(ms));
      stringstream s;
      s << "Msg " << i + 1;
      q.put(s.str());
    }
}

void receive(ConcurrentQueue<string> & q, nat_t num_msgs)
{
  for (int i = 0; i < num_msgs; ++i)
    cout << q.get() << endl;
}

int main()
{
  ConcurrentQueue<string> msg_queue;

  nat_t n = 100;
  nat_t d = 1000;
  
  thread t1{send, ref(msg_queue), n, d};
  thread t2{receive, ref(msg_queue), n};

  t1.join();
  t2.join();
  
  return 0;
}
