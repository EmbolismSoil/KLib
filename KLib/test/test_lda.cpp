#include "LDA.h"
#include <fstream>
#include <iostream>
#include <sstream>
using namespace LDA;

std::vector<std::string> split(const std::string &s, char delim) 
{
  std::stringstream ss(s);
  std::string item;
  std::vector<std::string> elems;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }

  return elems;
}

int main(int argc, const char* argv[])
{
    if (argc != 2)
    {
	std::cout << "./LDA <path_to_data>" << std::endl;
        return -1;
    }

    std::ifstream f(argv[1]);
    std::string line;
    
    std::vector<std::vector<std::string> > docs;
    uint64_t n = 0;
    while(std::getline(f, line))
    {
        std::vector<std::string> doc;
        doc = split(line, ' ');
        n += doc.size();
        docs.push_back(doc);
    }
    
    LDAModel lda(docs, std::vector<double>(20, 0.01), std::vector<double>(n, 5.0), 10, 1000);
    lda.run();
    
    for(uint64_t i = 0; i < 20; ++i)
    {
        std::vector<std::string> words;
        lda.getTopKWords(i, 5, words);
        std::cout << "topic " << i << ": ";
        for (auto const& w : words){
            std::cout << w << ", ";
        }
        std::cout << std::endl;
    }
    return 0;
}
