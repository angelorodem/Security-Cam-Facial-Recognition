#pragma once

#include "imports.h"
#include "Chinese/chinesecluster.h"

#include "cadastro.h"
#include "registrausuario.h"
#include "camera.h"

namespace Ui {
    class Cadastro;
}

class Cadastro : public QDialog {
    Q_OBJECT

  public:
    explicit Cadastro(QString ip_whats_p, QString ip_camera_p,
                      std::shared_ptr<ChineseCluster> _clstr);
    explicit Cadastro(QString ip_whats_p, int device_nbr, std::shared_ptr<ChineseCluster> _clstr);
    ~Cadastro();

  private slots:
    void on_pronto_clicked();
    void nao_add_invalido();
    void nao_add_sem_pessoa();
    void success();
    void erro_grave();
    void recebe_pic(QPixmap imagem);
  private:
    std::shared_ptr<ChineseCluster> cluster;
    dlib::frontal_face_detector detector;
    dlib::shape_predictor sp;
    anet_type net;
    QString ip_whats;
    QString ip_camera;

    void keyPressEvent(QKeyEvent *e);

    int device_n = -1;
    Ui::Cadastro *ui;
};


