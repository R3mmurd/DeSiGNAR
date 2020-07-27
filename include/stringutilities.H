/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# ifndef DSGSTRINGUTILITIES_H
# define DSGSTRINGUTILITIES_H

# include <types.H>

namespace Designar
{
  template <typename T> class DynArray;

  std::string q(const std::string & s, const std::string & b)
  {
    return b + s + b;
  }

  std::string sq(const std::string & s)
  {
    return q(s, "'");
  }

  std::string dq(const std::string & s)
  {
    return q(s, "\"");
  }
  
  template <typename ContainerType = DynArray<std::string>>
  ContainerType split_string(const std::string &, const std::string &);

  template <typename ContainerType = DynArray<std::string>>
  ContainerType split_string_re(const std::string &, const std::regex &);

  template <typename ContainerType = DynArray<std::string>>
  std::string join_string(const std::string &, const ContainerType &);
  
  template <typename ContainerType>
  ContainerType split_string(const std::string & str, const std::string & sep)
  {
    ContainerType ss;

    if (str.empty())
      return ss;

    size_t sep_sz = sep.size();

    size_t beg = 0;
    size_t end = str.find(sep);

    while (end != std::string::npos)
      {
	ss.append(str.substr(beg, end - beg));
	beg = end + sep_sz;
	end = str.find(sep, beg);
      }

    ss.append(str.substr(beg, str.size() - beg));

    return ss;
  }

  template <typename ContainerType>
  ContainerType split_string_re(const std::string & str,
				const std::regex & pattern)
  {
    ContainerType ss;

    if (str.empty())
      return ss;

    std::string s = str;
    std::smatch m;
    
    while (regex_search(s, m, pattern))
      {
	ss.append(m.prefix().str());
	s = m.suffix().str();
      }

    ss.append(s);
    
    return ss;
  }
    
  template <typename ContainerType>
  std::string join_string(const std::string & sep, const ContainerType & ss)
  {
    std::string ret_val = "";
    
    if (ss.is_empty())
      return ret_val;

    auto it = ss.begin();
    ret_val = it.get_current();
    it.next();

    for ( ; it != ss.end(); ++it)
      ret_val += sep + *it;

    return ret_val;
  }
  
} // end namespace Designar

# endif // DSGSTRINGUTILITIES_H
