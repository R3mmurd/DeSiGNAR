/*
  This file is part of Designar.
  Copyright (C) 2017 by Alejandro J. Mujica

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  Any user request of this software, write to 

  Alejandro Mujica

  aledrums@gmail.com
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
