#pragma once

#include "imports.h"
#include "Chinese/chinesecluster.h"

class RegistraUsuario : public QObject {
    Q_OBJECT
  signals:
    void nao_add_invalido();
    void nao_add_sem_pessoa();
    void erro_grave();
    void success();
    void send_pic(QPixmap imagem);

  private:
    std::shared_ptr<ChineseCluster> cluster;
    QString url;
  public:
    RegistraUsuario(std::shared_ptr<ChineseCluster> cluster_, QString url_);


    std::vector<dlib::matrix<dlib::rgb_pixel>> jitter_image(dlib::matrix<dlib::rgb_pixel>
                                            &img);

    void registra_novo_usuario(cv::Mat &foto, QString nome, QString numero,
                               anet_type &net,
                               dlib::frontal_face_detector &detector, dlib::shape_predictor &sp);

    void envia_whatsapp(QString nome, QString numero);

    void learn_from_photo(cv::Mat &foto, QString nome, anet_type &net,
                          dlib::frontal_face_detector &detector, dlib::shape_predictor &sp);

    void salvar_nova_pessoa(cv::Mat &foto, std::string nome);
    QNetworkAccessManager *nm;
};

