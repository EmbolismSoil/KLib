#include "algorithm/LDA.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <set>
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
    if (argc != 3)
    {
	std::cout << "./LDA <path_to_data> <stopwords>" << std::endl;
        return -1;
    }

    std::ifstream f(argv[1]);
    std::ifstream sf(argv[2]);

    std::string line;
    std::vector<std::vector<std::string> > docs;
    uint64_t n = 0;

    std::set<std::string> stopwords;
    while(std::getline(sf, line))
    {
        stopwords.insert(line);
    }

    while(std::getline(f, line))
    {
        std::vector<std::string> doc;
        std::vector<std::string> filteredDoc;

        doc = split(line, ' ');
        for (auto const& w : doc)
        {
            if (stopwords.find(w) != stopwords.end()){
                continue;
            }else{
                filteredDoc.push_back(w);
            }
        }
        n += filteredDoc.size();
        docs.push_back(filteredDoc);
    }


    LDAModel lda(docs, std::vector<double>(50, 0.01), std::vector<double>(n, 5.0), 20, 200);
    lda.run();
    
    for(uint64_t i = 0; i < 50; ++i)
    {
        std::vector<std::string> words;
        lda.getTopKWords(i, 20, words);
        std::cout << "topic " << i << ": ";
        for (auto const& w : words){
            std::cout << w << ", ";
        }
        std::cout << std::endl;
    }

    lda.save("model.lda");
    return 0;
}
