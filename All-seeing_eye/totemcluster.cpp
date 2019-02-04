#include "totemcluster.h"
#include <dlib/matrix.h>

constexpr float
TotemCluster::totem_init_radious; // reduzir muito se ativado totem merge
constexpr float TotemCluster::snap_radious;

std::mutex TotemCluster::tc_data_lock;
std::mutex TotemCluster::tc_nome_lock;

TotemCluster::TotemCluster() {
    nomes_faces = new std::vector<std::string>();
}

/*
 *  Implementar clustering totem
 *      para cada nova amostra verificar se está dentro da area de um tótem,
 *      se estiver na área de um tótem,
 *          assume amostra como parte do cluster deste tótem,
 *      caso não estiver,
 *          analisar se a amostra está perto o suficiente de um tótem para realizar snapping (usando constante de snapping + porcentagem_raio)
 *          caso amostra esteja na area de snap
 *              assume amostra como parte do tótem de snap
 *              totem de snap tem sua area aumentada em uma PORCENTAGEM x (20%?)
 *          caso nenhum tótem esteja na area de snap
 *              assume amostra como um novo tótem (cluster)
*/
std::vector<uint32_t> TotemCluster::fit_data(std::vector<matrix<float, 0, 1>>
                                             &vec_face) {
    std::lock_guard<std::mutex> lock_(tc_data_lock);

    uint32_t vec_face_size = vec_face.size();
    std::vector<uint32_t> result;
    for (uint32_t faces_counter = 0; faces_counter < vec_face_size;
         ++faces_counter) {
        uint32_t totens_size = totens.size();

        float best_snap_dist = 10000;
        uint32_t best_snap_dist_index = UINT32_MAX;
        float best_core_dist = 10000;
        uint32_t best_core_dist_index = UINT32_MAX;

        //<0> current size
        //<1> descriptor centroid

        for (uint32_t totens_counter = 0; totens_counter < totens_size;
             ++totens_counter) {

            float len = dlib::length(vec_face[faces_counter] - std::get<1>
                                     (totens[totens_counter])); // distancia entre totem e nova face

            float& radious = std::get<0>(totens[totens_counter]);
            if ((len < radious) && (len < best_core_dist)) { //se dist for menor q radius e melhor q a core

                best_core_dist = len;
                best_core_dist_index = totens_counter;
            } else if ((len < snap_radious + radious) && (len < best_snap_dist)) {

                best_snap_dist = len;
                best_snap_dist_index = totens_counter;
            }
        }
        if (best_core_dist < best_snap_dist) {
                        qDebug() << "[Core]" << "SnDis: " << best_snap_dist << " CrDis: " << best_core_dist;
            if (best_core_dist < 0.28) {
                // se a imagem tiver valor baixo, pega ela e faz média com peso 1/3
                qDebug() << "Centroide ajustado";
                std::vector<dlib::matrix<float, 0, 1>> media;
                media.push_back(std::get<1>(totens[best_core_dist_index]));
                media.push_back(std::get<1>(totens[best_core_dist_index]));
                media.push_back(vec_face[faces_counter]);
                std::get<1>(totens[best_core_dist_index]) = mean(mat(media));
            }

            result.push_back(best_core_dist_index);

        } else if (best_core_dist > best_snap_dist) {

            qDebug() << "[Snap]" << "SnDis: " << best_snap_dist << " CrDis: " << best_core_dist;
            result.push_back(best_snap_dist_index);
            //check for totem merge
        }

        //caso ambos tenham a mesma distancia
        if ((best_core_dist == best_snap_dist) && (best_core_dist == 10000)) {
            qDebug() << "Novo totem" ;
            totens.push_back(make_tuple(totem_init_radious,
                                        vec_face[faces_counter]));

            result.push_back(last_totem);
            last_totem.store(last_totem.load() + 1);

        }
    }
    return result;
}

string TotemCluster::get_nome(const uint32_t at) const {
    return nomes_faces->at(at);
}

void TotemCluster::set_nome(const uint32_t at, string nome) {
    //TODO: locks
    nomes_faces[0][at] = nome;

}

void TotemCluster::add_nome(std::string nome) {
    //TODO: lock
    nomes_faces->push_back(nome);
}

