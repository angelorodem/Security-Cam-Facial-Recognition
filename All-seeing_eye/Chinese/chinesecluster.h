#pragma once

#include "../imports.h"
#include "../seb/Seb.h"
#include "pessoa.h"
#include "ball.h"

#include <QObject>
#include <QTimer>



class ChineseCluster {
  private:
    void processa_dados();
  public:

    struct Analise {
        Analise(float dist, std::shared_ptr<Pessoa> pess) : distancia(dist),
            pessoa(pess) {}
        float distancia;
        std::shared_ptr<Pessoa> pessoa;
    };

    typedef double FT;
    typedef Seb::Point<FT> Point;
    typedef std::vector<Point> PointVector;
    typedef Seb::Smallest_enclosing_ball<FT> Miniball;

    ChineseCluster();

    std::vector<std::shared_ptr<dlib::matrix<float, 0, 1>>> all_data;
    std::vector<std::shared_ptr<dlib::matrix<float, 0, 1>>> unknown_data;

    std::vector<Ball> balls;
    std::vector<std::shared_ptr<Pessoa>> pessoas;
    std::vector<std::shared_ptr<Pessoa>> pessoas_adicionar;

    std::mutex swap_mutex;
    std::mutex pessoa_mutex;
    std::mutex unk_mutex;

    void adicionar_pessoa(std::shared_ptr<dlib::matrix<float, 0, 1>> referencia,
                          std::string nome);

    std::shared_ptr<std::vector<ChineseCluster::Analise>> analisa_dados(
                std::vector<dlib::matrix<float, 0, 1>> &novos);

    void popula_all_data();
    void cria_grafo(std::vector<dlib::sample_pair> &pares);
    uint64_t clusterisa(std::vector<dlib::sample_pair> &pares,
                        std::vector<uint64_t> &labels);
    void atribui_amostra_cluster(std::vector<uint64_t> &labels);
    void adiciona_desconhecidos();
    void agrupa_amostras_em_pessoas(std::vector<uint64_t> &labels);
    void randomiza_ordem_encodings();
    void cria_pessoa_para_desconhecido(std::vector<uint64_t> &labels);
    void encontra_bounding_ball();

    std::shared_ptr<std::thread> update_loop;

    [[noreturn]] void run_processa();
};

