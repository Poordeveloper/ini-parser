.ini parser
==========

An .ini parser in C++ for parsing ini configuration files. All in one header file and support multiple
levels.

example:

```cpp

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
```

Please check INI::Parser::dump function for iteration in original order.

## License

Distributed under the [MIT](https://tldrlegal.com/license/mit-license) license.
