/** 
 * The MIT License (MIT)
 * Copyright (c) <2015> <carriez.md@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
  * 
  */
  
#ifndef INI_HPP
#define INI_HPP

#include <cassert>
#include <map>
#include <list>
#include <stdexcept>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>

namespace INI {

struct Level 
{
  Level() : parent(NULL), depth(0) {}
  Level(Level* p) : parent(p), depth(0) {}

  typedef std::map<std::string, std::string> value_map_t;
  typedef std::map<std::string, Level> section_map_t;
  typedef std::list<value_map_t::const_iterator> values_t;
  typedef std::list<section_map_t::const_iterator> sections_t;
  value_map_t values;
  section_map_t sections;
  values_t ordered_values; // original order in the ini file
  sections_t ordered_sections;
  Level* parent;
  size_t depth;

  const std::string& operator[](const std::string& name) { return values[name]; }
  Level& operator()(const std::string& name) { return sections[name]; }
};

class Parser
{
public:
  Parser(const char* fn);
  Parser(std::istream& f) : f_(&f), ln_(0) { parse(top_); }
  Level& top() { return top_; }
  void dump(std::ostream& s) { dump(s, top(), ""); }

private:
  void dump(std::ostream& s, const Level& l, const std::string& sname);
  void parse(Level& l);
  void parseSLine(std::string& sname, size_t& depth);
  void err(const char* s);

private:
  Level top_;
  std::ifstream f0_;
  std::istream* f_;
  std::string line_;
  size_t ln_;
};

inline void
Parser::err(const char* s)
{
  char buf[256];
  sprintf(buf, "%s on line #%ld", s, ln_);
  throw std::runtime_error(buf);
}

inline std::string trim(const std::string& s)
{
  char p[] = " \t\r\n";
  long sp = 0;
  long ep = s.length() - 1;
  for (; sp <= ep; ++sp)
    if (!strchr(p, s[sp])) break;
  for (; ep >= 0; --ep)
    if (!strchr(p, s[ep])) break;
  return s.substr(sp, ep-sp+1);
}

inline 
std::string value(const std::string& s)
{
    std::string c = "#;";
    std::string v;
    size_t ep = std::string::npos;
    for(size_t i = 0; i < c.size(); i++)
    {
        ep = s.find(c[i]);
        if(ep != std::string::npos)
            break;
    }
    if(ep != std::string::npos)
        v=s.substr(0, ep);
    return v;
}

inline 
Parser::Parser(const char* fn) : f0_(fn), f_(&f0_), ln_(0)
{ 
  if (!f0_) 
    throw std::runtime_error(std::string("failed to open file: ") + fn);

  parse(top_); 
}

inline void 
Parser::parseSLine(std::string& sname, size_t& depth)
{
  depth = 0;
  for (; depth < line_.length(); ++depth)
    if (line_[depth] != '[') break;

  sname = line_.substr(depth, line_.length() - 2*depth);
}

inline void
Parser::parse(Level& l)
{
  while (std::getline(*f_, line_)) {
    ++ln_;
    if (line_[0] == '#' || line_[0] == ';') continue;
    line_ = trim(line_);
    if (line_.empty()) continue;
    if (line_[0] == '[') {
      size_t depth;
      std::string sname;
      parseSLine(sname, depth);
      Level* lp = NULL;
      Level* parent = &l;
      if (depth > l.depth + 1)
        err("section with wrong depth");
      if (l.depth == depth-1)
        lp = &l.sections[sname];
      else {
        lp = l.parent;
        size_t n = l.depth - depth;
        for (size_t i = 0; i < n; ++i) lp = lp->parent;
        parent = lp;
        lp = &lp->sections[sname];
      }
      if (lp->depth != 0)
        err("duplicate section name on the same level");
      if (!lp->parent) {
        lp->depth = depth;
        lp->parent = parent;
      }
      parent->ordered_sections.push_back(parent->sections.find(sname));
      parse(*lp);
    } else {
      size_t n = line_.find('=');
      if (n == std::string::npos)
        err("no '=' found");
      std::string v = value(trim(line_.substr(n+1, line_.length()-n-1)));
      std::pair<Level::value_map_t::const_iterator, bool> res = 
        l.values.insert(std::make_pair(trim(line_.substr(0, n)), 
              v));
      if (!res.second)
        err("duplicated key found");
      l.ordered_values.push_back(res.first);
    }
  }
}

inline void
Parser::dump(std::ostream& s, const Level& l, const std::string& sname)
{
  if (!sname.empty()) s << '\n';
  for (size_t i = 0; i < l.depth; ++i) s << '[';
  if (!sname.empty()) s << sname;
  for (size_t i = 0; i < l.depth; ++i) s << ']';
  if (!sname.empty()) s << std::endl;
  for (Level::values_t::const_iterator it = l.ordered_values.begin(); it != l.ordered_values.end(); ++it)
    s << (*it)->first << '=' << (*it)->second << std::endl;
  for (Level::sections_t::const_iterator it = l.ordered_sections.begin(); it != l.ordered_sections.end(); ++it) {
    assert((*it)->second.depth == l.depth+1);
    dump(s, (*it)->second, (*it)->first);
  }
}

}

#endif // INI_HPP

