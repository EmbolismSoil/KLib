
#include <memory>
#include "algorithm/LDA.h"
#include <fstream>
#include <iostream>
#include <sstream>

using namespace LDA;

int main(int argc, const char* argv[])
{
    if (argc != 2){
        std::cout << argv[0] << " <path to model file>" << std::endl;
        return -1;
    }

    auto lda = LDAModel::load(argv[1]);

    for(uint64_t i = 0; i < 50; ++i)
    {
        std::vector<std::string> words;
        lda->getTopKWords(i, 5, words);
        std::cout << "topic " << i << ": ";
        for (auto const& w : words){
            std::cout << w << ", ";
        }
        std::cout << std::endl;
    }
    
    return 0; 
}
