#pragma once

#include "imports.h"

#include "alg.h"
#include "cadastro.h"

using namespace cv;
using namespace dlib;


namespace Ui {
    class GUI;
}

class GUI : public QWidget {
    Q_OBJECT


  public:
    explicit GUI(QWidget *parent = 0);

    ~GUI();

  public slots:
    void get_thread(std::thread *thrd);
    void get_detection(QString nome_cam, QString pessoa, quint32 id);
    void get_not_detection(quint32 id);
    void delete_camera(quint32 id);
  private slots:
    void on_adicionar_but_clicked();

    void on_novos_usuarios_clicked();

    void on_isNetwork_stateChanged(int arg1);

  private:
    void keyPressEvent(QKeyEvent *e);

    std::vector<dlib::matrix<rgb_pixel>> jitter_image(dlib::matrix<rgb_pixel>
                                      &img);

    void preload_knolege(std::string path, anet_type &net, std::shared_ptr<ChineseCluster> cluster);
    void learn_from_file(std::string path, anet_type &net, std::shared_ptr<ChineseCluster> cluster,
                         frontal_face_detector &detector, shape_predictor &sp);
    void salvar_nova_pessoa(cv::Mat &foto, std::string nome);

    Ui::GUI *ui;

    QVarLengthArray<QWidget *> widgets_list;
    QVarLengthArray<Alg *> algs_list;


    QVarLengthArray<std::thread *> thread_cameras;
    QVarLengthArray<std::string> rtsp_ips;

    std::shared_ptr<ChineseCluster> cluster;

    std::vector<std::pair<quint32, QListWidgetItem *>> lista_detecoes;



};


