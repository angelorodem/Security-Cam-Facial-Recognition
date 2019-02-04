#pragma once

#include "imports.h"

class Pessoa {
  private:
    static std::atomic<uint32_t> unique_id_st;
  public:

    Pessoa(uint32_t val, std::string nome_, std::shared_ptr<dlib::matrix<float, 0, 1>> referencia_);

    std::vector<std::shared_ptr<dlib::matrix<float, 0, 1>>> embeds;
    uint32_t temp_cluster_id;
    std::string nome = "";
    std::shared_ptr<dlib::matrix<float, 0, 1>> referencia;
    bool desconhecida = false;
    uint32_t unique_id;

};


