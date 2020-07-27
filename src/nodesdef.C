/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# include <nodesdef.H>

namespace Designar
{
  
  void DL::split(DL & l, DL & r)
  {
    assert(l.is_empty());
    assert(r.is_empty());
    
    while (not this->is_empty())
      {
	l.insert_prev(this->remove_next());
	
	if (not this->is_empty())
	  r.insert_next(this->remove_prev());
      }
  }

} // end namespace Designar
