//
// Created by lee on 18-7-15.
//

#ifndef LDA_LDA_H
#define LDA_LDA_H

#include <stdint.h>
#include <vector>
#include <string>
#include <memory.h>
#include <unordered_map>
#include <numeric>
#include <algorithm>
#include <random>
#include <set>

namespace LDA{
    class LDAModel {
    public:
        LDAModel(std::vector<std::vector<std::string> > const& docs,
                 std::vector<double> const& betas, std::vector<double> const& alphas,
                 uint64_t K, uint64_t const max_iter):

            _betas(betas),
            _alphas(alphas),
            _max_iters(max_iter),
            _K(K),
            _words_num_of_topics(K, 0),
            _topic_word_distribution(K)
        {
            _build_vocabulary(docs);
            _build_docs(docs);
            _build_paramters();
        }

        void run()
        {
            for (uint64_t iter = 0; iter < _max_iters; ++iter){
                for (uint64_t m = 0; m < _docs.size(); ++m){
                    for (uint64_t t = 0; t < _docs[m].size(); ++t){
                        _do_sample(m, t);
                    }
                }
            }

            _build_topic_word_distribution();
        }

        void getTopKWords(uint64_t const topic, uint64_t const k, std::vector<std::string> & words)
        {
            std::vector<std::pair<uint64_t, double> > const& t = _topic_word_distribution[topic];
            for (uint64_t i = 0; i < k && i < t.size(); ++i)
            {
                std::pair<uint64_t, double> const& item = t[i];
                std::string const& w = _vocabulary[item.first];
                words.push_back(w);
            }
        }

        std::vector<std::vector<uint64_t> > const& getZ()
        {
            return _Z;
        }

        std::vector<std::string> const& getVocabulary()
        {
            return _vocabulary;
        }

        std::vector<std::vector<uint64_t> > const& getDocs()
        {
            return _docs;
        }

    private:
        std::vector<double> const _betas;
        std::vector<double> const _alphas;
        std::vector<std::vector<uint64_t > > _docs;
        std::vector<std::vector<uint64_t > > _docs_z;
        std::vector<uint64_t > _docs_v_num;

        uint64_t const _max_iters;
        uint64_t const _K;
        uint64_t _betas_sum;
        uint64_t _alphas_sum;

        std::unordered_map<std::string, uint64_t > _vocabulary_map;
        std::vector<std::string> _vocabulary;

        uint64_t _nv; //total numbers of _vocabulary
        std::vector<std::vector<uint64_t > > _Z;//topics

        std::vector<std::vector<uint64_t > > _word_topic_distribution; //mat(_nv, _K)
        std::vector<uint64_t > _words_num_of_topics;
        std::vector<std::vector<std::pair<uint64_t, double> > > _topic_word_distribution;

        void _build_topic_word_distribution()
        {
           for(uint64_t w = 0; w < _word_topic_distribution.size(); ++w)
           {
                std::vector<uint64_t> const& topics = _word_topic_distribution[w];
                uint64_t sum = std::accumulate(topics.begin(), topics.end(), uint64_t(0));                
                std::vector<double> dis;
                std::transform(topics.begin(), topics.end(), std::back_inserter(dis), [sum](uint64_t const& n){return double(n)/double(sum);});
                for(uint64_t i = 0; i < _K; ++i){
                    _topic_word_distribution[i].push_back(std::make_pair(w, dis[i]));
                }
           }


           for(uint64_t i = 0; i < _K; ++i)
           {
               std::sort(_topic_word_distribution[i].begin(), _topic_word_distribution[i].end(), 
                               [this](std::pair<uint64_t, double> const& lhs, std::pair<uint64_t, double> const& rhs){return lhs.second > rhs.second;});
           }
        }       

        void _build_vocabulary(std::vector<std::vector<std::string> >  const& docs)
        {
            std::set<std::string> vocabulary;
            for (auto const& doc : docs){
                for (auto const& word : doc){
                    vocabulary.insert(word);
                }
            }

            uint64_t idx = 0;
            for(auto const& word : vocabulary){
                _vocabulary_map[word] = idx++;
                _vocabulary.push_back(word);
            }

            _nv = vocabulary.size();
        }

        void _build_docs(std::vector<std::vector<std::string> >  const& docs)
        {
            std::vector<double> p(_K, 1.0/_K);

            for(auto const& doc: docs){
                std::vector<uint64_t > normalized_doc;
                std::vector<uint64_t > word_topics;
                std::transform(doc.begin(), doc.end(), std::back_inserter(normalized_doc), [this](std::string const& w){return _vocabulary_map[w];});
                std::transform(doc.begin(), doc.end(), std::back_inserter(word_topics), [this, p](std::string const& w){return _multinomial(p);});
                _docs.push_back(normalized_doc);
                _Z.push_back(word_topics);
            }
        }

        void _build_paramters()
        {
            std::vector<uint64_t > tmp(_K, 0);
            std::vector<std::vector<uint64_t > > word_topic_distribution(_nv, tmp);
            _word_topic_distribution.swap(word_topic_distribution);

            for  (std::vector<std::vector<uint64_t > >::size_type i = 0; i < _docs.size(); ++i)
            {
                std::vector<uint64_t > doc_z(_K, 0);
                for (std::vector<uint64_t >::size_type j = 0; j < _docs[i].size(); ++j)
                {
                    uint64_t word = _docs[i][j];
                    uint64_t topic = _Z[i][j];
                    doc_z[topic] += 1;
                    _word_topic_distribution[word][topic] += 1;
                    _words_num_of_topics[topic] += 1;
                }

                _docs_z.push_back(doc_z);
                _docs_v_num.push_back(_docs[i].size());

                _betas_sum = std::accumulate(_betas.begin(), _betas.end(), 0UL);
                _alphas_sum = std::accumulate(_alphas.begin(), _alphas.end(), 0UL);
            }
        }

        void _do_sample(uint64_t m, uint64_t t)
        {
            std::vector<double> p_z(_K, 0);

            uint64_t topic = _Z[m][t];
            uint64_t  word = _docs[m][t];

            _word_topic_distribution[word][topic] -= 1;
            _words_num_of_topics[topic] -= 1;
            _docs_z[m][topic] -= 1;
            _docs_v_num[m] -= 1;

            double p_z_sum = 0.0;
            for (std::vector<double >::size_type k = 0; k < _K; ++k){
                double p_z_w = (_word_topic_distribution[word][k] + _alphas[word]) / (_words_num_of_topics[k] + _alphas_sum);
                double p_doc_z = (_docs_z[m][k] + _betas[k]) / (_docs_v_num[m] + _betas_sum);
                double p = p_z_w * p_doc_z;
                p_z_sum += p;
                p_z[k] = p;
            }

            std::transform(p_z.begin(), p_z.end(), p_z.begin(), [p_z_sum](double const& p){return p / p_z_sum;});

            topic = _multinomial(p_z);
            _word_topic_distribution[word][topic] += 1;
            _words_num_of_topics[topic] += 1;
            _docs_z[m][topic] += 1;
            _docs_v_num[m] += 1;
        }

        uint64_t _multinomial(std::vector<double> const& p){
            std::random_device rd;  // 将用于为随机数引擎获得种子
            std::mt19937 gen(rd());
            std::uniform_real_distribution<double> dis(0, 1.0);

            double n = dis(gen);
            double sum = 0.0;

            for (std::vector<double>::size_type i = 0; i < p.size(); ++i)
            {
                sum += p[i];
                if (n <= sum)
                {
                    return i;
                }
            }
        }
    };
}


#endif //LDA_LDA_H
