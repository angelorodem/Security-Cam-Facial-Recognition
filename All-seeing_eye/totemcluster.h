#ifndef TOTEMCLUSTER_H
#define TOTEMCLUSTER_H

#include "imports.h"
#include "totemcluster.h"


using namespace dlib;

using namespace std;

//DATALESS CLUSTER

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

class TotemCluster {
    public:
        TotemCluster();
        std::vector<uint32_t> fit_data(std::vector<matrix<float, 0, 1> > &vec_face);
        std::atomic<uint32_t> last_totem = {0}; //unique totem id

        std::string get_nome(const uint32_t at) const;

        void set_nome(const uint32_t at, std::string nome);
        void add_nome(std::string nome);

        static std::mutex tc_nome_lock;
        static std::mutex tc_data_lock;

        std::vector<tuple<float, matrix<float, 0, 1>>>
        totens; //index <0> usado no totem merge para identificar totens "irmãos"




        uint32_t get_nomes_size() const {
            return nomes_faces->size();
        }

        uint32_t get_totens_size() const {
            std::lock_guard<std::mutex> lock(tc_data_lock);
            return totens.size();
        }


    private:

        std::vector<std::string> *nomes_faces;

        static constexpr float totem_init_radious = 0.45;
        static constexpr float snap_radious = 0.06;




};




#endif // TOTEMCLUSTER_H
