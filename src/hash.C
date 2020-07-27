/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# include <hash.H>

namespace Designar
{

  nat_t super_fast_hash(void * key, nat_t len)
  {
    const unsigned char * data = reinterpret_cast<unsigned char *>(key);
    
    nat_t hash = len, tmp;
    
    int rem;
    
    if (len <= 0 or data == nullptr)
      return 0;
    
    rem = len & 3;
    len >>= 2;
    
    for ( ; len > 0; --len)
      {
	hash += get16bits(data);
	tmp   = (get16bits(data + 2) << 11) ^ hash;
	hash  = (hash << 16) ^ tmp;
	data += 2 * sizeof(uint16_t);
	hash += hash >> 11;
      }
    
    switch (rem) 
      {
      case 3: 
	hash += get16bits (data);
	hash ^= hash << 16;
	hash ^= ((signed char)data[sizeof (uint16_t)]) << 18;
	hash += hash >> 11;
	break;
      case 2: 
	hash += get16bits (data);
	hash ^= hash << 11;
	hash += hash >> 17;
	break;
      case 1: 
	hash += (signed char)*data;
	hash ^= hash << 10;
	hash += hash >> 1;
      }
    
    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;
    
    return hash;
  }

} // end namespace Designar
