#include "utils/containerutils.hpp"
#include "String/string_converter.hpp"
#include <iostream>
#include <map>

struct A
{
    A(std::vector<std::string> const &v):
     _v(v){}

     std::string toString() const
     {
        boost::format fmt("<type A>: {std::vector<std::string> _v = %s}");
        fmt % KLib::tostr(_v);
        return fmt.str(); 
     }

     bool operator < (A const& rhs) const
     {
         return _v.size() < rhs._v.size();
     }
private:
     std::vector<std::string> _v;
};

int main(void)
{
    std::vector<std::string> vs;
    vs.push_back("hello");
    vs.push_back("world");
    vs.push_back("bye");

    std::cout << KLib::tostr(vs) << std::endl;

    std::vector<std::vector<std::string> > vvs;
    vvs.push_back(vs);
    vvs.push_back(vs);

    std::cout << KLib::tostr(vvs) << std::endl;
    std::map<std::string, std::vector<std::string> > mv;

    mv["test"] = vs;
    mv["tostr"] = vs;

    std::cout << KLib::tostr(mv) << std::endl;

    std::set<A> s;
    s.insert(A(vs));

    std::cout << KLib::tostr(s) << std::endl;
    return 0;
}