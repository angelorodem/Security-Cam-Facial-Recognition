#include "chinesecluster.h"


ChineseCluster::ChineseCluster() {
    update_loop = std::make_shared<std::thread>(&ChineseCluster::run_processa,
                  this);
}


void ChineseCluster::run_processa() {
    while (1) {
        QTime dieTime = QTime::currentTime().addSecs(1);

        while (QTime::currentTime() < dieTime) {
            QApplication::processEvents(QEventLoop::AllEvents, 50);
        }

        processa_dados();
    }
}

void ChineseCluster::processa_dados() {
    if (unknown_data.empty() && pessoas_adicionar.empty()) {
        return;
    }

    std::unique_lock<std::mutex> lk_pm(pessoa_mutex);
    popula_all_data();
    std::vector<dlib::sample_pair> pares;
    cria_grafo(pares);
    std::vector<uint64_t> labels;
    clusterisa(pares, labels);
    atribui_amostra_cluster(labels);
    adiciona_desconhecidos();
    agrupa_amostras_em_pessoas(labels);
    randomiza_ordem_encodings();
    cria_pessoa_para_desconhecido(labels);
    encontra_bounding_ball();
    all_data.shrink_to_fit();
    unknown_data.shrink_to_fit();
    pessoas_adicionar.shrink_to_fit();
}


std::shared_ptr<std::vector<ChineseCluster::Analise>> ChineseCluster::analisa_dados(
std::vector<dlib::matrix<float, 0, 1>> &novos) {
    //qDebug() << "Start analisa";
    std::unique_lock<std::mutex> lk_swp(swap_mutex);
    std::unique_lock<std::mutex> lk_unk(unk_mutex);
    std::shared_ptr<std::vector<Analise>> retrn =
                                           std::make_shared<std::vector<Analise>>();

    for (uint32_t nv_c = 0; nv_c < novos.size(); ++nv_c) {
        float min_dist = 9999;
        uint32_t min_dist_index = 0;

        for (uint32_t balls_c = 0; balls_c < balls.size(); ++balls_c) {
            Ball &ball = balls[balls_c];
            float dist = dlib::length(novos[nv_c] - ball.centro);

            if (dist < min_dist && dist <= (ball.raio + 0.02f)) {
                //qDebug() << "Set";
                min_dist = dist;
                min_dist_index = balls_c;
                //maybe return some day
            }
        }

        if (min_dist < 9999) {
            Ball &ball = balls[min_dist_index];                    ;
            unknown_data.push_back(std::make_shared<dlib::matrix<float, 0, 1>>(novos[nv_c]));
            retrn->push_back(Analise(min_dist, ball.pessoa));
        } else {
            unknown_data.push_back(std::make_shared<dlib::matrix<float, 0, 1>>(novos[nv_c]));
            retrn->push_back(Analise(min_dist, nullptr));
        }
    }

    return retrn;
    /*
    Tranca com shared_mutex
    para cada amostra
    verifica se amostra está proxima do centro de cada Ball em balls e tem distancia menor que o raio do circulo
    se sim, coloca no vetor de saida id e nome do usuário e salva na comunidade do usuário
    se não for de ninguem adiciona ao unkown_data e retorna nome vazio e id nulo
    */
}

void ChineseCluster::popula_all_data() {
    all_data.clear();

    for (uint32_t ppl_c = 0; ppl_c < pessoas.size();
         ++ppl_c) { // coloca dados da comunidade
        for (uint32_t comn_c = 0; comn_c < pessoas[ppl_c]->embeds.size(); ++comn_c) {
            all_data.push_back(pessoas[ppl_c]->embeds[comn_c]);
        }

        pessoas[ppl_c]->embeds.clear();
    }

    std::unique_lock<std::mutex> lk(unk_mutex);

    for (uint32_t unk_c = 0; unk_c < unknown_data.size();
         ++unk_c) { //coloca dados desconhecidos
        all_data.push_back(unknown_data[unk_c]);
    }

    unknown_data.clear();
    lk.unlock();

    for (uint32_t ppl_add_c = 0; ppl_add_c < pessoas_adicionar.size();
         ++ppl_add_c) { //pessoas a adicionar
        all_data.push_back(pessoas_adicionar[ppl_add_c]->referencia);
    }

    for (uint32_t ppl_c = 0; ppl_c < pessoas.size();
         ++ppl_c) { //coloca referencia ppl
        all_data.push_back(pessoas[ppl_c]->referencia);
    }
}

void ChineseCluster::cria_grafo(std::vector<dlib::sample_pair> &pares) {
    for (uint32_t i = 0; i < all_data.size(); ++i) {
        for (uint32_t j = i; j < all_data.size(); ++j) {
            if (dlib::length(*all_data[i] - *all_data[j]) < 0.56f) {
                pares.push_back(dlib::sample_pair(i, j));
            }
        }
    }
}

uint64_t ChineseCluster::clusterisa(std::vector<dlib::sample_pair> &pares,
                                    std::vector<uint64_t> &labels) {
    return dlib::chinese_whispers(pares, labels);
}

void ChineseCluster::atribui_amostra_cluster(std::vector<uint64_t> &labels) {
    for (int32_t ppl_c = pessoas.size() - 1; ppl_c >= 0; --ppl_c) {
        pessoas[ppl_c]->temp_cluster_id = labels.back();
        all_data.erase(all_data.end() - 1);
        labels.erase(labels.end() - 1);
    }

    for (int32_t ppl_add_c = pessoas_adicionar.size() - 1; ppl_add_c >= 0;
         --ppl_add_c) {
        pessoas_adicionar[ppl_add_c]->temp_cluster_id = labels.back();
        all_data.erase(all_data.end() - 1);
        labels.erase(labels.end() - 1);
    }
}

void ChineseCluster::adiciona_desconhecidos() {
    // qDebug() << "e";
    for (int32_t ppl_add_c = 0; ppl_add_c < pessoas_adicionar.size(); ++ppl_add_c) {
        bool merged = false;

        //qDebug() << "e";
        for (int32_t ppl_c = 0; ppl_c < pessoas.size(); ++ppl_c) {
            std::shared_ptr<Pessoa> pessoa = pessoas[ppl_c];

            if (pessoa->temp_cluster_id == pessoas_adicionar[ppl_add_c]->temp_cluster_id
                && pessoa->desconhecida) {
                qDebug() << "UNKN TO REGISTERED" << QString::fromStdString(
                             pessoas_adicionar[ppl_add_c]->nome);
                pessoa->desconhecida = false;
                pessoa->nome = pessoas_adicionar[ppl_add_c]->nome;
                pessoa->referencia = pessoas_adicionar[ppl_add_c]->referencia;
                pessoas_adicionar.erase(pessoas_adicionar.begin() + ppl_add_c);
                ppl_add_c--;
                merged = true;
                break;
            }
        }

        //qDebug() << "f";
        if (!merged) {
            qDebug() << "we added " << QString::fromStdString(
                         pessoas_adicionar[ppl_add_c]->nome);
            pessoas.push_back(pessoas_adicionar[ppl_add_c]);
            pessoas_adicionar.erase(pessoas_adicionar.begin() + ppl_add_c);
            ppl_add_c--;
        }

        // qDebug() << "Iter " << QString::number(ppl_add_c) << " | " << QString::number(pessoas_adicionar.size());
    }
}

void ChineseCluster::agrupa_amostras_em_pessoas(std::vector<uint64_t> &labels) {
    //qDebug() << "g";
    for (uint32_t ppl_c = 0; ppl_c < pessoas.size(); ++ppl_c) {
        for (int32_t labs_c = 0; labs_c < labels.size(); ++labs_c) {
            if (pessoas[ppl_c]->temp_cluster_id == labels[labs_c]) {
                if (pessoas[ppl_c]->embeds.size() < 60) {
                    pessoas[ppl_c]->embeds.push_back(all_data[labs_c]);
                    all_data.erase(all_data.begin() + labs_c);
                    labels.erase(labels.begin() + labs_c);
                    labs_c--;
                } else {
                    //qDebug() << "Max encode para: " << QString::fromStdString(pessoas[ppl_c]->nome);
                    //tratar melhor
                    all_data.erase(all_data.begin() + labs_c);
                    labels.erase(labels.begin() + labs_c);
                    labs_c--;
                }
            }
        }
    }
}

void ChineseCluster::randomiza_ordem_encodings() {
    //  qDebug() << "h";
    for (uint32_t ppl_c = 0; ppl_c < pessoas.size();
         ++ppl_c) { //randomiza ordem de embeds
        std::vector<std::shared_ptr<dlib::matrix<float, 0, 1>>> &ref =
            pessoas[ppl_c]->embeds;
        std::random_shuffle(ref.begin(), ref.end());

        if (pessoas[ppl_c]->embeds.size() > 50) {
            for (int var = 0; var < 5; ++var) {
                pessoas[ppl_c]->embeds.erase(pessoas[ppl_c]->embeds.begin() + var);
            }
        }
    }
}

void ChineseCluster::cria_pessoa_para_desconhecido(std::vector<uint64_t>
        &labels) {
    //qDebug() << "i";
    const uint32_t initial_size = pessoas.size();

    for (uint32_t unkn_c = 0; unkn_c < labels.size(); ++unkn_c) { //desconhecidos
        uint64_t &unk_id = labels[unkn_c];
        bool exists = false;

        for (int64_t ppl_c_extra = 0; ppl_c_extra < pessoas.size() - initial_size;
             ++ppl_c_extra) {
            // qDebug() << "ix";
            std::shared_ptr<Pessoa> unkn_person = pessoas[initial_size + ppl_c_extra];

            // qDebug() << "iu";
            if (unk_id == unkn_person->temp_cluster_id) {
                unkn_person->embeds.push_back(all_data[unkn_c]);
                all_data.erase(all_data.begin() + unkn_c);
                labels.erase(labels.begin() + unkn_c);
                unkn_c--;
                exists = true;
                break;
            }
        }

        if (!exists) {
            pessoas.push_back(std::make_shared<Pessoa>(unk_id,
                              "Desconhecido ID: " + std::to_string(unk_id),
                              all_data[unkn_c]));
            pessoas.back()->desconhecida = true;
            all_data.erase(all_data.begin() + unkn_c);
            labels.erase(labels.begin() + unkn_c);
            unkn_c--;
        }
    }
}

void ChineseCluster::encontra_bounding_ball() {
    //qDebug() << "j";
    std::vector<Ball> balls_temp;
    ////////------- CALCULA MINIMAL BALL

    for (uint32_t ppl_c = 0; ppl_c < pessoas.size(); ++ppl_c) {
        PointVector S;
        //  qDebug() << *pessoas[ppl_c]->referencia;
        {
            std::vector<double> coords(pessoas[ppl_c]->referencia->begin(),
                                       pessoas[ppl_c]->referencia->end());
            S.push_back(Point(128, coords.begin()));
        }

        for (uint32_t embeds_c = 0; embeds_c < pessoas[ppl_c]->embeds.size();
             ++embeds_c) {
            std::vector<double> coords(pessoas[ppl_c]->embeds[embeds_c]->begin(),
                                       pessoas[ppl_c]->embeds[embeds_c]->end());
            S.push_back(Point(128, coords.begin()));
        }

        Miniball mb(128, S);
        FT rad = mb.radius() == 0 ? 0.10 : mb.radius(); // is this wrong?
        Miniball::Coordinate_iterator center_it = mb.center_begin();
        dlib::matrix<float, 0, 1> hold;
        hold.set_size(128);

        //qDebug() << "a " << QString::number(static_cast<float>(center_it[0]));
        for (int j = 0; j < 128; ++j) {
            hold(j) = static_cast<float>(center_it[j]);
        }

        balls_temp.push_back(Ball(hold, rad, pessoas[ppl_c]));
    }

    //qDebug() << "balls: " << balls_temp.size();
    std::unique_lock<std::mutex> lk(swap_mutex);
    balls.clear();
    balls.swap(balls_temp);
}




void ChineseCluster::adicionar_pessoa(
    std::shared_ptr<dlib::matrix<float, 0, 1>> referencia, std::string nome) {
    std::unique_lock<std::mutex> lk(pessoa_mutex);
    qDebug() << "Adicionar pessoa";
    pessoas_adicionar.push_back(std::make_shared<Pessoa>(0, nome, referencia));
}

/*
unordered_map = hash_table
//---

Criar bounding circle com os dados coletados do chinese wisperers
checar em tempo de execução se nova face está neste circulo,
e para cada circulo selecionar o que tem menor distancia até o centro,
quanto mais proximo do centro, maior o nível de confiança que a face pertence ao grupo.


//----
class pessoa{
    embed* referencia
    qstring nome
    uint32 temp_cluster_id
    vec<embed*> embeds_da_pessoa //embeds clusterisados no mesmo cluster que a referencia

}
1 - referencia
2 - nome
3 - temp cluster id
4 - resultado do agrupamento em que se encaixa a referencia

vector<pessoa*> pessoas

old_data = vec<embed*>

//colocar todas as referencias no inicio, testar se fica ali para retrieve sequencial rápido
old_data = pessoas[all].referencia + pessoas[all].embeddings

new_data = dados_novos_coletados;

vector<sample_pair> pares;

///--- size_t j = i; não começar do inicio olhar exemplo

for new_data
    old_data.push_back(new_data[item])
    for old_data -1
        measure dist old_data.back to old_data
        if dist < 0.55
            push pares(old_data (last item),old_data (atual) )

resultados = vec<uint64>
int quant = chineseWisperers(pares,resultados)

if quant < pessoas
    erro

para cada pessoa
    pessoa[i].temp_cluster_id = resultados[i]


*/

