#include "ini.hpp"

#include <sstream>

void test1();
void test2();
void test3();
void test4();
void test5();

int main()
{
  test1();
  test2();
  test3();
  test4();
  test5();

  return 0;
}

void test1()
{
  std::stringstream ss;

  ss << 
    "a=1\n"
    "b=1\n\n"
    "[e]\n"
    "ea=1\n"
    "eb=1\n\n"
    "[c]\n"
    "ca=2\n"
    "cb=2\n\n"
    "[[d]]\n"
    "da=3\n"
    "db=3\n\n"
    "[A]\n"
    "Aa=4\n"
    "Ab=4\n";

  INI::Parser p(ss);
  std::stringstream out;
  p.dump(out);
  assert(out.str() == ss.str());

  assert(p.top()["a"]=="1");
  assert(p.top()("e")["ea"]=="1");
}

void test2()
{
  std::stringstream ss;

  ss << 
    "a=1\n"
    "a=1\n\n";

  try {
    INI::Parser p(ss);
  } catch (std::runtime_error& e) {
    assert(std::string(e.what()) == "duplicated key found on line #2");
  }
}

void test3()
{
  std::stringstream ss;

  ss << 
    "a=1\n"
    "b\n";

  try {
    INI::Parser p(ss);
  } catch (std::runtime_error& e) {
    assert(std::string(e.what()) == "no '=' found on line #2");
  }
}

void test4()
{
  std::stringstream ss;

  ss << 
    "a=1\n"
    "[b]\n"
    "[[[a]]]\n";

  try {
    INI::Parser p(ss);
  } catch (std::runtime_error& e) {
    assert(std::string(e.what()) == "section with wrong depth on line #3");
  }
}

void test5()
{
  std::stringstream ss;

  ss << 
    "a=1\n"
    "[b]\n"
    "[b]\n";

  try {
    INI::Parser p(ss);
  } catch (std::runtime_error& e) {
    assert(std::string(e.what()) == "duplicate section name on the same level on line #3");
  }
}

