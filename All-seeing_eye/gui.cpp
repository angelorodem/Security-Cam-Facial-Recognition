#include "gui.h"
#include "ui_gui.h"

GUI::GUI(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GUI) {
    ui->setupUi(this);
    ui->device_n->setHidden(true);
    cluster = std::make_shared<ChineseCluster>();
    //qDebug() << "dir: " << QDir::tempPath().toStdString() ;
    QFile::copy(":/utils/face.dat",  QDir::tempPath() + "/face.dat");
    QFile::copy(":/utils/shape.dat", QDir::tempPath() + "/shape.dat");
    anet_type net;
    deserialize(QString(QDir::toNativeSeparators(QDir::tempPath() +
                        "/face.dat")).toStdString())
            >> net;
    qDebug() << "Preload";
    preload_knolege(QString(QDir::toNativeSeparators(QDir::currentPath()) +
                            QDir::toNativeSeparators("/knolege/")).toStdString(),
                    net, cluster);
    ui->cameras_layout->setAlignment(Qt::AlignTop);
    //learn_from_file("/home/ollegn/git/all-seeing_eye/raw_photos/",net,cluster,detector,sp);
}

void GUI::preload_knolege(std::string path, anet_type &net,
                          std::shared_ptr<ChineseCluster> cluster) {
    qDebug() << "Treinando" ;
    QStringList list = QDir(QString::fromStdString(path)).entryList();
    dlib::matrix<rgb_pixel> img;

    for (int i = 0; i < list.size(); ++i) {
        if (!list[i].contains(".png")) {
            continue;
        }

        cv::Mat face = imread(path + list[i].toStdString());
        //qDebug() << path+list[i].toStdString() ;

        if (!face.data) {
            qDebug() << "Erro!" ;
            continue;
        }

        cvtColor(face, face, COLOR_BGR2RGB);
        dlib::assign_image(img, dlib::cv_image<rgb_pixel>(face));
        QStringList pieces = list[i].split(".");
        qDebug() << "Treinado: " << pieces[0];
        std::shared_ptr<matrix<float, 0, 1>> hold =
                                              std::make_shared<dlib::matrix<float, 0, 1>>
                                              (mean(mat(net(jitter_image(img)))));
        cluster->adicionar_pessoa(hold, pieces[0].toStdString());
    }

    //resultado_totem = cluster->fit_data(vec_face); //---
    /*if (resultado_totem.size() != cluster->get_nomes_size()) {
        qDebug() << "Tamanhos diferentes!!!" ;
    }*/
}

void GUI::learn_from_file(std::string path, anet_type &net,
                          std::shared_ptr<ChineseCluster> cluster,
                          frontal_face_detector &detector, shape_predictor &sp) {
    qDebug() << "treinando apartir do bruto" ;
    QStringList list = QDir(QString::fromStdString(path)).entryList();
    dlib::matrix<rgb_pixel> img;

    for (int i = 0; i < list.size(); ++i) {
        QFileInfo inf(QString::fromStdString(path + list[i].toStdString()));

        //qDebug() << path+list[i].toStdString() ;
        if (inf.suffix() == "png" || inf.suffix() == "jpg" || inf.suffix() == "jpeg") {
        } else {
            continue;
        }

        cv::Mat facez = imread(path + list[i].toStdString());
        //qDebug() << path+list[i].toStdString() ;

        if (!facez.data) {
            qDebug() << "Erro!" ;
            continue;
        }

        cvtColor(facez, facez, COLOR_BGR2RGB);
        dlib::assign_image(img, dlib::cv_image<rgb_pixel>(facez));
        matrix<rgb_pixel> face_chip_res;

        for (auto face : detector(img)) {
            dlib::full_object_detection shape = sp(img, face);
            matrix<rgb_pixel> face_chip;
            extract_image_chip(img, get_face_chip_details(shape, 150, 0.25), face_chip);
            face_chip_res = std::move(face_chip);
        }

        // vec_face.push_back(mean(mat(net(jitter_image(face_chip_res)))));
        QStringList pieces = list[i].split(".");
        qDebug() << "Treinado: " << pieces[0];
        std::shared_ptr<dlib::matrix<float, 0, 1>> hold =
                std::make_shared<dlib::matrix<float, 0, 1>>(mean(mat(net(jitter_image(
                            face_chip_res)))));
        cluster->adicionar_pessoa(hold, pieces[0].toStdString());
        cv::Mat colorc;
        cvtColor(toMat(face_chip_res), colorc, COLOR_RGB2BGR);
        salvar_nova_pessoa(colorc, pieces[0].toStdString());
    }

    //resultado_totem = cluster->fit_data(vec_face); //---
}

void GUI::salvar_nova_pessoa(Mat &foto, std::string nome) {
    if (nome.empty()) {
        return;
    }

    imwrite(QString(QDir::toNativeSeparators(QDir::currentPath()) +
                    QDir::toNativeSeparators("/knolege/")).toStdString() + nome +
            ".png", foto);
}

GUI::~GUI() {
    delete ui;
}

void GUI::get_thread(std::thread *thrd) {
    thread_cameras.push_back(thrd);
}

void GUI::get_detection(QString nome_cam, QString pessoa, quint32 id) {
    QListWidgetItem *item = new QListWidgetItem;
    item->setText("[" + nome_cam + "] " + pessoa);
    ui->lista_usuario->insertItem(ui->lista_usuario->count(), item);
    lista_detecoes.push_back(std::make_pair(id, item));
    ui->log->moveCursor(QTextCursor::End);
    ui->log->insertHtml("<font color=\"Red\">[DETECTADO]:</font>");
    ui->log->insertHtml("<font color=\"Blue\">[" +
                        QTime::currentTime().toString("hh:mm:ss") + "]:</font>");
    ui->log->insertHtml("{<font color=\"Purple\">" + nome_cam + "</font>}");
    ui->log->insertHtml(" - <font color=\"Green\">" + pessoa + "</font><br />\n");
}

void GUI::get_not_detection(quint32 id) {
    for (uint32_t i = 0; i < lista_detecoes.size(); ++i) {
        if (lista_detecoes[i].first == id) {
            delete ui->lista_usuario->takeItem(ui->lista_usuario->row(
                                                   lista_detecoes[i].second));
            lista_detecoes.erase(lista_detecoes.begin() + i);
            break;
        }
    }
}

void GUI::delete_camera(quint32 id) {
    uint32_t id_del = 0;

    for (uint32_t var = 0; var < algs_list.size(); ++var) {
        if (algs_list[var]->this_alg_id == id) {
            id_del = var;
        }
    }

    delete widgets_list[id_del];
    delete algs_list[id_del];
    widgets_list.erase(widgets_list.begin() + id_del);
    algs_list.erase(algs_list.begin() + id_del);
}


/*
    Alg alg;
    alg.exec("rtsp://192.168.110.253/Streaming/Channels/1");
    QApplication::exit(0);

*/
//rtsp://192.168.110.253/Streaming/Channels/1
void GUI::on_adicionar_but_clicked() {  //rtsp://192.168.110.253/h264
    //Alg alg;
    //alg.exec("rtsp://192.168.110.251/h264");
    //QApplication::exit(0);
    if (ui->rtsp_camera->text().isEmpty()) {
        qDebug() << "Sem rtsp";
        return;
    }

    if (ui->nome_camera->text().isEmpty()) {
        qDebug() << "Sem nome";
        return;
    }

    QWidget *wdg = new QWidget();
    widgets_list.push_back(wdg);
    QHBoxLayout *lay = new QHBoxLayout(wdg);
    QLabel *label_nome = new QLabel(ui->nome_camera->text(), wdg);
    lay->addWidget(label_nome);
    label_nome->show();
    QLabel *label_status = new QLabel("Offline", wdg);
    label_status->show();
    lay->addWidget(label_status);
    QPushButton *push_connect = new QPushButton("Conectar", wdg);
    lay->addWidget(push_connect);
    push_connect->show();
    QPushButton *push_view = new QPushButton("Visualizar", wdg);
    lay->addWidget(push_view);
    push_view->show();
    QPushButton *push_close = new QPushButton("X", wdg);
    lay->addWidget(push_close);
    push_close->setFixedWidth(30);
    push_close->show();
    Alg *alg = new Alg(static_cast<uint32_t>(widgets_list.size()) - 1);
    algs_list.push_back(alg);
    alg->parametros(ui->rtsp_camera->text().toStdString(), cluster,
                    ui->nome_camera->text().toStdString());
    connect(push_view, SIGNAL(clicked(bool)), alg, SLOT(view_switch()), Qt::QueuedConnection);
    connect(push_connect, SIGNAL(clicked(bool)), alg, SLOT(start_op()), Qt::QueuedConnection);
    connect(push_connect, SIGNAL(clicked(bool)), push_connect, SLOT(setEnabled(bool)));
    connect(alg, SIGNAL(send_status(QString)), label_status, SLOT(setText(QString)),
            Qt::QueuedConnection);
    connect(push_close, SIGNAL(clicked(bool)), alg, SLOT(delete_camera()));
    connect(alg, SIGNAL(send_detection(QString, QString, quint32)), this, SLOT(get_detection(QString,
            QString,
            quint32)), Qt::QueuedConnection);
    connect(alg, SIGNAL(close_camera(quint32)), this, SLOT(delete_camera(quint32)));
    connect(alg, SIGNAL(send_not_detected(quint32)), this, SLOT(get_not_detection(quint32)),
            Qt::QueuedConnection);
    connect(ui->detect_spin, SIGNAL(valueChanged(int)), alg, SLOT(set_intervalo(int)));
    connect(ui->face_tracking_check, SIGNAL(toggled(bool)), alg, SLOT(set_tracking(bool)));
    connect(ui->color_check, SIGNAL(toggled(bool)), alg, SLOT(set_use_color(bool)));
    connect(ui->enhance_check, SIGNAL(toggled(bool)), alg, SLOT(set_use_enhance(bool)));
    connect(ui->show_detections, SIGNAL(toggled(bool)), alg, SLOT(set_show_detections(bool)));
    connect(ui->skip_check, SIGNAL(toggled(bool)), alg, SLOT(set_enable_skip(bool)));
    ui->cameras_layout->addWidget(wdg);
}

void GUI::keyPressEvent(QKeyEvent *e) {
    QWidget::keyPressEvent(e);
    /*if (e->key() == Qt::Key_Escape) {
        cluster->add_mode.store(!cluster->add_mode.load());
        qDebug() << "mode change!" ;
        e->accept();
    }*/
}

std::vector<dlib::matrix<rgb_pixel>> GUI::jitter_image(dlib::matrix<rgb_pixel> &img) {
    thread_local dlib::rand rnd;
    std::vector<dlib::matrix<rgb_pixel>> crops;

    for (int i = 0; i < 100; ++i) {
        crops.push_back(dlib::jitter_image(img, rnd));
    }

    return crops;
}

void GUI::on_novos_usuarios_clicked() {
    if (ui->ip_whatsapp->text().isEmpty() || ui->ip_whatsapp->text().isNull()) {
        return;
    }

    if (ui->isNetwork->isChecked()) {
        Cadastro cad(ui->ip_whatsapp->text(), ui->ip_camera_cadastro->text(), cluster);
        cad.show();
        cad.exec();
    } else {
        Cadastro cad(ui->ip_whatsapp->text(), ui->device_n->value(), cluster);
        cad.show();
        cad.exec();
    }
}

void GUI::on_isNetwork_stateChanged(int arg1) {
    ui->device_n->setHidden(!ui->device_n->isHidden());
    ui->ip_camera_cadastro->setHidden(!ui->ip_camera_cadastro->isHidden());
}
