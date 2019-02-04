#include "pessoa.h"


std::atomic<uint32_t> Pessoa::unique_id_st = {0};

Pessoa::Pessoa(uint32_t val, std::string nome_,
               std::shared_ptr<dlib::matrix<float, 0, 1>> referencia_) {
    nome = nome_;
    temp_cluster_id = val;
    referencia = referencia_;
    unique_id = unique_id_st.load();
    unique_id_st.store(unique_id_st.load() + 1);
}
